from typing import List

try:
    from PySide6 import QtCore
except ImportError as e:
    raise SystemExit("PySide6 is required. Install with: pip install PySide6") from e


class RtBridge(QtCore.QObject):
    """
    Self-contained real-time parser for the Qt app.
    Parses the same ASCII protocol used by firmware/Tk GUI:
    - handshake: literal "handshake"
    - parameter names: plain lines until "END"
    - controllers: "!<controller>" then "!!<param>" … and "!END"
    - rt data: frames containing 'c' with numeric payload per the existing scheme
    """

    handshakeReceived = QtCore.Signal(str)
    parameterNamesReceived = QtCore.Signal(list)
    controllersReceived = QtCore.Signal(list, list)
    controllerMatrixReceived = QtCore.Signal(list)
    rtDataUpdated = QtCore.Signal(list)

    def __init__(self, parent=None):
        super().__init__(parent)
        # Name/controller state
        self._handshake = False
        self._collecting_names = True
        self._names: List[str] = []
        self._controllers: List[str] = []
        self._controller_params: List[List[str]] = []
        self._temp_params: List[str] = []
        self._controllers_done = False
        self._controller_matrix: List[List[str]] = []
        self._rows_68: List[List[str]] = []
        self._rows_36: List[List[str]] = []
        self._rows_38: List[List[str]] = []

        # Stream parse state (port of minimal logic)
        self._event_count_regex = QtCore.QRegularExpression("[0-9]+")
        self._start_transmission = False
        self._command = None
        self._num_count = 0
        self._buffer: List[str] = []
        self._payload: List[float] = []
        self._data_length = 0

        # Handshake payload reassembly state
        self._collecting_handshake_payload = False
        self._handshake_payload_buf: str = ""

    @QtCore.Slot(bytes)
    def feed_bytes(self, data: bytes):
        try:
            s = data.decode("utf-8")
        except Exception:
            return

        # Handshake
        if s == "READY":
            print("RtBridge::feed_bytes->Handshake header received")
            self._handshake = True
            # Begin collecting the initial long handshake payload split across notifications
            self._collecting_handshake_payload = True
            self._handshake_payload_buf = ""
            return

        # If we're collecting the extended handshake payload, accumulate until newline
        if self._collecting_handshake_payload:
            self._handshake_payload_buf += s
            if "\n" in self._handshake_payload_buf:
                line, _, _ = self._handshake_payload_buf.partition("\n")
                # Split by commas and drop empty entries
                tokens = [tok.strip() for tok in line.split(",") if tok.strip()]
                
                # Print the payload in the same format as the Serial Monitor
                payload_line = line.replace('|', '\n')
                print("RtBridge::feed_bytes->Handshake payload received:")
                for payload_row in payload_line.split('\n'):
                    if payload_row.strip():
                        print(f"    {payload_row}")
                joined_tokens = ", ".join(tokens)
                payload_str = "READY" if not joined_tokens else f"READY, {joined_tokens}"
                self.handshakeReceived.emit(payload_str)

                # Parse controllers and parameter headers from the payload blob
                rows = [row.strip() for row in payload_line.split("\n") if row.strip()]
                controller_rows = []
                param_names = []
                self._rows_68 = []
                self._rows_36 = []
                self._rows_38 = []
                current_joint = None
                current_rows: List[str] = []
                
                for row in rows:
                    parts_raw = row.split(",")
                    parts = [part.strip() for part in parts_raw if part.strip()]
                    if not parts:
                        continue
                    prefix = parts[0].lower()
                    if prefix == 'f':
                        # Legacy fetch command header; keep data if the row has content.
                        if len(parts) > 1:
                            parts = parts[1:]
                        else:
                            continue
                    if prefix == 't':
                        param_names = [p.strip() for p in parts[1:] if p.strip()]
                        print(f"RtBridge::feed_bytes->Parameter names: {param_names}")
                        continue
                    if prefix == '?':
                        # End-of-handshake sentinel
                        continue
                    
                    # Each row format: [JointName, JointID, ControllerName, ParamCount, ...params]
                    # Example: ['Ankle(L)', '68', 'zeroTorqu', '2', 'use_pid', 'p_gain', 'i_gain', 'd_gain']
                    controller_rows.append(parts)
                    
                    # Group by joint name for display blocks
                    if len(parts) >= 2:
                        joint_name = parts[0]
                        joint_id = parts[1]
                        
                        if current_joint is None:
                            current_joint = joint_name
                        elif current_joint != joint_name:
                            formatted_block = "\n".join(current_rows)
                            if joint_id == '68':
                                self._rows_68.append(formatted_block)
                            elif joint_id == '36':
                                self._rows_36.append(formatted_block)
                            elif joint_id == '38':
                                self._rows_38.append(formatted_block)
                            current_rows = []
                            current_joint = joint_name
                        
                        row_string = ",".join(parts)
                        current_rows.append(row_string)

                if current_rows and current_joint:
                    # Flush last group - need to determine ID from the last row
                    if controller_rows:
                        last_id = controller_rows[-1][1] if len(controller_rows[-1]) > 1 else None
                        formatted_block = "\n".join(current_rows)
                        if last_id == '68':
                            self._rows_68.append(formatted_block)
                        elif last_id == '36':
                            self._rows_36.append(formatted_block)
                        elif last_id == '38':
                            self._rows_38.append(formatted_block)

                if param_names:
                    self._param_names = list(param_names)
                    self.parameterNamesReceived.emit(list(param_names))

                for block in self._rows_68:
                    print(f"RtBridge::feed_bytes->Rows with 68:\n{block}\n")
                for block in self._rows_36:
                    print(f"RtBridge::feed_bytes->Rows with 36:\n{block}\n")
                for block in self._rows_38:
                    print(f"RtBridge::feed_bytes->Rows with 38:\n{block}\n")

                if controller_rows:
                    # Build matrix: [JointName, JointID, ControllerName, ControllerID, Param1, Param2, ...]
                    self._controller_matrix = []
                    for row in controller_rows:
                        if len(row) >= 3:
                            # row[0] = joint name (e.g., "Ankle(L)")
                            # row[1] = joint ID (e.g., "68")
                            # row[2] = controller name (e.g., "pjmc_plus")
                            # row[3] = controller ID (e.g., "11")
                            # row[4:] = parameter names
                            joint_name = row[0]
                            joint_id = row[1]
                            controller_name = row[2]
                            controller_id = row[3] if len(row) > 3 else "0"
                            params = row[4:] if len(row) > 4 else []
                            
                            # Create display row: [Joint(ID), JointID, ControllerName, ControllerID, Param1, Param2, ...]
                            display_row = [f"{joint_name} ({joint_id})", joint_id, controller_name, controller_id] + params
                            self._controller_matrix.append(display_row)
                    
                    if self._controller_matrix:
                        self.controllerMatrixReceived.emit(list(self._controller_matrix))

                # Done collecting extended handshake
                self._collecting_handshake_payload = False
                self._handshake_payload_buf = ""
                # Treat the handshake payload as the complete parameter preamble
                self._collecting_names = False
                self._names.clear()
                self._controllers_done = True
            return

        # Parameter names first, plain strings until END
        # Accept all lines (including those containing the letter 'c'); only exclude controller-prefixed lines
        if self._handshake and self._collecting_names and not s.startswith("!"):
            if s == "END":
                self._collecting_names = False
                if self._names:
                    self.parameterNamesReceived.emit(list(self._names))
            else:
                self._names.append(s)
            return

        # Controllers and their parameters using ! protocol
        if self._handshake and s.startswith("!"):
            # strip leading '!'
            payload = s[1:]
            if payload == 'END':
                # Close out the last controller params if any
                if self._temp_params:
                    self._controller_params.append(self._temp_params)
                    self._temp_params = []
                # Build 2D controller-parameter matrix: [ [controller, param1, param2, ...], ... ]
                self._controller_matrix = []
                for i, ctrl in enumerate(self._controllers):
                    params = self._controller_params[i] if i < len(self._controller_params) else []
                    row = [ctrl] + list(params)
                    self._controller_matrix.append(row)
                if self._controllers:
                    self.controllersReceived.emit(list(self._controllers), list(self._controller_params))
                    self.controllerMatrixReceived.emit(list(self._controller_matrix))
                self._controllers_done = True
                return
            # parameter vs controller
            if payload.startswith("!"):
                # parameter name
                self._temp_params.append(payload[1:])
            else:
                # new controller begins
                if self._temp_params:
                    self._controller_params.append(self._temp_params)
                    self._temp_params = []
                self._controllers.append(payload)
            return

        # Real-time data frames
        if 'c' in s:
            parts = s.split('c')
            if len(parts) < 2:
                return
            event_info = parts[0]
            event_data = parts[1]
            # Extract count from event_info using regex
            m = self._event_count_regex.match(event_info)
            if not m.hasMatch():
                return
            try:
                self._data_length = int(m.captured(0))
            except Exception:
                return

            event_without_count = f"{event_info[0]}{event_info[1]}{event_data}"
            # Parse stream similar to original logic
            for ch in event_without_count:
                if ch == 'S' and not self._start_transmission:
                    self._start_transmission = True
                    continue
                elif self._start_transmission:
                    if not self._command:
                        self._command = ch
                    elif ch == 'n':
                        self._num_count += 1
                        token = ''.join(self._buffer)
                        try:
                            val = float(token) / 100.0
                        except Exception:
                            val = None
                        self._buffer.clear()
                        if val is not None:
                            self._payload.append(val)
                        if self._num_count == self._data_length:
                            # Drop spurious single-value frames (e.g., fragmented BLE chunks)
                            if self._data_length <= 1:
                                self._reset_stream()
                                return
                            # Emit payload; pad/crop to 16 entries for safety
                            values = list(self._payload)
                            if len(values) < 16:
                                values.extend([0.0] * (16 - len(values)))
                            elif len(values) > 16:
                                values = values[:16]
                            self.rtDataUpdated.emit(values)
                            # reset state
                            self._reset_stream()
                        else:
                            continue
                    else:
                        if self._data_length != 0:
                            self._buffer.append(ch)
                        else:
                            return
                else:
                    return

    def _reset_stream(self):
        self._start_transmission = False
        self._command = None
        self._data_length = 0
        self._num_count = 0
        self._payload.clear()
        self._buffer.clear()
