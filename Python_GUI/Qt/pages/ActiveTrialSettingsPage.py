try:
    from PySide6 import QtCore, QtWidgets
except ImportError as e:
    raise SystemExit("PySide6 is required. Install with: pip install PySide6") from e


class ActiveTrialSettingsPage(QtWidgets.QWidget):
    """Settings page to manually enter controller/parameter/value without text fields.
    Uses only spinboxes and checkboxes to avoid on-screen keyboard usage.
    """

    applyRequested = QtCore.Signal(list)  # [isBilateral, joint, controller, parameter, value]
    cancelRequested = QtCore.Signal()

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setObjectName("ActiveTrialSettingsPage")
        self._controller_matrix: list[list[str]] = []
        self._build_ui()

    def _build_ui(self):
        layout = QtWidgets.QVBoxLayout(self)
        layout.setContentsMargins(16, 16, 16, 16)
        layout.setSpacing(12)

        title = QtWidgets.QLabel("Update Controller Settings")
        title.setAlignment(QtCore.Qt.AlignCenter)
        f = title.font(); f.setPointSize(22); title.setFont(f)
        layout.addWidget(title)

        # Controller/parameter matrix viewer
        self.table = QtWidgets.QTableWidget()
        self.table.setEditTriggers(QtWidgets.QAbstractItemView.NoEditTriggers)
        self.table.setSelectionBehavior(QtWidgets.QAbstractItemView.SelectRows)
        self.table.setSelectionMode(QtWidgets.QAbstractItemView.SingleSelection)
        tf = self.table.font(); tf.setPointSize(16); self.table.setFont(tf)
        self.table.verticalHeader().setDefaultSectionSize(44)
        self.table.horizontalHeader().setDefaultSectionSize(140)
        # Auto-populate controller/parameter on selection
        self.table.cellClicked.connect(self._on_cell_clicked)
        layout.addWidget(self.table, 1)

        # Controls area
        form = QtWidgets.QGridLayout()
        row = 0

        self.chk_bilateral = QtWidgets.QCheckBox("Bilateral mode")
        bf = self.chk_bilateral.font(); bf.setPointSize(18); self.chk_bilateral.setFont(bf)
        form.addWidget(self.chk_bilateral, row, 0, 1, 2)
        row += 1

        lbl_joint = QtWidgets.QLabel("Joint (1-6)")
        lf = lbl_joint.font(); lf.setPointSize(18); lbl_joint.setFont(lf)
        self.spin_joint = QtWidgets.QSpinBox()
        self.spin_joint.setRange(1, 6)
        self.spin_joint.setValue(1)
        sf = self.spin_joint.font(); sf.setPointSize(18); self.spin_joint.setFont(sf)
        self.spin_joint.setMinimumHeight(56)
        form.addWidget(lbl_joint, row, 0)
        form.addWidget(self.spin_joint, row, 1)
        row += 1

        lbl_controller = QtWidgets.QLabel("Controller")
        lcf = lbl_controller.font(); lcf.setPointSize(18); lbl_controller.setFont(lcf)
        self.combo_controller = QtWidgets.QComboBox()
        ccf = self.combo_controller.font(); ccf.setPointSize(18); self.combo_controller.setFont(ccf)
        self.combo_controller.setMinimumHeight(56)
        self.combo_controller.currentIndexChanged.connect(self._on_controller_changed)
        form.addWidget(lbl_controller, row, 0)
        form.addWidget(self.combo_controller, row, 1)
        row += 1

        lbl_param = QtWidgets.QLabel("Parameter")
        lpf = lbl_param.font(); lpf.setPointSize(18); lbl_param.setFont(lpf)
        self.combo_param = QtWidgets.QComboBox()
        cpf = self.combo_param.font(); cpf.setPointSize(18); self.combo_param.setFont(cpf)
        self.combo_param.setMinimumHeight(56)
        form.addWidget(lbl_param, row, 0)
        form.addWidget(self.combo_param, row, 1)
        row += 1

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

        # Wire buttons
        self.btn_apply.clicked.connect(self._on_apply)
        self.btn_cancel.clicked.connect(self.cancelRequested.emit)

    def set_controller_matrix(self, matrix: list):
        """Populate the table with a 2D matrix: [ [controller, p1, p2, ...], ... ]."""
        try:
            self._controller_matrix = list(matrix) if matrix else []
        except Exception:
            self._controller_matrix = []

        # Determine max columns
        max_cols = 0
        for row in self._controller_matrix:
            if len(row) > max_cols:
                max_cols = len(row)
        if max_cols == 0:
            max_cols = 1

        self.table.clear()
        self.table.setRowCount(len(self._controller_matrix))
        self.table.setColumnCount(max_cols)

        headers = ["Controller"] + [f"Param {i}" for i in range(1, max_cols)]
        self.table.setHorizontalHeaderLabels(headers)

        for r, data in enumerate(self._controller_matrix):
            for c in range(max_cols):
                text = data[c] if c < len(data) else ""
                item = QtWidgets.QTableWidgetItem(str(text))
                self.table.setItem(r, c, item)

        self.table.resizeColumnsToContents()

        # Also populate controller combo with names from column 0
        try:
            self.combo_controller.blockSignals(True)
            self.combo_controller.clear()
            names = [str(row[0]) for row in self._controller_matrix if row]
            if not names:
                names = ["(none)"]
            self.combo_controller.addItems(names)
            # Trigger parameter load for the first controller
            self._on_controller_changed(self.combo_controller.currentIndex())
        except Exception:
            pass
        finally:
            try:
                self.combo_controller.blockSignals(False)
            except Exception:
                pass

    @QtCore.Slot()
    def _on_apply(self):
        is_bilateral = self.chk_bilateral.isChecked()
        joint = int(self.spin_joint.value())
        controller = int(self.combo_controller.currentIndex())
        parameter = int(self.combo_param.currentIndex())
        value = float(self.spin_value.value())
        payload = [is_bilateral, joint, controller, parameter, value]
        self.applyRequested.emit(payload)

    @QtCore.Slot(int, int)
    def _on_cell_clicked(self, row: int, column: int):
        # Map row to controller index; column>0 maps to parameter index (column-1)
        try:
            self.combo_controller.setCurrentIndex(int(row))
            param_index = max(0, int(column) - 1)
            self.combo_param.setCurrentIndex(param_index)
        except Exception:
            pass

    @QtCore.Slot(int)
    def _on_controller_changed(self, idx: int):
        # Populate parameters combo from selected controller row
        try:
            self.combo_param.blockSignals(True)
            self.combo_param.clear()
            if 0 <= idx < len(self._controller_matrix):
                row = self._controller_matrix[idx]
                # row[0] is controller name; parameters begin at index 1
                params = row[1:] if len(row) > 1 else []
                if not params:
                    params = ["(no params)"]
                self.combo_param.addItems([str(p) for p in params])
            else:
                self.combo_param.addItem("(no params)")
        except Exception:
            pass
        finally:
            try:
                self.combo_param.blockSignals(False)
            except Exception:
                pass


