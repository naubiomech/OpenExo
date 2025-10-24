import sys

try:
    from PySide6 import QtCore, QtWidgets, QtGui
except ImportError as e:
    raise SystemExit("PySide6 is required. Install with: pip install PySide6") from e

from pages.ScanPage import ScanWindowQt
from pages.ActiveTrialPage import ActiveTrialPage
from services.QtExoDeviceManager import QtExoDeviceManager
from services.RtBridge import RtBridge


class MainWindow(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("OpenExo - Qt")
        self.resize(1000, 700)

        self.stack = QtWidgets.QStackedWidget()
        self.setCentralWidget(self.stack)

        # Pages
        self.scan_page = ScanWindowQt()
        self.trial_page = ActiveTrialPage()

        self.stack.addWidget(self.scan_page)
        self.stack.addWidget(self.trial_page)

        # Simple top bar navigation
        toolbar = self.addToolBar("Nav")
        act_scan = QtGui.QAction("Scan", self)
        act_trial = QtGui.QAction("Active Trial", self)
        act_disc = QtGui.QAction("Disconnect", self)
        toolbar.addAction(act_scan)
        toolbar.addAction(act_trial)
        toolbar.addAction(act_disc)
        act_scan.triggered.connect(lambda: self.stack.setCurrentWidget(self.scan_page))
        act_trial.triggered.connect(lambda: self.stack.setCurrentWidget(self.trial_page))
        act_disc.triggered.connect(self._on_disconnect)

        # When scan page connects, enable trial start
        self.scan_page.btn_start_trial.clicked.connect(self._go_trial)
        self.scan_page.connectRequested.connect(self._on_connect_requested)

        # Services
        self.qt_dev = QtExoDeviceManager(self)
        # Bind scan page scanner to use the Qt device manager for scanning
        try:
            self.scan_page.bind_device_manager(self.qt_dev)
        except Exception:
            pass
        self.rt_bridge = RtBridge(self)
        # Wire bytes to parser and route RT data to plots
        self.qt_dev.dataReceived.connect(self.rt_bridge.feed_bytes)
        self.rt_bridge.rtDataUpdated.connect(self._on_rt_update)
        self.rt_bridge.handshakeReceived.connect(self._on_handshake)
        self.rt_bridge.parameterNamesReceived.connect(self._on_param_names)
        self.rt_bridge.controllersReceived.connect(self._on_controllers)

        # Device control wiring from ActiveTrialPage
        self.trial_page.deviceStartRequested.connect(self._on_device_start)
        self.trial_page.deviceStopRequested.connect(self._on_device_stop)
        self.trial_page.recalibrateFSRRequested.connect(self._on_recal_fsr)
        self.trial_page.sendPresetFSRRequested.connect(self._on_send_preset_fsr)
        self.trial_page.markTrialRequested.connect(self._on_mark)
        self.trial_page.endTrialRequested.connect(self._on_end_trial)
        self.trial_page.saveCsvRequested.connect(self._on_save_csv)
        self.trial_page.updateControllerRequested.connect(self._on_update_controller)
        self.trial_page.bioFeedbackRequested.connect(self._on_bio_feedback)
        self.trial_page.machineLearningRequested.connect(self._on_machine_learning)
        # Update Scan page status from device manager
        self.qt_dev.log.connect(self._on_dev_log)
        self.qt_dev.error.connect(self._on_dev_error)
        self.qt_dev.connected.connect(self._on_dev_connected)
        self.qt_dev.disconnected.connect(self._on_dev_disconnected)

    def _go_trial(self):
        self.stack.setCurrentWidget(self.trial_page)
        # Stop simulation so live data drives plots if available
        self.trial_page.stop_sim()
        # Begin trial sequence (E -> L -> R + thresholds) to ensure FSRs stream
        try:
            self.qt_dev.beginTrial()
        except Exception:
            pass

    @QtCore.Slot(str)
    def _on_connect_requested(self, mac: str):
        # Start BLE connection via Qt device manager
        self.qt_dev.set_mac(mac)
        self.qt_dev.connect()

    @QtCore.Slot(list)
    def _on_rt_update(self, values):
        # Map first four channels to two plots
        try:
            page = self.trial_page
            page.apply_values(values)
        except Exception:
            pass

    @QtCore.Slot()
    def _on_handshake(self):
        # Acknowledge handshake so firmware can continue sending names/data
        try:
            self.scan_page.status.setText("Handshake received; sending ACK…")
        except Exception:
            pass
        try:
            # '$' is the ACK byte used in the legacy protocol
            self.qt_dev.write(b'$')
        except Exception:
            pass

    @QtCore.Slot(list)
    def _on_param_names(self, names):
        # After receiving parameter names list (ended by END), send ACK so firmware continues (controllers or data)
        try:
            self.scan_page.status.setText(f"Received {len(names)} param names; sending ACK…")
        except Exception:
            pass
        try:
            self.qt_dev.write(b'$')
        except Exception:
            pass

    @QtCore.Slot(list, list)
    def _on_controllers(self, controllers, controller_params):
        # After receiving controllers/params (!… !END), send ACK and optionally start streaming
        try:
            self.scan_page.status.setText(f"Received {len(controllers)} controllers; sending ACK…")
        except Exception:
            pass
        try:
            self.qt_dev.write(b'$')
        except Exception:
            pass

    @QtCore.Slot()
    def _on_device_start(self):
        # Begin trial sequence (ensure FSR calibration and presets)
        try:
            self.qt_dev.beginTrial()
            # Also stop local sim
            self.trial_page.stop_sim()
        except Exception:
            pass

    @QtCore.Slot()
    def _on_device_stop(self):
        # Stop device streaming ('G')
        try:
            self.qt_dev.write(b'G')
        except Exception:
            pass

    @QtCore.Slot()
    def _on_recal_fsr(self):
        try:
            self.qt_dev.calibrateFSRs()
        except Exception:
            pass

    @QtCore.Slot()
    def _on_send_preset_fsr(self):
        try:
            self.qt_dev.sendPresetFsrValues()
        except Exception:
            pass

    @QtCore.Slot()
    def _on_mark(self):
        pass

    @QtCore.Slot()
    def _on_end_trial(self):
        try:
            self.qt_dev.motorOff()
            self.qt_dev.stopTrial()
            self.qt_dev.disconnect()
        except Exception:
            pass

    @QtCore.Slot()
    def _on_disconnect(self):
        try:
            self.qt_dev.disconnect()
        except Exception:
            pass

    @QtCore.Slot()
    def _on_save_csv(self):
        # Placeholder: integrate with data logging pipeline if available
        pass

    @QtCore.Slot()
    def _on_update_controller(self):
        # Placeholder: open/update controller parameters UI
        pass

    @QtCore.Slot()
    def _on_bio_feedback(self):
        # Placeholder hook
        pass

    @QtCore.Slot()
    def _on_machine_learning(self):
        # Placeholder hook
        pass

    @QtCore.Slot(str)
    def _on_dev_log(self, msg: str):
        try:
            self.scan_page.status.setText(msg)
        except Exception:
            pass

    @QtCore.Slot(str)
    def _on_dev_error(self, msg: str):
        try:
            self.scan_page.status.setText(f"Connection failed: {msg}")
            self.scan_page.btn_save_connect.setEnabled(True)
            self.scan_page.btn_start_trial.setEnabled(False)
        except Exception:
            pass

    @QtCore.Slot(str, str)
    def _on_dev_connected(self, name: str, addr: str):
        try:
            self.scan_page.status.setText(f"Connected: {name} {addr}")
            self.scan_page.btn_save_connect.setEnabled(True)
            self.scan_page.btn_start_trial.setEnabled(True)
        except Exception:
            pass

    @QtCore.Slot()
    def _on_dev_disconnected(self):
        try:
            self.scan_page.status.setText("Disconnected")
            self.scan_page.btn_save_connect.setEnabled(True)
            self.scan_page.btn_start_trial.setEnabled(False)
        except Exception:
            pass


