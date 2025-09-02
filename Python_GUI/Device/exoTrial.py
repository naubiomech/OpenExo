import asyncio

import Data.DataToCsv as DataToCsv
import Data.SaveModelData as saveModelData  # (unused here but keeping your import)

# -------------------------
# (Optional) CLI menus
# -------------------------

def trialMenu():
    while True:
        print(
            """
------------------------
|1. Update Torque      |
|2. End Trial          |
------------------------"""
        )
        try:
            option = int(input())
        except Exception:
            option = 0
        if option in (1, 2):
            return option
        print("Choose a valid option")

def updateTorqueMenu():
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
        try:
            joint = float(input())
        except Exception:
            joint = -1
        if 1 <= joint <= 6:
            break
        print("Choose a valid option")
    print("Enter Controller Number: ")
    controller = float(input())
    print("Enter Parameter: ")
    parameter = float(input())
    print("Enter Value: ")
    value = float(input())

    isBilateral = (bilateralOption in ("y", "Y", ""))
    return [isBilateral, joint, controller, parameter, value]

# -------------------------
# Helpers
# -------------------------

def lbsToKilograms(pounds):
    return float(pounds) * 0.45359237

# -------------------------
# Dual-only Trial
# -------------------------

class ExoTrial:
    """
    Dual-only trial controller compatible with your ScanWindow calls:
      await trial.calibrate(deviceManager)
      await trial.beginTrial(deviceManager)
      await trial.beginTrialDebug(deviceManager)
      await trial.systemUpdate(deviceManager)    # optional CLI loop
    """

    def __init__(self, isKilograms, weight, isAssist):
        self.csvWriter = DataToCsv.CsvWritter()
        self.isKilograms = isKilograms
        self.weight = weight if isKilograms else lbsToKilograms(weight)
        self.isAssist = isAssist  # True -> assist mode, False -> resist

    # -----------------------------------------------------------------

    async def calibrate(self, deviceManager):
        """
        Dual calibration: torque (and FSR if available).
        """
        # Torque calibration on both
        try:
            resT = await deviceManager.calibrateTorque_both()
            print("CalibrateTorque (both):", resT)
        except AttributeError:
            # fallback if dual helper missing
            print("Missing calibrateTorque_both; please add it to ExoDeviceManager.")

        # FSR calibration on both (optional but typical)
        try:
            resF = await deviceManager.calibrateFSRs_both()
            print("CalibrateFSRs (both):", resF)
        except AttributeError:
            print("Missing calibrateFSRs_both; please add it to ExoDeviceManager.")

    # -----------------------------------------------------------------

    async def beginTrial(self, deviceManager):
        """
        Start the dual trial:
          - set mode (assist/resist)
          - start stream
          - motors ON
          - send current FSR preset (optional but matches your single flow)
        """
        print("Starting dual trial…")
        await asyncio.sleep(0.5)

        # Mode
        try:
            if self.isAssist:
                await deviceManager.switchToAssist_both()
            else:
                await deviceManager.switchToResist_both()
        except AttributeError:
            print("Missing switchToAssist_both/switchToResist_both in ExoDeviceManager.")

        # Start stream + motors
        try:
            await deviceManager.start_stream_both()
        except AttributeError:
            print("Missing start_stream_both; please add it to ExoDeviceManager.")

        try:
            # Example in Trial begin flow (before motorOn_both)
            await deviceManager.start_motors_both()
            await asyncio.sleep(0.05)  # tiny dwell helps some stacks
            await deviceManager.motorOn_both()
            print("Motors ON (both)")
        except AttributeError:
            print("Missing motorOn_both in ExoDeviceManager.")

        # FSR preset (header 'R' + two doubles) — mirrors your single sendPresetFsrValues()
        try:
            left = getattr(deviceManager, "curr_left_fsr_value", 0.25)
            right = getattr(deviceManager, "curr_right_fsr_value", 0.25)
            await deviceManager.sendFsrPreset_both(left, right)
            print(f"FSR preset sent to both (L={left}, R={right})")
        except AttributeError:
            print("sendFsrPreset_both not found; you can add it or skip FSR preset.")

    # -----------------------------------------------------------------

    async def beginTrialDebug(self, deviceManager):
        """
        Debug variant: stream ON, motors OFF — safe to watch data.
        """
        print("Starting dual trial (DEBUG)…")
        await asyncio.sleep(0.5)
        try:
            await deviceManager.start_stream_both()
        except AttributeError:
            print("Missing start_stream_both.")
        try:
            await deviceManager.motorOff_both()
            print("Motors OFF (debug)")
        except AttributeError:
            print("Missing motorOff_both.")

    # -----------------------------------------------------------------

    async def systemUpdate(self, deviceManager):
        """
        Optional CLI loop to update torque values during the trial.
        GUI users can ignore this and drive updates from buttons instead.
        """
        menuSelection = int(trialMenu())
        while menuSelection != 2:
            parameter_list = updateTorqueMenu()
            # Send torque updates to BOTH
            try:
                await deviceManager.updateTorqueValues_both(parameter_list)
            except AttributeError:
                print("Missing updateTorqueValues_both; add it to ExoDeviceManager.")
            menuSelection = int(trialMenu())

        # End trial
        await self._shutdown_and_save(deviceManager)

    # -----------------------------------------------------------------

    async def _shutdown_and_save(self, deviceManager):
        """
        Shared shutdown: motors off → stop trial → disconnect → CSV.
        """
        try:
            await deviceManager.motorOff_both()
        except AttributeError:
            print("Missing motorOff_both.")

        try:
            await deviceManager.stopTrial_both()
        except AttributeError:
            print("Missing stopTrial_both.")

        # Proactively disconnect both
        try:
            await deviceManager.disconnect_all()
        except Exception as e:
            print("disconnect_all error:", e)

        # Write CSV (current RealTimeProcessor aggregate)
        self.loadDataToCSV(deviceManager, disconnect=True)

    # -----------------------------------------------------------------

    def loadDataToCSV(self, deviceManager, disconnect=False):
        # Grab per-device buffers (if you created them)
        exo_map = deviceManager.get_all_exo_data()
        print(f"Found {len(exo_map)} devices with data")
        
        for addr, exo in exo_map.items():
            print(f"Device {addr} has {len(exo.epochTime)} data points")

        if exo_map:
            # One CSV per connected device
            for addr, exo in exo_map.items():
                tag = addr.replace(":", "").replace("-", "")
                print(f"Writing CSV for device {addr} with tag {tag}")
                self.csvWriter.writeToCsv(
                    exo,
                    file_tag=tag,
                    disconnected=disconnect,
                )
        else:
            # Legacy single buffer fallback
            print("No device-specific data found, using main processor")
            print(f"Main processor has {len(deviceManager._realTimeProcessor._exo_data.epochTime)} data points")
            self.csvWriter.writeToCsv(
                deviceManager._realTimeProcessor._exo_data,
                file_tag="SINGLE",
                disconnected=disconnect,
            )
