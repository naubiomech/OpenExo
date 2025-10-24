try:
    from PySide6 import QtCore, QtWidgets
except ImportError as e:
    raise SystemExit("PySide6 is required. Install with: pip install PySide6") from e


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
        self._build_ui()

    def _build_ui(self):
        layout = QtWidgets.QVBoxLayout(self)
        layout.setContentsMargins(16, 16, 16, 16)
        layout.setSpacing(12)

        title = QtWidgets.QLabel("Update Controller Settings (Basic)")
        title.setAlignment(QtCore.Qt.AlignCenter)
        f = title.font(); f.setPointSize(22); title.setFont(f)
        layout.addWidget(title)

        form = QtWidgets.QGridLayout()
        row = 0

        # Bilateral
        self.chk_bilateral = QtWidgets.QCheckBox("Bilateral mode")
        bf = self.chk_bilateral.font(); bf.setPointSize(18); self.chk_bilateral.setFont(bf)
        form.addWidget(self.chk_bilateral, row, 0, 1, 2)
        row += 1

        # Joints with legacy naming
        lbl_joint = QtWidgets.QLabel("Joint")
        lf = lbl_joint.font(); lf.setPointSize(18); lbl_joint.setFont(lf)
        self.combo_joint = QtWidgets.QComboBox()
        jf = self.combo_joint.font(); jf.setPointSize(18); self.combo_joint.setFont(jf)
        self.combo_joint.setMinimumHeight(56)
        # Legacy joint names and mapping indices
        self._joint_names = [
            "Left hip",
            "Left knee",
            "Left ankle",
            "Left elbow",
            "Right hip",
            "Right knee",
            "Right ankle",
            "Right elbow",
        ]
        # Map to indices consistent with legacy mapping
        self._joint_name_to_index = {
            "Right hip": 1,
            "Left hip": 2,
            "Right knee": 3,
            "Left knee": 4,
            "Right ankle": 5,
            "Left ankle": 6,
            "Right elbow": 7,
            "Left elbow": 8,
        }
        self.combo_joint.addItems(self._joint_names)
        form.addWidget(lbl_joint, row, 0)
        form.addWidget(self.combo_joint, row, 1)
        row += 1

        # Controller index
        lbl_controller = QtWidgets.QLabel("Controller Index")
        lcf = lbl_controller.font(); lcf.setPointSize(18); lbl_controller.setFont(lcf)
        self.combo_controller = QtWidgets.QComboBox()
        ccf = self.combo_controller.font(); ccf.setPointSize(18); self.combo_controller.setFont(ccf)
        self.combo_controller.setMinimumHeight(56)
        # Provide a reasonable index range (0..50)
        self.combo_controller.addItems([str(i) for i in range(0, 51)])
        form.addWidget(lbl_controller, row, 0)
        form.addWidget(self.combo_controller, row, 1)
        row += 1

        # Parameter index
        lbl_param = QtWidgets.QLabel("Parameter Index")
        lpf = lbl_param.font(); lpf.setPointSize(18); lbl_param.setFont(lpf)
        self.combo_param = QtWidgets.QComboBox()
        cpf = self.combo_param.font(); cpf.setPointSize(18); self.combo_param.setFont(cpf)
        self.combo_param.setMinimumHeight(56)
        self.combo_param.addItems([str(i) for i in range(0, 51)])
        form.addWidget(lbl_param, row, 0)
        form.addWidget(self.combo_param, row, 1)
        row += 1

        # Value
        lbl_value = QtWidgets.QLabel("Value")
        lvf = lbl_value.font(); lvf.setPointSize(18); lbl_value.setFont(lvf)
        self.spin_value = QtWidgets.QDoubleSpinBox()
        self.spin_value.setDecimals(4)
        self.spin_value.setRange(-100000.0, 100000.0)
        self.spin_value.setSingleStep(0.1)
        self.spin_value.setValue(0.0)
        svf = self.spin_value.font(); svf.setPointSize(18); self.spin_value.setFont(svf)
        self.spin_value.setMinimumHeight(56)
        form.addWidget(lbl_value, row, 0)
        form.addWidget(self.spin_value, row, 1)

        layout.addLayout(form)

        # Buttons
        btn_row = QtWidgets.QHBoxLayout()
        self.btn_apply = QtWidgets.QPushButton("Apply")
        self.btn_cancel = QtWidgets.QPushButton("Cancel")
        for b in (self.btn_apply, self.btn_cancel):
            fb = b.font(); fb.setPointSize(18); b.setFont(fb)
            b.setMinimumHeight(56)
            b.setMinimumWidth(200)
            b.setStyleSheet("padding: 10px 16px;")
        btn_row.addStretch(1)
        btn_row.addWidget(self.btn_cancel)
        btn_row.addWidget(self.btn_apply)
        layout.addLayout(btn_row)

        # Wire
        self.btn_apply.clicked.connect(self._on_apply)
        self.btn_cancel.clicked.connect(self.cancelRequested.emit)

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
        self.applyRequested.emit(payload)


