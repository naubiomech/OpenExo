import asyncio
import struct
import threading
from typing import Optional

try:
    from PySide6 import QtCore
except ImportError as e:
    raise SystemExit("PySide6 is required. Install with: pip install PySide6") from e

try:
    from bleak import BleakClient, BleakScanner
    BLE_AVAILABLE = True
except Exception:
    BLE_AVAILABLE = False


UART_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
UART_TX_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"  # Write
UART_RX_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"  # Notify


class QtExoDeviceManager(QtCore.QObject):
    """
    Qt-native device manager for BLE (standalone for the Qt app).
    - Keeps Python_GUI/Device/ code untouched
    - Emits Qt signals for UI/pages to consume
    - Provides minimal BLE: scan/connect/disconnect/notify/write
    """

    connected = QtCore.Signal(str, str)     # name, address
    disconnected = QtCore.Signal()
    error = QtCore.Signal(str)
    log = QtCore.Signal(str)
    dataReceived = QtCore.Signal(bytes)     # raw bytes from UART RX notify
    scanResults = QtCore.Signal(list)       # list[(name, address)]

    def __init__(self, parent=None):
        super().__init__(parent)
        self._mac: Optional[str] = None
        self._client: Optional[object] = None
        self._is_connecting = False
        self._is_connected = False
        # Persistent asyncio loop running in a background thread
        self._loop: Optional[asyncio.AbstractEventLoop] = None
        self._loop_thread: Optional[threading.Thread] = None
        # Store last FSR values
        self._curr_left_fsr_value: float = 0.25
        self._curr_right_fsr_value: float = 0.25
        # Joint dictionary matching legacy manager
        self.jointDictionary = {
            1: 33.0,
            2: 65.0,
            3: 34.0,
            4: 66.0,
            5: 36.0,
            6: 68.0,
            7: 40.0,
            8: 72.0,
        }

    # Public API
    @QtCore.Slot(str)
    def set_mac(self, mac: str):
        self._mac = mac

    @QtCore.Slot()
    def scan(self):
        if not BLE_AVAILABLE:
            self.error.emit("Bleak not available. Install with: pip install bleak")
            self.scanResults.emit([])
            return
        self._ensure_loop()

        async def _run_scan():
            results = {}
            try:
                self.log.emit("Scanning (UART UUID filter)…")
                for i in range(3):
                    print(f"[QtExoDeviceManager] scan attempt {i+1} of 3")
                    try:
                        device = await BleakScanner.find_device_by_filter(self._filter_exo)
                        if device:
                            results[device.address] = device.name or "Unknown"
                    except Exception as ex:
                        print(f"[QtExoDeviceManager] scan error: {ex}")
            finally:
                lst = [(name, addr) for addr, name in results.items()]
                self.scanResults.emit(lst)

        asyncio.run_coroutine_threadsafe(_run_scan(), self._loop)

    @QtCore.Slot()
    def connect(self):
        if not BLE_AVAILABLE:
            self.error.emit("Bleak not available. Install with: pip install bleak")
            return
        if not self._mac:
            self.error.emit("No MAC address set")
            return
        if self._is_connecting or self._is_connected:
            return

        self._is_connecting = True
        print(f"[QtExoDeviceManager] connect requested -> mac={self._mac}")
        self.log.emit(f"Connecting to {self._mac}…")
        self._ensure_loop()

        async def _run_connect():
            try:
                attempts = 4
                for attempt in range(attempts):
                    self.log.emit(f"Attempt {attempt+1} of {attempts}")
                    # exoDeviceManager.py approach: filter by UART service UUID during scan
                    self.log.emit("Scanning with UART UUID filter…")
                    device = None
                    try:
                        device = await BleakScanner.find_device_by_filter(self._filter_exo, timeout=10.0)
                    except Exception as se:
                        print(f"[QtExoDeviceManager] find_device_by_filter error: {se}")

                    if device:
                        self.log.emit(f"Found: {device.name} - {device.address}")
                        # If a specific MAC was requested, ensure match
                        if self._mac and device.address != self._mac:
                            self.log.emit("Found device does not match the specified address.")
                            device = None
                        else:
                            # Try to connect
                            self.log.emit("Connecting…")
                            print(f"[QtExoDeviceManager] connecting to {device.name} {device.address}")
                            def _disc_cb(_):
                                try:
                                    self.log.emit("Disconnected")
                                    self.disconnected.emit()
                                except Exception:
                                    pass
                            client = BleakClient(device, disconnected_callback=_disc_cb)
                            ok = await client.connect()
                            print(f"[QtExoDeviceManager] connect() returned={ok}, is_connected={getattr(client, 'is_connected', False)}")
                            if not getattr(client, "is_connected", False):
                                # Cleanup and retry next attempt
                                try:
                                    await client.disconnect()
                                except Exception:
                                    pass
                                await asyncio.sleep(2)
                                continue

                            # Touch services to populate cache
                            _ = client.services

                            def _on_rx(sender, data: bytearray):
                                try:
                                    self.dataReceived.emit(bytes(data))
                                except Exception:
                                    pass

                            self.log.emit("Starting notifications…")
                            await client.start_notify(UART_RX_UUID, _on_rx)

                            self._client = client
                            self._is_connected = True
                            self.connected.emit(device.name or "", device.address)
                            self.log.emit("Connected and notifications started")
                            print("[QtExoDeviceManager] connected; notify started")
                            return
                    else:
                        self.log.emit("No device found.")

                # If we exit loop without returning
                self.error.emit("Max attempts reached. Could not connect.")
            except Exception as ex:
                self.error.emit(str(ex))
                print(f"[QtExoDeviceManager] connect error: {ex}")
            finally:
                self._is_connecting = False

        asyncio.run_coroutine_threadsafe(_run_connect(), self._loop)


    @QtCore.Slot()
    def disconnect(self):
        if not self._client and not self._loop:
            return

        async def _run_disconnect():
            try:
                if self._client:
                    try:
                        await self._client.stop_notify(UART_RX_UUID)
                    except Exception:
                        pass
                    try:
                        await self._client.disconnect()
                    except Exception:
                        pass
                    self.log.emit("Disconnected")
            except Exception as ex:
                self.error.emit(str(ex))
            finally:
                self._client = None
                self._is_connected = False
                self.disconnected.emit()

        if self._loop:
            fut = asyncio.run_coroutine_threadsafe(_run_disconnect(), self._loop)

            def _stop_loop(_):
                try:
                    if self._loop:
                        self._loop.call_soon_threadsafe(self._loop.stop)
                except Exception:
                    pass
                finally:
                    self._loop = None
                    self._loop_thread = None

            fut.add_done_callback(_stop_loop)

    @QtCore.Slot(bytes)
    def write(self, payload: bytes):
        if not self._client or not self._loop:
            self.error.emit("Not connected")
            return

        async def _run_write():
            try:
                await self._client.write_gatt_char(UART_TX_UUID, payload, response=False)
            except Exception as ex:
                self.error.emit(str(ex))

        asyncio.run_coroutine_threadsafe(_run_write(), self._loop)

    # ----- Command helpers and API parity with legacy ExoDeviceManager -----
    def _ensure_connected(self) -> bool:
        if not (self._client and self._loop and self._is_connected):
            self.error.emit("Not connected")
            return False
        return True

    def _submit(self, coro):
        return asyncio.run_coroutine_threadsafe(coro, self._loop)

    @QtCore.Slot()
    def startExoMotors(self):
        if not self._ensure_connected():
            return

        async def _do():
            try:
                await asyncio.sleep(1)
                await self._client.write_gatt_char(UART_TX_UUID, b"E", response=False)
                self.log.emit("Start motors command sent")
            except Exception as ex:
                self.error.emit(str(ex))

        self._submit(_do())

    @QtCore.Slot()
    def calibrateTorque(self):
        if not self._ensure_connected():
            return

        async def _do():
            try:
                await self._client.write_gatt_char(UART_TX_UUID, b"H", response=False)
                self.log.emit("Calibrate torque command sent")
            except Exception as ex:
                self.error.emit(str(ex))

        self._submit(_do())

    @QtCore.Slot()
    def calibrateFSRs(self):
        if not self._ensure_connected():
            return

        async def _do():
            try:
                await self._client.write_gatt_char(UART_TX_UUID, b"L", response=False)
                self.log.emit("Calibrate FSRs command sent")
            except Exception as ex:
                self.error.emit(str(ex))

        self._submit(_do())

    @QtCore.Slot()
    def motorOff(self):
        if not self._ensure_connected():
            return

        async def _do():
            try:
                await self._client.write_gatt_char(UART_TX_UUID, b"w", response=True)
                self.log.emit("Motor OFF command sent")
            except Exception as ex:
                self.error.emit(str(ex))

        self._submit(_do())

    @QtCore.Slot()
    def motorOn(self):
        if not self._ensure_connected():
            return

        async def _do():
            try:
                await self._client.write_gatt_char(UART_TX_UUID, b"x", response=True)
                self.log.emit("Motor ON command sent")
            except Exception as ex:
                self.error.emit(str(ex))

        self._submit(_do())

    @QtCore.Slot(list)
    def updateTorqueValues(self, parameter_list: list):
        if not self._ensure_connected():
            return

        async def _do():
            try:
                totalLoops = 1
                loopCount = 0
                float_values = parameter_list

                if float_values and bool(float_values[0]) is True:
                    totalLoops = 2

                while loopCount != totalLoops:
                    await self._client.write_gatt_char(UART_TX_UUID, b"f", response=False)

                    for i in range(1, len(float_values)):
                        if i == 1:
                            key = float_values[1]
                            joint_val = self.jointDictionary.get(key)
                            if joint_val is None:
                                raise ValueError("Invalid joint selection")
                            if loopCount == 1 and key % 2 == 0:
                                val = joint_val - 32
                            elif loopCount == 1 and key % 2 != 0:
                                val = joint_val + 32
                            else:
                                val = joint_val
                            float_bytes = struct.pack("<d", float(val))
                        else:
                            float_bytes = struct.pack("<d", float(float_values[i]))
                        await self._client.write_gatt_char(UART_TX_UUID, float_bytes, response=False)

                    loopCount += 1
                self.log.emit("Torque parameters updated")
            except Exception as ex:
                self.error.emit(str(ex))

        self._submit(_do())

    @QtCore.Slot(float, float)
    def sendFsrValues(self, left_fsr: float, right_fsr: float):
        if not self._ensure_connected():
            return

        async def _do():
            try:
                self._curr_left_fsr_value = float(left_fsr)
                self._curr_right_fsr_value = float(right_fsr)
                await self._client.write_gatt_char(UART_TX_UUID, b"R", response=False)
                for fsr_value in (self._curr_left_fsr_value, self._curr_right_fsr_value):
                    fsr_bytes = struct.pack("<d", float(fsr_value))
                    await self._client.write_gatt_char(UART_TX_UUID, fsr_bytes, response=False)
                self.log.emit("FSR values sent")
            except Exception as ex:
                self.error.emit(str(ex))

        self._submit(_do())

    @QtCore.Slot()
    def sendPresetFsrValues(self):
        if not self._ensure_connected():
            return

        async def _do():
            try:
                await self._client.write_gatt_char(UART_TX_UUID, b"R", response=False)
                for fsr_value in (self._curr_left_fsr_value, self._curr_right_fsr_value):
                    fsr_bytes = struct.pack("<d", float(fsr_value))
                    await self._client.write_gatt_char(UART_TX_UUID, fsr_bytes, response=False)
                self.log.emit("Preset FSR values sent")
            except Exception as ex:
                self.error.emit(str(ex))

        self._submit(_do())

    @QtCore.Slot()
    def stopTrial(self):
        if not self._ensure_connected():
            return

        async def _do():
            try:
                await self._client.write_gatt_char(UART_TX_UUID, b"G", response=False)
                self.log.emit("Stop trial command sent")
            except Exception as ex:
                self.error.emit(str(ex))

        self._submit(_do())

    @QtCore.Slot()
    def switchToAssist(self):
        if not self._ensure_connected():
            return

        async def _do():
            try:
                await self._client.write_gatt_char(UART_TX_UUID, b"c", response=False)
                self.log.emit("Assist mode command sent")
            except Exception as ex:
                self.error.emit(str(ex))

        self._submit(_do())

    @QtCore.Slot()
    def switchToResist(self):
        if not self._ensure_connected():
            return

        async def _do():
            try:
                await self._client.write_gatt_char(UART_TX_UUID, b"S", response=False)
                self.log.emit("Resist mode command sent")
            except Exception as ex:
                self.error.emit(str(ex))

        self._submit(_do())

    @QtCore.Slot(float)
    def sendStiffness(self, stiffness: float):
        if not self._ensure_connected():
            return

        async def _do():
            try:
                await self._client.write_gatt_char(UART_TX_UUID, b"A", response=False)
                stiff_bytes = struct.pack("<d", float(stiffness))
                await self._client.write_gatt_char(UART_TX_UUID, stiff_bytes, response=False)
                self.log.emit(f"Stiffness sent: {stiffness}")
            except Exception as ex:
                self.error.emit(str(ex))

        self._submit(_do())

    @QtCore.Slot(object)
    def newStiffness(self, stiffnessInput):
        try:
            val = float(stiffnessInput)
        except Exception:
            self.error.emit("Invalid stiffness value")
            return
        self.sendStiffness(val)

    @QtCore.Slot()
    def play(self):
        if not self._ensure_connected():
            return

        async def _do():
            try:
                await self._client.write_gatt_char(UART_TX_UUID, b"X", response=True)
                self.log.emit("Play command sent")
            except Exception as ex:
                self.error.emit(str(ex))

        self._submit(_do())

    @QtCore.Slot()
    def send_acknowledgement(self):
        if not self._ensure_connected():
            return

        async def _do():
            try:
                await self._client.write_gatt_char(UART_TX_UUID, b"$", response=False)
                self.log.emit("Ack sent")
            except Exception as ex:
                self.error.emit(str(ex))

        self._submit(_do())

    @QtCore.Slot()
    def beginTrial(self):
        """Mirror legacy beginTrial: start motors, calibrate torque, calibrate FSRs, send preset FSR values."""
        if not self._ensure_connected():
            return

        async def _do():
            try:
                await asyncio.sleep(1)
                # Start motors/stream
                await self._client.write_gatt_char(UART_TX_UUID, b"E", response=False)
                # Calibrate torque sensors
                await self._client.write_gatt_char(UART_TX_UUID, b"H", response=False)
                # Calibrate FSRs
                await self._client.write_gatt_char(UART_TX_UUID, b"L", response=False)
                # Send preset FSR values
                await self._client.write_gatt_char(UART_TX_UUID, b"R", response=False)
                for fsr_value in (self._curr_left_fsr_value, self._curr_right_fsr_value):
                    fsr_bytes = struct.pack("<d", float(fsr_value))
                    await self._client.write_gatt_char(UART_TX_UUID, fsr_bytes, response=False)
                self.log.emit("Begin trial sequence sent")
            except Exception as ex:
                self.error.emit(str(ex))

        self._submit(_do())

    def _ensure_loop(self):
        if self._loop and self._loop_thread and self._loop_thread.is_alive():
            return
        self._loop = asyncio.new_event_loop()

        def _runner():
            asyncio.set_event_loop(self._loop)
            self._loop.run_forever()

        self._loop_thread = threading.Thread(target=_runner, daemon=True)
        self._loop_thread.start()

    # exoDeviceManager-style BLE filter (UART service UUID)
    @staticmethod
    def _filter_exo(device, adv) -> bool:
        try:
            uuids = set((adv.service_uuids or []))
            return UART_SERVICE_UUID.lower() in {u.lower() for u in uuids}
        except Exception:
            return False

    # Removed invalid get_char_handle; bleak accepts UUIDs directly


