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
ERROR_CHAR_UUID = "33b65d43-611c-11ed-9b6a-0242ac120002"  # Notify


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
    deviceErrorReceived = QtCore.Signal(str)  # error messages from ErrorChar notify
    scanResults = QtCore.Signal(list)       # list[(name, address)]
    scanProgress = QtCore.Signal(int)       # scan progress percentage (0-100)
    connectScanProgress = QtCore.Signal(int) # scanning phase during connection (0-100)
    connectionProgress = QtCore.Signal(int) # connection progress percentage (0-100)

    def __init__(self, parent=None):
        super().__init__(parent)
        self._mac: Optional[str] = None
        self._client: Optional[object] = None
        self._is_connecting = False
        self._is_connected = False
        self._error_notify_enabled = False
        self._intentional_disconnect = False  # Track if disconnect was intentional
        # Persistent asyncio loop running in a background thread
        self._loop: Optional[asyncio.AbstractEventLoop] = None
        self._loop_thread: Optional[threading.Thread] = None
        # Store last FSR values
        self._curr_left_fsr_value: float = 0.25
        self._curr_right_fsr_value: float = 0.25

    # Public API

    def _mark_disconnected(self, reason: str = ""):
        """Reset internal connection state after an unexpected disconnect."""
        self._is_connected = False
        self._is_connecting = False
        self._client = None

        # Only emit signals if this was NOT an intentional disconnect
        if not self._intentional_disconnect:
            if reason:
                self.log.emit(f"Disconnected ({reason})")
            else:
                self.log.emit("Disconnected")
            self.disconnected.emit()
        
        # Reset flag for next time
        self._intentional_disconnect = False

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
                self.scanProgress.emit(0)
                self.log.emit("Scanning for devices (UART UUID filter)…")
                print(f"[QtExoDeviceManager] Starting BLE scan with UART service filter")
                
                # Simulate progress during scan with periodic updates
                scan_duration = 10.0  # seconds
                update_interval = 0.5  # update every 0.5 seconds
                updates = int(scan_duration / update_interval)
                
                # Start the scan in background
                scan_task = asyncio.create_task(
                    BleakScanner.discover(
                        timeout=scan_duration,
                        service_uuids=[UART_SERVICE_UUID]
                    )
                )
                
                # Update progress while scanning
                for i in range(updates):
                    if scan_task.done():
                        break
                    progress = int((i + 1) / updates * 90)  # Go up to 90%
                    self.scanProgress.emit(progress)
                    await asyncio.sleep(update_interval)
                
                # Wait for scan to complete
                devices = await scan_task
                self.scanProgress.emit(95)
                
                print(f"[QtExoDeviceManager] Scan found {len(devices)} device(s)")
                
                # Filter and collect all matching devices
                for device in devices:
                    try:
                        # Double-check with our filter (belt and suspenders approach)
                        if device.address not in results:
                            results[device.address] = device.name or "Unknown"
                            print(f"[QtExoDeviceManager] Found: {device.name} ({device.address})")
                    except Exception as ex:
                        print(f"[QtExoDeviceManager] Error processing device: {ex}")
                
                self.scanProgress.emit(100)
                
                if not results:
                    self.log.emit("No OpenExo devices found")
                else:
                    self.log.emit(f"Found {len(results)} OpenExo device(s)")
                    
            except Exception as ex:
                self.error.emit(f"Scan error: {ex}")
                print(f"[QtExoDeviceManager] scan error: {ex}")
                self.scanProgress.emit(0)  # Reset on error
            finally:
                # Convert to list format: [(name, address), ...]
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
        # If our flags say "connected" but the underlying client isn't, recover.
        if self._client and self._is_connected and not getattr(self._client, "is_connected", False):
            self._mark_disconnected("stale client")
        if self._is_connecting or self._is_connected:
            return

        self._is_connecting = True
        print(f"[QtExoDeviceManager] connect requested -> mac={self._mac}")
        self.log.emit(f"Connecting to {self._mac}…")
        self._ensure_loop()

        async def _run_connect():
            try:
                self.connectScanProgress.emit(0)
                self.connectionProgress.emit(0)
                # Wrap entire connection process in timeout (40 seconds total - 4 attempts × 10s each)
                async with asyncio.timeout(40):
                    attempts = 4
                    for attempt in range(attempts):
                        # Scanning phase progress: each attempt gets 25% (0-100%)
                        scan_base = int((attempt / attempts) * 100)
                        self.connectScanProgress.emit(scan_base)
                        
                        self.log.emit(f"Attempt {attempt+1} of {attempts}")
                        self.log.emit("Scanning for device…")
                        
                        device = None
                        try:
                            # Each scan attempt has 8 second timeout with progress updates
                            scan_duration = 8.0
                            scan_start = asyncio.get_event_loop().time()
                            
                            # Start scanning in background
                            scan_task = asyncio.create_task(
                                BleakScanner.find_device_by_filter(self._filter_exo, timeout=scan_duration)
                            )
                            
                            # Update progress while scanning
                            while not scan_task.done():
                                elapsed = asyncio.get_event_loop().time() - scan_start
                                scan_progress = min(elapsed / scan_duration, 1.0)
                                # Map to current attempt's 25% range
                                progress = scan_base + int(scan_progress * 25)
                                self.connectScanProgress.emit(progress)
                                await asyncio.sleep(0.2)  # Update every 200ms
                            
                            device = await scan_task
                        except Exception as se:
                            print(f"[QtExoDeviceManager] find_device_by_filter error: {se}")

                        if device:
                            # Device found - hide scanning bar, start connection bar
                            self.connectScanProgress.emit(100)  # Complete scanning
                            self.log.emit(f"Found: {device.name} - {device.address}")
                            # If a specific MAC was requested, ensure match
                            if self._mac and device.address != self._mac:
                                self.log.emit("Found device does not match the specified address.")
                                device = None
                            else:
                                # Hide scanning bar, start connecting phase
                                self.connectScanProgress.emit(-1)  # Signal to hide scanning bar
                                self.connectionProgress.emit(20)
                                self.log.emit("Connecting to device…")
                                print(f"[QtExoDeviceManager] connecting to {device.name} {device.address}")
                                def _disc_cb(_):
                                    try:
                                        self._mark_disconnected("link lost")
                                    except Exception:
                                        pass
                                client = BleakClient(device, disconnected_callback=_disc_cb)
                                ok = await client.connect()
                                self.connectionProgress.emit(50)
                                print(f"[QtExoDeviceManager] connect() returned={ok}, is_connected={getattr(client, 'is_connected', False)}")
                                if not getattr(client, "is_connected", False):
                                    # Cleanup and retry next attempt
                                    self.connectionProgress.emit(0)
                                    self.connectScanProgress.emit(scan_base)  # Keep scan progress
                                    try:
                                        await client.disconnect()
                                    except Exception:
                                        pass
                                    await asyncio.sleep(2)
                                    continue

                                # Touch services to populate cache
                                self.connectionProgress.emit(65)
                                _ = client.services

                                def _on_rx(sender, data: bytearray):
                                    try:
                                        self.dataReceived.emit(bytes(data))
                                    except Exception:
                                        pass

                                def _on_error(sender, data: bytearray):
                                    try:
                                        msg = bytes(data).decode("utf-8", errors="ignore").strip("\x00").strip()
                                        if msg:
                                            self.deviceErrorReceived.emit(msg)
                                    except Exception:
                                        pass

                                self.connectionProgress.emit(75)
                                self.log.emit("Starting notifications…")
                                await client.start_notify(UART_RX_UUID, _on_rx)
                                self._error_notify_enabled = False
                                try:
                                    await client.start_notify(ERROR_CHAR_UUID, _on_error)
                                    self._error_notify_enabled = True
                                except Exception as ex:
                                    self.log.emit("Error characteristic not found; continuing with UART only.")
                                    print(f"[QtExoDeviceManager] error char notify failed: {ex}")

                                self.connectionProgress.emit(90)
                                self._client = client
                                self._is_connected = True
                                self.connected.emit(device.name or "", device.address)
                                self.connectionProgress.emit(100)
                                self.log.emit("Connected and notifications started")
                                print("[QtExoDeviceManager] connected; notify started")
                                return
                        else:
                            self.log.emit("No device found.")

                    # If we exit loop without returning
                    self.error.emit("Max attempts reached. Could not connect.")
            except asyncio.TimeoutError:
                self.connectionProgress.emit(0)
                self.error.emit("Connection timeout after 40 seconds")
                print(f"[QtExoDeviceManager] connect timeout after 40 seconds")
            except Exception as ex:
                self.connectionProgress.emit(0)
                self.error.emit(str(ex))
                print(f"[QtExoDeviceManager] connect error: {ex}")
            finally:
                self._is_connecting = False

        asyncio.run_coroutine_threadsafe(_run_connect(), self._loop)


    @QtCore.Slot()
    def disconnect(self):
        """Immediately disconnect from device (intentional disconnect)."""
        if not self._client:
            return
        
        # Mark as intentional so we don't show disconnect notification
        self._intentional_disconnect = True
        
        # Immediately mark as disconnected in UI
        self._is_connected = False
        self._is_connecting = False
        
        # Store client reference and clear it immediately
        client_to_disconnect = self._client
        self._client = None
        
        # Do the actual BLE disconnect in background (non-blocking)
        async def _run_disconnect():
            try:
                if client_to_disconnect:
                    try:
                        await client_to_disconnect.stop_notify(UART_RX_UUID)
                    except Exception:
                        pass
                    if self._error_notify_enabled:
                        try:
                            await client_to_disconnect.stop_notify(ERROR_CHAR_UUID)
                        except Exception:
                            pass
                    try:
                        await client_to_disconnect.disconnect()
                    except Exception:
                        pass
                print("[QtExoDeviceManager] Disconnect complete")
            except Exception as ex:
                print(f"[QtExoDeviceManager] Disconnect error: {ex}")

        if self._loop:
            # Fire and forget - don't wait for disconnect to complete
            asyncio.run_coroutine_threadsafe(_run_disconnect(), self._loop)
            # Don't stop the loop - keep it running for reconnection

    @QtCore.Slot(bytes)
    def write(self, payload: bytes):
        if not self._client or not self._loop:
            # Silently fail if not connected (avoids errors during disconnect)
            return

        async def _run_write():
            try:
                if self._client:  # Double-check client still exists
                    await self._client.write_gatt_char(UART_TX_UUID, payload, response=False)
            except Exception as ex:
                # Only emit error if we're still supposed to be connected
                if self._is_connected:
                    self.error.emit(str(ex))

        asyncio.run_coroutine_threadsafe(_run_write(), self._loop)

    # ----- Command helpers and API parity with legacy ExoDeviceManager -----
    def _ensure_connected(self) -> bool:
        if not (self._client and self._loop and self._is_connected):
            self.error.emit("Not connected")
            return False

        # Check if the asyncio loop thread is still alive
        if not (self._loop_thread and self._loop_thread.is_alive()):
            self._mark_disconnected("event loop died")
            self.error.emit("Not connected - event loop stopped")
            return False

        # If the OS link dropped, BleakClient.is_connected will be False.
        if not getattr(self._client, "is_connected", False):
            self._mark_disconnected("stale client")
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
                use_bilateral = bool(float_values[0]) if float_values else False

                mirror_val = None
                if float_values and len(float_values) > 1 and use_bilateral:
                    key = int(float_values[1])
                    side_bits = key & 0x60
                    if side_bits in (0x20, 0x40):
                        totalLoops = 2
                        mirror_val = key ^ 0x60

                while loopCount != totalLoops:
                    await self._client.write_gatt_char(UART_TX_UUID, b"f", response=False)

                    for i in range(1, len(float_values)):
                        if i == 1:
                            key = int(float_values[1])
                            if use_bilateral and mirror_val is not None:
                                val = key if loopCount == 0 else mirror_val
                            else:
                                val = key
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
                # await self._client.write_gatt_char(UART_TX_UUID, b"H", response=False) # Commented out by ZL because added new button for torque calibration
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
