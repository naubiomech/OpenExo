import sys
import math
import time
from collections import deque

try:
    from PySide6 import QtCore, QtWidgets, QtGui
except ImportError as e:
    raise SystemExit("PySide6 is required. Install with: pip install PySide6") from e

try:
    import pyqtgraph as pg
except ImportError as e:
    raise SystemExit("pyqtgraph is required. Install with: pip install pyqtgraph") from e

from utils import (
    UIConfig, PlotConfig,
    load_logo, create_separator, create_section_label,
    apply_button_style_batch, set_size_policy_fixed_height
)


class ActiveTrialPage(QtWidgets.QWidget):
    """Active Trial page with two stacked real-time plots (simulated data)."""

    # Signals to be handled by MainWindow (placeholders can be wired later)
    endTrialRequested = QtCore.Signal()
    saveCsvRequested = QtCore.Signal()
    updateControllerRequested = QtCore.Signal()
    bioFeedbackRequested = QtCore.Signal()
    machineLearningRequested = QtCore.Signal()
    recalibrateFSRRequested = QtCore.Signal()
    sendPresetFSRRequested = QtCore.Signal()
    recalibrateTorqueRequested = QtCore.Signal()
    markTrialRequested = QtCore.Signal()
    deviceStartRequested = QtCore.Signal()
    deviceStopRequested = QtCore.Signal()
    csvPreambleChanged = QtCore.Signal(str)

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setObjectName("ActiveTrialPage")
        self._base_button_font_size = 13  # Store base size for scaling
        self._build_ui()
        self._init_state()
    
    def resizeEvent(self, event):
        """Dynamically adjust font sizes based on window width"""
        super().resizeEvent(event)
        # Scale font based on window width (base: 900px)
        width = self.width()
        scale_factor = max(0.7, min(1.5, width / 900.0))  # Keep between 70% and 150%
        
        # Update button font sizes
        new_font_size = int(self._base_button_font_size * scale_factor)
        for btn in [self.btn_toggle_points, self.btn_end_trial, self.btn_save_csv,
                    self.btn_set_preamble, self.btn_update_controller, self.btn_bio_feedback,
                    self.btn_ml, self.btn_recal_fsr, self.btn_send_preset_fsr, self.btn_recal_torque,
                    self.btn_mark, self.btn_pause_play]:
            f = btn.font()
            f.setPointSize(new_font_size)
            btn.setFont(f)

    def _build_ui(self):
        # Main horizontal layout: left controls, right plots
        main = QtWidgets.QHBoxLayout(self)
        main.setContentsMargins(UIConfig.MARGIN_PAGE, UIConfig.MARGIN_PAGE, UIConfig.MARGIN_PAGE, UIConfig.MARGIN_PAGE)
        main.setSpacing(UIConfig.SPACING_XLARGE)

        # Left controls column
        controls = QtWidgets.QVBoxLayout()
        controls.setSpacing(UIConfig.SPACING_MEDIUM)
        
        # Top row with OpenExo logo and battery
        logo_row = QtWidgets.QHBoxLayout()
        
        # OpenExo logo on left - smaller for compact layout
        openexo_logo = load_logo(
            "OpenExo.png",
            UIConfig.LOGO_OPENEXO_SMALL_WIDTH,
            UIConfig.LOGO_OPENEXO_SMALL_HEIGHT
        )
        if openexo_logo:
            logo_row.addWidget(openexo_logo)
        
        logo_row.addStretch(1)
        
        # Battery level label on the right
        self.lbl_battery = QtWidgets.QLabel("Battery: --")
        self.lbl_battery.setStyleSheet(f"font-size: {UIConfig.FONT_SMALL}pt; color: {UIConfig.COLOR_SUCCESS}; font-weight: bold;")
        self.lbl_battery.setAlignment(QtCore.Qt.AlignRight | QtCore.Qt.AlignVCenter)
        logo_row.addWidget(self.lbl_battery)
        
        controls.addLayout(logo_row)
        controls.addSpacing(UIConfig.SPACING_SMALL)
        
        title = QtWidgets.QLabel("Active Trial")
        f = title.font(); f.setPointSize(UIConfig.FONT_SUBTITLE); title.setFont(f)
        controls.addWidget(title)
        controls.addSpacing(UIConfig.MARGIN_PAGE)

        # ═══════ PRIORITY: CRITICAL CONTROLS ═══════
        # End Trial - Big prominent button at top
        self.btn_end_trial = QtWidgets.QPushButton("END TRIAL")
        controls.addWidget(self.btn_end_trial)
        
        # Add more spacing between End Trial and Pause
        controls.addSpacing(UIConfig.SPACING_XXLARGE)
        
        # Pause/Play button
        self.btn_pause_play = QtWidgets.QPushButton("Pause")
        self.is_paused = False
        controls.addWidget(self.btn_pause_play)
        
        # Separator
        controls.addSpacing(UIConfig.SPACING_XLARGE)
        controls.addWidget(create_separator())
        
        # ═══════ COMMON ACTIONS ═══════
        controls.addWidget(create_section_label("Common Actions"))
        controls.addSpacing(UIConfig.SPACING_SMALL)
        
        self.btn_update_controller = QtWidgets.QPushButton("Update Controller")
        controls.addWidget(self.btn_update_controller)
        self.btn_update_controller.setEnabled(True)
        
        self.btn_mark = QtWidgets.QPushButton("Mark Trial (0)")
        controls.addWidget(self.btn_mark)
        
        self.btn_save_csv = QtWidgets.QPushButton("Save & New CSV")
        controls.addWidget(self.btn_save_csv)
        
        # Separator
        controls.addSpacing(UIConfig.SPACING_XLARGE)
        controls.addWidget(create_separator())
        
        # ═══════ SETTINGS ═══════
        controls.addWidget(create_section_label("Settings"))
        controls.addSpacing(UIConfig.SPACING_SMALL)
        
        self.btn_set_preamble = QtWidgets.QPushButton("Set CSV Prefix")
        controls.addWidget(self.btn_set_preamble)
        
        self.btn_toggle_points = QtWidgets.QPushButton("Toggle Data Points")
        controls.addWidget(self.btn_toggle_points)
        
        # Separator
        controls.addSpacing(UIConfig.SPACING_XLARGE)
        controls.addWidget(create_separator())
        
        # ═══════ ADVANCED ═══════
        controls.addWidget(create_section_label("Advanced"))
        controls.addSpacing(UIConfig.SPACING_SMALL)
        
        self.btn_bio_feedback = QtWidgets.QPushButton("Bio Feedback")
        controls.addWidget(self.btn_bio_feedback)
        
        self.btn_ml = QtWidgets.QPushButton("Machine Learning")
        controls.addWidget(self.btn_ml)
        
        self.btn_recal_fsr = QtWidgets.QPushButton("Recalibrate FSRs")
        controls.addWidget(self.btn_recal_fsr)
        
        self.btn_send_preset_fsr = QtWidgets.QPushButton("Send Preset FSR")
        controls.addWidget(self.btn_send_preset_fsr)

        self.btn_recal_torque = QtWidgets.QPushButton("Recalibrate Torque Sensor")
        controls.addWidget(self.btn_recal_torque)
        
        controls.addStretch(1)

        # Right plots column
        plots_col = QtWidgets.QVBoxLayout()
        # Create a GraphicsLayoutWidget to host two plot rows
        self.graph = pg.GraphicsLayoutWidget()
        plots_col.addWidget(self.graph, 1)

        # Top plot (e.g., Controller vs Measurement)
        self.plot_top = self.graph.addPlot(row=0, col=0)
        self.plot_top.showGrid(x=True, y=True, alpha=PlotConfig.GRID_ALPHA)
        self.plot_top.addLegend()
        self.plot_top.setLabel("left", "Top Signal")
        self.plot_top.setLabel("bottom", "t (s)")
        self.curve_top_cmd = self.plot_top.plot(
            pen=pg.mkPen(PlotConfig.COLOR_CONTROLLER, width=PlotConfig.CURVE_WIDTH),
            name='Controller'
        )
        self.curve_top_meas = self.plot_top.plot(
            pen=pg.mkPen(PlotConfig.COLOR_MEASUREMENT, width=PlotConfig.CURVE_WIDTH),
            name='Measurement'
        )

        # Bottom plot (e.g., additional signals)
        self.plot_bottom = self.graph.addPlot(row=1, col=0)
        self.plot_bottom.showGrid(x=True, y=True, alpha=PlotConfig.GRID_ALPHA)
        self.plot_bottom.addLegend()
        self.plot_bottom.setLabel("left", "Bottom Signal")
        self.plot_bottom.setLabel("bottom", "t (s)")
        self.curve_bot_a = self.plot_bottom.plot(
            pen=pg.mkPen(PlotConfig.COLOR_SIGNAL_A, width=PlotConfig.CURVE_WIDTH),
            name='Signal A'
        )
        self.curve_bot_b = self.plot_bottom.plot(
            pen=pg.mkPen(PlotConfig.COLOR_SIGNAL_B, width=PlotConfig.CURVE_WIDTH),
            name='Signal B'
        )

        # Wrap controls in a widget
        controls_widget = QtWidgets.QWidget()
        controls_widget.setLayout(controls)
        # No max width - let it grow with window
        
        # Assemble - controls and plots both expand proportionally
        main.addWidget(controls_widget, 1)  # Stretch factor 1 - controls can grow
        main.addLayout(plots_col, 3)  # Stretch factor 3 - plots get more space

        # Wiring (device control; sim controls available via methods if needed)
        self.btn_pause_play.clicked.connect(self._on_pause_play_clicked)
        self.btn_toggle_points.clicked.connect(self._toggle_points)
        # Emit-only wiring; MainWindow can connect these to actual actions
        self.btn_end_trial.clicked.connect(self.endTrialRequested.emit)
        self.btn_save_csv.clicked.connect(self.saveCsvRequested.emit)
        self.btn_set_preamble.clicked.connect(self._on_set_preamble_clicked)
        self.btn_update_controller.clicked.connect(self.updateControllerRequested.emit)
        self.btn_bio_feedback.clicked.connect(self.bioFeedbackRequested.emit)
        self.btn_ml.clicked.connect(self.machineLearningRequested.emit)
        self.btn_recal_fsr.clicked.connect(self.recalibrateFSRRequested.emit)
        self.btn_send_preset_fsr.clicked.connect(self.sendPresetFSRRequested.emit)
        self.btn_recal_torque.clicked.connect(self.recalibrateTorqueRequested.emit)
        self.btn_mark.clicked.connect(self.markTrialRequested.emit)

        # Apply consistent button styling
        buttons = [
            self.btn_toggle_points, self.btn_end_trial, self.btn_save_csv,
            self.btn_set_preamble, self.btn_update_controller, self.btn_bio_feedback,
            self.btn_ml, self.btn_recal_fsr, self.btn_send_preset_fsr, self.btn_recal_torque,
            self.btn_mark, self.btn_pause_play,
        ]
        apply_button_style_batch(buttons, height=UIConfig.BTN_HEIGHT_SMALL, padding="6px 10px")
        
        # Set size policy for all buttons
        for btn in buttons:
            set_size_policy_fixed_height(btn)
            btn.setStyleSheet(btn.styleSheet() + " margin-bottom: 2px;")
        
        # Special styling for End Trial button - make it prominent and red
        f = self.btn_end_trial.font()
        f.setPointSize(UIConfig.FONT_MEDIUM)
        f.setBold(True)
        self.btn_end_trial.setFont(f)
        self.btn_end_trial.setMinimumHeight(UIConfig.BTN_HEIGHT_LARGE)
        self.btn_end_trial.setStyleSheet(
            f"background-color: {UIConfig.COLOR_CRITICAL}; color: white; padding: 10px; "
            "margin-bottom: 4px; font-weight: bold; border-radius: 4px;"
        )
        
        # Special styling for Pause button - make it blue
        self.btn_pause_play.setStyleSheet(
            f"background-color: {UIConfig.COLOR_ACTION}; color: white; padding: 8px; "
            "margin-bottom: 4px; font-weight: bold; border-radius: 4px;"
        )

    def _init_state(self):
        # Fixed-size buffers for plotting (seconds-window * rate)
        self.rate_hz = PlotConfig.RATE_HZ
        self.window_secs = PlotConfig.WINDOW_SECS
        self.maxlen = self.rate_hz * self.window_secs
        self.t0 = time.time()

        self.t_vals = deque(maxlen=self.maxlen)
        self.top_cmd_vals = deque(maxlen=self.maxlen)
        self.top_meas_vals = deque(maxlen=self.maxlen)
        self.bot_a_vals = deque(maxlen=self.maxlen)
        self.bot_b_vals = deque(maxlen=self.maxlen)

        # Timer for updates
        self.timer = QtCore.QTimer(self)
        self.timer.timeout.connect(self._on_tick)
        self.timer.setTimerType(QtCore.Qt.PreciseTimer)
        # Toggle state: which 4-value block to plot (0..3 or 4..7)
        self._block_index = 0  # 0 = data[0..3], 1 = data[4..7]
        # Store dynamic parameter names from device
        self._param_names = []
        # Track real data start time
        self._real_data_t0 = None

    def set_update_controller_enabled(self, enabled: bool):
        try:
            self.btn_update_controller.setEnabled(bool(enabled))
        except Exception:
            pass

    def update_mark_count(self, count: int):
        """Update the Mark Trial button to show current count."""
        try:
            self.btn_mark.setText(f"Mark Trial ({count})")
        except Exception:
            pass

    def update_battery_level(self, voltage: float):
        """Update the battery level display."""
        try:
            self.lbl_battery.setText(f"Battery: {voltage:.2f}V")
            # Change color if low (< 11V is typical low for 3S lipo, or 0V)
            if voltage < UIConfig.BATTERY_LOW_VOLTAGE:
                self.lbl_battery.setStyleSheet(
                    f"font-size: {UIConfig.FONT_SMALL}pt; color: {UIConfig.COLOR_WARNING}; font-weight: bold;"
                )
            else:
                self.lbl_battery.setStyleSheet(
                    f"font-size: {UIConfig.FONT_SMALL}pt; color: {UIConfig.COLOR_SUCCESS}; font-weight: bold;"
                )
        except Exception:
            pass

    def clear_plots(self):
        """Clear all plot data and reset timing."""
        try:
            # Clear all data buffers
            self.t_vals.clear()
            self.top_cmd_vals.clear()
            self.top_meas_vals.clear()
            self.bot_a_vals.clear()
            self.bot_b_vals.clear()
            
            # Reset timing
            self._real_data_t0 = None
            self.t0 = time.time()
            
            # Reset pause state when clearing plots
            self.is_paused = False
            self.btn_pause_play.setText("Pause")
            
            # Update plots to show empty data
            self.curve_top_cmd.setData([], [])
            self.curve_top_meas.setData([], [])
            self.curve_bot_a.setData([], [])
            self.curve_bot_b.setData([], [])
        except Exception:
            pass

    def set_channel_labels(self, param_names: list):
        """Update plot labels with dynamic parameter names from device handshake."""
        try:
            self._param_names = list(param_names) if param_names else []
            # Update labels for current block
            self._update_labels()
        except Exception:
            pass

    def _update_labels(self):
        """Update plot titles and legend names based on current block index."""
        if not self._param_names or len(self._param_names) < 4:
            return
        
        base = 4 * self._block_index
        # Ensure we have enough parameter names for the selected block
        if len(self._param_names) < base + 4:
            return
        
        try:
            # Update top plot title and curve names
            top_cmd_name = self._param_names[base + 0] if base + 0 < len(self._param_names) else 'Top Cmd'
            top_meas_name = self._param_names[base + 1] if base + 1 < len(self._param_names) else 'Top Meas'
            self.plot_top.setTitle(f"{top_cmd_name} vs {top_meas_name}")
            
            # Update legend items
            self.plot_top.legend.clear()
            self.curve_top_cmd.opts['name'] = top_cmd_name
            self.curve_top_meas.opts['name'] = top_meas_name
            self.plot_top.legend.addItem(self.curve_top_cmd, top_cmd_name)
            self.plot_top.legend.addItem(self.curve_top_meas, top_meas_name)
            
            # Update bottom plot title and curve names
            bot_a_name = self._param_names[base + 2] if base + 2 < len(self._param_names) else 'Bottom A'
            bot_b_name = self._param_names[base + 3] if base + 3 < len(self._param_names) else 'Bottom B'
            self.plot_bottom.setTitle(f"{bot_a_name} vs {bot_b_name}")
            
            # Update legend items
            self.plot_bottom.legend.clear()
            self.curve_bot_a.opts['name'] = bot_a_name
            self.curve_bot_b.opts['name'] = bot_b_name
            self.plot_bottom.legend.addItem(self.curve_bot_a, bot_a_name)
            self.plot_bottom.legend.addItem(self.curve_bot_b, bot_b_name)
        except Exception:
            pass

    def _on_pause_play_clicked(self):
        """Toggle between pause and play states."""
        if not self.is_paused:
            # Pause
            self.is_paused = True
            self.btn_pause_play.setText("Play")
            self.deviceStopRequested.emit()  # Stop motors
        else:
            # Play
            self.is_paused = False
            self.btn_pause_play.setText("⏸ Pause")
            self.deviceStartRequested.emit()  # Start motors

    def _on_set_preamble_clicked(self):
        """Show dialog to set CSV filename preamble."""
        text, ok = QtWidgets.QInputDialog.getText(
            self,
            "Set CSV Filename Prefix + Save new CSV",
            "Enter prefix for CSV filenames (e.g., 'OutdoorShod'):",
            QtWidgets.QLineEdit.Normal,
            ""
        )
        if ok and text:
            # Sanitize filename
            text = "".join(c for c in text if c.isalnum() or c in ('_', '-'))
            self.csvPreambleChanged.emit(text)
            QtWidgets.QMessageBox.information(
                self,
                "CSV Prefix Set",
                f"CSV files will be saved as:\n{text}_trial_YYYYMMDD_HHMMSS.csv"
            )

    # Public API to integrate later with bridges
    def start_sim(self):
        if not self.timer.isActive():
            self.t0 = time.time()
            self._real_data_t0 = None  # Reset real data timing when starting sim
            self.timer.start(int(1000 / self.rate_hz))

    def stop_sim(self):
        if self.timer.isActive():
            self.timer.stop()
        # Reset real data timing when switching from sim to real data
        self._real_data_t0 = None

    def _toggle_points(self):
        # Toggle which 4-value block we plot
        self._block_index = 1 - self._block_index
        self.btn_toggle_points.setText(
            "Show Data 0-3" if self._block_index == 1 else "Show Data 4-7"
        )
        # Update labels for the new block
        self._update_labels()

    def apply_values(self, values: list):
        """Update plots from incoming rtDataUpdated(values).
        Uses indices 0..3 or 4..7 depending on toggle state.
        """
        if not values or len(values) < 4:
            return
        base = 4 * self._block_index
        # Ensure we have enough values for selected block
        if len(values) < base + 4:
            return
        # Use actual wall-clock time instead of synthetic increments
        if self._real_data_t0 is None:
            self._real_data_t0 = time.time()
        t_next = time.time() - self._real_data_t0
        self.t_vals.append(t_next)
        # Map to curves
        self.top_cmd_vals.append(values[base + 0])
        self.top_meas_vals.append(values[base + 1])
        self.bot_a_vals.append(values[base + 2])
        self.bot_b_vals.append(values[base + 3])
        # Update
        self.curve_top_cmd.setData(self.t_vals, self.top_cmd_vals)
        self.curve_top_meas.setData(self.t_vals, self.top_meas_vals)
        self.curve_bot_a.setData(self.t_vals, self.bot_a_vals)
        self.curve_bot_b.setData(self.t_vals, self.bot_b_vals)

    # Update callback
    def _on_tick(self):
        t = time.time() - self.t0
        # Simulate smooth signals (replace later with real-time data)
        # Top: controller is a sine, measurement lags and has noise
        top_cmd = 0.7 * math.sin(2 * math.pi * 0.5 * t)
        top_meas = 0.7 * math.sin(2 * math.pi * 0.5 * (t - 0.05)) + 0.05 * math.sin(2 * math.pi * 4 * t)
        # Bottom: signals with different frequencies
        bot_a = 0.5 * math.sin(2 * math.pi * 0.2 * t)
        bot_b = 0.5 * math.cos(2 * math.pi * 0.35 * t)

        self.t_vals.append(t)
        self.top_cmd_vals.append(top_cmd)
        self.top_meas_vals.append(top_meas)
        self.bot_a_vals.append(bot_a)
        self.bot_b_vals.append(bot_b)

        # Update curves (x as t in seconds)
        # For speed: setData with lists (pyqtgraph accepts list/ndarray)
        self.curve_top_cmd.setData(self.t_vals, self.top_cmd_vals)
        self.curve_top_meas.setData(self.t_vals, self.top_meas_vals)
        self.curve_bot_a.setData(self.t_vals, self.bot_a_vals)
        self.curve_bot_b.setData(self.t_vals, self.bot_b_vals)


# Standalone demo
def _demo():
    app = QtWidgets.QApplication(sys.argv)
    w = ActiveTrialPage()
    w.resize(900, 600)
    w.show()
    w.start_sim()
    sys.exit(app.exec())


if __name__ == "__main__":
    _demo()

