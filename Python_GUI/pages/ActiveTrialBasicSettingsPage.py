try:
    from PySide6 import QtCore, QtWidgets
except ImportError as e:
    raise SystemExit("PySide6 is required. Install with: pip install PySide6") from e

from utils import (
    UIConfig, JointConfig, SettingsManager,
    style_button, style_combo_box, style_spinbox
)


class ActiveTrialBasicSettingsPage(QtWidgets.QWidget):
    """Fallback settings page shown when no controller metadata is available.
    Uses dropdowns for joint, controller index, and parameter index (legacy-style naming),
    plus bilateral toggle and numeric value input. No text fields to avoid keyboard.
    """

    applyRequested = QtCore.Signal(list)  # [isBilateral, joint, controller, parameter, value]
    cancelRequested = QtCore.Signal()

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setObjectName("ActiveTrialBasicSettingsPage")
        self._bilateral_state = False
        self._last_selection = {
            "bilateral": False,
            "joint": "Left hip",
            "controller": 0,
            "parameter": 0,
            "value": 0.0,
        }
        self._joint_names = JointConfig.JOINT_NAMES
        self._joint_name_to_index = JointConfig.NAME_TO_INDEX
        self._build_ui()
        self._load_settings()
        self._restore_last_selection()

    def _build_ui(self):
        layout = QtWidgets.QVBoxLayout(self)
        layout.setContentsMargins(UIConfig.MARGIN_FORM, UIConfig.MARGIN_FORM, UIConfig.MARGIN_FORM, UIConfig.MARGIN_FORM)
        layout.setSpacing(UIConfig.SPACING_XXLARGE)

        title = QtWidgets.QLabel("Update Controller Settings (Basic)")
        title.setAlignment(QtCore.Qt.AlignCenter)
        f = title.font(); f.setPointSize(UIConfig.FONT_TITLE); title.setFont(f)
        layout.addWidget(title)

        form = QtWidgets.QGridLayout()
        row = 0

        # Bilateral
        self.chk_bilateral = QtWidgets.QCheckBox("Bilateral mode")
        bf = self.chk_bilateral.font(); bf.setPointSize(UIConfig.FONT_LARGE); self.chk_bilateral.setFont(bf)
        self.chk_bilateral.setChecked(self._bilateral_state)
        self.chk_bilateral.stateChanged.connect(self._on_bilateral_changed)
        form.addWidget(self.chk_bilateral, row, 0, 1, 2)
        row += 1

        # Joints with legacy naming
        lbl_joint = QtWidgets.QLabel("Joint")
        lf = lbl_joint.font(); lf.setPointSize(UIConfig.FONT_LARGE); lbl_joint.setFont(lf)
        self.combo_joint = QtWidgets.QComboBox()
        style_combo_box(self.combo_joint, height=UIConfig.BTN_HEIGHT_XLARGE, font_size=UIConfig.FONT_LARGE)
        self.combo_joint.addItems(self._joint_names)
        form.addWidget(lbl_joint, row, 0)
        form.addWidget(self.combo_joint, row, 1)
        row += 1

        # Controller index
        lbl_controller = QtWidgets.QLabel("Controller Index")
        lcf = lbl_controller.font(); lcf.setPointSize(UIConfig.FONT_LARGE); lbl_controller.setFont(lcf)
        self.combo_controller = QtWidgets.QComboBox()
        style_combo_box(self.combo_controller, height=UIConfig.BTN_HEIGHT_XLARGE, font_size=UIConfig.FONT_LARGE)
        # Provide a reasonable index range (0..50)
        self.combo_controller.addItems([str(i) for i in range(0, 51)])
        form.addWidget(lbl_controller, row, 0)
        form.addWidget(self.combo_controller, row, 1)
        row += 1

        # Parameter index
        lbl_param = QtWidgets.QLabel("Parameter Index")
        lpf = lbl_param.font(); lpf.setPointSize(UIConfig.FONT_LARGE); lbl_param.setFont(lpf)
        self.combo_param = QtWidgets.QComboBox()
        style_combo_box(self.combo_param, height=UIConfig.BTN_HEIGHT_XLARGE, font_size=UIConfig.FONT_LARGE)
        self.combo_param.addItems([str(i) for i in range(0, 51)])
        form.addWidget(lbl_param, row, 0)
        form.addWidget(self.combo_param, row, 1)
        row += 1

        # Value
        lbl_value = QtWidgets.QLabel("Value")
        lvf = lbl_value.font(); lvf.setPointSize(UIConfig.FONT_LARGE); lbl_value.setFont(lvf)
        self.spin_value = QtWidgets.QDoubleSpinBox()
        self.spin_value.setDecimals(4)
        self.spin_value.setRange(-100000.0, 100000.0)
        self.spin_value.setSingleStep(0.1)
        self.spin_value.setValue(0.0)
        style_spinbox(self.spin_value, height=UIConfig.BTN_HEIGHT_XLARGE, font_size=UIConfig.FONT_LARGE)
        form.addWidget(lbl_value, row, 0)
        form.addWidget(self.spin_value, row, 1)

        layout.addLayout(form)

        # Buttons
        btn_row = QtWidgets.QHBoxLayout()
        self.btn_apply = QtWidgets.QPushButton("Apply")
        self.btn_cancel = QtWidgets.QPushButton("Cancel")
        style_button(self.btn_apply, height=UIConfig.BTN_HEIGHT_XLARGE, width=UIConfig.BTN_WIDTH_MEDIUM,
                    font_size=UIConfig.FONT_LARGE, padding="10px 16px")
        style_button(self.btn_cancel, height=UIConfig.BTN_HEIGHT_XLARGE, width=UIConfig.BTN_WIDTH_MEDIUM,
                    font_size=UIConfig.FONT_LARGE, padding="10px 16px")
        btn_row.addStretch(1)
        btn_row.addWidget(self.btn_cancel)
        btn_row.addWidget(self.btn_apply)
        layout.addLayout(btn_row)

        # Wire
        self.btn_apply.clicked.connect(self._on_apply)
        self.btn_cancel.clicked.connect(self.cancelRequested.emit)

    def _load_settings(self):
        """Load all settings from file."""
        try:
            self._bilateral_state = SettingsManager.get_bool("bilateral", False)
            self._last_selection["bilateral"] = self._bilateral_state
            print(f"[BasicSettings] Loaded bilateral state: {self._bilateral_state}")
            
            joint = SettingsManager.get_setting("last_basic_joint", "Left hip")
            self._last_selection["joint"] = joint
            
            controller = SettingsManager.get_int("last_basic_controller", 0)
            self._last_selection["controller"] = controller
            
            parameter = SettingsManager.get_int("last_basic_parameter", 0)
            self._last_selection["parameter"] = parameter
            
            value = SettingsManager.get_float("last_basic_value", 0.0)
            self._last_selection["value"] = value
        except Exception as e:
            print(f"Error loading basic settings: {e}")

    def _save_settings(self):
        """Save all settings to file."""
        try:
            updates = {
                "bilateral": str(self._bilateral_state),
                "last_basic_joint": str(self._last_selection.get("joint", "Left hip")),
                "last_basic_controller": str(self._last_selection.get("controller", 0)),
                "last_basic_parameter": str(self._last_selection.get("parameter", 0)),
                "last_basic_value": str(self._last_selection.get("value", 0.0)),
            }
            SettingsManager.update_settings(updates)
            print(f"[BasicSettings] Saved settings")
        except Exception as e:
            print(f"Error saving settings: {e}")
    
    def _restore_last_selection(self):
        """Restore UI controls to last saved selection."""
        try:
            # Restore bilateral checkbox
            bilateral = self._last_selection.get("bilateral", False)
            self.chk_bilateral.setChecked(bilateral)
            
            # Restore joint selection
            joint_name = self._last_selection.get("joint", "Left hip")
            idx = self.combo_joint.findText(joint_name)
            if idx >= 0:
                self.combo_joint.setCurrentIndex(idx)
            
            # Restore controller selection
            controller = self._last_selection.get("controller", 0)
            if controller < self.combo_controller.count():
                self.combo_controller.setCurrentIndex(controller)
            
            # Restore parameter selection
            parameter = self._last_selection.get("parameter", 0)
            if parameter < self.combo_param.count():
                self.combo_param.setCurrentIndex(parameter)
            
            # Restore value
            value = self._last_selection.get("value", 0.0)
            self.spin_value.setValue(value)
            
            print(f"[BasicSettings] Restored last selection: {self._last_selection}")
        except Exception as e:
            print(f"Error restoring last selection: {e}")

    @QtCore.Slot(int)
    def _on_bilateral_changed(self, state):
        """Called when bilateral checkbox changes."""
        self._bilateral_state = bool(state)
        self._save_settings()

    @QtCore.Slot()
    def _on_apply(self):
        is_bilateral = self.chk_bilateral.isChecked()
        # Map joint name to index per legacy mapping
        joint_name = self.combo_joint.currentText()
        joint_index = self._joint_name_to_index.get(joint_name, 1)
        controller = int(self.combo_controller.currentText())
        parameter = int(self.combo_param.currentText())
        value = float(self.spin_value.value())
        payload = [is_bilateral, joint_index, controller, parameter, value]
        
        # Save last selection for next time
        self._last_selection = {
            "bilateral": is_bilateral,
            "joint": joint_name,
            "controller": controller,
            "parameter": parameter,
            "value": value,
        }
        self._save_settings()
        
        self.applyRequested.emit(payload)


