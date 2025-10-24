import asyncio
import os
import sys
import threading

from typing import List, Tuple

try:
    from PySide6 import QtCore, QtWidgets
except ImportError as e:
    raise SystemExit("PySide6 is required. Install with: pip install PySide6") from e


from services.QtExoDeviceManager import QtExoDeviceManager


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
    SETTINGS_FILE = os.path.join(os.path.dirname(os.path.dirname(__file__)), "Saved_Data", "saved_device.txt")

    def __init__(self):
        super().__init__()
        self.setWindowTitle("OpenExo - Scan (Qt)")
        self.resize(900, 600)

        self.selected_address: str | None = None
        self.selected_name: str | None = None

        self._build_ui()
        self._wire_workers()
        self._load_saved_device()

    def _build_ui(self):
        layout = QtWidgets.QVBoxLayout(self)
        layout.setContentsMargins(16, 16, 16, 16)
        layout.setSpacing(12)

        self.title = QtWidgets.QLabel("OpenExo GUI - Qt Scan")
        self.title.setAlignment(QtCore.Qt.AlignCenter)
        font = self.title.font()
        font.setPointSize(22)
        self.title.setFont(font)
        layout.addWidget(self.title)

        self.status = QtWidgets.QLabel("Not Connected")
        self.status.setAlignment(QtCore.Qt.AlignCenter)
        f2 = self.status.font(); f2.setPointSize(14); self.status.setFont(f2)
        layout.addWidget(self.status)

        # Button row
        btn_row = QtWidgets.QHBoxLayout()
        self.btn_scan = QtWidgets.QPushButton("Start Scan")
        self.btn_load = QtWidgets.QPushButton("Load Saved Device")
        btn_row.addWidget(self.btn_scan)
        btn_row.addWidget(self.btn_load)
        layout.addLayout(btn_row)

        # Devices list
        self.list_devices = QtWidgets.QListWidget()
        lf = self.list_devices.font(); lf.setPointSize(16); self.list_devices.setFont(lf)
        self.list_devices.setStyleSheet("QListWidget::item{ height: 44px; padding: 6px 10px; }")
        layout.addWidget(self.list_devices, 1)

        # Action row (Save & Connect on left, Start Trial next)
        action_row = QtWidgets.QHBoxLayout()
        self.btn_save_connect = QtWidgets.QPushButton("Save & Connect")
        self.btn_start_trial = QtWidgets.QPushButton("Start Trial")
        self.btn_start_trial.setEnabled(False)
        action_row.addWidget(self.btn_save_connect)
        action_row.addWidget(self.btn_start_trial)
        layout.addLayout(action_row)

        # Signals
        self.btn_scan.clicked.connect(self.on_scan)
        self.btn_load.clicked.connect(self.on_load_saved)
        self.btn_save_connect.clicked.connect(self.on_save_and_connect)
        self.list_devices.itemSelectionChanged.connect(self.on_selected)

        # Touch-friendly styling for iPad
        def _style(btn: QtWidgets.QPushButton):
            fb = btn.font(); fb.setPointSize(16); btn.setFont(fb)
            btn.setMinimumHeight(56)
            btn.setMinimumWidth(220)
            btn.setStyleSheet("padding: 12px 20px;")

        for b in (self.btn_scan, self.btn_load, self.btn_save_connect, self.btn_start_trial):
            _style(b)

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
        self.status.setText("Scanning…")
        self.list_devices.clear()
        self.btn_scan.setEnabled(False)
        self.btn_save_connect.setEnabled(False)
        if self._scanner is None:
            self.status.setText("Scanner not ready")
            self.btn_scan.setEnabled(True)
            return
        self._scanner.scan_once()

    @QtCore.Slot()
    def on_load_saved(self):
        if os.path.exists(self.SETTINGS_FILE):
            try:
                with open(self.SETTINGS_FILE, "r") as f:
                    addr = f.read().strip()
                if addr:
                    self.selected_address = addr
                    self.status.setText(f"Loaded saved device: {addr}")
                    self.btn_save_connect.setEnabled(True)
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
        self.btn_save_connect.setEnabled(False)
        # Emit request for Qt device manager to handle connection
        self.connectRequested.emit(self.selected_address)

    # Called by MainWindow after it creates QtExoDeviceManager
    def bind_device_manager(self, qt_dev: QtExoDeviceManager):
        self._scanner = DeviceScannerWorker(qt_dev)
        self._scanner.resultsReady.connect(self._on_scan_results)
        self._scanner.error.connect(self._on_error)

    # Worker callbacks
    @QtCore.Slot(list)
    def _on_scan_results(self, results: List[Tuple[str, str]]):
        self.btn_scan.setEnabled(True)
        self.list_devices.clear()
        if not results:
            self.status.setText("No devices found")
            return
        self.status.setText("Scan complete")
        for name, addr in results:
            item = QtWidgets.QListWidgetItem(f"{name} - {addr}")
            item.setData(QtCore.Qt.UserRole, (name, addr))
            self.list_devices.addItem(item)

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
        self.status.setText(message)

    @QtCore.Slot(str)
    def _on_connect(self, message: str):
        self.status.setText(message)

def main():
    app = QtWidgets.QApplication(sys.argv)
    w = ScanWindowQt()
    w.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()


