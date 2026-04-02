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

from utils import UIConfig, create_separator, create_section_label, style_button


class BioFeedbackPage(QtWidgets.QWidget):
    """Biofeedback page for FSR targets and live monitoring."""

    backRequested = QtCore.Signal()
    deviceStartRequested = QtCore.Signal()
    deviceStopRequested = QtCore.Signal()
    recalibrateFSRRequested = QtCore.Signal()
    markTrialRequested = QtCore.Signal()

    _LEG_FSR_INDEX = {
        "Left Leg": 7,
        "Right Leg": 5,
    }

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setObjectName("BioFeedbackPage")
        self._is_plotting = False
        self._is_paused = False
        self._target_value = None
        self._targets_reached = 0
        self._above_goal = False
        self._current_leg = "Left Leg"
        self._left_fsr_index = self._LEG_FSR_INDEX["Left Leg"]
        self._right_fsr_index = self._LEG_FSR_INDEX["Right Leg"]

        self._t0 = time.time()
        self._max_points = 200
        self._t_vals = deque(maxlen=self._max_points)
        self._fsr_vals = deque(maxlen=self._max_points)
        self._target_vals = deque(maxlen=self._max_points)

        self._build_ui()

    def _build_ui(self):
        layout = QtWidgets.QVBoxLayout(self)
        layout.setContentsMargins(UIConfig.MARGIN_PAGE, UIConfig.MARGIN_PAGE, UIConfig.MARGIN_PAGE, UIConfig.MARGIN_PAGE)
        layout.setSpacing(UIConfig.SPACING_LARGE)

        header = QtWidgets.QHBoxLayout()
        self.btn_back = QtWidgets.QPushButton("Back")
        style_button(self.btn_back, height=UIConfig.BTN_HEIGHT_MEDIUM, width=UIConfig.BTN_WIDTH_SMALL,
                     font_size=UIConfig.FONT_MEDIUM, padding="6px 12px")
        header.addWidget(self.btn_back)

        title = QtWidgets.QLabel("Biofeedback")
        f = title.font()
        f.setPointSize(UIConfig.FONT_TITLE)
        title.setFont(f)
        title.setAlignment(QtCore.Qt.AlignCenter)
        header.addStretch(1)
        header.addWidget(title, 1)
        header.addStretch(1)

        self.btn_pause = QtWidgets.QPushButton("Pause")
        style_button(self.btn_pause, height=UIConfig.BTN_HEIGHT_MEDIUM, width=UIConfig.BTN_WIDTH_SMALL,
                     font_size=UIConfig.FONT_MEDIUM, padding="6px 12px")
        header.addWidget(self.btn_pause)

        layout.addLayout(header)
        layout.addWidget(create_separator())

        self.plot = pg.PlotWidget()
        self.plot.showGrid(x=True, y=True, alpha=0.3)
        self.plot.setLabel("left", "FSR Value")
        self.plot.setLabel("bottom", "t (s)")
        self.plot.addLegend()
        self._curve_fsr = self.plot.plot(pen=pg.mkPen('b', width=2), name="FSR")
        self._curve_target = self.plot.plot(pen=pg.mkPen('r', width=2, style=QtCore.Qt.DashLine), name="Target")
        layout.addWidget(self.plot, 1)

        info_row = QtWidgets.QHBoxLayout()
        self.lbl_targets = QtWidgets.QLabel("Targets Reached: 0")
        self.lbl_target_value = QtWidgets.QLabel("No target set")
        info_row.addWidget(self.lbl_targets)
        info_row.addStretch(1)
        info_row.addWidget(self.lbl_target_value)
        layout.addLayout(info_row)

        layout.addWidget(create_separator())
        layout.addWidget(create_section_label("Controls"))

        controls = QtWidgets.QHBoxLayout()
        self.btn_set_target = QtWidgets.QPushButton("Set Target Value")
        self.btn_reset_target = QtWidgets.QPushButton("Reset Target")
        self.btn_reset_target.setEnabled(False)
        style_button(self.btn_set_target, height=UIConfig.BTN_HEIGHT_MEDIUM, width=UIConfig.BTN_WIDTH_MEDIUM,
                     font_size=UIConfig.FONT_MEDIUM, padding="6px 12px")
        style_button(self.btn_reset_target, height=UIConfig.BTN_HEIGHT_MEDIUM, width=UIConfig.BTN_WIDTH_MEDIUM,
                     font_size=UIConfig.FONT_MEDIUM, padding="6px 12px")
        controls.addWidget(self.btn_set_target)
        controls.addWidget(self.btn_reset_target)
        controls.addStretch(1)
        layout.addLayout(controls)

        advanced = QtWidgets.QHBoxLayout()
        self.btn_toggle_leg = QtWidgets.QPushButton(self._current_leg)
        self.btn_mark = QtWidgets.QPushButton("Mark Trial (0)")
        self.btn_recal_fsr = QtWidgets.QPushButton("Recalibrate FSRs")
        style_button(self.btn_toggle_leg, height=UIConfig.BTN_HEIGHT_MEDIUM, width=UIConfig.BTN_WIDTH_SMALL,
                     font_size=UIConfig.FONT_MEDIUM, padding="6px 12px")
        style_button(self.btn_mark, height=UIConfig.BTN_HEIGHT_MEDIUM, width=UIConfig.BTN_WIDTH_SMALL,
                     font_size=UIConfig.FONT_MEDIUM, padding="6px 12px")
        style_button(self.btn_recal_fsr, height=UIConfig.BTN_HEIGHT_MEDIUM, width=UIConfig.BTN_WIDTH_SMALL,
                     font_size=UIConfig.FONT_MEDIUM, padding="6px 12px")
        advanced.addWidget(self.btn_toggle_leg)
        advanced.addWidget(self.btn_mark)
        advanced.addWidget(self.btn_recal_fsr)
        advanced.addStretch(1)
        advanced.addWidget(QtWidgets.QLabel("Left FSR Index"))
        self.spin_left_fsr = QtWidgets.QSpinBox()
        self.spin_left_fsr.setRange(0, 255)
        self.spin_left_fsr.setValue(self._left_fsr_index)
        self.spin_left_fsr.setButtonSymbols(QtWidgets.QAbstractSpinBox.UpDownArrows)
        self.spin_left_fsr.setMinimumHeight(UIConfig.BTN_HEIGHT_LARGE)
        self.spin_left_fsr.setMinimumWidth(90)
        self.spin_left_fsr.setStyleSheet("QSpinBox::up-button, QSpinBox::down-button { width: 24px; }")
        advanced.addWidget(self.spin_left_fsr)
        advanced.addWidget(QtWidgets.QLabel("Right FSR Index"))
        self.spin_right_fsr = QtWidgets.QSpinBox()
        self.spin_right_fsr.setRange(0, 255)
        self.spin_right_fsr.setValue(self._right_fsr_index)
        self.spin_right_fsr.setButtonSymbols(QtWidgets.QAbstractSpinBox.UpDownArrows)
        self.spin_right_fsr.setMinimumHeight(UIConfig.BTN_HEIGHT_LARGE)
        self.spin_right_fsr.setMinimumWidth(90)
        self.spin_right_fsr.setStyleSheet("QSpinBox::up-button, QSpinBox::down-button { width: 24px; }")
        advanced.addWidget(self.spin_right_fsr)
        layout.addLayout(advanced)

        self.btn_back.clicked.connect(self.backRequested.emit)
        self.btn_pause.clicked.connect(self._toggle_pause)
        self.btn_set_target.clicked.connect(self._on_set_target)
        self.btn_reset_target.clicked.connect(self._on_reset_target)
        self.btn_toggle_leg.clicked.connect(self._toggle_leg)
        self.btn_mark.clicked.connect(self.markTrialRequested.emit)
        self.btn_recal_fsr.clicked.connect(self.recalibrateFSRRequested.emit)
        self.spin_left_fsr.valueChanged.connect(self._on_fsr_index_changed)
        self.spin_right_fsr.valueChanged.connect(self._on_fsr_index_changed)

    def start_plotting(self):
        self._is_plotting = True
        self._t0 = time.time()
        self._reset_plot_data()

    def stop_plotting(self):
        self._is_plotting = False

    def update_mark_count(self, count: int):
        try:
            self.btn_mark.setText(f"Mark Trial ({count})")
        except Exception:
            pass

    def apply_values(self, values: list):
        if not self._is_plotting or not values:
            return
        idx = self._left_fsr_index if self._current_leg == "Left Leg" else self._right_fsr_index
        if len(values) <= idx:
            return
        try:
            fsr_val = float(values[idx])
        except Exception:
            return

        t_next = time.time() - self._t0
        self._t_vals.append(t_next)
        self._fsr_vals.append(fsr_val)

        if self._target_value is not None:
            self._target_vals.append(self._target_value)
        else:
            self._target_vals.append(float("nan"))

        self._curve_fsr.setData(self._t_vals, self._fsr_vals)
        self._curve_target.setData(self._t_vals, self._target_vals)

        if self._target_value is not None:
            if fsr_val > self._target_value and not self._above_goal:
                self._above_goal = True
                self._targets_reached += 1
                self._update_targets_label()
                QtWidgets.QApplication.beep()
                self._flash_success()
            elif fsr_val <= self._target_value:
                self._above_goal = False

    def _toggle_pause(self):
        if not self._is_paused:
            self._is_paused = True
            self.btn_pause.setText("Play")
            self.deviceStopRequested.emit()
        else:
            self._is_paused = False
            self.btn_pause.setText("Pause")
            self.deviceStartRequested.emit()

    def _on_set_target(self):
        value, ok = QtWidgets.QInputDialog.getDouble(
            self,
            "Set Target Value",
            "Target value:",
            0.5,
            -100000.0,
            100000.0,
            3
        )
        if ok:
            self._target_value = float(value)
            self._update_target_label()
            self.btn_reset_target.setEnabled(True)

    def _on_reset_target(self):
        self._target_value = None
        self._update_target_label()
        self.btn_reset_target.setEnabled(False)
        self._above_goal = False

    def _toggle_leg(self):
        self._current_leg = "Right Leg" if self._current_leg == "Left Leg" else "Left Leg"
        self.btn_toggle_leg.setText(self._current_leg)
        self._reset_plot_data()
    
    def _on_fsr_index_changed(self):
        self._left_fsr_index = int(self.spin_left_fsr.value())
        self._right_fsr_index = int(self.spin_right_fsr.value())

    def _reset_plot_data(self):
        self._t_vals.clear()
        self._fsr_vals.clear()
        self._target_vals.clear()
        self._curve_fsr.setData([], [])
        self._curve_target.setData([], [])

    def _update_target_label(self):
        if self._target_value is None:
            self.lbl_target_value.setText("No target set")
        else:
            self.lbl_target_value.setText(f"Target value: {self._target_value:.3f}")

    def _update_targets_label(self):
        self.lbl_targets.setText(f"Targets Reached: {self._targets_reached}")

    def _flash_success(self):
        window = self.window()
        if window:
            window.setStyleSheet("QMainWindow, QWidget { background-color: #2e7d32; }")
        else:
            self.setStyleSheet("background-color: #2e7d32;")

        def _clear():
            if window:
                window.setStyleSheet("")
            else:
                self.setStyleSheet("")

        QtCore.QTimer.singleShot(600, _clear)

