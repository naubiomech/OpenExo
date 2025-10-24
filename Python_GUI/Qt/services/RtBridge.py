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

    handshakeReceived = QtCore.Signal()
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

        # Stream parse state (port of minimal logic)
        self._event_count_regex = QtCore.QRegularExpression("[0-9]+")
        self._start_transmission = False
        self._command = None
        self._num_count = 0
        self._buffer: List[str] = []
        self._payload: List[float] = []
        self._data_length = 0

    @QtCore.Slot(bytes)
    def feed_bytes(self, data: bytes):
        try:
            s = data.decode("utf-8")
        except Exception:
            return

        # Handshake
        if s == "handshake":
            self._handshake = True
            self.handshakeReceived.emit()
            return

        # Parameter names first, plain strings until END
        if self._handshake and self._collecting_names and "c" not in s and not s.startswith("!"):
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

