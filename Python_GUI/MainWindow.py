import sys
import csv
import time
import os
import logging
import traceback
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
from utils import exo_config
from utils.debug import dprint


class MainWindow(QtWidgets.QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("OpenExo - Qt")
        # Compact default window size (resizable)
        self.setMinimumSize(776, 400)
        self.resize(776, 400)
        
        # Setup logger for MainWindow
        self.logger = logging.getLogger("OpenExo.MainWindow")
        self.logger.info("Initializing MainWindow...")

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
        except Exception as e:
            self.logger.error(f"Failed to bind device manager to scan page: {e}")
            self.logger.debug(traceback.format_exc())
        self.rt_bridge = RtBridge(self)
        # Wire bytes to parser and route RT data to plots
        self.qt_dev.dataReceived.connect(self.rt_bridge.feed_bytes)
        self.rt_bridge.rtDataUpdated.connect(self._on_rt_update)
        self.rt_bridge.handshakeReceived.connect(self._on_handshake)
        self.rt_bridge.parameterNamesReceived.connect(self._on_param_names)
        self.rt_bridge.controllersReceived.connect(self._on_controllers)
        # Receive flattened 2D matrix of controllers and parameters
        self.rt_bridge.controllerMatrixReceived.connect(self._on_controller_matrix)
        # Receive controller -> default parameter values database
        self.rt_bridge.controllerValuesReceived.connect(self._on_controller_values)

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
        # In-memory database of current controller parameter values:
        #   { (joint_id:int, controller_id:int): [value_str, ...] }
        # Seeded from the firmware handshake (line 6 of each controller csv),
        # mutated locally whenever the user taps Apply, and DROPPED at the end
        # of every trial so each new connection starts from device-of-truth.
        self._controller_values = {}
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
        
        # Display log file location for debugging
        log_path = self.qt_dev.get_log_file_path()
        if log_path and log_path != "Log file not available":
            dprint(f"\n{'='*80}")
            dprint(f"Device Manager Log File: {log_path}")
            dprint(f"{'='*80}\n")

    def resizeEvent(self, event):
        super().resizeEvent(event)
        size = self.size()
        dprint(f"[MainWindow] size={size.width()}x{size.height()}")

    def _go_trial(self):
        self.stack.setCurrentWidget(self.trial_page)
        # Stop simulation so live data drives plots if available
        self.trial_page.stop_sim()
        # Clear old plot data
        try:
            self.trial_page.clear_plots()
        except Exception as e:
            self.logger.error(f"Failed to clear plots: {e}")
            self.logger.debug(traceback.format_exc())
        # Ensure CSV logging is started automatically with timestamped filename
        try:
            if self._csv_file is None:
                self._start_csv_auto()
        except Exception as e:
            self.logger.error(f"Failed to start CSV logging: {e}")
            self.logger.debug(traceback.format_exc())
        # Begin trial sequence (E -> L -> R + thresholds) to ensure FSRs stream
        try:
            self.qt_dev.beginTrial()
        except Exception as e:
            self.logger.error(f"Failed to begin trial: {e}")
            self.logger.debug(traceback.format_exc())

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
            except Exception as e:
                self.logger.error(f"Failed to apply values to bio feedback page: {e}")
                self.logger.debug(traceback.format_exc())
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
                    except Exception as e:
                        self.logger.error(f"Failed to write CSV header: {e}")
                        self.logger.debug(traceback.format_exc())
                # Write row - only include first 10 data values
                epoch_time = time.time()
                data_values = values[:10] if len(values) > 10 else values
                row = [f"{epoch_time:.6f}", str(self._mark_counter)] + [f"{v:.6f}" for v in data_values]
                try:
                    self._csv_writer.writerow(row)
                except Exception as e:
                    self.logger.error(f"Failed to write CSV row: {e}")
                    self.logger.debug(traceback.format_exc())
            
            # Update battery level (assuming it's in the data somewhere)
            try:
                if len(values) > 10:
                    battery_voltage = values[10]  # Battery is typically at index 10
                    self.trial_page.update_battery_level(battery_voltage)
            except Exception as e:
                self.logger.error(f"Failed to update battery level: {e}")
                self.logger.debug(traceback.format_exc())
        except Exception as e:
            self.logger.error(f"Failed to process RT update: {e}")
            self.logger.debug(traceback.format_exc())

    @QtCore.Slot(str)
    def _on_handshake(self, payload: str):
        try:
            self.logger.info("Handshake payload received")
            dprint(f"MainWindow::_on_handshake -> Handshake payload received")
        except Exception as e:
            self.logger.error(f"Failed to log handshake: {e}")
            self.logger.debug(traceback.format_exc())
        try:
            self.scan_page.status.setText("Handshake received; controller parameters incoming…")
        except Exception as e:
            self.logger.error(f"Failed to update status text for handshake: {e}")
            self.logger.debug(traceback.format_exc())

    @QtCore.Slot(list)
    def _on_param_names(self, names):
        # After receiving parameter names list (ended by END), send ACK so firmware continues (controllers or data)
        try:
            self.logger.info(f"Received {len(names)} parameter names")
            self.scan_page.status.setText(f"Received {len(names)} param names; sending ACK…")
        except Exception as e:
            self.logger.error(f"Failed to update status for param names: {e}")
            self.logger.debug(traceback.format_exc())
        # Store for CSV header
        try:
            self._param_names = list(names)
        except Exception as e:
            self.logger.error(f"Failed to store param names: {e}")
            self.logger.debug(traceback.format_exc())
            self._param_names = []
        try:
            self.trial_page.set_channel_labels(self._param_names)
        except Exception as e:
            self.logger.error(f"Failed to set channel labels: {e}")
            self.logger.debug(traceback.format_exc())
        try:
            self.qt_dev.write(b'$')
        except Exception as e:
            self.logger.error(f"Failed to send ACK for param names: {e}")
            self.logger.debug(traceback.format_exc())

    @QtCore.Slot(list, list)
    def _on_controllers(self, controllers, controller_params):
        # After receiving controllers/params (!… !END), send ACK and optionally start streaming
        try:
            self.logger.info(f"Received {len(controllers)} controllers")
            self.scan_page.status.setText(f"Received {len(controllers)} controllers; sending ACK…")
        except Exception as e:
            self.logger.error(f"Failed to update status for controllers: {e}")
            self.logger.debug(traceback.format_exc())
        try:
            self.qt_dev.write(b'$')
        except Exception as e:
            self.logger.error(f"Failed to send ACK for controllers: {e}")
            self.logger.debug(traceback.format_exc())

    @staticmethod
    def _normalize_controller_value_key(k):
        """Coerce an incoming controller_values key to ``(int, int)``.

        Qt's queued signal/slot machinery can sometimes round-trip dict keys
        in a form different from what the sender intended (e.g. ``(int, int)``
        tuples may arrive as ``(str, str)``, lists, or even comma-joined
        strings depending on QVariant marshalling).  This helper accepts any
        of those and yields the canonical ``(joint_id, controller_id)`` tuple,
        or ``None`` if the key is unparseable.
        """
        try:
            if isinstance(k, str):
                parts = [p.strip() for p in k.replace("(", "").replace(")", "").split(",") if p.strip()]
            elif isinstance(k, (tuple, list)):
                parts = list(k)
            else:
                return None
            if len(parts) < 2:
                return None
            return (int(parts[0]), int(parts[1]))
        except (ValueError, TypeError):
            return None

    @QtCore.Slot(list)
    def _on_controller_values(self, values_list: list):
        """Receive default parameter values for every (joint, controller).

        ``values_list`` is a list of ``[joint_id:int, controller_id:int,
        [val_str, ...]]`` triples (the dict-with-tuple-keys form cannot be
        marshalled through PySide6's QVariant layer, so RtBridge ships a list
        and we rebuild the keyed dict here).

        These came from line 6 of each controller csv on the SD card and act
        as the seed of the local database.  We forward them straight to the
        settings page so it can render the current value for whatever the
        user selects.
        """
        try:
            normalized: dict = {}
            for entry in (values_list or []):
                if not isinstance(entry, (list, tuple)) or len(entry) < 3:
                    continue
                jid_raw, cid_raw, vals = entry[0], entry[1], entry[2]
                try:
                    jid = int(jid_raw)
                    cid = int(cid_raw)
                except (ValueError, TypeError):
                    continue
                normalized[(jid, cid)] = list(vals)
            self._controller_values = normalized
            self.logger.info(
                f"Received controller values for {len(self._controller_values)} (joint, controller) pairs; "
                f"sample_keys={list(self._controller_values.keys())[:5]}"
            )
        except Exception as e:
            self.logger.error(f"Failed to store controller values: {e}")
            self.logger.debug(traceback.format_exc())
            self._controller_values = {}
        # Push the seed into the settings page if it's been built already.
        try:
            self.settings_page.set_controller_values(self._controller_values)
        except Exception as e:
            self.logger.error(f"Failed to forward controller values to settings page: {e}")
            self.logger.debug(traceback.format_exc())

    @QtCore.Slot(list)
    def _on_controller_matrix(self, matrix):
        # Save the 2D [ [controller, p1, p2], ... ] structure in memory
        try:
            full_matrix = list(matrix)
            # Filter out joints that don't belong to the configured exo_name
            # in SDCard/config.ini.  This addresses the "hip showing up for a
            # unilateral ankle" report: the firmware decides which joints to
            # broadcast based on `_DefaultController` values, not on `name`,
            # so a stray hipDefaultController = step in the ini will leak hip
            # controllers into the handshake.  We hide those in the GUI.
            allowed = exo_config.allowed_joint_types()
            if allowed is not None:
                filtered = [row for row in full_matrix
                            if row and exo_config.is_joint_allowed(str(row[0]), allowed)]
                if len(filtered) != len(full_matrix):
                    self.logger.info(
                        f"Filtered controller matrix from {len(full_matrix)} to "
                        f"{len(filtered)} entries based on config.ini name "
                        f"(allowed joints: {sorted(allowed)})"
                    )
                self._controller_matrix = filtered
            else:
                self._controller_matrix = full_matrix
            self.logger.info(f"Received controller matrix with {len(self._controller_matrix)} entries")
        except Exception as e:
            self.logger.error(f"Failed to store controller matrix: {e}")
            self.logger.debug(traceback.format_exc())
            self._controller_matrix = []
        has_matrix = bool(self._controller_matrix)
        try:
            self.settings_page.set_controller_matrix(self._controller_matrix if has_matrix else [])
        except Exception as e:
            self.logger.error(f"Failed to set controller matrix in settings page: {e}")
            self.logger.debug(traceback.format_exc())
        # Update Controller button is always enabled
        # If no matrix, it will show the legacy basic settings page
        try:
            self.trial_page.set_update_controller_enabled(True)
        except Exception as e:
            self.logger.error(f"Failed to enable update controller button: {e}")
            self.logger.debug(traceback.format_exc())

    @QtCore.Slot()
    def _on_device_start(self):
        # Resume motors (play functionality) - just turn motors back on
        try:
            self.logger.info("Turning motors ON")
            self.qt_dev.motorOn()
        except Exception as e:
            self.logger.error(f"Failed to turn motors on: {e}")
            self.logger.debug(traceback.format_exc())

    @QtCore.Slot()
    def _on_device_stop_motors(self):
        # Turn off motors (pause functionality)
        try:
            self.logger.info("Turning motors OFF")
            self.qt_dev.motorOff()
        except Exception as e:
            self.logger.error(f"Failed to turn motors off: {e}")
            self.logger.debug(traceback.format_exc())

    @QtCore.Slot(str)
    def _on_csv_preamble_changed(self, preamble: str):
        """Update CSV filename preamble."""
        self._csv_preamble = preamble
        self.logger.info(f"CSV preamble set to: {preamble}")
        dprint(f"CSV preamble set to: {preamble}")
         # If we're currently logging, roll over immediately (no popup)
        if self._csv_file is not None:
            try:
                self._csv_file.flush()
                self._csv_file.close()
            except Exception as e:
                self.logger.error(f"Failed to close CSV file during preamble change: {e}")
                self.logger.debug(traceback.format_exc())

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
            except Exception as e:
                self.logger.error(f"Failed to update status text after preamble change: {e}")
                self.logger.debug(traceback.format_exc())

    @QtCore.Slot()
    def _on_recal_fsr(self):
        try:
            self.logger.info("Calibrating FSRs")
            self.qt_dev.calibrateFSRs()
        except Exception as e:
            self.logger.error(f"Failed to calibrate FSRs: {e}")
            self.logger.debug(traceback.format_exc())

    @QtCore.Slot()
    def _on_send_preset_fsr(self):
        try:
            self.logger.info("Sending preset FSR values")
            self.qt_dev.sendPresetFsrValues()
        except Exception as e:
            self.logger.error(f"Failed to send preset FSR values: {e}")
            self.logger.debug(traceback.format_exc())

    @QtCore.Slot()
    def _on_recal_torque(self):
        try:
            self.logger.info("Calibrating torque")
            self.qt_dev.calibrateTorque()
        except Exception as e:
            self.logger.error(f"Failed to calibrate torque: {e}")
            self.logger.debug(traceback.format_exc())

    @QtCore.Slot()
    def _on_mark(self):
        # Increment trial mark counter
        self._mark_counter += 1
        self.logger.info(f"Trial marked: {self._mark_counter}")
        try:
            self.scan_page.status.setText(f"Trial marked: {self._mark_counter}")
        except Exception as e:
            self.logger.error(f"Failed to update scan page status for mark: {e}")
            self.logger.debug(traceback.format_exc())
        try:
            self.trial_page.update_mark_count(self._mark_counter)
        except Exception as e:
            self.logger.error(f"Failed to update trial page mark count: {e}")
            self.logger.debug(traceback.format_exc())
        try:
            self.bio_feedback_page.update_mark_count(self._mark_counter)
        except Exception as e:
            self.logger.error(f"Failed to update bio feedback page mark count: {e}")
            self.logger.debug(traceback.format_exc())

    @QtCore.Slot()
    def _on_end_trial(self):
        try:
            self.logger.info("Ending trial...")
            # Send stop trial and motor off commands immediately
            try:
                self.qt_dev.write(b'G')  # Stop trial
                self.qt_dev.write(b'w')  # Motor off - CRITICAL SAFETY COMMAND
                self.logger.info("Sent stop trial and motor off commands")
            except Exception as e:
                self.logger.error(f"CRITICAL: Failed to send stop/motor off commands: {e}")
                self.logger.debug(traceback.format_exc())

            # Wipe the in-memory controller info DB so the next trial seeds
            # fresh defaults from the firmware handshake (per spec: controller
            # information must be destroyed at end of trial).
            self._destroy_controller_db("end trial")

            # Reset scan page buttons immediately
            self.scan_page.btn_start_trial.setEnabled(False)
            self.scan_page.btn_calibrate_torque.setEnabled(False)
            self.scan_page.btn_save_connect.setEnabled(True)
            
            # Clear trial page plots
            try:
                self.trial_page.clear_plots()
            except Exception as e:
                self.logger.error(f"Failed to clear trial page plots: {e}")
                self.logger.debug(traceback.format_exc())
            
            # Navigate to scan page immediately
            self.stack.setCurrentWidget(self.scan_page)
            
            # Wait 200ms to ensure motor off command is sent before disconnecting
            QtCore.QTimer.singleShot(200, self.qt_dev.disconnect)
            
            # Stop CSV if running
            if self._csv_file is not None:
                try:
                    self._csv_file.flush(); self._csv_file.close()
                    self.logger.info(f"CSV file closed: {self._csv_path_last}")
                except Exception as e:
                    self.logger.error(f"Failed to close CSV file: {e}")
                    self.logger.debug(traceback.format_exc())
                self._csv_file = None
                self._csv_writer = None
                self._csv_header_written = False
                self._t0 = None
                self._mark_counter = 0  # Reset mark counter
                try:
                    if self._csv_path_last:
                        self.scan_page.status.setText(f"Trial ended. CSV saved: {self._csv_path_last}")
                except Exception as e:
                    self.logger.error(f"Failed to update status text after trial end: {e}")
                    self.logger.debug(traceback.format_exc())
                try:
                    self.trial_page.update_mark_count(0)
                except Exception as e:
                    self.logger.error(f"Failed to reset mark count: {e}")
                    self.logger.debug(traceback.format_exc())
        except Exception as e:
            self.logger.error(f"Failed to end trial: {e}")
            self.logger.debug(traceback.format_exc())

    def _destroy_controller_db(self, reason: str = ""):
        """Drop all cached controller metadata + parameter values.

        Called whenever the trial ends or the link drops so the next session
        always seeds itself fresh from the device.
        """
        if self._controller_values or self._controller_matrix:
            self.logger.info(
                f"Destroying controller DB ({reason}): "
                f"matrix={len(self._controller_matrix)} entries, "
                f"values={len(self._controller_values)} entries"
            )
        self._controller_values = {}
        self._controller_matrix = []
        try:
            self.settings_page.set_controller_values({})
            self.settings_page.set_controller_matrix([])
        except Exception as e:
            self.logger.error(f"Failed to clear settings page after DB destroy: {e}")
            self.logger.debug(traceback.format_exc())

    @QtCore.Slot()
    def _on_disconnect(self):
        try:
            self.logger.info("Manual disconnect requested")
            # Disconnect immediately (non-blocking, no popup)
            self.qt_dev.disconnect()
            # Drop cached controller info; a fresh handshake will repopulate.
            self._destroy_controller_db("manual disconnect")
            
            # Navigate to scan page immediately
            self.stack.setCurrentWidget(self.scan_page)
            # Stop CSV if running
            if self._csv_file is not None:
                try:
                    self._csv_file.flush(); self._csv_file.close()
                    self.logger.info(f"CSV file closed on disconnect: {self._csv_path_last}")
                except Exception as e:
                    self.logger.error(f"Failed to close CSV on disconnect: {e}")
                    self.logger.debug(traceback.format_exc())
                self._csv_file = None
                self._csv_writer = None
                self._csv_header_written = False
                self._t0 = None
                self._mark_counter = 0  # Reset mark counter
                try:
                    if self._csv_path_last:
                        self.scan_page.status.setText(f"Disconnected. CSV saved: {self._csv_path_last}")
                except Exception as e:
                    self.logger.error(f"Failed to update status on disconnect: {e}")
                    self.logger.debug(traceback.format_exc())
                try:
                    self.trial_page.update_mark_count(0)
                except Exception as e:
                    self.logger.error(f"Failed to reset mark count on disconnect: {e}")
                    self.logger.debug(traceback.format_exc())
        except Exception as e:
            self.logger.error(f"Failed to disconnect: {e}")
            self.logger.debug(traceback.format_exc())

    @QtCore.Slot()
    def _navigate_to_scan(self):
        try:
            self._on_disconnect()
        except Exception as e:
            self.logger.error(f"Failed during navigate to scan: {e}")
            self.logger.debug(traceback.format_exc())
        self.stack.setCurrentWidget(self.scan_page)

    @QtCore.Slot()
    def _on_save_csv(self):
        """Save current CSV file and immediately start a new one."""
        try:
            saved_path = None
            # Close current CSV if open
            if self._csv_file is not None:
                self.logger.info("Saving current CSV and starting new one")
                dprint("Saving current CSV and starting new one")
                try:
                    self._csv_file.flush()
                    self._csv_file.close()
                    saved_path = self._csv_path_last
                    self.logger.info(f"CSV saved: {saved_path}")
                except Exception as e:
                    self.logger.error(f"Failed to save CSV: {e}")
                    self.logger.debug(traceback.format_exc())
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
                self.logger.error(f"Failed to show CSV save confirmation: {e}")
                self.logger.debug(traceback.format_exc())
                dprint(f"Error showing confirmation: {e}")
        except Exception as e:
            self.logger.error(f"Failed in _on_save_csv: {e}")
            self.logger.debug(traceback.format_exc())
            dprint(f"Error in _on_save_csv: {e}")

    @QtCore.Slot()
    def _on_update_controller(self):
        # Choose page based on whether we have controller metadata (handshake + matrix)
        has_matrix = bool(self._controller_matrix)
        self.logger.info(f"Update controller requested, has_matrix={has_matrix}")
        if has_matrix:
            try:
                self.settings_page.set_controller_matrix(self._controller_matrix)
            except Exception as e:
                self.logger.error(f"Failed to set controller matrix in settings: {e}")
                self.logger.debug(traceback.format_exc())
            self.stack.setCurrentWidget(self.settings_page)
        else:
            self.stack.setCurrentWidget(self.basic_settings_page)

    @QtCore.Slot()
    def _on_bio_feedback(self):
        self.logger.info("Navigating to bio feedback page")
        try:
            self.bio_feedback_page.start_plotting()
        except Exception as e:
            self.logger.error(f"Failed to start plotting on bio feedback page: {e}")
            self.logger.debug(traceback.format_exc())
        self.stack.setCurrentWidget(self.bio_feedback_page)

    def _on_bio_feedback_back(self):
        self.logger.info("Navigating back from bio feedback page")
        try:
            self.bio_feedback_page.stop_plotting()
        except Exception as e:
            self.logger.error(f"Failed to stop plotting on bio feedback page: {e}")
            self.logger.debug(traceback.format_exc())
        self.stack.setCurrentWidget(self.trial_page)

    @QtCore.Slot()
    def _on_machine_learning(self):
        # Placeholder hook
        pass

    @QtCore.Slot(list)
    def _on_apply_settings(self, payload):
        # payload: [isBilateral, joint_id, controller_id, parameter_idx, value]
        # Apply does TWO things, in order:
        #   1) push the new value to the firmware over BLE
        #   2) mirror the change into the local controller-value database
        self.logger.info(f"Applying settings: {payload}")
        try:
            self.qt_dev.updateTorqueValues(payload)
        except Exception as e:
            self.logger.error(f"Failed to update torque values: {e}")
            self.logger.debug(traceback.format_exc())

        try:
            is_bilateral, joint_id, controller_id, param_idx, value = payload
            self._update_controller_value_db(
                joint_id=int(joint_id) if joint_id is not None else None,
                controller_id=int(controller_id),
                param_idx=int(param_idx),
                value=value,
                bilateral=bool(is_bilateral),
            )
        except Exception as e:
            self.logger.error(f"Failed to mirror update into controller value DB: {e}")
            self.logger.debug(traceback.format_exc())
        # Return to trial page
        try:
            self.stack.setCurrentWidget(self.trial_page)
        except Exception as e:
            self.logger.error(f"Failed to navigate to trial page after settings: {e}")
            self.logger.debug(traceback.format_exc())

    def _update_controller_value_db(self, joint_id, controller_id, param_idx,
                                    value, bilateral=False):
        """Write a single parameter value into the local DB and refresh UI.

        When ``bilateral`` is set we also mirror the write to the opposite
        side joint (``joint_id ^ 0x60`` matches firmware/iOS behaviour).
        Values are stored as strings so int-vs-float can be inferred at
        render time (the firmware wire format is also string-based).
        """
        if joint_id is None:
            return

        joint_ids = [joint_id]
        if bilateral:
            joint_ids.append(joint_id ^ 0x60)

        # Format the value the same way an integer-looking input would print
        # (e.g. "5" stays "5", "0.25" stays "0.25").
        if isinstance(value, (int,)) or (isinstance(value, float) and value.is_integer()):
            value_str = str(int(value))
        else:
            value_str = str(value)

        for jid in joint_ids:
            key = (jid, controller_id)
            row = self._controller_values.get(key)
            if row is None:
                row = []
            # Pad with empty strings if the param index is beyond what we have
            while len(row) <= param_idx:
                row.append("")
            row[param_idx] = value_str
            self._controller_values[key] = row

        try:
            self.settings_page.set_controller_values(self._controller_values)
        except Exception as e:
            self.logger.error(f"Failed to refresh settings page after DB update: {e}")
            self.logger.debug(traceback.format_exc())

    @QtCore.Slot(str)
    def _on_dev_log(self, msg: str):
        try:
            self.scan_page.status.setText(msg)
        except Exception as e:
            self.logger.error(f"Failed to update scan page status: {e}")
            self.logger.debug(traceback.format_exc())

    @QtCore.Slot(str)
    def _on_dev_error(self, msg: str):
        self.logger.error(f"Device error: {msg}")
        try:
            self.scan_page.status.setText(f"Connection failed: {msg}")
            self.scan_page.btn_save_connect.setEnabled(True)
            self.scan_page.btn_start_trial.setEnabled(False)
            try:
                self.scan_page.btn_calibrate_torque.setEnabled(False)
            except Exception as e:
                self.logger.error(f"Failed to disable calibrate torque button: {e}")
                self.logger.debug(traceback.format_exc())
        except Exception as e:
            self.logger.error(f"Failed to handle device error UI update: {e}")
            self.logger.debug(traceback.format_exc())

    @QtCore.Slot(str, str)
    def _on_dev_connected(self, name: str, addr: str):
        self.logger.info(f"Device connected: {name} {addr}")
        try:
            self.scan_page.status.setText(f"Connected: {name} {addr}")
            # Don't enable Save & Connect after connection - it's already connected
            self.scan_page.btn_save_connect.setEnabled(False)
            self.scan_page.btn_start_trial.setEnabled(False)  # Enabled after torque calibration
            try:
                self.scan_page.btn_calibrate_torque.setEnabled(True)
            except Exception as e:
                self.logger.error(f"Failed to enable calibrate torque button: {e}")
                self.logger.debug(traceback.format_exc())
        except Exception as e:
            self.logger.error(f"Failed to update UI on connection: {e}")
            self.logger.debug(traceback.format_exc())
        # Clear old plot data on reconnect
        try:
            self.trial_page.clear_plots()
        except Exception as e:
            self.logger.error(f"Failed to clear plots on connection: {e}")
            self.logger.debug(traceback.format_exc())

    def _show_disconnect_warning(self):
        """Show disconnect warning dialog (called after disconnect handling completes)."""
        try:
            QtWidgets.QMessageBox.warning(
                self, 
                "Device Disconnected", 
                "The device has been unexpectedly disconnected.\n\nMotors have been turned off and the trial data has been saved."
            )
        except Exception as e:
            self.logger.error(f"Failed to show disconnect warning: {e}")
            self.logger.debug(traceback.format_exc())

    @QtCore.Slot()
    def _on_dev_disconnected(self):
        """Handle unexpected disconnects (intentional disconnects don't trigger this)."""
        self.logger.warning("Device disconnected unexpectedly")
        # The controller-info cache is tied to the live connection.
        self._destroy_controller_db("unexpected disconnect")
        try:
            self.scan_page.status.setText("Disconnected unexpectedly")
            self.scan_page.btn_save_connect.setEnabled(True)
            self.scan_page.btn_start_trial.setEnabled(False)
            try:
                self.scan_page.btn_calibrate_torque.setEnabled(False)
            except Exception as e:
                self.logger.error(f"Failed to disable calibrate torque button: {e}")
                self.logger.debug(traceback.format_exc())
            
            # Device is already disconnected, no need to send commands
            # (motorOff/stopTrial would fail with "Not connected" errors)
            self.logger.info("Device already disconnected - skipping motor off/stop trial commands")
            # Show non-blocking notification of unexpected disconnect
            # Use QTimer to defer the dialog so disconnect handling completes first
            try:
                QtCore.QTimer.singleShot(100, lambda: self._show_disconnect_warning())
            except Exception as e:
                self.logger.error(f"Failed to schedule disconnect warning dialog: {e}")
                self.logger.debug(traceback.format_exc())
            # Navigate back to the Scan page on unexpected disconnect
            self.stack.setCurrentWidget(self.scan_page)
            
            # Ensure CSV is closed and announce saved path
            if self._csv_file is not None:
                try:
                    self._csv_file.flush(); self._csv_file.close()
                    self.logger.info(f"CSV closed after disconnect: {self._csv_path_last}")
                except Exception as e:
                    self.logger.error(f"Failed to close CSV after disconnect: {e}")
                    self.logger.debug(traceback.format_exc())
                self._csv_file = None
                self._csv_writer = None
                self._csv_header_written = False
                self._t0 = None
                self._mark_counter = 0  # Reset mark counter
                try:
                    if self._csv_path_last:
                        self.scan_page.status.setText(f"Unexpected disconnect. CSV saved: {self._csv_path_last}")
                except Exception as e:
                    self.logger.error(f"Failed to update status after disconnect: {e}")
                    self.logger.debug(traceback.format_exc())
                try:
                    self.trial_page.update_mark_count(0)
                except Exception as e:
                    self.logger.error(f"Failed to reset mark count after disconnect: {e}")
                    self.logger.debug(traceback.format_exc())
        except Exception as e:
            self.logger.error(f"Failed to handle device disconnect: {e}")
            self.logger.debug(traceback.format_exc())

    def _start_csv_auto(self):
        # Save within Qt/Saved_Data
        base_dir = os.path.dirname(__file__)  # Python_GUI/Qt
        save_dir = os.path.join(base_dir, "Saved_Data")
        try:
            os.makedirs(save_dir, exist_ok=True)
        except Exception as e:
            self.logger.error(f"Failed to create Saved_Data directory: {e}")
            self.logger.debug(traceback.format_exc())
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
            self.logger.info(f"Started CSV logging to: {fname}")
            try:
                self.scan_page.status.setText(f"Logging to {fname}")
            except Exception as e:
                self.logger.error(f"Failed to update status with CSV path: {e}")
                self.logger.debug(traceback.format_exc())
            try:
                self.trial_page.update_mark_count(0)
            except Exception as e:
                self.logger.error(f"Failed to reset mark count: {e}")
                self.logger.debug(traceback.format_exc())
        except Exception as e:
            self.logger.error(f"Failed to start CSV auto-logging: {e}")
            self.logger.debug(traceback.format_exc())
            self._csv_file = None
            self._csv_writer = None

