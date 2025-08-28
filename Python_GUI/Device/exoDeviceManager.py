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

        self.device_addresses = []              # up to two
        self.connections = {}
        self.available_devices = {}  # List to store available devices
        self.device = None
        self.client = None
        self.services = None
        self.scanResults = None
        self.disconnecting_intentionally = False  # Flag for intentional disconnect

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
        if not isinstance(addrs, (list, tuple)) or not (1 <= len(addrs) <= 2):
            raise ValueError("set_deviceAddresses expects 1–2 addresses")
        self.device_addresses = list(addrs)
        
    # Set the BLE client
    def set_client(self, clientVal):
        self.client = clientVal

    # Set the services of the device
    def set_services(self, servicesVal):
        self.services = servicesVal

    @staticmethod
    def _looks_like_ascii(buf: bytes) -> bool:
        # True if buffer appears to be printable ASCII (firmware chatter)
        if not buf:
            return False
        return all(32 <= b <= 126 or b in (9, 10, 13) for b in buf)
    
    async def DataIn(self, sender: BleakGATTCharacteristic, data: bytearray, src_addr: Optional[str] = None):
        if getattr(self, "debug_rx", False):
            try:
                s = data.decode('ascii', 'ignore')
                print(f"[{src_addr}] RX ASCII:", s)
            except Exception:
                pass
        self._realTimeProcessor.processEvent(data)
        await self.MachineLearnerControl()
    
    async def _write(self, addr: str, char_uuid: str, data: bytes, response: bool = False):
        conn = self.connections.get(addr)
        if not conn or not conn.get("is_connected"):
            return False
        cli = conn["client"]
        ch = self.get_char_handle(char_uuid, addr=addr)
        lock = self._write_locks.setdefault(addr, asyncio.Lock())
        async with lock:
            await cli.write_gatt_char(ch, data, response)
        return True

    async def _fanout(self, char_uuid: str, data: bytes, response: bool = False):
        tasks = []
        for addr, conn in list(self.connections.items()):
            if conn.get("is_connected"):
                tasks.append(self._write(addr, char_uuid, data, response))
        if tasks:
            await asyncio.gather(*tasks, return_exceptions=True)

    async def motorOn_both(self):  await self._fanout(self.UART_TX_UUID, b"x", True)
    async def motorOff_both(self): await self._fanout(self.UART_TX_UUID, b"w", True)

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
        print("Using Bleak scan...")
        print("Scanning...")

        attempts = 4
        for attempt in range(attempts):
            print(f"Attempt {attempt + 1} of {attempts}")

            # Scan for devices using the filter
            device = await BleakScanner.find_device_by_filter(self.filterExo)

            if device:
                print(f"Found device: {device.name} - {device.address}")
                
                # Check if the found device's address matches the specified address
                if device.address == self.device_mac_address:
                    print("Matching device found. Connecting...")
                    
                    self.set_device(device)  # Set the device
                    self.set_client(BleakClient(device, disconnected_callback=self.handleDisconnect))
                    
                    # Try to connect to the Exo
                    try:
                        await self.client.connect()
                        self.isConnected = True
                        print("Is Exo connected: " + str(self.isConnected))
                        print(self.client)

                        # Get list of services from Exo
                        self.set_services(self.client.services) #Get_Services Not Supported in v1.0 and beyond of bleak library, previously was [await self.client.get_services()] instead of [self.client.services].
                        
                        # Start incoming data stream
                        await self.client.start_notify(self.UART_RX_UUID, self.DataIn)
                        return True  # Successful connection
                    except Exception as e:
                        print(f"Failed to connect: {e}")
                        return False  # Connection failed
                else:
                    print("Found device does not match the specified address.")
            else:
                print("No device found.")

        print("Max attempts reached. Could not connect.")
        return False  # Return False if all attempts fail

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

