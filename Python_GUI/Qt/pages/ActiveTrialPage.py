import sys
import math
import time
from collections import deque

try:
    from PySide6 import QtCore, QtWidgets
except ImportError as e:
    raise SystemExit("PySide6 is required. Install with: pip install PySide6") from e

try:
    import pyqtgraph as pg
except ImportError as e:
    raise SystemExit("pyqtgraph is required. Install with: pip install pyqtgraph") from e


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
    markTrialRequested = QtCore.Signal()
    deviceStartRequested = QtCore.Signal()
    deviceStopRequested = QtCore.Signal()

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setObjectName("ActiveTrialPage")
        self._build_ui()
        self._init_state()

    def _build_ui(self):
        # Main horizontal layout: left controls, right plots
        main = QtWidgets.QHBoxLayout(self)

        # Left controls column
        controls = QtWidgets.QVBoxLayout()
        title = QtWidgets.QLabel("Active Trial")
        f = title.font(); f.setPointSize(14); title.setFont(f)
        controls.addWidget(title)
        controls.addSpacing(8)

        # Toggle plotted data visibility
        self.btn_toggle_points = QtWidgets.QPushButton("Toggle Data Points")
        controls.addWidget(self.btn_toggle_points)
        controls.addSpacing(12)

        # End Trial
        self.btn_end_trial = QtWidgets.QPushButton("End Trial")
        controls.addWidget(self.btn_end_trial)
        # Save CSV (temp)
        self.btn_save_csv = QtWidgets.QPushButton("Save CSV")
        controls.addWidget(self.btn_save_csv)
        # Update Controller (temp)
        self.btn_update_controller = QtWidgets.QPushButton("Update Controller")
        controls.addWidget(self.btn_update_controller)
        # Bio Feedback (temp)
        self.btn_bio_feedback = QtWidgets.QPushButton("Bio Feedback")
        controls.addWidget(self.btn_bio_feedback)
        # Machine Learning (temp)
        self.btn_ml = QtWidgets.QPushButton("Machine Learning")
        controls.addWidget(self.btn_ml)
        # Recalibrate FSRs
        self.btn_recal_fsr = QtWidgets.QPushButton("Recalibrate FSRs")
        controls.addWidget(self.btn_recal_fsr)
        # Send Preset FSR Values
        self.btn_send_preset_fsr = QtWidgets.QPushButton("Send Preset FSR Values")
        controls.addWidget(self.btn_send_preset_fsr)
        # Mark Trial (temp)
        self.btn_mark = QtWidgets.QPushButton("Mark Trial")
        controls.addWidget(self.btn_mark)
        controls.addSpacing(12)

        # Start/Stop controls
        self.btn_start = QtWidgets.QPushButton("Start")
        self.btn_stop = QtWidgets.QPushButton("Stop")
        self.btn_stop.setEnabled(False)
        controls.addWidget(self.btn_start)
        controls.addWidget(self.btn_stop)
        controls.addStretch(1)

        # Right plots column
        plots_col = QtWidgets.QVBoxLayout()
        # Create a GraphicsLayoutWidget to host two plot rows
        self.graph = pg.GraphicsLayoutWidget()
        plots_col.addWidget(self.graph, 1)

        # Top plot (e.g., Controller vs Measurement)
        self.plot_top = self.graph.addPlot(row=0, col=0)
        self.plot_top.showGrid(x=True, y=True, alpha=0.3)
        self.plot_top.addLegend()
        self.plot_top.setLabel("left", "Top Signal")
        self.plot_top.setLabel("bottom", "t (s)")
        self.curve_top_cmd = self.plot_top.plot(pen=pg.mkPen('b', width=2), name='Controller')
        self.curve_top_meas = self.plot_top.plot(pen=pg.mkPen('r', width=2), name='Measurement')

        # Bottom plot (e.g., additional signals)
        self.plot_bottom = self.graph.addPlot(row=1, col=0)
        self.plot_bottom.showGrid(x=True, y=True, alpha=0.3)
        self.plot_bottom.addLegend()
        self.plot_bottom.setLabel("left", "Bottom Signal")
        self.plot_bottom.setLabel("bottom", "t (s)")
        self.curve_bot_a = self.plot_bottom.plot(pen=pg.mkPen('g', width=2), name='Signal A')
        self.curve_bot_b = self.plot_bottom.plot(pen=pg.mkPen('m', width=2), name='Signal B')

        # Assemble
        main.addLayout(controls, 0)
        main.addLayout(plots_col, 1)

        # Wiring (device control; sim controls available via methods if needed)
        self.btn_start.clicked.connect(self.deviceStartRequested.emit)
        self.btn_stop.clicked.connect(self.deviceStopRequested.emit)
        self.btn_toggle_points.clicked.connect(self._toggle_points)
        # Emit-only wiring; MainWindow can connect these to actual actions
        self.btn_end_trial.clicked.connect(self.endTrialRequested.emit)
        self.btn_save_csv.clicked.connect(self.saveCsvRequested.emit)
        self.btn_update_controller.clicked.connect(self.updateControllerRequested.emit)
        self.btn_bio_feedback.clicked.connect(self.bioFeedbackRequested.emit)
        self.btn_ml.clicked.connect(self.machineLearningRequested.emit)
        self.btn_recal_fsr.clicked.connect(self.recalibrateFSRRequested.emit)
        self.btn_send_preset_fsr.clicked.connect(self.sendPresetFSRRequested.emit)
        self.btn_mark.clicked.connect(self.markTrialRequested.emit)

        # Make buttons touch-friendly (iPad): larger font, padding, and height
        def _style(btn: QtWidgets.QPushButton):
            f = btn.font()
            f.setPointSize(16)
            btn.setFont(f)
            btn.setMinimumHeight(56)
            btn.setMinimumWidth(220)
            btn.setStyleSheet("padding: 12px 20px;")

        for b in (
            self.btn_toggle_points,
            self.btn_end_trial,
            self.btn_save_csv,
            self.btn_update_controller,
            self.btn_bio_feedback,
            self.btn_ml,
            self.btn_recal_fsr,
            self.btn_send_preset_fsr,
            self.btn_mark,
            self.btn_start,
            self.btn_stop,
        ):
            _style(b)

    def _init_state(self):
        # Fixed-size buffers for plotting (seconds-window * rate)
        self.rate_hz = 30
        self.window_secs = 10
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

    # Public API to integrate later with bridges
    def start_sim(self):
        if not self.timer.isActive():
            self.t0 = time.time()
            self.timer.start(int(1000 / self.rate_hz))
            self.btn_start.setEnabled(False)
            self.btn_stop.setEnabled(True)

    def stop_sim(self):
        if self.timer.isActive():
            self.timer.stop()
            self.btn_start.setEnabled(True)
            self.btn_stop.setEnabled(False)

    def _toggle_points(self):
        # Toggle which 4-value block we plot
        self._block_index = 1 - self._block_index
        self.btn_toggle_points.setText(
            "Show Data 0-3" if self._block_index == 1 else "Show Data 4-7"
        )

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
        # Advance synthetic time
        t_next = (self.t_vals[-1] + 1.0 / self.rate_hz) if self.t_vals else 0.0
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


