import asyncio

import Data.DataToCsv as DataToCsv
import Data.SaveModelData as saveModelData

# Options during trial. Change torque settings or end trial


def trialMenu():
    while True:
        print(
            """
------------------------
|1. Update Torque      |
|2. End Trial          |
------------------------"""
        )
        option = int(input())
        if option == 1 or option == 2:
            return option
        print("Choose a valid option")


# Options for updating the torque settings


def updateTorqueMenu():
    # Choosing yes runs both exo joints with the same settings at once. No only updates selected joint
    print("Run in Bilateral Mode? (y/n): ")
    bilateralOption = input()
    while True:
        print("Select Joint")
        print(
            """------------------
|1. Right Hip    |
|2. Left Hip     |
|3. Right Knee   |
|4. Left Knee    |
|5. Right Ankle  |
|6. Left Ankle   |
------------------"""
        )
        joint = float(input())
        if joint >= 1 and joint <= 6:
            break
        print("Choose a valid option")
    print("Enter Controller Number: ")
    controller = float(input())
    print("Enter Parameter: ")
    parameter = float(input())
    print("Enter Value: ")
    value = float(input())

    # check for bilateral                         Type y, Y, or hit enter to select yes for bilateral
    if bilateralOption == "y" or bilateralOption == "Y" or bilateralOption == "":
        isBilateral = True
    else:
        isBilateral = False

    return [isBilateral, joint, controller, parameter, value]


def lbsToKilograms(pounds):
    convertionConstant = 0.45359237  # Constant for converting lbs->kg
    return float(pounds) * convertionConstant


class ExoTrial:
    def __init__(self, isKilograms, weight, isAssist):
        self.csvWriter = DataToCsv.CsvWritter()
        self.isKilograms = isKilograms
        if not isKilograms:  # Convert from lbs->kg if weight is in lbs
            self.weight = lbsToKilograms(weight)
        else:
            self.weight = weight
        self.isAssist = isAssist

    # -----------------------------------------------------------------------------

    async def calibrate(self, deviceManager):  # sends start motor command to Exo

        await deviceManager.calibrateTorque()

    # -----------------------------------------------------------------------------

    # Start trial and send initial torque commands
    async def beginTrial(self, deviceManager):
        print("Starting trial...")
        await asyncio.sleep(1)
        await deviceManager.startExoMotors()  # Sets Exo motors to receive commands
        print("start motors\n")

        await deviceManager.calibrateFSRs()  # Begins Exo calibration
        print("calibrate fsr\n")
        # Send FSR value to Exo FSR
        await deviceManager.sendPresetFsrValues()

    # -----------------------------------------------------------------------------

    async def systemUpdate(self, deviceManager):  # Handles Next Steps After Baseline

        # Ensure to enter loop at least once
        menuSelection = int(trialMenu())
        while menuSelection != 2:  # Keep getting torque values until end trial
            parameter_list = updateTorqueMenu()  # Menu for updating torque

            # Send torque values to Exo
            await deviceManager.updateTorqueValues(parameter_list)

            menuSelection = int(trialMenu())  # Get trial menu for loop

        # End trial
        await deviceManager.motorOff()  # Turn off motors
        await deviceManager.stopTrial()  # Tell Exo to end trial
        deviceManager.handleDisconnect(
            deviceManager.client)  # Disconnect from Exo
        self.loadDataToCSV(deviceManager)  # Load data from Exo into CSV

    # -----------------------------------------------------------------------------

    # Loads exo data into csv
    def loadDataToCSV(self, deviceManager,disconnect = False):
        self.csvWriter.writeToCsv(deviceManager._realTimeProcessor._exo_data,disconnect)
