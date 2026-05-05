import asyncio
import os
import sys
import threading

from typing import List, Tuple

try:
    from PySide6 import QtCore, QtWidgets
except ImportError as e:
    raise SystemExit("PySide6 is required. Install with: pip install PySide6") from e

from services import QtExoDeviceManager
from utils import UIConfig, load_logo, style_button, apply_button_style_batch


UART_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"  # Nordic UART Service


class DeviceScannerWorker(QtCore.QObject):
    resultsReady = QtCore.Signal(list)  # List[Tuple[str, str]] (name, address)
    error = QtCore.Signal(str)

    def __init__(self, qt_dev: QtExoDeviceManager):
        super().__init__()
        self._qt_dev = qt_dev
        self._qt_dev.scanResults.connect(self._forward)
        self._qt_dev.error.connect(self.error)

    @QtCore.Slot()
    def scan_once(self):
        self._qt_dev.scan()

    @QtCore.Slot(list)
    def _forward(self, results):
        self.resultsReady.emit(results)


class ScanWindowQt(QtWidgets.QWidget):
    # Signal to request a connection via the Qt device manager (address)
    connectRequested = QtCore.Signal(str)

    def __init__(self):
        super().__init__()
        self.setWindowTitle("OpenExo - Scan (Qt)")
        # Compact default size (resizable)
        self.setMinimumSize(UIConfig.WINDOW_MIN_WIDTH, UIConfig.WINDOW_MIN_HEIGHT)
        self.resize(UIConfig.WINDOW_DEFAULT_WIDTH, UIConfig.WINDOW_DEFAULT_HEIGHT)
        
        #last progress to prevent backwards movement
        self._last_connection_progress = 0
        
        # Settings file path
        base_dir = os.path.dirname(os.path.dirname(__file__))
        self.SETTINGS_FILE = os.path.join(base_dir, "Saved_Data", "saved_device.txt")

        self.selected_address: str | None = None
        self.selected_name: str | None = None
        self._qt_dev: QtExoDeviceManager | None = None
        self._scanner: DeviceScannerWorker | None = None
        self._connected = False
        self._pending_scan = False

        self._build_ui()
        self._wire_workers()
        self._load_saved_device()

    def _build_ui(self):
        layout = QtWidgets.QVBoxLayout(self)
        layout.setContentsMargins(UIConfig.MARGIN_PAGE, 0, UIConfig.MARGIN_PAGE, 3)
        layout.setSpacing(0)  # Manual spacing control
        
        # Add spacing at the top
        layout.addSpacing(UIConfig.SPACING_SECTION)

        # Header row with logos and centered title
        header_row = QtWidgets.QHBoxLayout()
        header_row.setContentsMargins(0, 0, 0, 0)
        header_row.setSpacing(UIConfig.SPACING_SMALL)
        
        # Add OpenExo logo at left
        openexo_logo = load_logo(
            "OpenExo.png",
            UIConfig.LOGO_OPENEXO_WIDTH,
            UIConfig.LOGO_OPENEXO_HEIGHT,
            QtCore.Qt.AlignLeft | QtCore.Qt.AlignVCenter
        )
        if openexo_logo:
            header_row.addWidget(openexo_logo)
        
        # Add centered title
        self.title = QtWidgets.QLabel("OpenExo GUI - Qt Scan")
        self.title.setAlignment(QtCore.Qt.AlignCenter)
        self.title.setContentsMargins(0, 0, 0, 0)
        font = self.title.font()
        font.setPointSize(UIConfig.FONT_TITLE_LARGE)
        self.title.setFont(font)
        header_row.addWidget(self.title, 1)  # Stretch factor to center
        
        # Add Lab logo at right
        lab_logo = load_logo(
            "LabLogo.png",
            UIConfig.LOGO_LAB_WIDTH,
            UIConfig.LOGO_LAB_HEIGHT,
            QtCore.Qt.AlignRight | QtCore.Qt.AlignVCenter
        )
        if lab_logo:
            header_row.addWidget(lab_logo)
        
        layout.addLayout(header_row)
        layout.addSpacing(UIConfig.SPACING_HEADER)  # Spacing between header and status

        self.status = QtWidgets.QLabel("Not Connected")
        self.status.setAlignment(QtCore.Qt.AlignCenter)
        self.status.setContentsMargins(0, 0, 0, 0)
        f2 = self.status.font(); f2.setPointSize(UIConfig.FONT_MEDIUM); self.status.setFont(f2)
        layout.addWidget(self.status)
        layout.addSpacing(UIConfig.MARGIN_PAGE)  # Spacing between status and buttons

        # Button row
        btn_row = QtWidgets.QHBoxLayout()
        btn_row.setSpacing(UIConfig.SPACING_MEDIUM)
        btn_row.setContentsMargins(0, 0, 0, 0)
        self.btn_scan = QtWidgets.QPushButton("1. Start Scan")
        self.btn_load = QtWidgets.QPushButton("Load Saved Device")
        btn_row.addWidget(self.btn_scan)
        btn_row.addWidget(self.btn_load)
        layout.addLayout(btn_row)
        layout.addSpacing(UIConfig.SPACING_TINY)

        # Progress bar for scanning
        self.scan_progress = QtWidgets.QProgressBar()
        self.scan_progress.setRange(0, 100)
        self.scan_progress.setValue(0)
        self.scan_progress.setTextVisible(True)
        self.scan_progress.setFormat("Scanning... %p%")
        self.scan_progress.setMinimumHeight(UIConfig.BTN_HEIGHT_SMALL)
        self.scan_progress.setStyleSheet("""
            QProgressBar {
                border: 2px solid #555;
                border-radius: 5px;
                text-align: center;
                background-color: #2b2b2b;
                color: white;
                font-size: 14px;
            }
            QProgressBar::chunk {
                background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                                   stop:0 #4a9eff, stop:1 #2d7dd2);
                border-radius: 3px;
            }
        """)
        self.scan_progress.setVisible(False)  # Hidden by default
        layout.addWidget(self.scan_progress)
        layout.addSpacing(UIConfig.SPACING_TINY)

        # Progress bar for scanning during connection (looking for device)
        self.connect_scan_progress = QtWidgets.QProgressBar()
        self.connect_scan_progress.setRange(0, 100)
        self.connect_scan_progress.setValue(0)
        self.connect_scan_progress.setTextVisible(True)
        self.connect_scan_progress.setFormat("Looking for device... %p%")
        self.connect_scan_progress.setMinimumHeight(UIConfig.BTN_HEIGHT_SMALL)
        self.connect_scan_progress.setStyleSheet("""
            QProgressBar {
                border: 2px solid #555;
                border-radius: 5px;
                text-align: center;
                background-color: #2b2b2b;
                color: white;
                font-size: 14px;
            }
            QProgressBar::chunk {
                background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                                   stop:0 #f59e0b, stop:1 #d97706);
                border-radius: 3px;
            }
        """)
        self.connect_scan_progress.setVisible(False)  # Hidden by default
        layout.addWidget(self.connect_scan_progress)
        layout.addSpacing(UIConfig.SPACING_TINY)

        # Progress bar for connecting (BLE connection phase)
        self.connection_progress = QtWidgets.QProgressBar()
        self.connection_progress.setRange(0, 100)
        self.connection_progress.setValue(0)
        self.connection_progress.setTextVisible(True)
        self.connection_progress.setFormat("Connecting... %p%")
        self.connection_progress.setMinimumHeight(UIConfig.BTN_HEIGHT_SMALL)
        self.connection_progress.setStyleSheet("""
            QProgressBar {
                border: 2px solid #555;
                border-radius: 5px;
                text-align: center;
                background-color: #2b2b2b;
                color: white;
                font-size: 14px;
            }
            QProgressBar::chunk {
                background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                                   stop:0 #4ade80, stop:1 #22c55e);
                border-radius: 3px;
            }
        """)
        self.connection_progress.setVisible(False)  # Hidden by default
        layout.addWidget(self.connection_progress)
        layout.addSpacing(UIConfig.SPACING_TINY)

        # Devices list - fills space between top and bottom buttons
        self.list_devices = QtWidgets.QListWidget()
        self.list_devices.setSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Expanding)
        lf = self.list_devices.font(); lf.setPointSize(UIConfig.FONT_MEDIUM); self.list_devices.setFont(lf)
        self.list_devices.setStyleSheet(f"QListWidget::item{{ height: {UIConfig.LIST_ITEM_HEIGHT}px; padding: 3px 6px; }}")
        self.list_devices.setMinimumHeight(UIConfig.LIST_DEVICE_MIN_HEIGHT)
        layout.addWidget(self.list_devices, 1)  # Stretch factor 1 to fill space
        
        layout.addSpacing(5)

        # Action row (Connect, Calibrate Torque, Start Trial) - at bottom
        action_row = QtWidgets.QHBoxLayout()
        action_row.setSpacing(UIConfig.SPACING_MEDIUM)
        action_row.setContentsMargins(0, 0, 0, 0)
        self.btn_save_connect = QtWidgets.QPushButton("2. Connect")
        self.btn_save_connect.setEnabled(False)  # Disabled initially
        self.btn_start_trial = QtWidgets.QPushButton("4. Start Trial")
        self.btn_start_trial.setEnabled(False)
        self.btn_calibrate_torque = QtWidgets.QPushButton("3. Calibrate Torque")
        self.btn_calibrate_torque.setEnabled(False)
        action_row.addWidget(self.btn_save_connect)
        action_row.addWidget(self.btn_calibrate_torque)
        action_row.addWidget(self.btn_start_trial)
        layout.addLayout(action_row)
        
        # Add spacing at the bottom
        layout.addSpacing(UIConfig.SPACING_SECTION)

        # Signals
        self.btn_scan.clicked.connect(self.on_scan)
        self.btn_load.clicked.connect(self.on_load_saved)
        self.btn_save_connect.clicked.connect(self.on_save_and_connect)
        self.btn_calibrate_torque.clicked.connect(self.on_calibrate_torque)
        self.list_devices.itemSelectionChanged.connect(self.on_selected)

        # Apply consistent button styling
        buttons = [self.btn_scan, self.btn_load, self.btn_save_connect, self.btn_start_trial, self.btn_calibrate_torque]
        apply_button_style_batch(
            buttons,
            height=UIConfig.BTN_HEIGHT_MEDIUM,
            width=UIConfig.BTN_WIDTH_SMALL,
            font_size=UIConfig.FONT_MEDIUM,
            padding="6px 12px"
        )
        # Lock max height for compact layout
        for btn in buttons:
            btn.setMaximumHeight(UIConfig.BTN_HEIGHT_MEDIUM)

    def _wire_workers(self):
        # Use the shared QtExoDeviceManager instance provided by MainWindow after construction
        # We lazy-bind scanner to the device manager via bind_device_manager()
        self._scanner: DeviceScannerWorker | None = None
        # Connections are made in bind_device_manager()

    def _load_saved_device(self):
        try:
            if os.path.exists(self.SETTINGS_FILE):
                with open(self.SETTINGS_FILE, "r") as f:
                    addr = f.read().strip()
                if addr:
                    self.status.setText(f"Saved device available: {addr}")
                    # Do not auto-enable connect until user selects or loads
                    return
        except Exception:
            pass

    # UI handlers
    @QtCore.Slot()
    def on_scan(self):
        if self._scanner is None:
            self.status.setText("Scanner not ready")
            return
        self.btn_scan.setEnabled(False)
        self.btn_load.setEnabled(False)
        self.btn_save_connect.setEnabled(False)
        self.list_devices.clear()
        self.scan_progress.setValue(0)
        self.scan_progress.setVisible(True)
        self._pending_scan = True
        if self._qt_dev is not None and self._connected:
            try:
                #Disconnect immediately
                self._qt_dev.disconnect()
                self._connected = False
            except Exception:
                self._connected = False
        
        # Start scan immediately without waiting for disconnect
        self._start_scan_now()

    @QtCore.Slot()
    def on_load_saved(self):
        if os.path.exists(self.SETTINGS_FILE):
            try:
                with open(self.SETTINGS_FILE, "r") as f:
                    addr = f.read().strip()
                if addr:
                    self.selected_address = addr
                    self.status.setText(f"Connecting to saved device: {addr}")
                    # Reset progress and show only scanning bar initially
                    self._last_connection_progress = 0
                    self.connect_scan_progress.setValue(0)
                    self.connect_scan_progress.setVisible(True)
                    self.connection_progress.setValue(0)
                    self.connection_progress.setVisible(False)  # Hidden until device found
                    # Disable Load and Save & Connect buttons during connection
                    # Keep Start Scan enabled so user can disconnect and start new scan
                    self.btn_load.setEnabled(False)
                    self.btn_save_connect.setEnabled(False)
                    # Auto-connect to saved device
                    self.connectRequested.emit(addr)
                    return
            except Exception:
                pass
        self.status.setText("No saved device found")

    @QtCore.Slot()
    def on_selected(self):
        items = self.list_devices.selectedItems()
        if items:
            item = items[0]
            self.selected_name = item.data(QtCore.Qt.UserRole)[0]
            self.selected_address = item.data(QtCore.Qt.UserRole)[1]
            self.btn_save_connect.setEnabled(True)
        else:
            self.selected_name = None
            self.selected_address = None
            self.btn_save_connect.setEnabled(False)

    @QtCore.Slot()
    def on_save_and_connect(self):
        if not self.selected_address:
            self.status.setText("Select a device first")
            return
        # Save selection
        try:
            os.makedirs(os.path.dirname(self.SETTINGS_FILE), exist_ok=True)
            with open(self.SETTINGS_FILE, "w") as f:
                f.write(self.selected_address)
        except Exception as ex:
            self.status.setText(f"Save failed: {ex}")
            return

        self.status.setText(f"Connecting to: {self.selected_name or ''} {self.selected_address}")
        # Reset progress and show only scanning bar initially
        self._last_connection_progress = 0
        self.connect_scan_progress.setValue(0)
        self.connect_scan_progress.setVisible(True)
        self.connection_progress.setValue(0)
        self.connection_progress.setVisible(False)  # Hidden until device found
        self.btn_save_connect.setEnabled(False)
        # Emit request for Qt device manager to handle connection
        self.connectRequested.emit(self.selected_address)

    @QtCore.Slot()
    def on_calibrate_torque(self):
        if self._qt_dev is None:
            self.status.setText("Device manager not ready")
            return
        try:
            # Send torque calibration command
            self._qt_dev.calibrateTorque()

            # Keep Start Trial disabled for a short delay to allow calibration to settle
            self.btn_start_trial.setEnabled(False)

            self.status.setText("Torque calibration sent. Start Trial will be enabled in 3 seconds...")

            # Enable Start Trial after 3 seconds (only if still connected)
            def _enable_start_trial_if_connected():
                if self._connected and self._qt_dev is not None:
                    self.btn_start_trial.setEnabled(True)

            QtCore.QTimer.singleShot(3000, _enable_start_trial_if_connected)
        except Exception as ex:
            self.status.setText(f"Torque calibration failed: {ex}")

    # Called by MainWindow after it creates QtExoDeviceManager
    def bind_device_manager(self, qt_dev: QtExoDeviceManager):
        self._qt_dev = qt_dev
        self._scanner = DeviceScannerWorker(qt_dev)
        self._scanner.resultsReady.connect(self._on_scan_results)
        self._scanner.error.connect(self._on_error)
        qt_dev.connected.connect(self._on_device_connected)
        qt_dev.disconnected.connect(self._on_device_disconnected)
        qt_dev.scanProgress.connect(self._on_scan_progress)
        qt_dev.connectScanProgress.connect(self._on_connect_scan_progress)
        qt_dev.connectionProgress.connect(self._on_connection_progress)

    # Worker callbacks
    @QtCore.Slot(list)
    def _on_scan_results(self, results: List[Tuple[str, str]]):
        self._pending_scan = False
        self.btn_scan.setEnabled(True)
        self.scan_progress.setVisible(False)
        self.list_devices.clear()
        
        # Re-enable Load button if saved device exists
        if os.path.exists(self.SETTINGS_FILE):
            self.btn_load.setEnabled(True)
        
        # Continue with existing logic
        if os.path.exists(self.SETTINGS_FILE):
            try:
                with open(self.SETTINGS_FILE, "r") as f:
                    if f.read().strip():
                        self.btn_load.setEnabled(True)
            except Exception:
                pass
        
        if not results:
            self.status.setText("No devices found")
            return
        self.status.setText("Scan complete")
        for name, addr in results:
            item = QtWidgets.QListWidgetItem(f"{name} - {addr}")
            item.setData(QtCore.Qt.UserRole, (name, addr))
            self.list_devices.addItem(item)

    @QtCore.Slot(str, str)
    def _on_device_connected(self, name: str, address: str):
        self._connected = True
        self.connect_scan_progress.setVisible(False)
        self.connection_progress.setVisible(False)
        self.status.setText(f"Connected: {name} {address}")
        self.btn_start_trial.setEnabled(True)
        # Keep Save & Connect disabled after connection
        self.btn_save_connect.setEnabled(False)

    @QtCore.Slot()
    def _on_device_disconnected(self):
        self._connected = False
        if self._pending_scan:
            self._start_scan_now()

    def _start_scan_now(self):
        self.status.setText("Scanning…")
        if self._scanner is None:
            self.status.setText("Scanner not ready")
            self.btn_scan.setEnabled(True)
            self._pending_scan = False
            return
        self._scanner.scan_once()

    @QtCore.Slot(bool, str)
    def _on_connected(self, ok: bool, msg: str):
        self.btn_save_connect.setEnabled(True)
        if ok:
            self.status.setText(f"Connected: {self.selected_name or ''} {self.selected_address}")
            self.btn_start_trial.setEnabled(True)
        else:
            self.status.setText(f"Connection failed: {msg}")

    @QtCore.Slot(str)
    def _on_error(self, message: str):
        # A connect attempt failed (no device found, max attempts reached,
        # timeout, etc.).  The page must fall all the way back to the pre-load
        # state -- otherwise post-connect-only buttons (Calibrate Torque, Start
        # Trial) stay enabled from any prior session and the user can't tell
        # the link is actually down.
        self._reset_after_failed_connect(message)

    def _reset_after_failed_connect(self, message: str):
        """Restore the page to its disconnected state after a failed connect."""
        self.status.setText(message)
        # Hide every progress bar -- nothing is in flight anymore.
        self.scan_progress.setVisible(False)
        self.connect_scan_progress.setVisible(False)
        self.connection_progress.setVisible(False)
        self._last_connection_progress = 0
        # Forget any "we are connected" state from a previous session.
        self._connected = False
        # Buttons that only make sense after a real connection must be off.
        self.btn_start_trial.setEnabled(False)
        self.btn_calibrate_torque.setEnabled(False)
        # Re-enable the entry-point buttons so the user can try again.
        self.btn_scan.setEnabled(True)
        # Save & Connect needs a selection in the device list -- only enable it
        # if one is currently selected.
        self.btn_save_connect.setEnabled(bool(self.selected_address))
        if os.path.exists(self.SETTINGS_FILE):
            self.btn_load.setEnabled(True)

    @QtCore.Slot(str)
    def _on_connect(self, message: str):
        self.status.setText(message)

    @QtCore.Slot(int)
    def _on_scan_progress(self, progress: int):
        """Update the scan progress bar (for 'Start Scan' button)."""
        self.scan_progress.setValue(progress)
        if progress >= 100:
            # Hide progress bar when complete
            QtCore.QTimer.singleShot(500, lambda: self.scan_progress.setVisible(False))

    @QtCore.Slot(int)
    def _on_connect_scan_progress(self, progress: int):
        """Update the scanning phase progress during connection."""
        if progress == -1:
            # Signal to hide the scanning bar immediately
            self.connect_scan_progress.setVisible(False)
        else:
            self.connect_scan_progress.setValue(progress)
            if progress >= 100:
                # Hide when device found
                QtCore.QTimer.singleShot(200, lambda: self.connect_scan_progress.setVisible(False))

    @QtCore.Slot(int)
    def _on_connection_progress(self, progress: int):
        """Update the connection progress bar - only move forward, never backwards."""
        if progress == 0:
            # Reset on new connection attempt
            self._last_connection_progress = 0
            self.connection_progress.setValue(0)
            self.connection_progress.setVisible(False)
            # Don't hide - might be retrying
        elif progress > self._last_connection_progress:
            # Show connection bar when it starts (device found)
            if not self.connection_progress.isVisible():
                self.connection_progress.setVisible(True)
            # Only update if progress is moving forward
            self._last_connection_progress = progress
            self.connection_progress.setValue(progress)
            if progress >= 100:
                # Hide progress bar when complete
                QtCore.QTimer.singleShot(500, lambda: self._reset_connection_progress())
    
    def _reset_connection_progress(self):
        """Reset connection progress tracking."""
        self.connection_progress.setVisible(False)
        self.connect_scan_progress.setVisible(False)
        self._last_connection_progress = 0

def main():
    app = QtWidgets.QApplication(sys.argv)
    w = ScanWindowQt()
    w.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()


