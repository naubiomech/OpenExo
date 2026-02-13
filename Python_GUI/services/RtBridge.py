from typing import List
import time

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
        
        # Data rate monitoring
        self.DEBUG_DATA_RATE = True  # Set to False to disable data rate printing
        self._data_packet_count = 0
        self._bytes_received = 0
        self._data_rate_timer = QtCore.QTimer()
        self._data_rate_timer.timeout.connect(self._print_data_rate)
        self._data_rate_timer.start(1000)  # Print every 1 second
        self._last_packet_time = None
        self._packet_intervals = []  # Track time between packets for jitter analysis
        self._expected_hz = 100  # Expected data rate (will auto-detect)
        self._total_packets_received = 0
        self._total_time_elapsed = 0.0
        self._monitoring_start_time = None
        self._dropped_packet_count = 0
        
        # Additional BLE metrics
        self._ble_chunk_count = 0  # Number of BLE chunks received (not packets)
        self._ble_chunk_sizes = []  # Track BLE chunk sizes
        self._consecutive_drops = 0  # Track consecutive packet drops
        self._max_consecutive_drops = 0  # Worst consecutive drop streak
        self._stall_count = 0  # Number of times data stopped flowing >100ms
        self._last_stall_time = None
        
        self._handshake_payload_buf: str = ""

    @QtCore.Slot(bytes)
    def feed_bytes(self, data: bytes):
        # Track bytes received
        self._bytes_received += len(data)
        
        # Track BLE chunk metrics
        self._ble_chunk_count += 1
        chunk_size = len(data)
        self._ble_chunk_sizes.append(chunk_size)
        
        # Detect stalls (>100ms between any data)
        current_time = time.time()
        if self._last_stall_time is not None:
            stall_interval = (current_time - self._last_stall_time) * 1000
            if stall_interval > 100:
                self._stall_count += 1
        self._last_stall_time = current_time
        
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
                            
                            # Track data rate and timing
                            self._data_packet_count += 1
                            self._total_packets_received += 1
                            current_time = time.time()
                            
                            if self._monitoring_start_time is None:
                                self._monitoring_start_time = current_time
                            
                            if self._last_packet_time is not None:
                                interval = (current_time - self._last_packet_time) * 1000  # Convert to ms
                                self._packet_intervals.append(interval)
                                
                                # Detect dropped packets (interval > 2.5x expected)
                                # More conservative threshold to avoid false positives from jitter
                                expected_interval = 1000.0 / self._expected_hz if self._expected_hz > 0 else 14.3
                                if interval > expected_interval * 2.5:
                                    # Estimate how many packets were dropped
                                    dropped = int(round(interval / expected_interval)) - 1
                                    self._dropped_packet_count += max(0, dropped)
                                    self._consecutive_drops += dropped
                                    if self._consecutive_drops > self._max_consecutive_drops:
                                        self._max_consecutive_drops = self._consecutive_drops
                                else:
                                    # Reset consecutive drop counter on successful packet
                                    self._consecutive_drops = 0
                            
                            self._last_packet_time = current_time
                            
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
    
    def print_trial_summary(self):
        """Print comprehensive trial summary with all statistics."""
        if not self.DEBUG_DATA_RATE or self._total_packets_received == 0:
            return
        
        if self._monitoring_start_time is not None:
            total_time = time.time() - self._monitoring_start_time
        else:
            total_time = self._total_time_elapsed
        
        if total_time == 0:
            return
        
        overall_hz = self._total_packets_received / total_time
        total_bytes = self._total_packets_received * (self._bytes_received / max(1, self._data_packet_count))
        total_kb = total_bytes / 1024.0
        total_mb = total_kb / 1024.0
        
        # Calculate expected packets and loss
        expected_total = self._expected_hz * total_time
        total_lost = max(0, expected_total - self._total_packets_received)
        loss_pct = (total_lost / expected_total * 100) if expected_total > 0 else 0
        
        print("\n" + "="*60)
        print("           TRIAL DATA COLLECTION SUMMARY")
        print("="*60)
        print(f"  Duration: {total_time:.1f} seconds ({total_time/60:.1f} minutes)")
        print(f"  Total packets: {self._total_packets_received}")
        print(f"  Average rate: {overall_hz:.1f} Hz")
        print(f"  Expected rate: {self._expected_hz} Hz")
        print(f"  Total data: {total_bytes:.0f} bytes ({total_kb:.2f} KB / {total_mb:.2f} MB)")
        print(f"  Packet loss: {loss_pct:.2f}% (~{int(total_lost)} packets)")
        
        if self._max_consecutive_drops > 0:
            print(f"  Worst drop streak: {self._max_consecutive_drops} consecutive packets")
        
        # BLE reliability indicators
        if self._max_consecutive_drops == 0 and loss_pct < 1:
            reliability = "EXCELLENT - No significant interruptions"
        elif self._max_consecutive_drops < 5 and loss_pct < 5:
            reliability = "GOOD - Minor interruptions"
        elif self._max_consecutive_drops < 10 and loss_pct < 10:
            reliability = "FAIR - Noticeable interruptions"
        else:
            reliability = "POOR - Frequent interruptions"
        
        if overall_hz >= self._expected_hz * 0.95:
            quality = "EXCELLENT - Minimal packet loss"
        elif overall_hz >= self._expected_hz * 0.90:
            quality = "GOOD - Acceptable performance"
        elif overall_hz >= self._expected_hz * 0.80:
            quality = "FAIR - Some packet loss detected"
        else:
            quality = "POOR - Significant packet loss"
        
        print(f"  Data quality: {quality}")
        print(f"  BLE reliability: {reliability}")
        print("="*60 + "\n")
    
    def reset_monitoring(self):
        """Reset data rate monitoring statistics (call when starting new trial)."""
        self._data_packet_count = 0
        self._bytes_received = 0
        self._packet_intervals.clear()
        self._total_packets_received = 0
        self._total_time_elapsed = 0.0
        self._monitoring_start_time = None
        self._last_packet_time = None
        self._dropped_packet_count = 0
        self._ble_chunk_count = 0
        self._ble_chunk_sizes.clear()
        self._consecutive_drops = 0
        self._max_consecutive_drops = 0
        self._stall_count = 0
        self._last_stall_time = None
        
        print("[RtBridge] Data rate monitoring reset")

    @QtCore.Slot()
    def _print_data_rate(self):
        """Print comprehensive data collection statistics every second."""
        if not self.DEBUG_DATA_RATE:
            return
            
        if self._data_packet_count > 0:
            hz = self._data_packet_count
            bytes_per_sec = self._bytes_received
            kb_per_sec = bytes_per_sec / 1024.0
            
            # Calculate average packet size
            avg_packet_size = bytes_per_sec / self._data_packet_count if self._data_packet_count > 0 else 0
            
            # Update expected Hz based on observed rate (after first few seconds)
            if self._total_packets_received > 100 and hz > 10:
                self._expected_hz = hz
            
            # Calculate overall statistics
            if self._monitoring_start_time is not None:
                self._total_time_elapsed = time.time() - self._monitoring_start_time
                overall_hz = self._total_packets_received / self._total_time_elapsed if self._total_time_elapsed > 0 else 0
            else:
                overall_hz = 0
            
            # Calculate packet loss percentage (two methods)
            # Method 1: Based on timing gap detection
            gap_detected_drops = self._dropped_packet_count
            gap_loss_pct = 0.0
            if (self._data_packet_count + gap_detected_drops) > 0:
                gap_loss_pct = (gap_detected_drops / (self._data_packet_count + gap_detected_drops)) * 100
            
            # Method 2: Based on expected Hz vs actual count
            expected_packets_hz = self._expected_hz  # Expected in this 1-second window
            hz_detected_drops = max(0, expected_packets_hz - self._data_packet_count)
            hz_loss_pct = 0.0
            if expected_packets_hz > 0:
                hz_loss_pct = (hz_detected_drops / expected_packets_hz) * 100
            
            # Use Hz-based method as primary (more reliable)
            packet_loss_pct = hz_loss_pct
            actual_drops = hz_detected_drops
            
            # Calculate timing statistics
            if len(self._packet_intervals) > 0:
                avg_interval = sum(self._packet_intervals) / len(self._packet_intervals)
                min_interval = min(self._packet_intervals)
                max_interval = max(self._packet_intervals)
                jitter = max_interval - min_interval
                
                # Connection quality indicator
                quality = "EXCELLENT"
                if jitter > 50 or packet_loss_pct > 10:
                    quality = "POOR"
                elif jitter > 20 or packet_loss_pct > 5:
                    quality = "FAIR"
                elif jitter > 10 or packet_loss_pct > 2:
                    quality = "GOOD"
                
                # Calculate BLE chunk statistics
                avg_chunk_size = sum(self._ble_chunk_sizes) / len(self._ble_chunk_sizes) if self._ble_chunk_sizes else 0
                min_chunk = min(self._ble_chunk_sizes) if self._ble_chunk_sizes else 0
                max_chunk = max(self._ble_chunk_sizes) if self._ble_chunk_sizes else 0
                chunks_per_packet = self._ble_chunk_count / self._data_packet_count if self._data_packet_count > 0 else 0
                
                print(f"\n[RtBridge] ===== Data Rate Stats =====")
                print(f"  Rate: {hz} Hz (expected: {self._expected_hz} Hz)")
                print(f"  Throughput: {bytes_per_sec} bytes/sec ({kb_per_sec:.2f} KB/s)")
                print(f"  Avg packet: {avg_packet_size:.1f} bytes")
                print(f"  Timing: avg={avg_interval:.1f}ms, min={min_interval:.1f}ms, max={max_interval:.1f}ms")
                print(f"  Jitter: {jitter:.1f}ms")
                print(f"  Packet loss: {packet_loss_pct:.1f}% (~{int(actual_drops)} packets this second)")
                if gap_detected_drops > 0:
                    print(f"  Gap-detected drops: {gap_detected_drops} (from timing analysis)")
                if self._max_consecutive_drops > 0:
                    print(f"  Max consecutive drops: {self._max_consecutive_drops} packets")
                print(f"  BLE chunks: {self._ble_chunk_count} (avg {chunks_per_packet:.1f} per packet)")
                print(f"  BLE chunk size: avg={avg_chunk_size:.1f}B, min={min_chunk}B, max={max_chunk}B")
                if self._stall_count > 0:
                    print(f"  Data stalls: {self._stall_count} (>100ms gaps)")
                print(f"  Overall: {self._total_packets_received} packets in {self._total_time_elapsed:.1f}s (avg {overall_hz:.1f} Hz)")
                print(f"  Quality: {quality}")
                print(f"================================\n")
            else:
                print(f"[RtBridge] Data rate: {hz} Hz | {bytes_per_sec} bytes/sec ({kb_per_sec:.2f} KB/s) | "
                      f"Avg packet: {avg_packet_size:.1f} bytes")
            
            # Reset per-second counters
            self._data_packet_count = 0
            self._bytes_received = 0
            self._packet_intervals.clear()
            self._dropped_packet_count = 0
            self._ble_chunk_count = 0
            self._ble_chunk_sizes.clear()
            self._stall_count = 0