import tkinter as tk
from tkinter import (BOTTOM, CENTER, LEFT, RIGHT, TOP, E, N, S, StringVar, W,
                     X, Y, ttk)

from async_tkinter_loop import async_handler
from Widgets.Keyboard.custom_keyboard import CustomKeyboard
import json
import os
jointMap = {
    "Right hip": 1,
    "Left hip": 2,
    "Right knee": 3,
    "Left knee": 4,
    "Right ankle": 5,
    "Left ankle": 6,
    "Right elbow": 7,
    "Left elbow": 8,
}

class UpdateTorque(tk.Frame):  # Frame to start exo and calibrate
    SETTINGS_FILE = "saved_data/last_torque_settings.json"  # File to save and load settings

    def __init__(self, parent, controller):  # Constructor for Frame
        super().__init__(parent)  # Correctly initialize the tk.Frame part
        # Initialize variables
        self.controller = controller  # Controller object to switch frames
        self.previous_frame = None  # Track the previous frame
        self.current_input_index = 0  # Track the current input field for keyboard
        self.keyboard_window = None  # Single keyboard instance

        # Set the disconnection callback
        self.controller.deviceManager.on_disconnect = self.UpdateTorque_on_device_disconnected

        #UI Styling
        self.fontstyle = 'Segoe UI'

        self.bilateralButtonVar = StringVar()
        self.bilateralButtonVar.set("Bilateral Mode On")
        self.jointVar = StringVar(value="Select Joint")

        self.isBilateral = True

        self.create_widgets()
        # Load previous settings if available
        self.load_settings()

    def create_widgets(self):  # Frame UI elements
        # Back button to go back to Scan Window
        backButton = ttk.Button(self, text="Back", command=self.handle_back_button)
        backButton.pack(side=TOP, anchor=W, pady=10, padx=10)
        
        # Calibrate Menu label
        calibrationMenuLabel = ttk.Label(
            self, text="Update Controller Settings", font=(self.fontstyle, 30)
        )
        calibrationMenuLabel.pack(anchor=CENTER, side=TOP, pady=15)

        # Joint select (using tk.OptionMenu)
        joint_options = [
            "Left hip",
            "Left knee",
            "Left ankle",
            "Left elbow",
            "Right hip",
            "Right knee",
            "Right ankle",
            "Right elbow",
        ]
        self.jointVar.set(joint_options[0])  # Default value
        jointSelector = tk.OptionMenu(self, self.jointVar, *joint_options)
        jointSelector.config(font=(self.fontstyle, 20), width=15)
        menu = self.nametowidget(jointSelector.menuname)  # Access the menu part of OptionMenu
        menu.config(font=(self.fontstyle, 26))  # Larger font for options in the dropdown menu
        jointSelector.pack(pady=5)

        # Controller label
        controllerInputLabel = tk.Label(self, text="Controller", font=(self.fontstyle, 15))
        self.controllerInput = tk.Entry(self, font=(self.fontstyle, 16))  # Use Entry instead of Text for simpler input
        
        # Parameter Label
        parameterInputLabel = tk.Label(self, text="Parameter", font=(self.fontstyle, 15))
        self.parameterInput = tk.Entry(self, font=(self.fontstyle, 16)) 

        # Value label
        valueInputLabel = tk.Label(self, text="Value", font=(self.fontstyle, 15))
        self.valueInput = tk.Entry(self, font=(self.fontstyle, 16)) 

        self.inputs = [self.controllerInput, self.parameterInput, self.valueInput]  # Store input fields

        bilateralButton = ttk.Button(
            self,
            textvariable=self.bilateralButtonVar,
            width=15,
            command=self.toggleBilateral,
        )
        bilateralButton.pack(pady=5)

        # Single keyboard button
        keyboardButton = ttk.Button(
            self, text="Open Keyboard", command=self.start_keyboard_cycle
        )
        keyboardButton.pack(pady=10)

        controllerInputLabel.pack(pady=5)
        self.controllerInput.pack(padx=5)
        
        parameterInputLabel.pack(pady=5)
        self.parameterInput.pack(pady=5)

        valueInputLabel.pack(pady=5)
        self.valueInput.pack(pady=5)

        # Button to start trial
        updateTorqueButton = ttk.Button(
            self,
            text="Update Settings",
            width=10,
            command=async_handler(
                self.on_update_button_clicked,
                self.controllerInput,
                self.parameterInput,
                self.valueInput,
            ),
        )
        updateTorqueButton.pack(side=BOTTOM, fill=X, padx=20, pady=20)

    # Start the keyboard cycle, focusing on the first input field.
    def start_keyboard_cycle(self):
        self.current_input_index = 0  # Start with the first input field
        if not self.keyboard_window:  # Create the keyboard only if it doesn't already exist
            self.keyboard_window = tk.Toplevel(self)
            self.keyboard_window.title("Custom Keyboard")
            self.keyboard_window.protocol("WM_DELETE_WINDOW", self.close_keyboard)  # Handle manual close

            # Create the custom keyboard inside the Toplevel window
            self.keyboard = CustomKeyboard(
                self.keyboard_window, self.inputs[self.current_input_index], on_submit=self.keyboard_submit
            )
            self.keyboard.pack(fill=tk.BOTH, expand=True)

        self.update_keyboard_target()

    # Update the keyboard to target the current input field.
    def update_keyboard_target(self):
        target_input = self.inputs[self.current_input_index]
        self.keyboard.set_target(target_input)

    # Handle the keyboard submission and move to the next input field.
    def keyboard_submit(self, value):
        # Set the value for the current input field
        current_input = self.inputs[self.current_input_index]
        current_input.delete(0, tk.END)
        current_input.insert(0, value)

        # Move to the next input field
        self.current_input_index += 1
        if self.current_input_index < len(self.inputs):
            self.update_keyboard_target()
        else:
            self.close_keyboard()  # Close the keyboard after the last field

    # Close the keyboard and reset the current input index.
    def close_keyboard(self):
        if self.keyboard_window:
            self.keyboard_window.destroy()
            self.keyboard_window = None
        self.current_input_index = 0

    def handle_back_button(self):
        # Return to the previous frame
        active_trial_frame = self.controller.frames[self.previous_frame]
        active_trial_frame.newSelection(self)
        active_trial_frame.clear_both_plot()
        self.controller.show_frame(self.previous_frame)

        
    async def on_update_button_clicked(
        self, controllerInput, parameterInput, valueInput,
    ):
        selected_joint = self.jointVar.get()  # Get the selected joint
        joint_id = jointMap[selected_joint]

        await self.UpdateButtonClicked(
            self.isBilateral,
            joint_id,
            controllerInput,
            parameterInput,
            valueInput,
        )

        # Save settings after updating
        self.save_settings()

    async def UpdateButtonClicked(
        self, isBilateral, joint, controllerInput, parameterInput, valueInput,
    ):
        controllerVal = float(controllerInput.get()) 
        parameterVal = float(parameterInput.get())
        valueVal = float(valueInput.get())

        print(f"bilateral: {isBilateral}")
        print(f"joint: {joint}")
        print(f"controller: {controllerVal}")
        print(f"paramter: {parameterVal}")
        print(f"value: {valueVal}")

        # Set Torque
        await self.controller.deviceManager.updateTorqueValues(
            [isBilateral, joint, controllerVal, parameterVal, valueVal]
        )

        if self.previous_frame:
            self.controller.show_frame(self.previous_frame)
            active_trial_frame = self.controller.frames[self.previous_frame]
            active_trial_frame.newSelection(self)
        else:
            self.controller.show_frame("ActiveTrial")
            active_trial_frame = self.controller.frames["ActiveTrial"]
            active_trial_frame.newSelection(self)

    # Save the current settings to a file.
    def save_settings(self):
        settings = {
            "joint": self.jointVar.get(),
            "controller": self.controllerInput.get(),
            "parameter": self.parameterInput.get(),
            "value": self.valueInput.get(),
            "isBilateral": self.isBilateral,
        }
        with open(self.SETTINGS_FILE, "w") as f:
            json.dump(settings, f, indent=4)
        print(f"Settings saved: {settings}")

    # Load the last saved settings.
    def load_settings(self):
        if os.path.exists(self.SETTINGS_FILE):
            with open(self.SETTINGS_FILE, "r") as f:
                settings = json.load(f)
                self.jointVar.set(settings.get("joint", "Left hip"))
                self.controllerInput.insert(0, settings.get("controller", ""))
                self.parameterInput.insert(0, settings.get("parameter", ""))
                self.valueInput.insert(0, settings.get("value", ""))
                self.isBilateral = settings.get("isBilateral", True)
                self.bilateralButtonVar.set(
                    "Bilateral Mode On" if self.isBilateral else "Bilateral Mode Off"
                )
            print(f"Loaded settings: {settings}")

    def newSelection(self, event):
        self.jointVar.set(self.jointSelector.get())

    def UpdateTorque_on_device_disconnected(self):
        tk.messagebox.showwarning("Device Disconnected", "Please Reconnect")
        
        self.controller.trial.loadDataToCSV(
            self.controller.deviceManager, True
        )  # Load data from Exo into CSV
        self.controller.show_frame("ScanWindow")# Navigate back to the scan page
        self.controller.frames["ScanWindow"].show()  # Call show method to reset elements
            

    def toggleBilateral(self):
        if self.isBilateral is True:
            self.isBilateral = False
            self.bilateralButtonVar.set("Bilateral Mode Off")
        else:
            self.isBilateral = True
            self.bilateralButtonVar.set("Bilateral Mode On")