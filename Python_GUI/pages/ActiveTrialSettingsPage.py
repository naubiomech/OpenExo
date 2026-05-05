try:
    from PySide6 import QtCore, QtWidgets
except ImportError as e:
    raise SystemExit("PySide6 is required. Install with: pip install PySide6") from e

from utils import (
    UIConfig, JointConfig, SettingsManager,
    style_button, style_combo_box, style_spinbox,
    exo_config,
)
from utils.debug import dprint


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
        self._joint_controllers: dict = {}  # Maps joint name to list of controller indices
        # Live database of parameter values keyed by (joint_id:int, controller_id:int).
        # Populated from the firmware handshake (line 6 of each controller csv) and
        # mutated locally after every Apply.  Values are stored as strings so we can
        # render either ints or floats (firmware sends them as strings on the wire).
        self._controller_values: dict[tuple[int, int], list[str]] = {}
        # Maps a displayed table row (0-based) to the underlying controller
        # matrix index.  Re-built every time the table is re-rendered, so
        # cell clicks can map a tapped row back to a controller even though
        # each controller now spans two rows (names + values).
        self._row_to_controller: dict[int, int] = {}
        self._bilateral_state = False  # Store bilateral state
        self._last_selection = {
            "bilateral": False,
            "joint": None,
            "controller": None,
            "parameter": None,
            "value": 0.0,
        }
        self._build_ui()
        self._load_settings()  # Load saved settings

    def _build_ui(self):
        layout = QtWidgets.QVBoxLayout(self)
        layout.setContentsMargins(UIConfig.MARGIN_FORM, UIConfig.MARGIN_FORM, UIConfig.MARGIN_FORM, UIConfig.MARGIN_FORM)
        layout.setSpacing(UIConfig.SPACING_XXLARGE)

        title = QtWidgets.QLabel("Update Controller Settings")
        title.setAlignment(QtCore.Qt.AlignCenter)
        f = title.font(); f.setPointSize(UIConfig.FONT_TITLE); title.setFont(f)
        layout.addWidget(title)

        # Joint selector at the top
        joint_selector_layout = QtWidgets.QHBoxLayout()
        lbl_joint_selector = QtWidgets.QLabel("Select Joint:")
        ljsf = lbl_joint_selector.font(); ljsf.setPointSize(UIConfig.FONT_LARGE); lbl_joint_selector.setFont(ljsf)
        self.combo_joint = QtWidgets.QComboBox()
        style_combo_box(self.combo_joint, height=UIConfig.BTN_HEIGHT_XLARGE, font_size=UIConfig.FONT_LARGE)
        self.combo_joint.currentIndexChanged.connect(self._on_joint_changed)
        joint_selector_layout.addWidget(lbl_joint_selector)
        joint_selector_layout.addWidget(self.combo_joint, 1)
        layout.addLayout(joint_selector_layout)

        # Controller/parameter matrix viewer
        self.table = QtWidgets.QTableWidget()
        self.table.setEditTriggers(QtWidgets.QAbstractItemView.NoEditTriggers)
        self.table.setSelectionBehavior(QtWidgets.QAbstractItemView.SelectRows)
        self.table.setSelectionMode(QtWidgets.QAbstractItemView.SingleSelection)
        tf = self.table.font(); tf.setPointSize(UIConfig.FONT_SUBTITLE); self.table.setFont(tf)
        self.table.verticalHeader().setDefaultSectionSize(UIConfig.TABLE_ROW_HEIGHT)
        self.table.horizontalHeader().setDefaultSectionSize(UIConfig.TABLE_COL_WIDTH)
        # Auto-populate controller/parameter on selection
        self.table.cellClicked.connect(self._on_cell_clicked)
        layout.addWidget(self.table, 1)

        # Controls area
        form = QtWidgets.QGridLayout()
        row = 0

        self.chk_bilateral = QtWidgets.QCheckBox("Bilateral mode")
        bf = self.chk_bilateral.font(); bf.setPointSize(UIConfig.FONT_LARGE); self.chk_bilateral.setFont(bf)
        self.chk_bilateral.setChecked(self._bilateral_state)  # Load saved state
        self.chk_bilateral.stateChanged.connect(self._on_bilateral_changed)
        form.addWidget(self.chk_bilateral, row, 0, 1, 2)
        row += 1

        lbl_controller = QtWidgets.QLabel("Controller")
        lcf = lbl_controller.font(); lcf.setPointSize(UIConfig.FONT_LARGE); lbl_controller.setFont(lcf)
        self.combo_controller = QtWidgets.QComboBox()
        style_combo_box(self.combo_controller, height=UIConfig.BTN_HEIGHT_XLARGE, font_size=UIConfig.FONT_LARGE)
        self.combo_controller.currentIndexChanged.connect(self._on_controller_changed)
        form.addWidget(lbl_controller, row, 0)
        form.addWidget(self.combo_controller, row, 1)
        row += 1

        lbl_param = QtWidgets.QLabel("Parameter")
        lpf = lbl_param.font(); lpf.setPointSize(UIConfig.FONT_LARGE); lbl_param.setFont(lpf)
        self.combo_param = QtWidgets.QComboBox()
        style_combo_box(self.combo_param, height=UIConfig.BTN_HEIGHT_XLARGE, font_size=UIConfig.FONT_LARGE)
        # Pull the live value out of the DB whenever the user changes parameter.
        self.combo_param.currentIndexChanged.connect(self._on_param_changed)
        form.addWidget(lbl_param, row, 0)
        form.addWidget(self.combo_param, row, 1)
        row += 1

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

        # Wire buttons
        self.btn_apply.clicked.connect(self._on_apply)
        self.btn_cancel.clicked.connect(self.cancelRequested.emit)

    def set_controller_values(self, values: dict):
        """Seed/refresh the parameter-value database.

        ``values`` is keyed by ``(joint_id:int, controller_id:int)`` and maps
        to a list of value strings (one per parameter index).  Empty/missing
        rows are tolerated.  Calling this with ``{}`` effectively clears the
        DB (used at end-of-trial to honour the destroy-on-end requirement).

        We normalize every key to ``(int, int)`` on ingestion so subsequent
        lookups don't have to care whether Qt round-tripped the keys as
        tuples-of-int, tuples-of-str, or even comma-separated strings.
        """
        normalized: dict[tuple[int, int], list[str]] = {}
        try:
            for k, v in (values or {}).items():
                key = self._normalize_value_key(k)
                if key is None:
                    continue
                normalized[key] = list(v)
        except Exception as e:
            dprint(f"Error seeding controller value DB: {e}")
            normalized = {}
        self._controller_values = normalized
        dprint(
            f"[Settings] Seeded controller value DB with {len(self._controller_values)} "
            f"(joint, controller) entries; sample keys="
            f"{list(self._controller_values.keys())[:5]}"
        )
        # If a controller is currently selected, refresh the displayed value.
        try:
            self._refresh_value_from_db()
        except Exception:
            pass
        # Re-paint the controller table so the value cells pick up the freshly
        # seeded DB entries.  controllerValuesReceived can arrive AFTER
        # controllerMatrixReceived (RtBridge emits them in that order), so the
        # first table render in _on_joint_changed sees an empty values dict
        # and would leave the cells blank without this redraw.
        try:
            joint_idx = self.combo_joint.currentIndex()
            if joint_idx >= 0:
                self._on_joint_changed(joint_idx)
        except Exception as e:
            dprint(f"Error refreshing table after value DB update: {e}")

    def set_controller_matrix(self, matrix: list):
        """Populate the table with a 2D matrix: [ [Joint, Controller, p1, p2, ...], ... ]."""
        try:
            self._controller_matrix = list(matrix) if matrix else []
        except Exception:
            self._controller_matrix = []

        # Build joint mapping: { "Ankle(L) (68)": [0, 1, 2, ...], "Ankle(R) (36)": [10, 11, ...] }
        self._joint_controllers = {}
        for idx, row in enumerate(self._controller_matrix):
            if row and len(row) >= 2:
                joint_name = str(row[0])
                if joint_name not in self._joint_controllers:
                    self._joint_controllers[joint_name] = []
                self._joint_controllers[joint_name].append(idx)

        # Populate joint combo with unique joint names
        try:
            self.combo_joint.blockSignals(True)
            self.combo_joint.clear()
            joint_names = list(self._joint_controllers.keys())
            if not joint_names:
                joint_names = ["(none)"]
            self.combo_joint.addItems(joint_names)
            self.combo_joint.blockSignals(False)
            # Trigger joint change to populate table and controllers for first joint
            self._on_joint_changed(0)
            # Restore last selection after populating
            self._restore_last_selection()
        except Exception as e:
            dprint(f"Error populating joint combo: {e}")
            pass

    def _load_settings(self):
        """Load saved settings from file."""
        try:
            self._bilateral_state = SettingsManager.get_bool("bilateral", False)
            self._last_selection["bilateral"] = self._bilateral_state
            dprint(f"[Settings] Loaded bilateral state: {self._bilateral_state}")
            
            # Load last selection values
            joint = SettingsManager.get_setting("last_joint")
            if joint and joint != "None":
                self._last_selection["joint"] = joint
            
            controller = SettingsManager.get_setting("last_controller")
            if controller and controller != "None":
                self._last_selection["controller"] = controller
            
            param = SettingsManager.get_int("last_parameter", 0)
            if param is not None:
                self._last_selection["parameter"] = param
            
            value = SettingsManager.get_float("last_value", 0.0)
            if value is not None:
                self._last_selection["value"] = value
        except Exception as e:
            dprint(f"Error loading settings: {e}")

    def _save_settings(self):
        """Save settings to file."""
        try:
            updates = {"bilateral": str(self._bilateral_state)}
            
            # Only save non-None values
            joint = self._last_selection.get("joint")
            if joint and joint != "None":
                updates["last_joint"] = str(joint)
            
            controller = self._last_selection.get("controller")
            if controller and controller != "None":
                updates["last_controller"] = str(controller)
            
            parameter = self._last_selection.get("parameter")
            if parameter is not None:
                updates["last_parameter"] = str(parameter)
            
            value = self._last_selection.get("value")
            if value is not None:
                updates["last_value"] = str(value)
            
            SettingsManager.update_settings(updates)
            dprint(f"[Settings] Saved settings")
        except Exception as e:
            dprint(f"Error saving settings: {e}")

    def _on_bilateral_changed(self, state):
        """Save bilateral state when checkbox changes."""
        self._bilateral_state = bool(state)
        self._save_settings()

    def _index_of_default_controller(self, joint_name: str = None) -> int:
        """Return the controller-dropdown index of the config.ini default controller.

        ``joint_name`` defaults to the currently-selected joint.  Returns -1
        when ``config.ini`` has no default for the joint family or the
        controller name is not present in the matrix for that joint.
        """
        try:
            if joint_name is None:
                joint_name = self.combo_joint.currentText()
            default_name = exo_config.default_controller_for(joint_name)
            if not default_name:
                return -1
            for i in range(self.combo_controller.count()):
                if self.combo_controller.itemText(i) == default_name:
                    return i
        except Exception as e:
            dprint(f"[Settings] _index_of_default_controller error: {e}")
        return -1

    def _restore_last_selection(self):
        """Restore UI controls to last saved selection."""
        try:
            dprint(f"[Settings] Attempting to restore last selection: {self._last_selection}")
            
            # Restore bilateral checkbox
            bilateral = self._last_selection.get("bilateral", False)
            self.chk_bilateral.blockSignals(True)
            self.chk_bilateral.setChecked(bilateral)
            self.chk_bilateral.blockSignals(False)
            
            # Restore joint selection
            joint_name = self._last_selection.get("joint")
            if joint_name:
                self.combo_joint.blockSignals(True)
                idx = self.combo_joint.findText(joint_name)
                if idx >= 0:
                    self.combo_joint.setCurrentIndex(idx)
                    self.combo_joint.blockSignals(False)
                    self._on_joint_changed(idx)
                else:
                    self.combo_joint.blockSignals(False)
                    dprint(f"[Settings] Joint '{joint_name}' not found in dropdown")
            
            # Controller-selection priority:
            #   1. config.ini default for this joint family — ALWAYS wins when
            #      present.  This is what the operator declared in
            #      SDCard/config.ini (e.g. hipDefaultController = step) and
            #      per the original spec must be pre-selected on entry.
            #   2. saved last controller name (only used when config.ini has
            #      no default for the joint family)
            #
            # If we override the saved controller with the config.ini default,
            # we also reset the parameter index and value: the saved param/
            # value belong to the *previous* controller and would not map
            # cleanly onto the new controller's parameter list.
            controller_name = self._last_selection.get("controller")
            controller_idx = -1
            overrode_with_config_default = False
            default_idx = self._index_of_default_controller()
            if default_idx >= 0:
                controller_idx = default_idx
                default_name = self.combo_controller.itemText(default_idx)
                if controller_name and default_name != controller_name:
                    overrode_with_config_default = True
                    dprint(f"[Settings] Overriding saved controller "
                          f"'{controller_name}' with config.ini default "
                          f"'{default_name}'")
            elif controller_name:
                controller_idx = self.combo_controller.findText(controller_name)
                if controller_idx < 0:
                    dprint(f"[Settings] Controller '{controller_name}' not found")
            if controller_idx >= 0:
                self.combo_controller.blockSignals(True)
                self.combo_controller.setCurrentIndex(controller_idx)
                self.combo_controller.blockSignals(False)
                self._on_controller_changed(controller_idx)
                dprint(f"[Settings] Selected controller "
                      f"'{self.combo_controller.itemText(controller_idx)}' "
                      f"at index {controller_idx}")
            
            # Restore parameter selection (skipped when controller was
            # overridden by config.ini default, since saved index is for a
            # different controller)
            if overrode_with_config_default:
                param_idx = 0
            else:
                param_idx = self._last_selection.get("parameter", 0)
                if param_idx is None:
                    param_idx = 0
            self.combo_param.blockSignals(True)
            if param_idx < self.combo_param.count() and param_idx >= 0:
                self.combo_param.setCurrentIndex(param_idx)
                dprint(f"[Settings] Restored parameter index: {param_idx}")
            self.combo_param.blockSignals(False)
            
            # Restore value (also skipped when controller was overridden)
            if overrode_with_config_default:
                value = 0.0
            else:
                value = self._last_selection.get("value", 0.0)
                if value is None:
                    value = 0.0
            self.spin_value.setValue(float(value))
            
            dprint(f"[Settings] Successfully restored last selection")
        except Exception as e:
            dprint(f"[Settings] Error restoring last selection: {e}")
            from utils import debug as _debug
            if _debug.DEBUG_PRINTS:
                import traceback
                traceback.print_exc()

    @QtCore.Slot()
    def _on_apply(self):
        """Emit the selected joint, controller, parameter, and value."""
        try:
            is_bilateral = self.chk_bilateral.isChecked()
            
            # Get the selections
            joint_idx = self.combo_joint.currentIndex()
            joint_name = self.combo_joint.itemText(joint_idx)
            controller_local_idx = self.combo_controller.currentIndex()
            controller_name = self.combo_controller.currentText()
            parameter_idx = self.combo_param.currentIndex()
            parameter_name = self.combo_param.currentText()
            value = float(self.spin_value.value())
            
            dprint(f"\n=== Apply Settings ===")
            dprint(f"Joint: {joint_name} (dropdown idx: {joint_idx})")
            dprint(f"Controller: {controller_name} (local idx: {controller_local_idx})")
            dprint(f"Parameter: {parameter_name} (idx: {parameter_idx})")
            dprint(f"Value: {value}")
            dprint(f"Bilateral: {is_bilateral}")
            
            # Get the actual controller matrix index
            if joint_name in self._joint_controllers:
                controller_indices = self._joint_controllers[joint_name]
                if controller_local_idx < len(controller_indices):
                    matrix_row_idx = controller_indices[controller_local_idx]
                    
                    if matrix_row_idx < len(self._controller_matrix):
                        row = self._controller_matrix[matrix_row_idx]
                        # Matrix format: [Joint(ID), JointID, ControllerName, ControllerID, Param1, Param2, ...]
                        # row[0] = "Ankle(L) (68)"
                        # row[1] = "68" (joint ID)
                        # row[2] = "pjmc_plus" (controller name)
                        # row[3] = "11" (controller ID)
                        # row[4:] = parameters
                        
                        # Extract joint ID from row[1]
                        joint_id_raw = None
                        if len(row) > 1:
                            try:
                                joint_id_raw = int(row[1])
                            except (ValueError, IndexError):
                                dprint(f"Warning: Could not parse joint ID from row[1]='{row[1]}'")
                        
                        # Extract actual controller ID from row[3]
                        controller_id = controller_local_idx  # Default to local index if parsing fails
                        if len(row) > 3:
                            try:
                                controller_id = int(row[3])
                                dprint(f"Extracted controller ID {controller_id} from row[3]='{row[3]}'")
                            except (ValueError, TypeError):
                                dprint(f"Warning: Could not parse controller ID from row[3]='{row[3]}', using local idx {controller_local_idx}")
                        else:
                            dprint(f"Warning: Row too short (len={len(row)}), cannot extract controller ID from row[3]")
                        
                        # Use the actual joint_id_raw (like 65, 68) not the mapped joint_num
                        payload = [is_bilateral, joint_id_raw, controller_id, parameter_idx, value]
                        dprint(f"Payload: is_bilateral={is_bilateral}, joint_id={joint_id_raw}, controller_id={controller_id}, param_idx={parameter_idx}, value={value}")
                        dprint(f"Full row: {row}")
                        dprint(f"======================\n")
                        
                        # Save last selection for next time
                        self._last_selection = {
                            "bilateral": is_bilateral,
                            "joint": joint_name,
                            "controller": controller_name,
                            "parameter": parameter_idx,
                            "value": value,
                        }
                        dprint(f"[Settings] Saving last selection: {self._last_selection}")
                        self._save_settings()
                        
                        self.applyRequested.emit(payload)
                        return
            
            # Fallback if something goes wrong
            dprint(f"Warning: Falling back to default payload")
            payload = [is_bilateral, 1, 0, 0, value]
            self.applyRequested.emit(payload)
        except Exception as e:
            dprint(f"Error in _on_apply: {e}")
            from utils import debug as _debug
            if _debug.DEBUG_PRINTS:
                import traceback
                traceback.print_exc()

    @QtCore.Slot(int, int)
    def _on_cell_clicked(self, row: int, column: int):
        """When a table cell is clicked, update the dropdowns to match.

        Each controller occupies two rows (names row + values row), so we
        translate the table row back to the controller dropdown index via
        ``_row_to_controller`` (which is rebuilt every time the table is
        re-rendered).  Column 0 is the controller-name column; columns 1..N
        map directly to parameter indices.
        """
        try:
            # Resolve controller dropdown index from the displayed table row.
            matrix_idx = getattr(self, "_row_to_controller", {}).get(int(row))
            if matrix_idx is not None:
                joint_idx = self.combo_joint.currentIndex()
                joint_name = self.combo_joint.itemText(joint_idx)
                indices = self._joint_controllers.get(joint_name, [])
                try:
                    local_idx = indices.index(matrix_idx)
                except ValueError:
                    local_idx = (int(row) // 2)
                self.combo_controller.setCurrentIndex(local_idx)
            else:
                # Fallback: assume two rows per controller.
                self.combo_controller.setCurrentIndex(int(row) // 2)

            # Column 0 is the controller-name cell -- treat that as "first
            # parameter" so the spinbox still has a sensible default.
            param_index = max(0, int(column) - 1)
            self.combo_param.setCurrentIndex(param_index)
        except Exception as e:
            dprint(f"Error in _on_cell_clicked: {e}")
            pass

    @QtCore.Slot(int)
    def _on_joint_changed(self, idx: int):
        """When joint selection changes, update table to show only that joint's controllers."""
        try:
            joint_name = self.combo_joint.itemText(idx)
            
            # Filter the table to show only controllers for the selected joint
            if joint_name in self._joint_controllers:
                controller_indices = self._joint_controllers[joint_name]
                filtered_rows = [self._controller_matrix[i] for i in controller_indices if i < len(self._controller_matrix)]
                
                # Determine max columns from filtered rows
                max_cols = 0
                for row in filtered_rows:
                    if len(row) > max_cols:
                        max_cols = len(row)
                if max_cols == 0:
                    max_cols = 2
                
                # Update table.  Each controller takes TWO rows so its names
                # and values stay aligned even though sibling controllers have
                # different parameter sets:
                #
                #   | zeroTorqu | use_pid | p_gain | i_gain | d_gain |
                #   |  (values) | 0       | 0      | 0      | 0      |
                #   | franksCol | mass    | TroughNo | PeakNorm | start per | trough on | ...
                #   |  (values) | 200     | 4        | 3        | 84        | 16.4      | ...
                #
                # The matrix carries parameter NAMES; live values come from
                # self._controller_values[(jid, cid)] (seeded by line 6 of
                # each controller csv on the SD card).
                self.table.clear()
                num_params = max(0, max_cols - 4)
                self.table.setColumnCount(1 + num_params)
                self.table.setRowCount(2 * len(filtered_rows))

                # Generic numeric headers -- the per-controller param names
                # appear in the table cells of the "names" rows, so column
                # headers like "Param 1" would just be redundant clutter.
                header_names = ["Controller"] + [str(i + 1) for i in range(num_params)]
                self.table.setHorizontalHeaderLabels(header_names)

                # Map from displayed table row (the values row) -> controller
                # matrix index, so cell clicks can resolve back to a controller.
                self._row_to_controller = {}

                for r, data in enumerate(filtered_rows):
                    name_row = 2 * r
                    value_row = 2 * r + 1

                    # Resolve the matrix index this filtered row came from so
                    # cell clicks can map cleanly back to a (joint, controller).
                    try:
                        matrix_idx = controller_indices[r]
                    except (IndexError, TypeError):
                        matrix_idx = None
                    self._row_to_controller[name_row] = matrix_idx
                    self._row_to_controller[value_row] = matrix_idx

                    # Row N: controller name + parameter NAMES.
                    if len(data) > 2:
                        item = QtWidgets.QTableWidgetItem(str(data[2]))
                        self.table.setItem(name_row, 0, item)
                    for param_idx in range(num_params):
                        src = 4 + param_idx
                        text = str(data[src]) if src < len(data) else ""
                        self.table.setItem(name_row, param_idx + 1, QtWidgets.QTableWidgetItem(text))

                    # Row N+1: empty leftmost cell + parameter VALUES from the DB.
                    self.table.setItem(value_row, 0, QtWidgets.QTableWidgetItem(""))

                    try:
                        jid = int(data[1])
                        cid = int(data[3])
                    except (ValueError, IndexError, TypeError):
                        jid = cid = None
                    if jid is not None and cid is not None:
                        values_row = self._lookup_values(jid, cid)
                    else:
                        values_row = []

                    name_count = max(0, len(data) - 4)
                    render_count = min(num_params, name_count)
                    for param_idx in range(render_count):
                        cell_text = str(values_row[param_idx]) if param_idx < len(values_row) else ""
                        self.table.setItem(value_row, param_idx + 1, QtWidgets.QTableWidgetItem(cell_text))

                self.table.resizeColumnsToContents()
                
                # Update controller dropdown
                # Matrix format: [Joint(ID), JointID, ControllerName, ControllerID, ...]
                # Controller name is at index 2
                self.combo_controller.blockSignals(True)
                self.combo_controller.clear()
                controller_names = [row[2] if len(row) > 2 else "(unknown)" for row in filtered_rows]
                if controller_names:
                    self.combo_controller.addItems(controller_names)
                else:
                    self.combo_controller.addItem("(none)")
                # Pre-select the controller declared as default in config.ini
                # for this joint family (e.g. hipDefaultController = step).
                default_idx = self._index_of_default_controller(joint_name)
                if default_idx >= 0:
                    self.combo_controller.setCurrentIndex(default_idx)
                self.combo_controller.blockSignals(False)
                
                # Trigger parameter update for first controller
                self._on_controller_changed(0)
            else:
                # No controllers for this joint
                self.table.clear()
                self.table.setRowCount(0)
                self.table.setColumnCount(2)
                self.table.setHorizontalHeaderLabels(["Controller", "Parameters"])
                
                self.combo_controller.blockSignals(True)
                self.combo_controller.clear()
                self.combo_controller.addItem("(none)")
                self.combo_controller.blockSignals(False)
                
        except Exception as e:
            dprint(f"Error in _on_joint_changed: {e}")
            pass

    @staticmethod
    def _normalize_value_key(k):
        """Coerce an incoming controller_values key to ``(int, int)``.

        Qt's signal/slot machinery occasionally round-trips a dict's keys in a
        form different from what the sender intended (tuples-of-str, lists,
        even comma-joined strings depending on how QVariant marshals them
        across threads).  We accept any of those and yield the canonical
        ``(joint_id, controller_id)`` tuple, or ``None`` if the key is
        unparseable.
        """
        try:
            if isinstance(k, str):
                parts = [p.strip() for p in k.split(",") if p.strip()]
            elif isinstance(k, (tuple, list)):
                parts = [p for p in k]
            else:
                return None
            if len(parts) < 2:
                return None
            return (int(parts[0]), int(parts[1]))
        except (ValueError, TypeError):
            return None

    def _lookup_values(self, jid: int, cid: int) -> list:
        """Tolerant value-DB lookup that survives weirdly-typed dict keys."""
        target = (int(jid), int(cid))
        # Fast path: canonical tuple key.
        row = self._controller_values.get(target)
        if row is not None:
            return row
        # Slow path: scan the dict and normalize each key on the fly.  This
        # protects against dicts that arrived with not-yet-normalized keys
        # (e.g. (str, str) tuples or "65,3" strings) when the seeding path
        # didn't run for some reason.
        for k, v in self._controller_values.items():
            if self._normalize_value_key(k) == target:
                return v
        return []

    def _current_jid_cid(self) -> tuple[int | None, int | None]:
        """Return (joint_id, controller_id) for the active selection, or (None, None)."""
        try:
            joint_idx = self.combo_joint.currentIndex()
            joint_name = self.combo_joint.itemText(joint_idx)
            controller_local_idx = self.combo_controller.currentIndex()
        except Exception:
            return (None, None)
        if joint_name not in self._joint_controllers:
            return (None, None)
        controller_indices = self._joint_controllers[joint_name]
        if controller_local_idx < 0 or controller_local_idx >= len(controller_indices):
            return (None, None)
        matrix_idx = controller_indices[controller_local_idx]
        if matrix_idx >= len(self._controller_matrix):
            return (None, None)
        row = self._controller_matrix[matrix_idx]
        if len(row) < 4:
            return (None, None)
        try:
            jid = int(row[1])
            cid = int(row[3])
        except (ValueError, TypeError):
            return (None, None)
        return (jid, cid)

    @staticmethod
    def _parse_value_string(s: str) -> float:
        """Convert a CSV value cell into a float for the spinbox.

        We treat the raw firmware string as either int or float based on the
        presence of a decimal point / exponent (per spec: "send over as string,
        then determine if the value is a float or integer").  Non-numeric
        cells fall back to 0.0 so the UI doesn't blow up on garbage rows.
        """
        if s is None:
            return 0.0
        s = str(s).strip()
        if not s or s in ("?", "??"):
            return 0.0
        try:
            if "." in s or "e" in s.lower():
                return float(s)
            return float(int(s))
        except (ValueError, TypeError):
            try:
                return float(s)
            except (ValueError, TypeError):
                return 0.0

    def _refresh_value_from_db(self):
        """Push the DB value for the currently selected (joint, ctrl, param) into the spinbox."""
        jid, cid = self._current_jid_cid()
        if jid is None or cid is None:
            return
        row = self._lookup_values(jid, cid)
        if not row:
            return
        param_idx = self.combo_param.currentIndex()
        if param_idx < 0 or param_idx >= len(row):
            return
        try:
            self.spin_value.blockSignals(True)
            self.spin_value.setValue(self._parse_value_string(row[param_idx]))
        finally:
            self.spin_value.blockSignals(False)

    @QtCore.Slot(int)
    def _on_param_changed(self, idx: int):
        """When the parameter selection changes, fetch its current value from the DB."""
        try:
            self._refresh_value_from_db()
        except Exception as e:
            dprint(f"Error in _on_param_changed: {e}")

    @QtCore.Slot(int)
    def _on_controller_changed(self, idx: int):
        """Populate parameters combo from selected controller."""
        try:
            self.combo_param.blockSignals(True)
            self.combo_param.clear()
            
            # Get the current joint
            joint_idx = self.combo_joint.currentIndex()
            joint_name = self.combo_joint.itemText(joint_idx)
            
            if joint_name in self._joint_controllers:
                controller_indices = self._joint_controllers[joint_name]
                if idx < len(controller_indices):
                    matrix_idx = controller_indices[idx]
                    if matrix_idx < len(self._controller_matrix):
                        row = self._controller_matrix[matrix_idx]
                        # Matrix format: [Joint(ID), JointID, ControllerName, ControllerID, Param1, Param2, ...]
                        # Parameters start at index 4
                        params = row[4:] if len(row) > 4 else []
                        if not params:
                            params = ["(no params)"]
                        self.combo_param.addItems([str(p) for p in params])
                    else:
                        self.combo_param.addItem("(no params)")
                else:
                    self.combo_param.addItem("(no params)")
            else:
                self.combo_param.addItem("(no params)")
        except Exception as e:
            dprint(f"Error in _on_controller_changed: {e}")
            pass
        finally:
            try:
                self.combo_param.blockSignals(False)
            except Exception:
                pass
            # After repopulating params for the new controller, sync the
            # spinbox to the DB value of the first parameter so the user sees
            # the actual current setting instead of a stale leftover number.
            try:
                self._refresh_value_from_db()
            except Exception:
                pass


