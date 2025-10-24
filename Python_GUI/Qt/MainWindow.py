import sys
import csv
import time
import os
from datetime import datetime

try:
    from PySide6 import QtCore, QtWidgets, QtGui
except ImportError as e:
    raise SystemExit("PySide6 is required. Install with: pip install PySide6") from e

from pages.ScanPage import ScanWindowQt
from pages.ActiveTrialPage import ActiveTrialPage
from pages.ActiveTrialSettingsPage import ActiveTrialSettingsPage
from pages.ActiveTrialBasicSettingsPage import ActiveTrialBasicSettingsPage
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
        self.settings_page = ActiveTrialSettingsPage()
        self.basic_settings_page = ActiveTrialBasicSettingsPage()

        self.stack.addWidget(self.scan_page)
        self.stack.addWidget(self.trial_page)
        self.stack.addWidget(self.settings_page)
        self.stack.addWidget(self.basic_settings_page)

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
        # Hide manual navigation to prevent tab-like selection
        toolbar.setVisible(False)

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
        # Receive flattened 2D matrix of controllers and parameters
        self.rt_bridge.controllerMatrixReceived.connect(self._on_controller_matrix)

        # CSV logging state
        self._csv_file = None
        self._csv_writer = None
        self._csv_header_written = False
        self._param_names = []
        self._t0 = None
        self._csv_path_last = None
        # Store controller -> params 2D matrix
        self._controller_matrix = []

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

        # Settings page wiring
        self.settings_page.applyRequested.connect(self._on_apply_settings)
        self.settings_page.cancelRequested.connect(lambda: self.stack.setCurrentWidget(self.trial_page))
        self.basic_settings_page.applyRequested.connect(self._on_apply_settings)
        self.basic_settings_page.cancelRequested.connect(lambda: self.stack.setCurrentWidget(self.trial_page))

    def _go_trial(self):
        self.stack.setCurrentWidget(self.trial_page)
        # Stop simulation so live data drives plots if available
        self.trial_page.stop_sim()
        # Ensure CSV logging is started automatically with timestamped filename
        try:
            if self._csv_file is None:
                self._start_csv_auto()
        except Exception:
            pass
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
            # CSV logging
            if self._csv_writer is not None:
                if not self._csv_header_written:
                    header = ["t"]
                    if self._param_names:
                        header.extend(self._param_names[:len(values)])
                    else:
                        header.extend([f"data{i}" for i in range(len(values))])
                    try:
                        self._csv_writer.writerow(header)
                        self._csv_header_written = True
                        if self._t0 is None:
                            self._t0 = time.time()
                    except Exception:
                        pass
                # Write row
                t = 0.0 if self._t0 is None else (time.time() - self._t0)
                row = [f"{t:.3f}"] + [f"{v:.6f}" for v in values]
                try:
                    self._csv_writer.writerow(row)
                except Exception:
                    pass
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
        # Store for CSV header
        try:
            self._param_names = list(names)
        except Exception:
            self._param_names = []
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

    @QtCore.Slot(list)
    def _on_controller_matrix(self, matrix):
        # Save the 2D [ [controller, p1, p2], ... ] structure in memory
        try:
            self._controller_matrix = list(matrix)
        except Exception:
            self._controller_matrix = []

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
            # Stop CSV if running
            if self._csv_file is not None:
                try:
                    self._csv_file.flush(); self._csv_file.close()
                except Exception:
                    pass
                self._csv_file = None
                self._csv_writer = None
                self._csv_header_written = False
                self._t0 = None
                try:
                    if self._csv_path_last:
                        self.scan_page.status.setText(f"CSV saved: {self._csv_path_last}")
                except Exception:
                    pass
        except Exception:
            pass

    @QtCore.Slot()
    def _on_disconnect(self):
        try:
            self.qt_dev.disconnect()
            # Stop CSV if running
            if self._csv_file is not None:
                try:
                    self._csv_file.flush(); self._csv_file.close()
                except Exception:
                    pass
                self._csv_file = None
                self._csv_writer = None
                self._csv_header_written = False
                self._t0 = None
                try:
                    if self._csv_path_last:
                        self.scan_page.status.setText(f"CSV saved: {self._csv_path_last}")
                except Exception:
                    pass
        except Exception:
            pass

    @QtCore.Slot()
    def _on_save_csv(self):
        try:
            if self._csv_file is None:
                # Start logging
                fname, _ = QtWidgets.QFileDialog.getSaveFileName(self, "Save CSV", "", "CSV Files (*.csv)")
                if not fname:
                    return
                try:
                    self._csv_file = open(fname, "w", newline="")
                    self._csv_writer = csv.writer(self._csv_file)
                    self._csv_header_written = False
                    self._t0 = None
                    try:
                        self.scan_page.status.setText(f"Logging to {fname}")
                    except Exception:
                        pass
                except Exception:
                    self._csv_file = None
                    self._csv_writer = None
            else:
                # Stop logging
                try:
                    self._csv_file.flush(); self._csv_file.close()
                except Exception:
                    pass
                self._csv_file = None
                self._csv_writer = None
                self._csv_header_written = False
                self._t0 = None
                try:
                    self.scan_page.status.setText("CSV logging stopped")
                except Exception:
                    pass
        except Exception:
            pass

    @QtCore.Slot()
    def _on_update_controller(self):
        # Choose page based on whether we have controller metadata (handshake + matrix)
        has_matrix = bool(self._controller_matrix)
        if has_matrix:
            try:
                self.settings_page.set_controller_matrix(self._controller_matrix)
            except Exception:
                pass
            self.stack.setCurrentWidget(self.settings_page)
        else:
            self.stack.setCurrentWidget(self.basic_settings_page)

    @QtCore.Slot()
    def _on_bio_feedback(self):
        # Placeholder hook
        pass

    @QtCore.Slot()
    def _on_machine_learning(self):
        # Placeholder hook
        pass

    @QtCore.Slot(list)
    def _on_apply_settings(self, payload):
        # payload: [isBilateral, joint, controller, parameter, value]
        try:
            self.qt_dev.updateTorqueValues(payload)
        except Exception:
            pass
        # Return to trial page
        try:
            self.stack.setCurrentWidget(self.trial_page)
        except Exception:
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
            self.qt_dev.motorOff()
            self.qt_dev.stopTrial()
            # Popup dialog informing user
            try:
                QtWidgets.QMessageBox.information(self, "Device Disconnected", "The device has been disconnected.")
            except Exception:
                pass
            # Navigate back to the Scan page on disconnect
            self.stack.setCurrentWidget(self.scan_page)
            # Ensure CSV is closed and announce saved path
            if self._csv_file is not None:
                try:
                    self._csv_file.flush(); self._csv_file.close()
                except Exception:
                    pass
                self._csv_file = None
                self._csv_writer = None
                self._csv_header_written = False
                self._t0 = None
                try:
                    if self._csv_path_last:
                        self.scan_page.status.setText(f"CSV saved: {self._csv_path_last}")
                except Exception:
                    pass
        except Exception:
            pass

    def _start_csv_auto(self):
        base_dir = os.path.dirname(os.path.dirname(__file__))  # Python_GUI
        save_dir = os.path.join(base_dir, "Saved_Data")
        try:
            os.makedirs(save_dir, exist_ok=True)
        except Exception:
            pass
        ts = datetime.now().strftime("%Y%m%d_%H%M%S")
        fname = os.path.join(save_dir, f"trial_{ts}.csv")
        try:
            self._csv_file = open(fname, "w", newline="")
            self._csv_writer = csv.writer(self._csv_file)
            self._csv_header_written = False
            self._t0 = None
            self._csv_path_last = fname
            try:
                self.scan_page.status.setText(f"Logging to {fname}")
            except Exception:
                pass
        except Exception:
            self._csv_file = None
            self._csv_writer = None


