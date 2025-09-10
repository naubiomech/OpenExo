import re

from Device import chart_data, exoData, MLModel


class RealTimeProcessor:
    def __init__(self):
        self._event_count_regex = re.compile(
            "[0-9]+"
        )  # Regular Expression to find any number 1-9
        self._start_transmission = False
        self._command = None
        self._num_count = 0
        self._buffer = []
        self._payload = []
        self._result = ""
        self._exo_data = exoData.ExoData()
        self._chart_data = chart_data.ChartData()
        self._data_length = None
        self.x_time = 0
        self._predictor= MLModel.MLModel() #create the machine learning model object
        
        # Packet size tracking for DLE detection
        self._packet_sizes = []
        self._max_packet_size = 0
        self._packet_count = 0
        

    def processEvent(self, event):
        # Track packet size for DLE detection
        packet_size = len(event)
        self._packet_sizes.append(packet_size)
        self._max_packet_size = max(self._max_packet_size, packet_size)
        self._packet_count += 1
        
        # Print packet size info every 10 packets
        if self._packet_count % 10 == 0:
            print(f"BLE Packet Analysis: Count={self._packet_count}, Max Size={self._max_packet_size} bytes, Avg Size={sum(self._packet_sizes)/len(self._packet_sizes):.1f} bytes")
            if self._max_packet_size > 20:
                print("  ✅ DLE appears ENABLED (packets > 20 bytes)")
            else:
                print("  ❌ DLE appears DISABLED (packets ≤ 20 bytes)")
        
        # Decode data from bytearry->String
        dataUnpacked = event.decode("utf-8")
        # print(f"DEBUG: Raw BLE data received: {dataUnpacked}")  # Commented out for cleaner output
        if "c" in dataUnpacked:  # 'c' acts as a delimiter for data
            data_split = dataUnpacked.split(
                "c"
            )  # Split data into 2 messages using 'c' as divider
            event_data = data_split[1]  # Back half of split holds message data
            # Front half of split holds message information
            event_info = data_split[0]
            count_match = self._event_count_regex.search(
                event_info
            ).group()  # Look for data count described in data info
            self._data_length = int(count_match)
            start = event_info[0]  # Start of data
            cmd = event_info[1]  # Command the data holds
            # Data without the count
            event_without_count = f"{start}{cmd}{event_data}"
            # Parse the data and handle each part accordingly
            for element in event_without_count:
                if (
                    element == "S" and not self._start_transmission
                ):  # 'S' signifies that start of the message
                    self._start_transmission = True
                    continue  # Keep reading message
                elif self._start_transmission:  # if the message has started
                    if not self._command:
                        self._command = element  # if command is empty, set command to current element
                    elif element == "n":
                        self._num_count += 1  # Increase the num count of message
                        # Join the buffer to result
                        result = "".join(self._buffer)
                        double_parse = tryParseFloat(
                            result
                        )  # Parse the result and convert to double if possible, None if not possible
                        if double_parse is None:
                            continue  # Keep reading message
                        else:
                            self._payload.append(
                                double_parse / 100.0
                            )  # Add data to payload
                            self._buffer.clear()
                            if (
                                self._num_count == self._data_length
                            ):  # If the data length is equal to the data count
                                self.processMessage(
                                    self._command, self._payload, self._data_length
                                )
                                self._reset()  # Reset message variables for a new message
                            else:
                                continue  # Keep reading message
                    elif self._data_length != 0:
                        self._buffer.append(element)  # Add data to buffer
                    else:
                        return
                else:
                    return
        else:
            print("Unkown command!\n")

    def set_debug_event_listener(self, on_debug_event):
        self._on_debug_event = on_debug_event

    def processGeneralData(
        self, payload, datalength
    ):  # Place general data derived from message to Exo data
        self.x_time += 1
        data0 = payload[0] if len(payload) > 0 else 0  # rightTorque
        data1 = payload[1] if len(payload) > 1 else 0  # rightState
        data2 = payload[2] if len(payload) > 2 else 0  # rightSet
        data3 = payload[3] if len(payload) > 3 else 0  # leftTorque
        data4 = payload[4] if len(payload) > 4 else 0  # leftState
        data5 = payload[5] if len(payload) > 5 else 0  # leftSet
        data6 = payload[6] if datalength >= 7 and len(payload) > 6 else 0  # rightFsr
        data7 = payload[7] if datalength >= 8 and len(payload) > 7 else 0  # leftFsr
        data8 = payload[8] if datalength >= 9 and len(payload) > 8 else 0  # rightHeelFsr
        data9 = payload[9] if datalength >= 10 and len(payload) > 9 else 0  # leftHeelFsr
        data10 = payload[10] if datalength >= 11 and len(payload) > 10 else 0  # rightMotorCurrent
        data11 = payload[11] if datalength >= 12 and len(payload) > 11 else 0  # leftMotorCurrent

        # DEBUG: Print 12 data points
        print(f"12 Data Points: [0]{data0:.1f} [1]{data1:.1f} [2]{data2:.1f} [3]{data3:.1f} [4]{data4:.1f} [5]{data5:.1f} [6]{data6:.1f} [7]{data7:.1f} [8]{data8:.1f} [9]{data9:.1f} [10]{data10:.1f} [11]{data11:.1f}")

        self._chart_data.updateValues(
            data0,  # rightTorque
            data1,  # rightState
            data2,  # rightSet
            data3,  # leftTorque
            data4,  # leftState
            data5,  # leftSet
            data6,  # rightFsr
            data7,  # leftFsr
            data8,  # rightHeelFsr
            data9,  # leftHeelFsr
            data10,  # rightMotorCurrent
            data11,  # leftMotorCurrent
            0,      # placeholder
            0,      # placeholder
            0,      # placeholder
            0,      # placeholder
        )
        self._predictor.addDataPoints([data8, data9, data10, data11, 0, 0, 0, self._predictor.state]) #add data to model, if recording data
        
        self._predictor.predictModel([data8, data9, data10, data11, 0, 0, 0]) #predict results from model


        self._exo_data.addDataPoints(
            self.x_time,
            data0,  # rightTorque
            data1,  # rightState
            data2,  # rightSet
            data3,  # leftTorque
            data4,  # leftState
            data5,  # leftSet
            data6,  # rightFsr
            data7,  # leftFsr
            #store features
            data8,  # rightHeelFsr
            data9,  # leftHeelFsr
            data10,  # rightMotorCurrent
            data11,  # leftMotorCurrent
            0,      # placeholder
            self._predictor.prediction, #store prediction
            0,      # battery placeholder
            "Task", # Task placeholder
            0       # data12 placeholder
        )
        

    def processMessage(
        self, command, payload, dataLength
    ):  # Process message based on command. Only handles general data although other data is comming through
        # print(f"DEBUG: Processing command '{command}' with {dataLength} data points")  # Commented out for cleaner output
        if command == "?":  # General command
            self.processGeneralData(payload, dataLength)

    def _reset(self):  # Reset message variables
        self._start_transmission = False
        self._command = None
        self._data_length = None
        self._num_count = 0
        self._payload.clear()
        self._buffer.clear()

    def getPacketSizeStats(self):
        """Get packet size statistics for DLE detection"""
        if not self._packet_sizes:
            return "No packets received yet"
        
        avg_size = sum(self._packet_sizes) / len(self._packet_sizes)
        dle_status = "ENABLED" if self._max_packet_size > 20 else "DISABLED"
        
        return {
            "packet_count": self._packet_count,
            "max_packet_size": self._max_packet_size,
            "avg_packet_size": avg_size,
            "dle_status": dle_status,
            "all_sizes": self._packet_sizes.copy()
        }

    def UnkownDataCommand(self):
        return "Unkown Command!"


def tryParseFloat(stringVal):  # Try to parse float data from String
    try:
        return float(stringVal)  # If possible, return parsed
    except:
        return None  # If not, return None
