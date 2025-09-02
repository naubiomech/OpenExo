import csv

from datetime import datetime


class CsvWritter:
    def writeToCsv(self, exoData, *,file_tag: str = "", disconnected=False):
        print("Creating filedata")
        print(f"ExoData has {len(exoData.epochTime)} data points")
        # initialize array for output file
        fileData = []

        # establish field arrays for output file
        timestamps = ["EpochTimestamp"]  # New field for epoch time
        tStep = ["TStep"]
        rTorque = ["Data 0"]
        rState = ["Data 1"]
        rSetP = ["Data 2"]
        lTorque = ["Data 3"]
        LState = ["Data 4"]
        lSetP = ["Data 5"]
        rFsr = ["Data 6"]
        lFsr = ["Data 7"]

        #record our model features
        minSV = ["Data 8"]
        maxSV = ["Data 9"]
        # minSA = ["minSA"]
        # maxSA = ["maxSA"]
        # maxFSR = ["maxFSR"]
        # stancetime = ["StanceTime"]
        # swingtime = ["SwingTime"]

        #and predicted task/state
        # Task = ["Task"]
        mark = ["Mark"]

        for _ in exoData.epochTime:
            timestamps.append(_)
        for xt in exoData.tStep:
            tStep.append(xt)
        for rT in exoData.rTorque:
            rTorque.append(rT)
        for rS in exoData.rState:
            rState.append(rS)
        for rSP in exoData.rSetP:
            rSetP.append(rSP)
        for lT in exoData.lTorque:
            lTorque.append(lT)
        for lS in exoData.lState:
            LState.append(lS)
        for lSP in exoData.lSetP:
            lSetP.append(lSP)
        for rF in exoData.rFsr:
            rFsr.append(rF)
        for lF in exoData.lFsr:
            lFsr.append(lF)
        for max in exoData.MaxShankVel:
            maxSV.append(max)
        for min in exoData.MinShankVel:
            minSV.append(min)
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
        # for tS in exoData.tStep:
        #     tStep.append(tS)

        # add field array with data to output file
        fileData.append(timestamps)
        fileData.append(tStep)
        fileData.append(rTorque)
        fileData.append(rState)
        fileData.append(rSetP)
        fileData.append(lTorque)
        fileData.append(LState)
        fileData.append(lSetP)
        fileData.append(rFsr)
        fileData.append(lFsr)
        fileData.append(minSV)
        fileData.append(maxSV)

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

        base = datetime.now().strftime("%Y-%b-%d-%H-%M-%S")
        if file_tag:
            base += f"_{file_tag}"
        if disconnected:
            base += "_disconnected"
        fileName = f"{base}.csv"

        try:
            with open(fileName, "w", newline="") as csvFile:
                csv.writer(csvFile).writerows(fileDataTransposed)
                print("creating and opening file")
        finally:
            self._clear_exo_data(exoData)
            print("ExoData lists emptied")

    def rotateArray(self, arrayToFlip):
        return [list(row) for row in zip(*arrayToFlip)]

    def _clear_exo_data(self, exoData):
        attrs = [
            "tStep","rTorque","rSetP","rState",
            "lTorque","lSetP","lState",
            "lFsr","rFsr",
            "MinShankVel","MaxShankVel",
            "Mark","epochTime"
        ]
        for name in attrs:
            getattr(exoData, name).clear()