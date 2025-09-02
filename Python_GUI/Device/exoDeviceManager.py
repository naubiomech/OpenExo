import asyncio
import struct
import sys
from typing import Dict, List, Tuple, Optional, Callable

import numpy as np
from bleak import BleakClient, BleakScanner
from bleak.backends.characteristic import BleakGATTCharacteristic
from bleak.backends.device import BLEDevice
from bleak.backends.scanner import AdvertisementData

from Device import realTimeProcessor


class ExoDeviceManager:

    def __init__(self, on_disconnect: Optional[Callable] = None):
        self._realTimeProcessor = realTimeProcessor.RealTimeProcessor()
        self.on_disconnect = on_disconnect  # Store the callback
        self.processors = {} 
        self.device_addresses = []              # up to two
        self.connections = {}
        self.available_devices = {}  # List to store available devices
        self.device = None
        self.client = None
        self.services = None
        self.scanResults = None
        self.disconnecting_intentionally = False  # Flag for intentional disconnect

        self._write_locks = {}          # used by _write()
        self.debug_rx = False           # print ASCII from each device while bringing up
        self.raw_queues = {}            # addr -> asyncio.Queue[bytes]
        self.parsed_queues = {}         # optional, if you expose parsed frames later

        self.verbose_ascii = False
        self.verbose_writes = True

        self.characteristics = ""

        # Initialize FSR values
        self.curr_left_fsr_value = 0.25
        self.curr_right_fsr_value = 0.25

        self.device_mac_address = None

        # UUID characteristic
        self.UART_TX_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"  # Nordic NUS TX UUID
        self.UART_RX_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"  # Nordic NUS RX UUID
        self.UART_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"  # Nordic Service UUID

        # Joint dictionary to map menu selection to joint ID
        self.jointDictionary = {
            1: 33.0,
            2: 65.0,
            3: 34.0,
            4: 66.0,
            5: 36.0,
            6: 68.0,
            7: 40.0,
            8: 72.0
        }

        self.isConnected = False

    # Set scan results from the BLE scanner
    def set_scan_results(self, scans):
        self.scanResults = scans

    # Set the current device
    def set_device(self, deviceVal):
        self.device = deviceVal

    # Set the MAC address of the device
    def set_deviceAddress(self, deviceAddress):
        self.device_mac_address = deviceAddress

    def set_deviceAddresses(self, addrs):
        if not isinstance(addrs, (list, tuple)) or len(addrs) != 2:
            raise ValueError("Dual-only: expected exactly 2 addresses")
        self.device_addresses = list(addrs)
        
    # Set the BLE client
    def set_client(self, clientVal):
        self.client = clientVal

    # Set the services of the device
    def set_services(self, servicesVal):
        self.services = servicesVal

    def _ensure_processor(self, addr: str):
        if addr not in self.processors:
            self.processors[addr] = realTimeProcessor.RealTimeProcessor()

    def get_all_exo_data(self):
        """Return dict: addr -> ExoData (from each device's RealTimeProcessor)."""
        out = {}
        for addr, proc in self.processors.items():
            exo = getattr(proc, "_exo_data", None)
            if exo is not None:
                out[addr] = exo
        return out

    @staticmethod
    def _looks_like_ascii(buf: bytes) -> bool:
        # True if buffer appears to be printable ASCII (firmware chatter)
        if not buf:
            return False
        return all(32 <= b <= 126 or b in (9, 10, 13) for b in buf)
    
    async def DataIn(self, sender: BleakGATTCharacteristic, data: bytearray, src_addr: Optional[str] = None):
        # 0) optional console visibility
        if self.debug_rx and src_addr:
            try:
                s = data.decode("ascii", "ignore").strip()
                if s:
                    print(f"[{src_addr}] RX:", s)
            except Exception:
                pass

        # 1) stash raw bytes per-device
        if src_addr and src_addr in self.raw_queues:
            q = self.raw_queues[src_addr]
            if not q.full():
                q.put_nowait(bytes(data))

        # 2) ensure processor exists for this device
        if src_addr:
            self._ensure_processor(src_addr)
        
        # 3) always parse with the correct processor (your protocol is ASCII)
        try:
            proc = self.processors.get(src_addr) or self._realTimeProcessor
            proc.processEvent(bytes(data))
        except Exception as e:
            if self.verbose_ascii:
                print(f"[{src_addr}] parser error:", e)

        await self.MachineLearnerControl()

    async def capture_raw_for(self, seconds: float = 5.0):
        """Return dict: addr -> list[bytes] captured over 'seconds'."""
        snapshot_addrs = [a for a, c in self.connections.items() if c.get("is_connected")]
        out = {a: [] for a in snapshot_addrs}
        if not out:
            return out

        loop = asyncio.get_event_loop()
        t0 = loop.time()
        while loop.time() - t0 < seconds:
            for addr in snapshot_addrs:
                q = self.raw_queues.get(addr)
                if not q:
                    continue
                try:
                    pkt = await asyncio.wait_for(q.get(), timeout=0.05)
                    out[addr].append(pkt)
                except asyncio.TimeoutError:
                    pass
        return out

    async def _write(self, addr: str, char_uuid: str, data: bytes, response: bool = False):
        conn = self.connections.get(addr)
        if not conn or not conn.get("is_connected"):
            print(f"[{addr}] Device not connected in _write")
            return False
        cli = conn["client"]
        try:
            ch = self.get_char_handle(char_uuid, addr=addr)
            lock = self._write_locks.setdefault(addr, asyncio.Lock())
            async with lock:
                await cli.write_gatt_char(ch, data, response)
            return True
        except Exception as e:
            print(f"[{addr}] Write error: {e}")
            # Check if device disconnected
            if not cli.is_connected:
                print(f"[{addr}] Device disconnected during write")
                conn["is_connected"] = False
            return False

    async def _fanout_results(self, char_uuid: str, data: bytes, response: bool = False):
        """
        Send the same command to all connected devices.
        Returns: dict[address] -> True on success, or Exception on error.
        """
        results = {}
        tasks = []

        # queue tasks for all currently-connected devices
        for addr, conn in list(self.connections.items()):
            if conn.get("is_connected"):
                tasks.append(asyncio.create_task(self._write(addr, char_uuid, data, response)))
                results[addr] = None  # placeholder

        if not tasks:
            return results  # empty dict when nothing is connected

        # gather results while preserving order
        outs = await asyncio.gather(*tasks, return_exceptions=True)

        # map back to addresses in the same order we queued them
        i = 0
        for addr in list(results.keys()):
            out = outs[i]
            if isinstance(out, Exception):
                print(f"[{addr}] fanout error:", out)
                results[addr] = out                      # keep the exception for the UI
            else:
                results[addr] = (out is True) or True    # normalize success to True
            i += 1

        return results
    async def _fanout(self, char_uuid: str, data: bytes, response: bool = False):
        tasks = []
        for addr, conn in list(self.connections.items()):
            if conn.get("is_connected"):
                tasks.append(self._write(addr, char_uuid, data, response))
        if tasks:
            await asyncio.gather(*tasks, return_exceptions=True)

    async def start_motors_both(self):
        # 'E' — same as your single-device startExoMotors(), but fanned out
        return await self._fanout_results(self.UART_TX_UUID, b"E", False)

    async def motorOn_both(self):  await self._fanout(self.UART_TX_UUID, b"x", True)
    async def motorOff_both(self): await self._fanout(self.UART_TX_UUID, b"w", True)
    async def calibrateTorque_both(self):   return await self._fanout_results(self.UART_TX_UUID, b"H", False)
    async def calibrateFSRs_both(self):     return await self._fanout_results(self.UART_TX_UUID, b"L", False)
    async def start_stream_both(self):      await self._fanout_results(self.UART_TX_UUID, b"X", True)

    # Control the machine learning model based on incoming data
    async def MachineLearnerControl(self):
        if self._realTimeProcessor._predictor.modelExists:  # Check if the machine learning model exists
            if self._realTimeProcessor._predictor.controlMode == 1:  # If control is authorized
                if self._realTimeProcessor._exo_data.Task[-2] != self._realTimeProcessor._predictor.prediction:
                    print("Prediction Changed") 
                    # Check current prediction and adjust stiffness accordingly
                    if self._realTimeProcessor._predictor.prediction == 1:  # Level
                        await self.sendStiffness(0.5)
                    elif self._realTimeProcessor._predictor.prediction == 2:  # Descend
                        await self.sendStiffness(1)
                    elif self._realTimeProcessor._predictor.prediction == 3:  # Ascend
                        await self.sendStiffness(0)
                    else:
                        await self.sendStiffness(0.5)

    # Get characteristic handle by UUID
    def get_char_handle(self, char_UUID, addr=None):
        if addr is None:
            # primary (keeps old behavior)
            return self.services.get_service(self.UART_SERVICE_UUID).get_characteristic(char_UUID)
        # per-connection services
        conn = self.connections.get(addr)
        if not conn or not conn.get("services"):
            raise RuntimeError(f"No services cached for {addr}")
        return conn["services"].get_service(self.UART_SERVICE_UUID).get_characteristic(char_UUID)

    def _ensure_queues(self, addr: str):
        if addr not in self.raw_queues:
            self.raw_queues[addr] = asyncio.Queue(maxsize=2000)
        if addr not in self.parsed_queues:
            self.parsed_queues[addr] = asyncio.Queue(maxsize=2000)

    # Filter devices based on advertising data
    def filterExo(self, device: BLEDevice, adv: AdvertisementData):
        uuids = adv.service_uuids or []
        return any(u.lower() == self.UART_SERVICE_UUID.lower() for u in uuids)
    
    # Handle disconnect callback
    def handleDisconnect(self, _: BleakClient):
        print("Disconnecting...")  # Check if this is reached
        if not self.disconnecting_intentionally:  # Only call if not an intentional disconnect
            if self.on_disconnect:
                print("Calling disconnect callback...")
                self.on_disconnect()
            else:
                print("No disconnect callback assigned.")
        else:
            print("Disconnect was intentional, skipping callback.")

    # Disconnect from the BLE device
    async def disconnect(self):
        if self.client is not None and self.isConnected:
            try:
                self.disconnecting_intentionally = True  # Set the flag before disconnecting
                await self.client.disconnect()  # Disconnect from the client
                self.isConnected = False  # Update connection status
                print("Successfully disconnected from the device.")
            except Exception as e:
                print(f"Failed to disconnect: {e}")
            finally:
                self.disconnecting_intentionally = False  # Reset the flag after disconnecting
        else:
            print("No device is currently connected.")

    def _make_disconnect_cb(self, addr):
        def _cb(_: BleakClient):
            print(f"[{addr}] disconnected")
            if addr in self.connections:
                self.connections[addr]["is_connected"] = False
            # keep your existing UI callback
            if not self.disconnecting_intentionally and self.on_disconnect:
                self.on_disconnect()
        return _cb
    
    async def disconnect_all(self):
        self.disconnecting_intentionally = True
        try:
            for addr, conn in list(self.connections.items()):
                try:
                    if conn.get("client"):
                        await conn["client"].disconnect()
                except Exception as e:
                    print(f"[{addr}] disconnect error:", e)
                conn["is_connected"] = False
        finally:
            self.disconnecting_intentionally = False


    async def motorOn_both(self):
        tasks = []
        for addr, conn in self.connections.items():
            if conn.get("is_connected"):
                try:
                    char = self.get_char_handle(self.UART_TX_UUID, addr=addr)
                    tasks.append(conn["client"].write_gatt_char(char, b"x", True))
                except Exception as e:
                    print(f"[{addr}] motorOn error:", e)
        if tasks:
            await asyncio.gather(*tasks, return_exceptions=True)


    # Start motors of the Exo
    async def startExoMotors(self):
        await asyncio.sleep(1)
        print("using bleak start\n")
        command = bytearray(b"E")
        char = self.get_char_handle(self.UART_TX_UUID)

        try:
            await self.client.write_gatt_char(char, command, False)
        except Exception as e:
            print(f"An error occurred: {e}")

    # Calibrate the torque of the Exo
    async def calibrateTorque(self):
        print("using bleak torque\n")
        command = bytearray(b"H")
        char = self.get_char_handle(self.UART_TX_UUID)

        await self.client.write_gatt_char(char, command, False)

    # Calibrate the FSR sensors
    async def calibrateFSRs(self):
        print("using bleak FSR\n")
        command = bytearray(b"L")
        char = self.get_char_handle(self.UART_TX_UUID)

        await self.client.write_gatt_char(char, command, False)

    # Turn off motors
    async def motorOff(self):
        command = bytearray(b"w")
        char = self.get_char_handle(self.UART_TX_UUID)

        await self.client.write_gatt_char(char, command, True)

    # Turn on motors
    async def motorOn(self):
        command = bytearray(b"x")
        char = self.get_char_handle(self.UART_TX_UUID)

        await self.client.write_gatt_char(char, command, True)

    # In ExoDeviceManager
    async def updateTorqueValues_both(self, parameter_list):
        """
        Dual-only mirror of your single-device updateTorqueValues():
        Sends: 'f' + <double fields> to both devices, respecting your bilateral logic.
        parameter_list = [isBilateral(bool), joint(float 1..6), controller(float), parameter(float), value(float)]
        """
        float_values = parameter_list
        print(f"Updating torque values for both devices: {float_values}")
        
        # Check connection status first
        connected_devices = [addr for addr, conn in self.connections.items() if conn.get("is_connected")]
        print(f"Connected devices: {connected_devices}")
        
        if len(connected_devices) == 0:
            print("No devices connected, cannot update parameters")
            return
        
        # Send header 'f' to both devices with delay
        try:
            await self._fanout_results(self.UART_TX_UUID, b"f", False)
            await asyncio.sleep(0.1)  # Small delay after header
        except Exception as e:
            print(f"Error sending header to devices: {e}")
            return

        # We must send the same *sequence* of doubles to each device.
        # Do it per-device to preserve your joint +/- 32 logic.
        for addr, conn in list(self.connections.items()):
            if not conn.get("is_connected"): 
                print(f"[{addr}] Device not connected, skipping")
                continue
            try:
                ch = self.get_char_handle(self.UART_TX_UUID, addr=addr)
                print(f"[{addr}] Sending parameter data...")

                # loop the same way your single version does
                totalLoops = 2 if float_values[0] is True else 1
                for loopCount in range(totalLoops):
                    for i in range(1, len(float_values)):
                        if i == 1:
                            joint = float_values[1]
                            # +/- 32 when bilateral on second loop
                            if loopCount == 1 and joint % 2 == 0:
                                val = self.jointDictionary[joint] - 32
                            elif loopCount == 1 and joint % 2 != 0:
                                val = self.jointDictionary[joint] + 32
                            else:
                                val = self.jointDictionary[joint]
                            fb = struct.pack("<d", val)
                        else:
                            fb = struct.pack("<d", float_values[i])
                        
                        # Send with small delay between writes
                        await conn["client"].write_gatt_char(ch, fb, False)
                        await asyncio.sleep(0.05)  # Small delay between parameter writes
                        
                print(f"[{addr}] Parameter update completed successfully")
                await asyncio.sleep(0.1)  # Delay after each device
                
            except Exception as e:
                print(f"[{addr}] updateTorqueValues_both error:", e)
                # Check if device is still connected
                if not conn.get("is_connected"):
                    print(f"[{addr}] Device disconnected during parameter update")

    # Update torque values based on the parameters
    async def updateTorqueValues(self, parameter_list):
        totalLoops = 1
        loopCount = 0
        float_values = parameter_list

        # Check if bilateral mode
        if parameter_list[0] is True:
            totalLoops = 2
        # Loop if bilateral mode. No loop otherwise
        while loopCount != totalLoops:
            command = b"f"
            char = self.get_char_handle(self.UART_TX_UUID)
            await self.client.write_gatt_char(char, command, False)

            for i in range(1, len(float_values)):
                if i == 1:
                    # Adjust joint ID for bilateral mode
                    if loopCount == 1 and float_values[1] % 2 == 0:
                        print(f"joint: {self.jointDictionary[float_values[i]] - 32}")
                        float_bytes = struct.pack("<d", self.jointDictionary[float_values[i]] - 32)
                    elif loopCount == 1 and float_values[1] % 2 != 0:
                        print(f"joint: {self.jointDictionary[float_values[i]] + 32}")
                        float_bytes = struct.pack("<d", self.jointDictionary[float_values[i]] + 32)
                    else:
                        print(f"joint: {self.jointDictionary[float_values[i]]}")
                        float_bytes = struct.pack("<d", self.jointDictionary[float_values[i]])
                else:
                    print(float_values[i])
                    float_bytes = struct.pack("<d", float_values[i])
                char = self.get_char_handle(self.UART_TX_UUID)
                await self.client.write_gatt_char(char, float_bytes, False)

            loopCount += 1

    # Connect to a specific device
    async def connect(self, device):
        print(device)

    # Search for available BLE devices
    async def searchDevices(self):
        i = 0
        self.available_devices.clear()
        while i < 3:
            try:
                device = await BleakScanner.find_device_by_filter(self.filterExo)
                if device:
                    self.available_devices[device.address] = device.name  # Store name with address as key

                # Print all available devices every few seconds
                print("Currently available devices:")
                for address, name in self.available_devices.items():
                    print(f"{name} - {address}")

                await asyncio.sleep(1)  # Adjust the scan interval as needed
                i += 1
            except Exception as e:
                print(f"Error during device scan: {e}")
                return "false"  # Return false if an error occurs
        return self.available_devices
    
    # Scan for BLE devices and attempt to connect
    async def scanAndConnect(self):
        raise RuntimeError("Dual-only mode: use scanAndConnectMulti()")
        # print("Using Bleak scan...")
        # print("Scanning...")

        # attempts = 4
        # for attempt in range(attempts):
        #     print(f"Attempt {attempt + 1} of {attempts}")

        #     # Scan for devices using the filter
        #     device = await BleakScanner.find_device_by_filter(self.filterExo)

        #     if device:
        #         print(f"Found device: {device.name} - {device.address}")
                
        #         # Check if the found device's address matches the specified address
        #         if device.address == self.device_mac_address:
        #             print("Matching device found. Connecting...")
                    
        #             self.set_device(device)  # Set the device
        #             self.set_client(BleakClient(device, disconnected_callback=self.handleDisconnect))
                    
        #             # Try to connect to the Exo
        #             try:
        #                 await self.client.connect()
        #                 self.isConnected = True
        #                 print("Is Exo connected: " + str(self.isConnected))
        #                 print(self.client)

        #                 # Get list of services from Exo
        #                 self.set_services(self.client.services) #Get_Services Not Supported in v1.0 and beyond of bleak library, previously was [await self.client.get_services()] instead of [self.client.services].
                        
        #                 # Start incoming data stream
        #                 await self.client.start_notify(self.UART_RX_UUID, self.DataIn)
        #                 return True  # Successful connection
        #             except Exception as e:
        #                 print(f"Failed to connect: {e}")
        #                 return False  # Connection failed
        #         else:
        #             print("Found device does not match the specified address.")
        #     else:
        #         print("No device found.")

        # print("Max attempts reached. Could not connect.")
        # return False  # Return False if all attempts fail

    # ---- Dual-only helpers for trial flow ----
    async def stopTrial_both(self):
        # 'G' = stop trial (your single-device version exists)
        return await self._fanout_results(self.UART_TX_UUID, b"G", False)

    async def switchToAssist_both(self):
        # 'c' = assist mode
        return await self._fanout_results(self.UART_TX_UUID, b"c", False)

    async def switchToResist_both(self):
        # 'S' = resist mode
        return await self._fanout_results(self.UART_TX_UUID, b"S", False)

    async def sendFsrPreset_both(self, left: float, right: float):
        """
        Mirror of your single-device sendFsrValues() but dual-only:
        header 'R' then two doubles (left,right) to both devices.
        """
        # header
        hdr = await self._fanout_results(self.UART_TX_UUID, b"R", False)
        # payload (send twice: left then right)
        lb = struct.pack("<d", left)
        rb = struct.pack("<d", right)
        payL = await self._fanout_results(self.UART_TX_UUID, lb, False)
        payR = await self._fanout_results(self.UART_TX_UUID, rb, False)
        return {"header": hdr, "left": payL, "right": payR}

    async def sendStiffness_both(self, stiffness: float):
        # header 'A' + <double>
        hdr = await self._fanout_results(self.UART_TX_UUID, b"A", False)
        pay = await self._fanout_results(self.UART_TX_UUID, struct.pack("<d", float(stiffness)), False)
        return {"header": hdr, "payload": pay}
    
    async def scanAndConnectMulti(self):
        if len(self.device_addresses) != 2:
            raise RuntimeError("scanAndConnectMulti requires exactly two addresses")
        addrs = list(self.device_addresses)
        for a in addrs:
            self.connections.pop(a, None)

        connected = []
        try:
            for addr in addrs:
                print(f"[{addr}] scanning…")
                dev = await BleakScanner.find_device_by_address(addr, timeout=10.0)
                if not dev:
                    print(f"[{addr}] not found")
                    raise RuntimeError(f"{addr} not found")

                cli = BleakClient(dev, disconnected_callback=self._make_disconnect_cb(addr))
                await cli.connect()
                print(f"[{addr}] connected: {cli.is_connected}")
                services = cli.services

                async def _notify(sender, data, _addr=addr):
                    await self.DataIn(sender, data, src_addr=_addr)

                await cli.start_notify(self.UART_RX_UUID, _notify)
                print(f"[{addr}] notifications started")
                if hasattr(services, "characteristics"):
                    print(f"[{addr}] services cached: {len(services.characteristics)} characteristics total")

                self.connections[addr] = {
                    "device": dev,
                    "client": cli,
                    "services": services,
                    "is_connected": True,
                }
                self._ensure_queues(addr)
                self._ensure_processor(addr)

                connected.append(addr)

            return len(connected) == 2

        except Exception as e:
            print("scanAndConnectMulti error:", e)
            for addr in connected:
                try:
                    await self.connections[addr]["client"].disconnect()
                except Exception:
                    pass
                self.connections[addr]["is_connected"] = False
            return False

    async def calibrateFSRs_both(self):
        tasks = []
        for addr, conn in self.connections.items():
            if conn.get("is_connected"):
                try:
                    ch = self.get_char_handle(self.UART_TX_UUID, addr=addr)
                    tasks.append(conn["client"].write_gatt_char(ch, b"L", False))
                except Exception as e:
                    print(f"[{addr}] calibrateFSRs error:", e)
        if tasks:
            await asyncio.gather(*tasks, return_exceptions=True)

    # Send FSR values to Exo
    async def sendFsrValues(self, left_fsr, right_fsr):
        command = bytearray(b"R")  # Command to indicate sending FSR values
        char = self.get_char_handle(self.UART_TX_UUID)

        # Update the stored values
        self.curr_left_fsr_value = left_fsr
        self.curr_right_fsr_value = right_fsr

        # Send the command to indicate that we are sending FSR values
        await self.client.write_gatt_char(char, command, False)

        # Pack the left and right FSR values as doubles
        for fsr_value in (left_fsr, right_fsr):  # Loop over both values
            fsr_bytes = struct.pack("<d", fsr_value)  # Pack each value as double
            await self.client.write_gatt_char(char, fsr_bytes, False)

    # Send preset FSR values to Exo
    async def sendPresetFsrValues(self):
        await self.sendFsrValues(self.curr_left_fsr_value, self.curr_right_fsr_value)

    # Send stop trial command to Exo
    async def stopTrial(self):
        command = bytearray(b"G")
        char = self.get_char_handle(self.UART_TX_UUID)

        await self.client.write_gatt_char(char, command, False)

    # Switch to assist mode
    async def switchToAssist(self):
        command = bytearray(b"c")
        char = self.get_char_handle(self.UART_TX_UUID)

        await self.client.write_gatt_char(char, command, False)
        print("using bleak assist\n")

    # Switch to resist mode
    async def switchToResist(self):
        print("using bleak resist\n")
        command = bytearray(b"S")
        char = self.get_char_handle(self.UART_TX_UUID)

        await self.client.write_gatt_char(char, command, False)

    # Send stiffness values to Exo
    async def sendStiffness(self, stiffness):
        command = bytearray(b"A")  # Character code for stiffness
        char = self.get_char_handle(self.UART_TX_UUID)
        await self.client.write_gatt_char(char, command, False)  # Send the command code

        stiff_bytes = struct.pack("<d", stiffness)
        await self.client.write_gatt_char(char, stiff_bytes, False)  # Send the stiffness value
        print(f"Stiffness is {stiffness}")

    # Helper function to send new stiffness values to Exo
    async def newStiffness(self, stiffnessInput):
        stiffnessVal = float(stiffnessInput)
        await self.sendStiffness(stiffnessVal)  # Send the new stiffness value

    # Play
    async def play(self):
        command = bytearray(b"X")
        char = self.get_char_handle(self.UART_TX_UUID)

        await self.client.write_gatt_char(char, command, True)

