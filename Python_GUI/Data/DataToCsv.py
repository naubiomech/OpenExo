import csv

from datetime import datetime


class CsvWritter:
    def writeToCsv(self, exoData, disconnected=False):
        print("Creating filedata")
        # initialize array for output file
        fileData = []

        # establish field arrays for output file
        timestamps = ["EpochTimestamp"]  # New field for epoch time
        tStep = ["TStep"]
        rTorque = ["Data 0"]
        rSetP = ["Data 1"]
        rState = ["Data 2"]
        lTorque = ["Data 3"]
        lSetP = ["Data 4"]
        LState = ["Data 5"]
        lFsr = ["Data 6"]
        rFsr = ["Data 7"]

        #record our model features
        # minSV = ["minSV"]
        # maxSV = ["maxSV"]
        # minSA = ["minSA"]
        # maxSA = ["maxSA"]
        # maxFSR = ["maxFSR"]
        # stancetime = ["StanceTime"]
        # swingtime = ["SwingTime"]

        #and predicted task/state
        # Task = ["Task"]
        mark = ["Mark"]


        for xt in exoData.tStep:
            tStep.append(xt)
        for rT in exoData.rTorque:
            rTorque.append(rT)
        for rSP in exoData.rSetP:
            rSetP.append(rSP)
        for rS in exoData.rState:
            rState.append(rS)
        for lT in exoData.lTorque:
            lTorque.append(lT)
        for lSP in exoData.lSetP:
            lSetP.append(lSP)
        for lS in exoData.lState:
            LState.append(lS)
        for rF in exoData.rFsr:
            rFsr.append(rF)
        for lF in exoData.lFsr:
            lFsr.append(lF)
        # for min in exoData.MinShankVel:
        #     minSV.append(min)
        # for max in exoData.MaxShankVel:
        #     maxSV.append(max)
        # for inSA in exoData.MinShankAng:
        #     minSA.append(inSA)
        # for axSA in exoData.MaxShankAng:
        #     maxSA.append(axSA)
        # for fsr in exoData.MaxFSR:
        #     maxFSR.append(fsr)
        # for task in exoData.Task:
        #     Task.append(task)
        for trig in exoData.Mark:
            mark.append(trig)
        # for moment in exoData.StanceTime:
        #     stancetime.append(moment)
        # for moment in exoData.SwingTime:
        #     swingtime.append(moment)
        for tS in exoData.tStep:
            tStep.append(tS)
        # Add the epoch timestamps (current time in seconds) to the timestamps list
        for _ in exoData.epochTime:
            timestamps.append(_)

        # add field array with data to output file
        fileData.append(timestamps)
        fileData.append(tStep)
        fileData.append(rTorque)
        fileData.append(rSetP)
        fileData.append(rState)
        fileData.append(lTorque)
        fileData.append(lSetP)
        fileData.append(LState)
        fileData.append(lFsr)
        fileData.append(rFsr)
        # fileData.append(minSV)
        # fileData.append(maxSV)
        # fileData.append(minSA)
        # fileData.append(maxSA)
        # fileData.append(maxFSR)
        # fileData.append(stancetime)
        # fileData.append(swingtime)
        # fileData.append(Task)
        fileData.append(mark)

        
        # rotate 2D array to place lables on top
        fileDataTransposed = self.rotateArray(fileData)
        print("flipping array")

        today = datetime.now()
        base_file_name = today.strftime("%Y-%b-%d-%H-%M-%S")
        
        # Append disconnection notice to filename if applicable
        if disconnected:
            fileName = f"{base_file_name}_disconnected.csv"
        else:
            fileName = f"{base_file_name}.csv"


        try:
            with open(fileName, "w") as csvFile:  # Open file with file name
                csvwriter = csv.writer(csvFile)  # Prep file for csv data
                print("creating and opening file")

                # Write flipped 2D array to file
                csvwriter.writerows(fileDataTransposed)

                csvFile.close  # Close file

        finally:
            # ⇢  CLEAR THE BUFFER  ⇠
            self._clear_exo_data(exoData)
            print("ExoData lists emptied")

    def rotateArray(self, arrayToFlip):
        return [
            list(row) for row in zip(*arrayToFlip)
        ]  # Roate array so labels on left are on top

    def _clear_exo_data(self, exoData):
        """Empty all list‑type attributes in exoData."""
        attrs = [
            "tStep", "rTorque", "rSetP", "rState",
            "lTorque", "lSetP", "lState",
            "lFsr", "rFsr",
            # "MinShankVel", "MaxShankVel",
            # "MinShankAng", "MaxShankAng",
            # "MaxFSR", "StanceTime", "SwingTime","Task", 
            "Mark", "epochTime"
        ]
        for name in attrs:
            getattr(exoData, name).clear()