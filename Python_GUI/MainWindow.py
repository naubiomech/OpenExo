import sys
import csv
import time
import os
from datetime import datetime

try:
    from PySide6 import QtCore, QtWidgets, QtGui
except ImportError as e:
    raise SystemExit("PySide6 is required. Install with: pip install PySide6") from e

from pages import (
    ScanWindowQt,
    ActiveTrialPage,
    ActiveTrialSettingsPage,
    ActiveTrialBasicSettingsPage,
    BioFeedbackPage,
)
from services import QtExoDeviceManager, RtBridge


class MainWindow(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("OpenExo - Qt")
        # Compact default window size (resizable)
        self.setMinimumSize(776, 400)
        self.resize(776, 400)

        self.stack = QtWidgets.QStackedWidget()
        self.setCentralWidget(self.stack)

        # Pages
        self.scan_page = ScanWindowQt()
        self.trial_page = ActiveTrialPage()
        self.settings_page = ActiveTrialSettingsPage()
        self.basic_settings_page = ActiveTrialBasicSettingsPage()
        self.bio_feedback_page = BioFeedbackPage()

        self.stack.addWidget(self.scan_page)
        self.stack.addWidget(self.trial_page)
        self.stack.addWidget(self.settings_page)
        self.stack.addWidget(self.basic_settings_page)
        self.stack.addWidget(self.bio_feedback_page)
#     self.stack.setCurrentWidget(self.trial_page)

        # Simple top bar navigation
        toolbar = self.addToolBar("Nav")
        act_scan = QtGui.QAction("Scan", self)
        act_trial = QtGui.QAction("Active Trial", self)
        act_disc = QtGui.QAction("Disconnect", self)
        toolbar.addAction(act_scan)
        toolbar.addAction(act_trial)
        toolbar.addAction(act_disc)
        act_scan.triggered.connect(self._navigate_to_scan)
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
        self._mark_counter = 0  # Trial mark counter
        self._csv_preamble = ""  # Preamble for CSV filename
        # Store controller -> params 2D matrix
        self._controller_matrix = []
        # Track intentional disconnect
        self._intentional_disconnect = False
        # Suppress unexpected-disconnect popup for a brief window after intentional actions (e.g., End Trial)
        self._suppress_disconnect_popup_until = 0.0
        # Device control wiring from ActiveTrialPage
        self.trial_page.deviceStartRequested.connect(self._on_device_start)
        self.trial_page.deviceStopRequested.connect(self._on_device_stop_motors)
        self.trial_page.csvPreambleChanged.connect(self._on_csv_preamble_changed)
        self.trial_page.recalibrateFSRRequested.connect(self._on_recal_fsr)
        self.trial_page.sendPresetFSRRequested.connect(self._on_send_preset_fsr)
        self.trial_page.recalibrateTorqueRequested.connect(self._on_recal_torque)
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
        self.bio_feedback_page.backRequested.connect(self._on_bio_feedback_back)
        self.bio_feedback_page.deviceStartRequested.connect(self._on_device_start)
        self.bio_feedback_page.deviceStopRequested.connect(self._on_device_stop_motors)
        self.bio_feedback_page.recalibrateFSRRequested.connect(self._on_recal_fsr)
        self.bio_feedback_page.markTrialRequested.connect(self._on_mark)

    def resizeEvent(self, event):
        super().resizeEvent(event)
        size = self.size()
        print(f"[MainWindow] size={size.width()}x{size.height()}")

    def _go_trial(self):
        self.stack.setCurrentWidget(self.trial_page)
        # Stop simulation so live data drives plots if available
        self.trial_page.stop_sim()
        # Clear old plot data
        try:
            self.trial_page.clear_plots()
        except Exception:
            pass
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
            try:
                if self.stack.currentWidget() == self.bio_feedback_page:
                    self.bio_feedback_page.apply_values(values)
            except Exception:
                pass
            # CSV logging
            if self._csv_writer is not None:
                if not self._csv_header_written:
                    header = ["epoch", "mark"]
                    # Only include first 10 parameters (exclude battery and beyond)
                    if self._param_names:
                        header.extend(self._param_names[:10])
                    else:
                        header.extend([f"data{i}" for i in range(min(10, len(values)))])
                    try:
                        self._csv_writer.writerow(header)
                        self._csv_header_written = True
                        if self._t0 is None:
                            self._t0 = time.time()
                    except Exception:
                        pass
                # Write row - only include first 10 data values
                epoch_time = time.time()
                data_values = values[:10] if len(values) > 10 else values
                row = [f"{epoch_time:.6f}", str(self._mark_counter)] + [f"{v:.6f}" for v in data_values]
                try:
                    self._csv_writer.writerow(row)
                except Exception:
                    pass
            
            # Update battery level (assuming it's in the data somewhere)
            try:
                if len(values) > 10:
                    battery_voltage = values[10]  # Battery is typically at index 10
                    self.trial_page.update_battery_level(battery_voltage)
            except Exception:
                pass
        except Exception:
            pass

    @QtCore.Slot(str)
    def _on_handshake(self, payload: str):
        try:
            print(f"MainWindow::_on_handshake -> Handshake payload received")
        except Exception:
            pass
        try:
            self.scan_page.status.setText("Handshake received; controller parameters incoming…")
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
            self.trial_page.set_channel_labels(self._param_names)
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

    @QtCore.Slot(list)
    def _on_controller_matrix(self, matrix):
        # Save the 2D [ [controller, p1, p2], ... ] structure in memory
        try:
            self._controller_matrix = list(matrix)
        except Exception:
            self._controller_matrix = []
        has_matrix = bool(self._controller_matrix)
        try:
            self.settings_page.set_controller_matrix(self._controller_matrix if has_matrix else [])
        except Exception:
            pass
        # Update Controller button is always enabled
        # If no matrix, it will show the legacy basic settings page
        try:
            self.trial_page.set_update_controller_enabled(True)
        except Exception:
            pass

    @QtCore.Slot()
    def _on_device_start(self):
        # Resume motors (play functionality) - just turn motors back on
        try:
            self.qt_dev.motorOn()
        except Exception:
            pass

    @QtCore.Slot()
    def _on_device_stop_motors(self):
        # Turn off motors (pause functionality)
        try:
            self.qt_dev.motorOff()
        except Exception:
            pass

    @QtCore.Slot(str)
    def _on_csv_preamble_changed(self, preamble: str):
        """Update CSV filename preamble."""
        self._csv_preamble = preamble
        print(f"CSV preamble set to: {preamble}")
         # If we're currently logging, roll over immediately (no popup)
        if self._csv_file is not None:
            try:
                self._csv_file.flush()
                self._csv_file.close()
            except Exception:
                pass

            # reset state (same reset you already do in _on_save_csv)
            self._csv_file = None
            self._csv_writer = None
            self._csv_header_written = False
            self._t0 = None
            self._csv_path_last = None

            # start a new CSV using the new prefix
            self._start_csv_auto()

            # optional: show a non-blocking confirmation somewhere
            try:
                self.trial_page.set_status_text(f"CSV prefix set. New file started: {self._csv_path_last}")
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
    def _on_recal_torque(self):
        try:
            self.qt_dev.calibrateTorque()
        except Exception:
            pass

    @QtCore.Slot()
    def _on_mark(self):
        # Increment trial mark counter
        self._mark_counter += 1
        try:
            self.scan_page.status.setText(f"Trial marked: {self._mark_counter}")
        except Exception:
            pass
        try:
            self.trial_page.update_mark_count(self._mark_counter)
        except Exception:
            pass
        try:
            self.bio_feedback_page.update_mark_count(self._mark_counter)
        except Exception:
            pass

    @QtCore.Slot()
    def _on_end_trial(self):
        try:
            # Mark as intentional disconnect
            self._intentional_disconnect = True
            
            # A disconnect may fire more than once (explicit disconnect + OS callback);
            # suppress the "unexpected disconnect" popup for a short window.
            self._suppress_disconnect_popup_until = time.monotonic() + 3.0

            # Navigate to scan page immediately
            self.stack.setCurrentWidget(self.scan_page)
            
            # Send stop trial and motor off commands, then disconnect
            try:
                self.qt_dev.write(b'G')  # Stop trial
                QtCore.QTimer.singleShot(100, lambda: self.qt_dev.write(b'w'))  # Motor off after delay
                QtCore.QTimer.singleShot(500, self.qt_dev.disconnect)  # Disconnect after commands sent
            except Exception:
                pass
            
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
                self._mark_counter = 0  # Reset mark counter
                try:
                    if self._csv_path_last:
                        self.scan_page.status.setText(f"Trial ended. CSV saved: {self._csv_path_last}")
                except Exception:
                    pass
                try:
                    self.trial_page.update_mark_count(0)
                except Exception:
                    pass
        except Exception:
            pass

    @QtCore.Slot()
    def _on_disconnect(self):
        try:
            # Mark as intentional disconnect
            self._intentional_disconnect = True
            
            self._suppress_disconnect_popup_until = time.monotonic() + 3.0

            # Navigate to scan page immediately
            self.stack.setCurrentWidget(self.scan_page)
            
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
                self._mark_counter = 0  # Reset mark counter
                try:
                    if self._csv_path_last:
                        self.scan_page.status.setText(f"Disconnected. CSV saved: {self._csv_path_last}")
                except Exception:
                    pass
                try:
                    self.trial_page.update_mark_count(0)
                except Exception:
                    pass
        except Exception:
            pass

    @QtCore.Slot()
    def _navigate_to_scan(self):
        try:
            self._on_disconnect()
        except Exception:
            pass
        self.stack.setCurrentWidget(self.scan_page)

    @QtCore.Slot()
    def _on_save_csv(self):
        """Save current CSV file and immediately start a new one."""
        try:
            saved_path = None
            # Close current CSV if open
            if self._csv_file is not None:
                print("Saving current CSV and starting new one")
                try:
                    self._csv_file.flush()
                    self._csv_file.close()
                    saved_path = self._csv_path_last
                except Exception:
                    pass
                self._csv_file = None
                self._csv_writer = None
                self._csv_header_written = False
                self._t0 = None
                self._csv_path_last = None
            
            # Start a new CSV file immediately
            self._start_csv_auto()
            
            # Show confirmation banner
            try:
                if saved_path:
                    save_dir = os.path.dirname(saved_path)
                    filename = os.path.basename(saved_path)
                    msg = f"✓ CSV saved: {filename}\nDirectory: {save_dir}"
                    self.scan_page.status.setText(msg)
                    # Also show a message box for better visibility
                    QtWidgets.QMessageBox.information(
                        self,
                        "CSV Saved",
                        f"CSV file saved successfully:\n{filename}\n\nLocation: {save_dir}"
                    )
                else:
                    self.scan_page.status.setText("New CSV logging started")
            except Exception as e:
                print(f"Error showing confirmation: {e}")
                pass
        except Exception as e:
            print(f"Error in _on_save_csv: {e}")
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
        try:
            self.bio_feedback_page.start_plotting()
        except Exception:
            pass
        self.stack.setCurrentWidget(self.bio_feedback_page)

    def _on_bio_feedback_back(self):
        try:
            self.bio_feedback_page.stop_plotting()
        except Exception:
            pass
        self.stack.setCurrentWidget(self.trial_page)

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
            try:
                self.scan_page.btn_calibrate_torque.setEnabled(False)
            except Exception:
                pass
        except Exception:
            pass

    @QtCore.Slot(str, str)
    def _on_dev_connected(self, name: str, addr: str):
        try:
            self.scan_page.status.setText(f"Connected: {name} {addr}")
            # Don't enable Save & Connect after connection - it's already connected
            self.scan_page.btn_save_connect.setEnabled(False)
            self.scan_page.btn_start_trial.setEnabled(False)  # Enabled after torque calibration
            try:
                self.scan_page.btn_calibrate_torque.setEnabled(True)
            except Exception:
                pass
        except Exception:
            pass
        # Clear old plot data on reconnect
        try:
            self.trial_page.clear_plots()
        except Exception:
            pass

    @QtCore.Slot()
    def _on_dev_disconnected(self):
        try:
            # Check if this was an intentional disconnect (or immediately followed an intentional action)
            now = time.monotonic()
            is_intentional = self._intentional_disconnect or (now < self._suppress_disconnect_popup_until)
            # Reset flag (but keep the time window so duplicate disconnect signals don't trigger a popup)
            self._intentional_disconnect = False

            
            self.scan_page.status.setText("Disconnected")
            self.scan_page.btn_save_connect.setEnabled(True)
            self.scan_page.btn_start_trial.setEnabled(False)
            try:
                self.scan_page.btn_calibrate_torque.setEnabled(False)
            except Exception:
                pass
            
            # Only show popup for unintentional disconnects
            if not is_intentional:
                try:
                    self.qt_dev.motorOff()
                    self.qt_dev.stopTrial()
                except Exception:
                    pass
                # Popup dialog informing user of unexpected disconnect
                try:
                    QtWidgets.QMessageBox.warning(self, "Device Disconnected", "The device has been unexpectedly disconnected.")
                except Exception:
                    pass
                # Navigate back to the Scan page on unexpected disconnect
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
                self._mark_counter = 0  # Reset mark counter
                try:
                    if self._csv_path_last and not is_intentional:
                        self.scan_page.status.setText(f"Unexpected disconnect. CSV saved: {self._csv_path_last}")
                except Exception:
                    pass
                try:
                    self.trial_page.update_mark_count(0)
                except Exception:
                    pass
        except Exception:
            pass

    def _start_csv_auto(self):
        # Save within Qt/Saved_Data
        base_dir = os.path.dirname(__file__)  # Python_GUI/Qt
        save_dir = os.path.join(base_dir, "Saved_Data")
        try:
            os.makedirs(save_dir, exist_ok=True)
        except Exception:
            pass
        ts = datetime.now().strftime("%Y%m%d_%H%M%S")
        # Add preamble if set
        if self._csv_preamble:
            fname = os.path.join(save_dir, f"{self._csv_preamble}_trial_{ts}.csv")
        else:
            fname = os.path.join(save_dir, f"trial_{ts}.csv")
        try:
            self._csv_file = open(fname, "w", newline="")
            self._csv_writer = csv.writer(self._csv_file)
            self._csv_header_written = False
            self._t0 = None
            self._mark_counter = 0  # Reset mark counter for new trial
            self._csv_path_last = fname
            try:
                self.scan_page.status.setText(f"Logging to {fname}")
            except Exception:
                pass
            try:
                self.trial_page.update_mark_count(0)
            except Exception:
                pass
        except Exception:
            self._csv_file = None
            self._csv_writer = None

