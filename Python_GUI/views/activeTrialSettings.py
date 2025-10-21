import tkinter as tk
from tkinter import (BOTTOM, CENTER, LEFT, RIGHT, TOP, E, N, S, StringVar, W,
                     X, Y, ttk)

from async_tkinter_loop import async_handler
from Widgets.Keyboard.custom_keyboard import CustomKeyboard

from Device import realTimeProcessor

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
        self.controller_index = 0
        self.parameter_index = 0
        self.create_widgets()
        self.dynamic_dropdown = False
        # Don't update dropdowns at init; wait for newSelection when data might be available

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

        ''' OLD CODE - ELLIOTT
        # Controller label
        controllerInputLabel = tk.Label(self, text="Controller", font=(self.fontstyle, 15))
        self.controllerInput = tk.Entry(self, font=(self.fontstyle, 16))  # Use Entry instead of Text for simpler input
        
        # Parameter Label
        parameterInputLabel = tk.Label(self, text="Parameter", font=(self.fontstyle, 15))
        self.parameterInput = tk.Entry(self, font=(self.fontstyle, 16)) 
        '''
        # START OF ELLIOTTS CODE
        # Controller label and dropdown
        controllerInputLabel = tk.Label(self, text="Controller", font=(self.fontstyle, 15))
        self.controllerVar = StringVar()
        self.controllerInput = ttk.Combobox(
            self, 
            textvariable=self.controllerVar,
            state="readonly",
            font=(self.fontstyle, 16),
            width=20
        )
        controllerInputLabel.pack(pady=5)
        self.controllerInput.pack(padx=5)
        
        # Parameter Label and dropdown
        parameterInputLabel = tk.Label(self, text="Parameter", font=(self.fontstyle, 15))
        self.parameterVar = StringVar()
        self.parameterInput = ttk.Combobox(
            self, 
            textvariable=self.parameterVar,
            state="readonly",
            font=(self.fontstyle, 16),
            width=20
        )
        parameterInputLabel.pack(pady=5)
        self.parameterInput.pack(pady=5)
        # END OF ELLIOTTS CODE - PUT NEW PARAMETERS IN INPUT IF NEEDED

        # Manual entry fallbacks (shown when no controllers/parameters are received)
        self.manualControllerLabel = tk.Label(self, text="Controller Index", font=(self.fontstyle, 15))
        self.controllerIndexEntry = tk.Entry(self, font=(self.fontstyle, 16))
        self.manualParameterLabel = tk.Label(self, text="Parameter Index", font=(self.fontstyle, 15))
        self.parameterIndexEntry = tk.Entry(self, font=(self.fontstyle, 16))
        # Start hidden; toggle visible when needed
        # We use pack_forget/show logic in helper below

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

            ),
        )
        updateTorqueButton.pack(side=BOTTOM, fill=X, padx=20, pady=20)

    def _set_manual_mode(self, enabled: bool):
        """Toggle between dropdown mode and manual-index-entry mode."""
        try:
            if enabled:
                # Hide dropdowns
                self.controllerInput.pack_forget()
                self.parameterInput.pack_forget()
                # Show manual entries
                self.manualControllerLabel.pack(pady=5)
                self.controllerIndexEntry.pack(pady=5)
                self.manualParameterLabel.pack(pady=5)
                self.parameterIndexEntry.pack(pady=5)
            else:
                # Hide manual entries
                self.manualControllerLabel.pack_forget()
                self.controllerIndexEntry.pack_forget()
                self.manualParameterLabel.pack_forget()
                self.parameterIndexEntry.pack_forget()
                # Show dropdowns
                self.controllerInput.pack(padx=5)
                self.parameterInput.pack(pady=5)
            self.dynamic_dropdown = True
        except Exception:
            pass

    # START OF ELLIOTTS CODE
    def populate_controller_dropdown(self):
        """Populate the controller dropdown with available controllers"""
        try:
            controllers = self.controller.deviceManager._realTimeProcessor.controllers
            controller_parameters = self.controller.deviceManager._realTimeProcessor.controller_parameters
            
            print(f"DEBUG: Found controllers: {controllers}")
            print(f"DEBUG: Found controller parameters: {controller_parameters}")
            
            if controllers:
                self.controllerInput['values'] = controllers
                # Set to first controller if nothing selected
                if not self.controllerVar.get() or self.controllerVar.get() not in controllers:
                    self.controllerVar.set(controllers[0])
                # Bind the controller selection to update parameters
                self.controllerInput.bind('<<ComboboxSelected>>', self.on_controller_selected)
                # Use dropdowns when data exists
                self._set_manual_mode(False)
                # Trigger initial parameter population
                self.on_controller_selected()
            else:
                print("No controllers available")
                # Clear both dropdowns and indices when nothing is available
                self.controllerInput['values'] = []
                self.controllerVar.set("")
                self.parameterInput['values'] = []
                self.parameterVar.set("")
                self.controller_index = 0
                self.parameter_index = 0
                # Enable manual entry mode
                self._set_manual_mode(True)
                
        except Exception as e:
            print(f"Error populating controller dropdown: {e}")
            import traceback
            traceback.print_exc()

    def on_controller_selected(self, event=None):
        """When a controller is selected, update the parameter dropdown with only its parameters"""
        try:
            selected_controller = self.controllerVar.get()
            self.controllerInput.set(selected_controller)  # Ensure the selected controller is set
            controllers = self.controller.deviceManager._realTimeProcessor.controllers
            controller_parameters = self.controller.deviceManager._realTimeProcessor.controller_parameters
            
            print(f"DEBUG: Controller selected: {selected_controller}")
            
            # Find the index of the selected controller
            if controllers and selected_controller in controllers:
                self.controller_index = controllers.index(selected_controller)
                controller_index = self.controller_index
                print(f"DEBUG: Controller index: {controller_index}")
                
                # Get the parameters for this specific controller
                if controller_index < len(controller_parameters):
                    parameters = controller_parameters[controller_index]
                    print(f"DEBUG: Parameters for {selected_controller}: {parameters}")
                    
                    if parameters:
                        self.parameterInput['values'] = parameters
                        self.parameterVar.set(parameters[0])
                        print(f"DEBUG: Set parameter dropdown to: {parameters[0]}")
                    else:
                        self.parameterInput['values'] = []
                        self.parameterVar.set("")
                else:
                    self.parameterInput['values'] = []
                    self.parameterVar.set("")
                    print(f"DEBUG: No parameters found for controller index {controller_index}")
            else:
                self.parameterInput['values'] = []
                self.parameterVar.set("")
                print(f"DEBUG: Selected controller '{selected_controller}' not found in controllers list")

            # Ensure parameter_index remains consistent with the last parameter when using dropdowns
            print(f"DEBUG: Current parameter value index: {self.parameter_index}")
            print(f"DEBUG: Current parameter input: {self.parameterInput.get()}")
            print(f"DEBUG: Current controller input: {self.controllerInput.get()}")
                
        except Exception as e:
            print(f"Error updating parameter dropdown: {e}")
            import traceback
            traceback.print_exc()

    def update_dropdowns(self):
        """Update both dropdowns with current data"""
        # Always try to populate, even if previously empty
        self.populate_controller_dropdown()

    # END OF ELLIOTTS CODE

    def start_keyboard_cycle(self):
        """Start the keyboard cycle, focusing on the first input field."""
        if self.dynamic_dropdown:
            self.current_input_index = 2
        else:
            self.current_input_index = 0

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

    def update_keyboard_target(self):
        """Update the keyboard to target the current input field."""
        target_input = self.inputs[self.current_input_index]
        self.keyboard.set_target(target_input)

    def keyboard_submit(self, value):
        """Handle the keyboard submission and move to the next input field."""
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

    def close_keyboard(self):
        """Close the keyboard and reset the current input index."""
        if self.keyboard_window:
            self.keyboard_window.destroy()
            self.keyboard_window = None
        self.current_input_index = 0

    def handle_back_button(self):
        # Return to the previous frame
        if self.previous_frame:
            self.controller.show_frame(self.previous_frame)
            active_trial_frame = self.controller.frames[self.previous_frame]
            active_trial_frame.newSelection(self)
        else:
            self.controller.show_frame("ActiveTrial")
            active_trial_frame = self.controller.frames["ActiveTrial"]
            active_trial_frame.newSelection(self)










# HEY CONNOR THIS IS WHERE WE HANDLE THE UPDATE BUTTON BEING CLICKED











        
    async def on_update_button_clicked(
        self, #controllerInput, parameterInput, valueInput,
    ):
        selected_joint = self.jointVar.get()  # Get the selected joint
        joint_id = jointMap[selected_joint]

        print("Update Button Clicked")
        print(f"Selected Joint: {selected_joint} (ID: {joint_id})")
        #print(f"Controller Input: {self.controllerInput.get()}")
        #print(f"Parameter Input: {self.parameterInput.get()}")
        #print(f"Value Input: {self.valueInput.get()}")

        await self.UpdateButtonClicked(self.isBilateral, joint_id, self.controller_index, self.parameterInput, self.valueInput, )

    async def UpdateButtonClicked(
        self, isBilateral, joint, controllerInput, parameterInput, valueInput,
    ):
        # Determine controller/parameter values based on available data
        controllers = self.controller.deviceManager._realTimeProcessor.controllers
        if controllers:
            controllerVal = self.controller_index
            parameterVal = self.parameter_index
        else:
            # Read manual indices; default to 0 on parse failure
            try:
                controllerVal = int(self.controllerIndexEntry.get())
            except Exception:
                controllerVal = 0
            try:
                parameterVal = int(self.parameterIndexEntry.get())
            except Exception:
                parameterVal = 0
        print(f"controllerVal: {controllerVal}")
        print(f"parameterVal: {parameterVal}")
        valueVal = valueInput.get()

        print(f"bilateral: {isBilateral}")
        print(f"joint: {joint}")
        print(f"controller: {controllerVal}")
        print(f"paramter: {parameterVal}")
        print(f"value: {valueVal}")

        # Set Torque
        await self.controller.deviceManager.updateTorqueValues(
            [isBilateral, joint, float(controllerVal), float(parameterVal), float(valueVal)]
        )

        if self.previous_frame:
            self.controller.show_frame(self.previous_frame)
            active_trial_frame = self.controller.frames[self.previous_frame]
            active_trial_frame.newSelection(self)
        else:
            self.controller.show_frame("ActiveTrial")
            active_trial_frame = self.controller.frames["ActiveTrial"]
            active_trial_frame.newSelection(self)

    def newSelection(self, event):
        # Update dropdowns when the frame is shown/selected
        self.update_dropdowns()
        # Check if we actually have controller data now
        controllers = self.controller.deviceManager._realTimeProcessor.controllers
        if controllers:
            print(f"Controllers available on frame show: {controllers}")
        else:
            print("No controllers available on frame show")

    def show(self):
        """Called by app when this frame is raised; ensure dropdowns reflect latest data."""
        self.update_dropdowns()

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