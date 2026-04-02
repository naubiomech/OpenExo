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
    Uses raw joint/controller IDs and parameter index with a bilateral toggle.
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
            "joint_id": 0,
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

        lbl_joint = QtWidgets.QLabel("Joint ID")
        lf = lbl_joint.font(); lf.setPointSize(18); lbl_joint.setFont(lf)
        self.spin_joint_id = QtWidgets.QSpinBox()
        jf = self.spin_joint_id.font(); jf.setPointSize(18); self.spin_joint_id.setFont(jf)
        self.spin_joint_id.setMinimumHeight(56)
        self.spin_joint_id.setRange(0, 255)
        self.spin_joint_id.setButtonSymbols(QtWidgets.QAbstractSpinBox.UpDownArrows)
        self.spin_joint_id.setMinimumWidth(90)
        self.spin_joint_id.setStyleSheet("QSpinBox::up-button, QSpinBox::down-button { width: 24px; }")
        form.addWidget(lbl_joint, row, 0)
        form.addWidget(self.spin_joint_id, row, 1)
        row += 1

        # Controller index
        lbl_controller = QtWidgets.QLabel("Controller ID")
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
        self.spin_value.setButtonSymbols(QtWidgets.QAbstractSpinBox.UpDownArrows)
        self.spin_value.setMinimumWidth(90)
        self.spin_value.setStyleSheet("QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { width: 24px; }")
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
        import os
        base_dir = os.path.dirname(os.path.dirname(__file__))  # Qt directory
        settings_file = os.path.join(base_dir, "Saved_Data", "gui_settings.txt")
        try:
            if os.path.exists(settings_file):
                with open(settings_file, 'r') as f:
                    for line in f.readlines():
                        if line.startswith("bilateral="):
                            self._bilateral_state = line.split("=")[1].strip() == "True"
                            self._last_selection["bilateral"] = self._bilateral_state
                            print(f"[BasicSettings] Loaded bilateral state: {self._bilateral_state}")
                        elif line.startswith("last_basic_joint_id="):
                            try:
                                self._last_selection["joint_id"] = int(line.split("=")[1].strip())
                            except Exception:
                                pass
                        elif line.startswith("last_basic_joint="):
                            # Backward-compatible: map legacy labels to raw IDs.
                            legacy = line.split("=")[1].strip()
                            self._last_selection["joint"] = legacy
                            legacy_key = legacy.lower()
                            legacy_map = {
                                "left hip": 65,
                                "right hip": 33,
                                "left knee": 66,
                                "right knee": 34,
                                "left ankle": 68,
                                "right ankle": 36,
                                "left elbow": 72,
                                "right elbow": 40,
                            }
                            self._last_selection["joint_id"] = legacy_map.get(legacy_key, 0)
                        elif line.startswith("last_basic_controller="):
                            try:
                                self._last_selection["controller"] = int(line.split("=")[1].strip())
                            except:
                                pass
                        elif line.startswith("last_basic_parameter="):
                            try:
                                self._last_selection["parameter"] = int(line.split("=")[1].strip())
                            except:
                                pass
                        elif line.startswith("last_basic_value="):
                            try:
                                self._last_selection["value"] = float(line.split("=")[1].strip())
                            except:
                                pass
        except Exception as e:
            print(f"Error loading basic settings: {e}")

    def _save_settings(self):
        """Save all settings to file."""
        import os
        base_dir = os.path.dirname(os.path.dirname(__file__))  # Qt directory
        save_dir = os.path.join(base_dir, "Saved_Data")
        settings_file = os.path.join(save_dir, "gui_settings.txt")
        try:
            os.makedirs(save_dir, exist_ok=True)
            # Read existing settings
            existing = {}
            if os.path.exists(settings_file):
                with open(settings_file, 'r') as f:
                    for line in f.readlines():
                        if '=' in line:
                            key, val = line.strip().split('=', 1)
                            existing[key] = val
            # Update with current values
            existing["bilateral"] = str(self._bilateral_state)
            existing["last_basic_joint_id"] = str(self._last_selection.get("joint_id", 0))
            if hasattr(self, "combo_joint"):
                existing["last_basic_joint"] = str(self._last_selection.get("joint", "Left hip"))
            existing["last_basic_controller"] = str(self._last_selection.get("controller", 0))
            existing["last_basic_parameter"] = str(self._last_selection.get("parameter", 0))
            existing["last_basic_value"] = str(self._last_selection.get("value", 0.0))
            # Write all settings
            with open(settings_file, 'w') as f:
                for key, val in existing.items():
                    f.write(f"{key}={val}\n")
            print(f"[BasicSettings] Saved settings to {settings_file}")
        except Exception as e:
            print(f"Error saving settings: {e}")
    
    def _restore_last_selection(self):
        """Restore UI controls to last saved selection."""
        try:
            # Restore bilateral checkbox
            bilateral = self._last_selection.get("bilateral", False)
            self.chk_bilateral.setChecked(bilateral)
            
            # Restore joint selection
            if hasattr(self, "combo_joint"):
                joint_name = self._last_selection.get("joint", "Left hip")
                idx = self.combo_joint.findText(joint_name)
                if idx >= 0:
                    self.combo_joint.setCurrentIndex(idx)
            else:
                joint_id = int(self._last_selection.get("joint_id", 0))
                self.spin_joint_id.setValue(joint_id)
            
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
        if hasattr(self, "combo_joint"):
            joint_name = self.combo_joint.currentText()
            joint_id = self._joint_name_to_index.get(joint_name, 0)
        else:
            joint_name = self._last_selection.get("joint", "Left hip")
            joint_id = int(self.spin_joint_id.value())
        controller = int(self.combo_controller.currentText())
        parameter = int(self.combo_param.currentText())
        value = float(self.spin_value.value())
        payload = [is_bilateral, joint_id, controller, parameter, value]
        
        # Save last selection for next time
        self._last_selection = {
            "bilateral": is_bilateral,
            "joint": joint_name,
            "joint_id": joint_id,
            "controller": controller,
            "parameter": parameter,
            "value": value,
        }
        self._save_settings()
        
        self.applyRequested.emit(payload)

