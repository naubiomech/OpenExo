import csv

from datetime import datetime

'''
this file is essentially the same as DataToCSV.py, except trimmed down 
just to include the features and labels used to generate the training data for our machine learning model
label = human supvervised classification
task = machine predicted walking task 
'''
class CsvWritter:
    def writeToCsv(self, exoData, predictor):
        print("Creating filedata")
        # initialize array for output file
        fileData = []
        # establish field arrays for output file
        #tStep = ["TStep"]
        minSV = ["minSV"]
        maxSV = ["maxSV"]
        minSA = ["minSA"]
        maxSA = ["maxSA"]
        maxFSR = ["maxFSR"]
        stancetime = ["StanceTime"]
        swingtime = ["SwingTime"]
        labels = ["Labels"] #from human supervision

        # append data to field array
        """         
        for xt in exoData.tStep:
        tStep.append(xt) """
        for min in [row[0] for row in predictor.database]:
            minSV.append(min)
        for max in [row[1] for row in predictor.database]:
            maxSV.append(max)
        for inSA in [row[2] for row in predictor.database]:
            minSA.append(inSA)
        for axSA in [row[3] for row in predictor.database]:
            maxSA.append(axSA)
        for fsr in [row[4] for row in predictor.database]:
            maxFSR.append(fsr)
        for moment in [row[5] for row in predictor.database]:
            stancetime.append(moment)
        for moment in [row[6] for row in predictor.database]:
            swingtime.append(moment)
        for lab in [row[7] for row in predictor.database]:
            labels.append(lab)

        # add field array with data to output file
        #fileData.append(tStep)
        fileData.append(minSV)
        fileData.append(maxSV)
        fileData.append(minSA)
        fileData.append(maxSA)
        fileData.append(maxFSR)
        fileData.append(stancetime)
        fileData.append(swingtime)
        fileData.append(labels)


        # rotate 2D array to place lables on top
        fileDataTransposed = self.rotateArray(fileData)
        print("flipping array")

        today = datetime.now()  # Pull system time and date
        fileName = "ModelData"  # Format file name based on YYYY-MM-DD-HH:MM:SS
        fileName += ".csv"  # Add .csv to file name
        print("file is: ", fileName)

        with open(fileName, "w") as csvFile:  # Open file with file name
            csvwriter = csv.writer(csvFile)  # Prep file for csv data
            print("creating and opening file")

            # Write flipped 2D array to file
            csvwriter.writerows(fileDataTransposed)

            csvFile.close  # Close file

    def rotateArray(self, arrayToFlip):
        return [
            list(row) for row in zip(*arrayToFlip)
        ]  # Roate array so labels on left are on top
