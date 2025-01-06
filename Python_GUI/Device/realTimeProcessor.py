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
        

    def processEvent(self, event):
        # Decode data from bytearry->String
        dataUnpacked = event.decode("utf-8")
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
        rightTorque = payload[0] if len(payload) > 0 else 0
        rightState = payload[1] if len(payload) > 1 else 0
        rightSet = payload[2] if len(payload) > 2 else 0
        leftTorque = payload[3] if len(payload) > 3 else 0
        leftState = payload[4] if len(payload) > 4 else 0
        leftSet = payload[5] if len(payload) > 5 else 0
        rightFsr = payload[6] if datalength >= 7 and len(payload) > 6 else 0
        leftFsr = payload[7] if datalength >= 8 and len(payload) > 7 else 0
        minSV = payload[8] if datalength >= 9 and len(payload) > 8 else 0
        maxSV = payload[9] if datalength >= 10 and len(payload) > 9 else 0
        minSA = payload[10] if datalength >= 11 and len(payload) > 10 else 0
        maxSA = payload[11] if datalength >= 12 and len(payload) > 11 else 0
        battery = payload[12] if datalength >= 13 and len(payload) > 12 else 0
        maxFSR = payload[13] if datalength >= 14 and len(payload) > 13 else 0
        stancetime = payload[14] if datalength >= 15 and len(payload) > 14 else 0
        swingtime = payload[15] if datalength >= 16 and len(payload) > 15 else 0

        self._chart_data.updateValues(
            rightTorque,
            rightState,
            leftTorque,
            leftState,
            rightSet,
            leftSet,
            rightFsr,
            leftFsr,
        )
        self._predictor.addDataPoints([minSV,maxSV,minSA,maxSA,maxFSR,stancetime,swingtime,self._predictor.state]) #add data to model, if recording data
        
        self._predictor.predictModel([minSV,maxSV,minSA,maxSA, maxFSR,stancetime,swingtime]) #predict results from model


        self._exo_data.addDataPoints(
            self.x_time,
            rightTorque,
            rightState,
            rightSet,
            leftTorque,
            leftState,
            leftSet,
            rightFsr,
            leftFsr,
            #store features
            minSV,
            maxSV,
            minSA,
            maxSA,
            maxFSR,
            stancetime,
            swingtime,
            self._predictor.prediction, #store prediction
            battery
        )
        

    def processMessage(
        self, command, payload, dataLength
    ):  # Process message based on command. Only handles general data although other data is comming through
        if command == "?":  # General command
            self.processGeneralData(payload, dataLength)

    def _reset(self):  # Reset message variables
        self._start_transmission = False
        self._command = None
        self._data_length = None
        self._num_count = 0
        self._payload.clear()
        self._buffer.clear()

    def UnkownDataCommand(self):
        return "Unkown Command!"


def tryParseFloat(stringVal):  # Try to parse float data from String
    try:
        return float(stringVal)  # If possible, return parsed
    except:
        return None  # If not, return None
