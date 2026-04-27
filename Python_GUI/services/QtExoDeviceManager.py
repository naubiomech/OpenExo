import asyncio
import struct
import threading
import logging
import os
import sys
import traceback
from datetime import datetime
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

from utils.debug import dprint


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
        
        # Setup logging system
        self._setup_logging()

    def _setup_logging(self):
        """Setup file-based logging system for debugging and error tracking."""
        try:
            # Create logs directory in Saved_Data folder
            base_dir = os.path.dirname(os.path.dirname(__file__))  # Python_GUI folder
            log_dir = os.path.join(base_dir, "Saved_Data", "logs")
            os.makedirs(log_dir, exist_ok=True)
            
            # Create logger with timestamp in filename
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            log_file = os.path.join(log_dir, f"device_manager_{timestamp}.log")
            
            # Configure logger
            self.logger = logging.getLogger(f"QtExoDeviceManager_{id(self)}")
            self.logger.setLevel(logging.DEBUG)
            
            # Remove any existing handlers to avoid duplicates
            self.logger.handlers.clear()
            
            # File handler with detailed formatting
            # Create custom handler that flushes on ERROR/CRITICAL
            class FlushingFileHandler(logging.FileHandler):
                def emit(self, record):
                    super().emit(record)
                    if record.levelno >= logging.ERROR:
                        self.flush()
            
            file_handler = FlushingFileHandler(log_file, encoding='utf-8')
            file_handler.setLevel(logging.DEBUG)
            formatter = logging.Formatter(
                '%(asctime)s.%(msecs)03d | %(levelname)-8s | %(funcName)-25s | %(message)s',
                datefmt='%Y-%m-%d %H:%M:%S'
            )
            file_handler.setFormatter(formatter)
            self.logger.addHandler(file_handler)
            
            # Also add console handler for development
            console_handler = logging.StreamHandler()
            console_handler.setLevel(logging.INFO)
            console_handler.setFormatter(formatter)
            self.logger.addHandler(console_handler)
            
            self.logger.info("=" * 80)
            self.logger.info("QtExoDeviceManager initialized")
            self.logger.info(f"Log file: {log_file}")
            self.logger.info(f"Python version: {sys.version}")
            self.logger.info(f"BLE Available: {BLE_AVAILABLE}")
            self.logger.info("=" * 80)
            
            # Store log file path for retrieval
            self._log_file_path = log_file
            
            # Install exception hook to catch ANY unhandled exception
            self._install_exception_hooks()
            
        except Exception as ex:
            # Fallback to basic logger if file creation fails
            self.logger = logging.getLogger(f"QtExoDeviceManager_{id(self)}")
            self.logger.setLevel(logging.INFO)
            self._log_file_path = None
            dprint(f"Warning: Could not setup file logging: {ex}")
    
    def _install_exception_hooks(self):
        """Install hooks to catch unhandled exceptions."""
        # Store original exception hook
        self._original_excepthook = sys.excepthook
        
        def custom_excepthook(exc_type, exc_value, exc_traceback):
            """Log any unhandled exception before it crashes the app."""
            if issubclass(exc_type, KeyboardInterrupt):
                # Don't log keyboard interrupts
                sys.__excepthook__(exc_type, exc_value, exc_traceback)
                return
            
            self.logger.critical("=" * 80)
            self.logger.critical("UNHANDLED EXCEPTION DETECTED!")
            self.logger.critical("=" * 80)
            self.logger.critical(f"Exception Type: {exc_type.__name__}")
            self.logger.critical(f"Exception Value: {exc_value}")
            self.logger.critical("Traceback:")
            for line in traceback.format_tb(exc_traceback):
                self.logger.critical(line.strip())
            self.logger.critical("=" * 80)
            
            # Call original exception hook to maintain normal behavior
            self._original_excepthook(exc_type, exc_value, exc_traceback)
        
        sys.excepthook = custom_excepthook
        self.logger.info("Exception hooks installed - all unhandled exceptions will be logged")
    
    def get_log_file_path(self) -> str:
        """Get the path to the current log file."""
        return getattr(self, '_log_file_path', None) or "Log file not available"

    # Public API

    def _mark_disconnected(self, reason: str = ""):
        """Reset internal connection state after an unexpected disconnect."""
        self.logger.warning(f"Device disconnected. Reason: {reason or 'unknown'}, Intentional: {self._intentional_disconnect}")
        
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
            self.logger.info("Disconnected signal emitted to UI")
        else:
            self.logger.info("Intentional disconnect - no signal emitted")
        
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
                dprint(f"[QtExoDeviceManager] Starting BLE scan with UART service filter")
                
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
                
                dprint(f"[QtExoDeviceManager] Scan found {len(devices)} device(s)")
                
                # Filter and collect all matching devices
                for device in devices:
                    try:
                        # Double-check with our filter (belt and suspenders approach)
                        if device.address not in results:
                            results[device.address] = device.name or "Unknown"
                            dprint(f"[QtExoDeviceManager] Found: {device.name} ({device.address})")
                    except Exception as ex:
                        dprint(f"[QtExoDeviceManager] Error processing device: {ex}")
                
                self.scanProgress.emit(100)
                
                if not results:
                    self.log.emit("No OpenExo devices found")
                else:
                    self.log.emit(f"Found {len(results)} OpenExo device(s)")
                    
            except Exception as ex:
                self.error.emit(f"Scan error: {ex}")
                dprint(f"[QtExoDeviceManager] scan error: {ex}")
                self.scanProgress.emit(0)  # Reset on error
            finally:
                # Convert to list format: [(name, address), ...]
                lst = [(name, addr) for addr, name in results.items()]
                self.scanResults.emit(lst)

        asyncio.run_coroutine_threadsafe(_run_scan(), self._loop)

    @QtCore.Slot()
    def connect(self):
        self.logger.info(f"connect() called - MAC: {self._mac}")
        
        if not BLE_AVAILABLE:
            self.logger.error("BLE not available - Bleak not installed")
            self.error.emit("Bleak not available. Install with: pip install bleak")
            return
        if not self._mac:
            self.logger.error("Connect failed - no MAC address set")
            self.error.emit("No MAC address set")
            return
        # If our flags say "connected" but the underlying client isn't, recover.
        if self._client and self._is_connected and not getattr(self._client, "is_connected", False):
            self.logger.warning("Stale client detected - marking as disconnected")
            self._mark_disconnected("stale client")
        if self._is_connecting or self._is_connected:
            self.logger.warning("Already connecting or connected - ignoring connect request")
            return

        self._is_connecting = True
        dprint(f"[QtExoDeviceManager] connect requested -> mac={self._mac}")
        self.log.emit(f"Connecting to {self._mac}…")
        self.logger.info("Ensuring event loop is running")
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
                            dprint(f"[QtExoDeviceManager] find_device_by_filter error: {se}")

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
                                dprint(f"[QtExoDeviceManager] connecting to {device.name} {device.address}")
                                def _disc_cb(_):
                                    try:
                                        self._mark_disconnected("link lost")
                                    except Exception:
                                        pass
                                client = BleakClient(device, disconnected_callback=_disc_cb)
                                ok = await client.connect()
                                self.connectionProgress.emit(50)
                                dprint(f"[QtExoDeviceManager] connect() returned={ok}, is_connected={getattr(client, 'is_connected', False)}")
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
                                    dprint(f"[QtExoDeviceManager] error char notify failed: {ex}")

                                self.connectionProgress.emit(90)
                                self._client = client
                                self._is_connected = True
                                self.logger.info(f"Successfully connected to {device.name} ({device.address})")
                                self.connected.emit(device.name or "", device.address)
                                self.connectionProgress.emit(100)
                                self.log.emit("Connected and notifications started")
                                dprint("[QtExoDeviceManager] connected; notify started")
                                return
                        else:
                            self.log.emit("No device found.")

                    # If we exit loop without returning
                    self.logger.error("Connection failed - max attempts reached")
                    self.error.emit("Max attempts reached. Could not connect.")
            except asyncio.TimeoutError:
                self.logger.error("Connection timeout after 40 seconds")
                self.connectionProgress.emit(0)
                self.error.emit("Connection timeout after 40 seconds")
                dprint(f"[QtExoDeviceManager] connect timeout after 40 seconds")
            except Exception as ex:
                self.logger.exception(f"Connection error: {ex}")
                self.connectionProgress.emit(0)
                self.error.emit(str(ex))
                dprint(f"[QtExoDeviceManager] connect error: {ex}")
            finally:
                self._is_connecting = False
                self.logger.info("Connection attempt completed")

        asyncio.run_coroutine_threadsafe(_run_connect(), self._loop)


    @QtCore.Slot()
    def disconnect(self):
        """Immediately disconnect from device (intentional disconnect)."""
        self.logger.info("disconnect() called - intentional disconnect")
        
        if not self._client:
            self.logger.warning("disconnect() called but no client exists")
            return
        
        # Mark as intentional so we don't show disconnect notification
        self._intentional_disconnect = True
        
        # Immediately mark as disconnected in UI
        self._is_connected = False
        self._is_connecting = False
        
        # Store client reference and clear it immediately
        client_to_disconnect = self._client
        self._client = None
        
        self.logger.info("Client marked for disconnect, state cleared")
        
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
                dprint("[QtExoDeviceManager] Disconnect complete")
            except Exception as ex:
                dprint(f"[QtExoDeviceManager] Disconnect error: {ex}")

        if self._loop:
            # Fire and forget - don't wait for disconnect to complete
            asyncio.run_coroutine_threadsafe(_run_disconnect(), self._loop)
            # Don't stop the loop - keep it running for reconnection

    @QtCore.Slot(bytes)
    def write(self, payload: bytes):
        if not self._client or not self._loop:
            # Silently fail if not connected (avoids errors during disconnect)
            self.logger.debug(f"Write aborted - no client or loop. Payload: {payload}")
            return

        async def _run_write():
            try:
                if self._client:  # Double-check client still exists
                    self.logger.debug(f"Writing to UART: {payload}")
                    await self._client.write_gatt_char(UART_TX_UUID, payload, response=False)
                    self.logger.debug(f"Write successful: {payload}")
            except Exception as ex:
                self.logger.exception(f"Write failed for payload {payload}: {ex}")
                # Only emit error if we're still supposed to be connected
                if self._is_connected:
                    self.error.emit(str(ex))

        asyncio.run_coroutine_threadsafe(_run_write(), self._loop)

    # ----- Command helpers and API parity with legacy ExoDeviceManager -----
    def _ensure_connected(self) -> bool:
        self.logger.debug(f"Checking connection: client={self._client is not None}, loop={self._loop is not None}, connected={self._is_connected}")
        
        if not (self._client and self._loop and self._is_connected):
            self.logger.error("Connection check failed: missing client, loop, or not marked as connected")
            self.error.emit("Not connected")
            return False

        # Check if the asyncio loop thread is still alive
        if not (self._loop_thread and self._loop_thread.is_alive()):
            self.logger.critical("Event loop thread is dead!")
            self._mark_disconnected("event loop died")
            self.error.emit("Not connected - event loop stopped")
            return False

        # If the OS link dropped, BleakClient.is_connected will be False.
        if not getattr(self._client, "is_connected", False):
            self.logger.warning("BleakClient reports not connected (stale client)")
            self._mark_disconnected("stale client")
            self.error.emit("Not connected")
            return False

        self.logger.debug("Connection check passed")
        return True

    def _submit(self, coro):
        """Submit coroutine to event loop with error handling and logging."""
        try:
            if not self._loop:
                self.logger.critical("Cannot submit coroutine: event loop is None!")
                self.error.emit("Internal error: event loop not initialized")
                return None
            
            if not self._loop_thread or not self._loop_thread.is_alive():
                self.logger.critical("Cannot submit coroutine: event loop thread is not alive!")
                self.error.emit("Internal error: event loop thread stopped")
                return None
            
            self.logger.debug(f"Submitting coroutine: {coro.__name__ if hasattr(coro, '__name__') else str(coro)}")
            future = asyncio.run_coroutine_threadsafe(coro, self._loop)
            
            # Add callback to log any exceptions from the coroutine
            def _log_exception(fut):
                try:
                    # This will raise if the coroutine had an exception
                    fut.result()
                except Exception as ex:
                    # Only log if the exception wasn't already handled
                    self.logger.error(f"Unhandled exception in coroutine: {ex}", exc_info=True)
            
            future.add_done_callback(_log_exception)
            return future
        except Exception as ex:
            self.logger.exception(f"Error submitting coroutine: {ex}")
            self.error.emit(f"Internal error: {ex}")
            return None

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
        self.logger.info("calibrateTorque() called")
        
        if not self._ensure_connected():
            self.logger.warning("calibrateTorque() aborted - not connected")
            return

        async def _do():
            try:
                self.logger.info("Sending torque calibration command 'H'")
                await self._client.write_gatt_char(UART_TX_UUID, b"H", response=False)
                self.log.emit("Calibrate torque command sent")
                self.logger.info("Torque calibration command sent successfully")
            except Exception as ex:
                self.logger.exception(f"Error in calibrateTorque: {ex}")
                self.error.emit(str(ex))

        self._submit(_do())

    @QtCore.Slot()
    def calibrateFSRs(self):
        self.logger.info("calibrateFSRs() called")
        
        if not self._ensure_connected():
            self.logger.warning("calibrateFSRs() aborted - not connected")
            return

        async def _do():
            try:
                self.logger.info("Sending FSR calibration command 'L'")
                await self._client.write_gatt_char(UART_TX_UUID, b"L", response=False)
                self.log.emit("Calibrate FSRs command sent")
                self.logger.info("FSR calibration command sent successfully")
            except Exception as ex:
                self.logger.exception(f"Error in calibrateFSRs: {ex}")
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
        self.logger.info("beginTrial() called - starting trial sequence")
        
        if not self._ensure_connected():
            self.logger.warning("beginTrial() aborted - not connected")
            return

        async def _do():
            try:
                self.logger.info("Begin trial sequence starting")
                await asyncio.sleep(1)
                # Start motors/stream
                self.logger.debug("Sending command 'E' (start motors)")
                await self._client.write_gatt_char(UART_TX_UUID, b"E", response=False)
                # Calibrate torque sensors 
                # await self._client.write_gatt_char(UART_TX_UUID, b"H", response=False) # Commented out by ZL because added new button for torque calibration
                # Calibrate FSRs
                self.logger.debug("Sending command 'L' (calibrate FSRs)")
                await self._client.write_gatt_char(UART_TX_UUID, b"L", response=False)
                # Send preset FSR values
                self.logger.debug(f"Sending preset FSR values: left={self._curr_left_fsr_value}, right={self._curr_right_fsr_value}")
                await self._client.write_gatt_char(UART_TX_UUID, b"R", response=False)
                for fsr_value in (self._curr_left_fsr_value, self._curr_right_fsr_value):
                    fsr_bytes = struct.pack("<d", float(fsr_value))
                    await self._client.write_gatt_char(UART_TX_UUID, fsr_bytes, response=False)
                self.log.emit("Begin trial sequence sent")
                self.logger.info("Begin trial sequence completed successfully")
            except Exception as ex:
                self.logger.exception(f"Error in beginTrial: {ex}")
                self.error.emit(str(ex))

        self._submit(_do())

    def _ensure_loop(self):
        if self._loop and self._loop_thread and self._loop_thread.is_alive():
            self.logger.debug("Event loop already running")
            return
        
        self.logger.info("Creating new event loop thread")
        self._loop = asyncio.new_event_loop()

        def _runner():
            try:
                asyncio.set_event_loop(self._loop)
                self.logger.info("Event loop thread started")
                
                # Set exception handler for the event loop to catch async exceptions
                def _loop_exception_handler(loop, context):
                    exception = context.get('exception')
                    message = context.get('message', 'No message')
                    self.logger.error("=" * 80)
                    self.logger.error("ASYNC EXCEPTION IN EVENT LOOP")
                    self.logger.error(f"Message: {message}")
                    if exception:
                        self.logger.error(f"Exception: {exception}", exc_info=exception)
                    else:
                        self.logger.error(f"Context: {context}")
                    self.logger.error("=" * 80)
                
                self._loop.set_exception_handler(_loop_exception_handler)
                
                self._loop.run_forever()
                self.logger.warning("Event loop stopped running")
            except Exception as ex:
                self.logger.critical(f"Event loop thread crashed: {ex}", exc_info=True)
                self.logger.critical("=" * 80)

        self._loop_thread = threading.Thread(target=_runner, daemon=True, name="BLE-EventLoop")
        self._loop_thread.start()
        self.logger.info(f"Event loop thread created (thread_id: {self._loop_thread.ident}, name: {self._loop_thread.name})")

    # exoDeviceManager-style BLE filter (UART service UUID)
    @staticmethod
    def _filter_exo(device, adv) -> bool:
        try:
            uuids = set((adv.service_uuids or []))
            return UART_SERVICE_UUID.lower() in {u.lower() for u in uuids}
        except Exception:
            return False

    # Removed invalid get_char_handle; bleak accepts UUIDs directly
