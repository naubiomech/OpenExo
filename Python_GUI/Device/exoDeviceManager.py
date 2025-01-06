import asyncio
import struct
import sys

import numpy as np
from bleak import BleakClient, BleakScanner
from bleak.backends.characteristic import BleakGATTCharacteristic
from bleak.backends.device import BLEDevice
from bleak.backends.scanner import AdvertisementData

from Device import realTimeProcessor


class ExoDeviceManager:

    def __init__(self, on_disconnect=None):
        self._realTimeProcessor = realTimeProcessor.RealTimeProcessor()
        self.on_disconnect = on_disconnect  # Store the callback

        self.deviceAddress = None
        self.available_devices = {}  # List to store available devices
        self.device = None
        self.client = None
        self.services = None
        self.scanResults = None
        self.disconnecting_intentionally = False  # Flag for intentional disconnect

        self.characteristics = ""

        # Initialize FSR values
        self.curr_left_fsr_value = 0.25
        self.curr_right_fsr_value = 0.25

        self.device_mac_address = None

        # UUID characteristic
        self.UART_TX_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"  # Nordic NUS TX UUID
        self.UART_RX_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"  # Nordic NUS RX UUID
        self.UART_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"  # Nordic Service UUID

        # Joint dictionary to map menu selection to joint ID
        self.jointDictionary = {
            1: 33.0,
            2: 65.0,
            3: 34.0,
            4: 66.0,
            5: 36.0,
            6: 68.0,
            7: 40.0,
            8: 72.0
        }

        self.isConnected = False

    # Set scan results from the BLE scanner
    def set_scan_results(self, scans):
        self.scanResults = scans

    # Set the current device
    def set_device(self, deviceVal):
        self.device = deviceVal

    # Set the MAC address of the device
    def set_deviceAddress(self, deviceAddress):
        self.device_mac_address = deviceAddress

    # Set the BLE client
    def set_client(self, clientVal):
        self.client = clientVal

    # Set the services of the device
    def set_services(self, servicesVal):
        self.services = servicesVal

    # Handle incoming data from BLE characteristic
    async def DataIn(self, sender: BleakGATTCharacteristic, data: bytearray):
        self._realTimeProcessor.processEvent(data)
        await self.MachineLearnerControl()

    # Control the machine learning model based on incoming data
    async def MachineLearnerControl(self):
        if self._realTimeProcessor._predictor.modelExists:  # Check if the machine learning model exists
            if self._realTimeProcessor._predictor.controlMode == 1:  # If control is authorized
                if self._realTimeProcessor._exo_data.Task[-2] != self._realTimeProcessor._predictor.prediction:
                    print("Prediction Changed") 
                    # Check current prediction and adjust stiffness accordingly
                    if self._realTimeProcessor._predictor.prediction == 1:  # Level
                        await self.sendStiffness(0.5)
                    elif self._realTimeProcessor._predictor.prediction == 2:  # Descend
                        await self.sendStiffness(1)
                    elif self._realTimeProcessor._predictor.prediction == 3:  # Ascend
                        await self.sendStiffness(0)
                    else:
                        await self.sendStiffness(0.5)

    # Get characteristic handle by UUID
    def get_char_handle(self, char_UUID):
        return self.services.get_service(self.UART_SERVICE_UUID).get_characteristic(char_UUID)

    # Filter devices based on advertising data
    def filterExo(self, device: BLEDevice, adv: AdvertisementData):
        return self.UART_SERVICE_UUID.lower() in adv.service_uuids

    # Handle disconnect callback
    def handleDisconnect(self, _: BleakClient):
        print("Disconnecting...")  # Check if this is reached
        if not self.disconnecting_intentionally:  # Only call if not an intentional disconnect
            if self.on_disconnect:
                print("Calling disconnect callback...")
                self.on_disconnect()
            else:
                print("No disconnect callback assigned.")
        else:
            print("Disconnect was intentional, skipping callback.")

    # Disconnect from the BLE device
    async def disconnect(self):
        if self.client is not None and self.isConnected:
            try:
                self.disconnecting_intentionally = True  # Set the flag before disconnecting
                await self.client.disconnect()  # Disconnect from the client
                self.isConnected = False  # Update connection status
                print("Successfully disconnected from the device.")
            except Exception as e:
                print(f"Failed to disconnect: {e}")
            finally:
                self.disconnecting_intentionally = False  # Reset the flag after disconnecting
        else:
            print("No device is currently connected.")

    # Start motors of the Exo
    async def startExoMotors(self):
        await asyncio.sleep(1)
        print("using bleak start\n")
        command = bytearray(b"E")
        char = self.get_char_handle(self.UART_TX_UUID)

        try:
            await self.client.write_gatt_char(char, command, False)
        except Exception as e:
            print(f"An error occurred: {e}")

    # Calibrate the torque of the Exo
    async def calibrateTorque(self):
        print("using bleak torque\n")
        command = bytearray(b"H")
        char = self.get_char_handle(self.UART_TX_UUID)

        await self.client.write_gatt_char(char, command, False)

    # Calibrate the FSR sensors
    async def calibrateFSRs(self):
        print("using bleak FSR\n")
        command = bytearray(b"L")
        char = self.get_char_handle(self.UART_TX_UUID)

        await self.client.write_gatt_char(char, command, False)

    # Turn off motors
    async def motorOff(self):
        command = bytearray(b"w")
        char = self.get_char_handle(self.UART_TX_UUID)

        await self.client.write_gatt_char(char, command, True)

    # Update torque values based on the parameters
    async def updateTorqueValues(self, parameter_list):
        totalLoops = 1
        loopCount = 0
        float_values = parameter_list

        # Check if bilateral mode
        if parameter_list[0] is True:
            totalLoops = 2
        # Loop if bilateral mode. No loop otherwise
        while loopCount != totalLoops:
            command = b"f"
            char = self.get_char_handle(self.UART_TX_UUID)
            await self.client.write_gatt_char(char, command, False)

            for i in range(1, len(float_values)):
                if i == 1:
                    # Adjust joint ID for bilateral mode
                    if loopCount == 1 and float_values[1] % 2 == 0:
                        print(f"joint: {self.jointDictionary[float_values[i]] - 32}")
                        float_bytes = struct.pack("<d", self.jointDictionary[float_values[i]] - 32)
                    elif loopCount == 1 and float_values[1] % 2 != 0:
                        print(f"joint: {self.jointDictionary[float_values[i]] + 32}")
                        float_bytes = struct.pack("<d", self.jointDictionary[float_values[i]] + 32)
                    else:
                        print(f"joint: {self.jointDictionary[float_values[i]]}")
                        float_bytes = struct.pack("<d", self.jointDictionary[float_values[i]])
                else:
                    print(float_values[i])
                    float_bytes = struct.pack("<d", float_values[i])
                char = self.get_char_handle(self.UART_TX_UUID)
                await self.client.write_gatt_char(char, float_bytes, False)

            loopCount += 1

    # Connect to a specific device
    async def connect(self, device):
        print(device)

    # Search for available BLE devices
    async def searchDevices(self):
        i = 0
        self.available_devices.clear()
        while i < 3:
            try:
                device = await BleakScanner.find_device_by_filter(self.filterExo)
                if device:
                    self.available_devices[device.address] = device.name  # Store name with address as key

                # Print all available devices every few seconds
                print("Currently available devices:")
                for address, name in self.available_devices.items():
                    print(f"{name} - {address}")

                await asyncio.sleep(1)  # Adjust the scan interval as needed
                i += 1
            except Exception as e:
                print(f"Error during device scan: {e}")
                return "false"  # Return false if an error occurs
        return self.available_devices
    
    # Scan for BLE devices and attempt to connect
    async def scanAndConnect(self):
        print("Using Bleak scan...")
        print("Scanning...")

        attempts = 4
        for attempt in range(attempts):
            print(f"Attempt {attempt + 1} of {attempts}")

            # Scan for devices using the filter
            device = await BleakScanner.find_device_by_filter(self.filterExo)

            if device:
                print(f"Found device: {device.name} - {device.address}")
                
                # Check if the found device's address matches the specified address
                if device.address == self.device_mac_address:
                    print("Matching device found. Connecting...")
                    
                    self.set_device(device)  # Set the device
                    self.set_client(BleakClient(device, disconnected_callback=self.handleDisconnect))
                    
                    # Try to connect to the Exo
                    try:
                        await self.client.connect()
                        self.isConnected = True
                        print("Is Exo connected: " + str(self.isConnected))
                        print(self.client)

                        # Get list of services from Exo
                        self.set_services(await self.client.get_services())
                        
                        # Start incoming data stream
                        await self.client.start_notify(self.UART_RX_UUID, self.DataIn)
                        return True  # Successful connection
                    except Exception as e:
                        print(f"Failed to connect: {e}")
                        return False  # Connection failed
                else:
                    print("Found device does not match the specified address.")
            else:
                print("No device found.")

        print("Max attempts reached. Could not connect.")
        return False  # Return False if all attempts fail

    # Send FSR values to Exo
    async def sendFsrValues(self, left_fsr, right_fsr):
        command = bytearray(b"R")  # Command to indicate sending FSR values
        char = self.get_char_handle(self.UART_TX_UUID)

        # Update the stored values
        self.curr_left_fsr_value = left_fsr
        self.curr_right_fsr_value = right_fsr

        # Send the command to indicate that we are sending FSR values
        await self.client.write_gatt_char(char, command, False)

        # Pack the left and right FSR values as doubles
        for fsr_value in (left_fsr, right_fsr):  # Loop over both values
            fsr_bytes = struct.pack("<d", fsr_value)  # Pack each value as double
            await self.client.write_gatt_char(char, fsr_bytes, False)

    # Send preset FSR values to Exo
    async def sendPresetFsrValues(self):
        await self.sendFsrValues(self.curr_left_fsr_value, self.curr_right_fsr_value)

    # Send stop trial command to Exo
    async def stopTrial(self):
        command = bytearray(b"G")
        char = self.get_char_handle(self.UART_TX_UUID)

        await self.client.write_gatt_char(char, command, False)

    # Switch to assist mode
    async def switchToAssist(self):
        command = bytearray(b"c")
        char = self.get_char_handle(self.UART_TX_UUID)

        await self.client.write_gatt_char(char, command, False)
        print("using bleak assist\n")

    # Switch to resist mode
    async def switchToResist(self):
        print("using bleak resist\n")
        command = bytearray(b"S")
        char = self.get_char_handle(self.UART_TX_UUID)

        await self.client.write_gatt_char(char, command, False)

    # Send stiffness values to Exo
    async def sendStiffness(self, stiffness):
        command = bytearray(b"A")  # Character code for stiffness
        char = self.get_char_handle(self.UART_TX_UUID)
        await self.client.write_gatt_char(char, command, False)  # Send the command code

        stiff_bytes = struct.pack("<d", stiffness)
        await self.client.write_gatt_char(char, stiff_bytes, False)  # Send the stiffness value
        print(f"Stiffness is {stiffness}")

    # Helper function to send new stiffness values to Exo
    async def newStiffness(self, stiffnessInput):
        stiffnessVal = float(stiffnessInput)
        await self.sendStiffness(stiffnessVal)  # Send the new stiffness value

    # Play
    async def play(self):
        command = bytearray(b"X")
        char = self.get_char_handle(self.UART_TX_UUID)

        await self.client.write_gatt_char(char, command, True)

    # Pause
    async def pause(self):
        command = bytearray(b"W")
        char = self.get_char_handle(self.UART_TX_UUID)

        await self.client.write_gatt_char(char, command, True)
