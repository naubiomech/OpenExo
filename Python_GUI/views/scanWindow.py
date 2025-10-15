import tkinter as tk
from tkinter import (BOTH, BOTTOM, DISABLED, StringVar, X, Y, ttk, font)
from async_tkinter_loop import async_handler
from PIL import ImageTk, Image, ImageEnhance

import os

# Frame to scan for exoskeleton devices
class ScanWindow(tk.Frame):
    SETTINGS_FILE = "Saved_Data/saved_device.txt"  # File to save and load previous torque settings

    # Initialize class
    def __init__(self, parent, controller):
        super().__init__(parent)
        self.controller = controller  # Reference to the main application controller
        self.saved_address = None

        # Set the disconnection callback
        self.controller.deviceManager.on_disconnect = self.ScanWindow_on_device_disconnected

        # Variables to hold device information
        self.deviceNameText = StringVar(value="Not Connected")  # Default device name text
        self.selected_device_name = None  # Selected device name
        self.selected_device_address = None  # Selected device address

        # UI elements
        self.scanning_animation_running = False  # Flag for animation state
        self.fontstyle = 'Segoe UI'
        self.create_widgets()  # Create UI elements
        self.load_device_available() #Check if loaded devices avalible

    # Create all UI elements
    def create_widgets(self):

        original_width, original_height = 1939, 354
        scale_factor = 9  # or whatever you prefer

        resized_width = int(original_width / scale_factor)
        resized_height = int(original_height / scale_factor)

        # Open + Resize
        background_image = Image.open("./Resources/Images/LabLogo.png").convert("RGBA")
        background_image = background_image.resize((resized_width, resized_height), Image.Resampling.LANCZOS)
        self.background_bg_image = ImageTk.PhotoImage(background_image)

        # Make the Canvas match the new width/height
        canvas = tk.Canvas(self, width=resized_width, height=resized_height, highlightthickness=0)
        canvas.create_image(0, 0, image=self.background_bg_image, anchor="nw")
        canvas.grid(row=7, column=1, sticky="se", padx=5, pady=10)

        # Load and place the smaller image behind the timer and battery
        small_image = Image.open("./Resources/Images/OpenExo.png").convert("RGBA")
        small_image = small_image.resize((int(1736*.075), int(336*.075)))  # Resize the image to a smaller size
        self.small_bg_image = ImageTk.PhotoImage(small_image)

        # Create a Canvas for the smaller image
        small_canvas = tk.Canvas(self, width=int(1736*.075), height=int(336*.075), highlightthickness=0)
        small_canvas.create_image(0, 0, image=self.small_bg_image, anchor="nw")
        small_canvas.grid(row=0, column=1, sticky="ne", padx=5, pady=10)  # Top-right corner


        # Style configuration
        style = ttk.Style()
        style.configure('TButton', font=(self.fontstyle, 12), padding=10)
        style.configure('TLabel', font=(self.fontstyle, 16))
        style.configure('TListbox', font=(self.fontstyle, 14))

        # Title label on top of the image
        titleLabel = ttk.Label(self, text="OpenExo GUI V1.05", font=(self.fontstyle, 30))
        titleLabel.grid(row=1, column=0, columnspan=2, pady=0, sticky="n")  # Center instructions
        
        # Initial device name display
        deviceNameLabel = ttk.Label(self, textvariable=self.deviceNameText)
        deviceNameLabel.grid(row=2, column=0, columnspan=2, pady=0, sticky="n")  # Center device name

        # Buttons container
        button_frame = ttk.Frame(self)
        button_frame.grid(row=3, column=0, columnspan=2, pady=10)  # Center button frame

        # Button to start scanning for devices
        self.startScanButton = ttk.Button(button_frame, text="Start Scan",
                                           command=async_handler(self.on_start_scan_button_clicked))
        self.startScanButton.grid(row=0, column=0, padx=5)

        # Button to load saved device info
        self.loadDeviceButton = ttk.Button(button_frame, text="Load Saved Device",
                                            command=async_handler(self.on_load_device_button_clicked))
        self.loadDeviceButton.grid(row=0, column=1, padx=5)

        # Listbox to display scanned devices
        self.deviceListbox = tk.Listbox(self, font=(self.fontstyle, 14), width=50, height=5)
        self.deviceListbox.grid(row=4, column=0, columnspan=2, pady=5)  # Center Listbox
        self.deviceListbox.bind("<<ListboxSelect>>", self.on_device_selected)  # Bind selection event

        # Action buttons
        action_button_frame = ttk.Frame(self)
        action_button_frame.grid(row=6, column=0, columnspan=2, pady=10, sticky="n")  # Center action button frame

        # Button to start the trial (initially disabled)
        self.startTrialButton = ttk.Button(action_button_frame, text="Start Trial",
                                            command=async_handler(self.on_start_trial_button_clicked),
                                            state=DISABLED)
        self.startTrialButton.grid(row=0, column=0, padx=5)

        # Button to start the debug (initially disabled)
        self.debugButton = ttk.Button(action_button_frame, text="Debug",
                                            command=async_handler(self.on_start_trial_debug_button_clicked),
                                            state=DISABLED)
        self.debugButton.grid(row=0, column=1, padx=5)

        # Calibrate Torque button
        self.calTorqueButton = ttk.Button(action_button_frame, text="Calibrate Torque",
                                           command=async_handler(self.on_calibrate_torque_button_clicked),
                                           state=DISABLED)
        self.calTorqueButton.grid(row=0, column=2, padx=5)

        # Connect button
        self.connectButton = ttk.Button(action_button_frame, text="Connect",
                                        command=async_handler(self.on_connect_button_clicked),
                                        state=DISABLED)  # Initially disabled
        self.connectButton.grid(row=0, column=3, padx=5)

        # Button to save the selected device
        self.saveDeviceButton = ttk.Button(action_button_frame, text="Save & Connect",
                                            command=async_handler(self.on_save_device_button_clicked),
                                            state=DISABLED)  # Initially disabled
        self.saveDeviceButton.grid(row=0, column=3, padx=5)

        # Configure grid weights for centering
        for i in range(8):  # Assuming there are 7 rows
            self.grid_rowconfigure(i, weight=1)
        for j in range(2):  # Assuming 2 columns
            self.grid_columnconfigure(j, weight=1)

        # Create overlay frame for parameter waiting popup (add this around line 128, before grid_rowconfigure)
        self.parameter_waiting_frame = ttk.Frame(self, relief="raised", borderwidth=3)
        self.parameter_waiting_frame.configure(style='Popup.TFrame')

        # Configure a background style for the popup
        style.configure('Popup.TFrame', background='#f0f0f0')

        # Message label
        waiting_message = ttk.Label(
            self.parameter_waiting_frame, 
            text="Waiting for parameters to be recieved..",
            font=(self.fontstyle, 14),
            justify="center"
        )
        waiting_message.grid(row=0, column=0, padx=40, pady=30)

        # Back button
        back_button = ttk.Button(
            self.parameter_waiting_frame,
            text="OK",
            command=self.hide_waiting_popup
        )
        back_button.grid(row=1, column=0, pady=(0, 20))

    # Callback for device disconnection
    def ScanWindow_on_device_disconnected(self):
        """Handles disconnection of the device."""
        self.deviceNameText.set("Disconnected After Scan Try Again")  # Update device name text
        self.stop_scanning_animation()  # Stop animation on disconnect
        self.reset_elements()

    # Save the selected device address to a file
    async def on_save_device_button_clicked(self):
        """Saves the currently selected device to a file."""
        await self.controller.deviceManager.disconnect()  # Disconnects from any devices
        self.startScanButton.config(state=DISABLED)
        self.connectButton.config(state=DISABLED)
        self.saveDeviceButton.config(state=DISABLED)
        self.loadDeviceButton.config(state=DISABLED)
        if self.selected_device_address:
            with open(self.SETTINGS_FILE, "w") as file:
                file.write(self.selected_device_address)
            print(f"Saved device: {self.selected_device_name} - {self.selected_device_address}")
            
        connect_message = f"Connecting to: {self.selected_device_name} {self.selected_device_address}"
        self.deviceNameText.set(connect_message)
        self.controller.deviceManager.set_deviceAddress(self.selected_device_address)  # Set the device address
        self.saved_address = self.selected_device_address
        # Attempt to connect to the device
        success = await self.controller.deviceManager.scanAndConnect()
        
        if success:
            self.startTrialButton.config(state="normal")  # Enable Start Trial button
            self.debugButton.config(state="normal")  # Enable Start Trial button
            self.calTorqueButton.config(state="normal")   # Enable Calibrate Torque button
            self.connectButton.config(state=DISABLED)
            self.saveDeviceButton.config(state=DISABLED)
            self.loadDeviceButton.config(state=DISABLED)
            self.deviceNameText.set(f"Connected: {self.selected_device_name} {self.selected_device_address}")
        else:
            self.deviceNameText.set("Connection Failed, Please Restart Device")  # Update text if connection fails
            self.reset_elements()
        self.startScanButton.config(state="normal")

    def load_device_available(self):
        if os.path.exists(self.SETTINGS_FILE):
            with open(self.SETTINGS_FILE, "r") as file:
                self.saved_address = file.read().strip()
                if self.saved_address:  # Check if the file is not empty
                    self.deviceNameText.set(f"Loading saved device: {self.saved_address}")
                    self.controller.deviceManager.set_deviceAddress(self.saved_address)
                    self.loadDeviceButton.config(state="normal")
        else:
            self.loadDeviceButton.config(state=DISABLED)
                    
    # Load saved device address from a file and connect to it
    async def on_load_device_button_clicked(self):
        """Loads the saved device from a file and connects to it."""
        self.loadDeviceButton.config(state=DISABLED)
        self.startScanButton.config(state=DISABLED)
        self.startTrialButton.config(state=DISABLED)
        self.debugButton.config(state=DISABLED)
        self.calTorqueButton.config(state=DISABLED)
        await self.controller.deviceManager.disconnect()  # Disconnects from any devices
        self.deviceListbox.delete(0, tk.END)
        self.deviceNameText.set(f"Loading saved device: {self.saved_address}")
        success = await self.controller.deviceManager.scanAndConnect()

        if success:
            self.deviceNameText.set(f"Connected: {self.selected_device_name} {self.saved_address}")
            self.startTrialButton.config(state="normal")
            self.debugButton.config(state="normal")
            self.calTorqueButton.config(state="normal")
            self.loadDeviceButton.config(state=DISABLED)
            self.saveDeviceButton.config(state=DISABLED)
        else:
            self.deviceNameText.set("Connection Failed, Please Restart Device")
            self.loadDeviceButton.config(state="normal")
        
        self.startScanButton.config(state="normal")

    async def on_calibrate_torque_button_clicked(self):
        """Handles the Calibrate Torque button click."""
        await self.controller.trial.calibrate(self.controller.deviceManager)

    async def on_connect_button_clicked(self):
        """Handles the Connect button click."""
        self.startScanButton.config(state=DISABLED)
        self.connectButton.config(state=DISABLED)
        self.saveDeviceButton.config(state=DISABLED)
        self.loadDeviceButton.config(state=DISABLED)
        connect_message = f"Connecting to: {self.selected_device_name} {self.selected_device_address}"
        self.deviceNameText.set(connect_message)
        self.controller.deviceManager.set_deviceAddress(self.selected_device_address)  # Set the device address

        # Attempt to connect to the device
        success = await self.controller.deviceManager.scanAndConnect()
        
        if success:
            self.startTrialButton.config(state="normal")  # Enable Start Trial button
            self.debugButton.config(state = "normal")
            self.calTorqueButton.config(state="normal")   # Enable Calibrate Torque button
            self.deviceNameText.set(f"Connected: {self.selected_device_name} {self.selected_device_address}")
        else:
            self.deviceNameText.set("Connection Failed, Please Restart Device")  # Update text if connection fails
            self.connectButton.config(state="normal")

        self.startScanButton.config(state="normal")

        if self.saved_address is not None:
            self.loadDeviceButton.config(state="normal")

    async def on_start_trial_button_clicked(self):
        """Handles the Start Trial button click."""

        # Only allow the trial window to show if all parameters have been recieved
        if(self.controller.deviceManager._realTimeProcessor.parameters_recieved):
            await self.startTrialButtonClicked()  # Initiate the trial process
        else:
            self.show_waiting_popup()

    async def on_start_trial_debug_button_clicked(self):
        """Handles the Start Trial button click."""
        await self.startTrialDebugButtonClicked()
        
    async def on_start_scan_button_clicked(self):
        """Starts scanning for devices when the button is clicked."""
        self.reset_elements()
        self.startScanButton.config(state=DISABLED)
        self.loadDeviceButton.config(state=DISABLED)
        await self.controller.deviceManager.disconnect()  # Disconnects from any devices
        self.deviceNameText.set("Scanning...")  # Update device name text
        self.start_scanning_animation()  # Start the animation        
        await self.startScanButtonClickedHandler()  # Initiate the scanning process
        self.startScanButton.config(state="normal")  # Re-enable the Start Scan button after scanning
        self.stop_scanning_animation()  # Stop the animation after scanning

        if self.saved_address is not None:
            self.loadDeviceButton.config(state="normal")

        """Uncomment for testing purposes to skip scanning"""
        # active_trial_frame = self.controller.frames["ActiveTrial"]
        # active_trial_frame.disable_interactions()  # Disable buttons in ActiveTrial frame
        
        # # Show ActiveTrial frame
        # self.controller.show_frame("ActiveTrial")
        # await self.controller.trial.calibrate(self.controller.deviceManager)  # Calibrate devices
        # await self.controller.trial.beginTrial(self.controller.deviceManager)  # Begin the trial

        # # Starts new selection once Active trial has started
        # active_trial_frame.newSelection(self)
        # active_trial_frame.startClock()

    async def startScanButtonClickedHandler(self):
        """Starts scanning for devices and updates the UI accordingly."""

        available_devices = await self.controller.deviceManager.searchDevices()

        if(available_devices != "false" and available_devices != "NoDevice"):
            self.deviceNameText.set("Scan Complete")
            self.startScanButton.config(state="normal")

            # Update Listbox with the scanned devices
            self.deviceListbox.delete(0, tk.END)  # Clear the Listbox
            for address, name in available_devices.items():
                self.deviceListbox.insert(tk.END, f"{name} - {address}")

            # Check if there are available devices
            if not available_devices:
                self.deviceNameText.set("No Devices Found")

            if self.saved_address is not None:
                self.loadDeviceButton.config(state="normal")
        elif available_devices == "NoDevice":
            self.deviceNameText.set("No Devices Found")
            self.startScanButton.config(state="normal")
        else:
            self.deviceNameText.set("BlueTooth Error")
            self.startScanButton.config(state="normal")
        
    # Handle device selection from the Listbox
    def on_device_selected(self, event):
        """Handles the selection of a device from the Listbox."""
        selected_index = self.deviceListbox.curselection()
        if selected_index:  # Check if any item is selected
            selected_device_info = self.deviceListbox.get(selected_index)
            self.selected_device_name, self.selected_device_address = selected_device_info.split(" - ")
            self.connectButton.config(state="normal")  # Enable Connect button
            self.saveDeviceButton.config(state="normal")  # Enable Save & Connect button
            print(self.selected_device_name)  # Debug output
        else:
            self.connectButton.config(state=DISABLED)  # Disable Connect button if no selection
            self.saveDeviceButton.config(state=DISABLED)  # Disable Save & Connect button if no selection

    def start_scanning_animation(self):
        """Starts the scanning animation."""
        self.scanning_animation_running = True
        self.animate_scanning_dots(0)

    def stop_scanning_animation(self):
        """Stops the scanning animation."""
        self.scanning_animation_running = False

    def animate_scanning_dots(self, count):
        """Animates the scanning dots to indicate scanning progress."""
        if not self.scanning_animation_running:
            return

        dots = "." * ((count % 3) + 1)  # Cycle through 1 to 3 dots
        self.deviceNameText.set(f"Scanning{dots}")

        # Schedule the next update
        self.after(500, self.animate_scanning_dots, count + 1)

    def changeName(self):
        """Handles the Start Trial button click."""
        if self.selected_device_address:  # Ensure a device is selected
            # Change the title to the selected MAC address
            self.controller.change_title(f"Device: {self.selected_device_address}")

    async def startTrialButtonClicked(self):

        self.changeName()
        """Switches frame to ActiveTrial and begins the trial."""
        active_trial_frame = self.controller.frames["ActiveTrial"]
        active_trial_frame.update_dropdown_values()
        active_trial_frame.disable_interactions()  # Disable buttons in ActiveTrial frame
        active_trial_frame.clear_both_plot()

        # Show ActiveTrial frame
        self.controller.show_frame("ActiveTrial")
        await self.controller.trial.calibrate(self.controller.deviceManager)  # Calibrate devices
        await self.controller.trial.beginTrial(self.controller.deviceManager)  # Begin the trial

        # Starts new selection once Active trial has started
        active_trial_frame.newSelection(self)
        active_trial_frame.startClock()

    async def startTrialDebugButtonClicked(self):

        """Switches frame to ActiveTrial and begins the trial in debug mode."""
        active_trial_frame = self.controller.frames["ActiveTrial"]
        active_trial_frame.disable_interactions()  # Disable buttons in ActiveTrial frame
        active_trial_frame.clear_both_plot()

        # Show ActiveTrial frame
        self.controller.show_frame("ActiveTrial")
        await self.controller.trial.calibrate(self.controller.deviceManager)  # Calibrate devices
        await self.controller.trial.beginTrialDebug(self.controller.deviceManager)  # Begin the trial

        # Starts new selection once Active trial has started
        active_trial_frame.newSelection(self)
        active_trial_frame.pauseMotorButton()
        active_trial_frame.startClock()

    def loadDeviceAvailible(self):
        if self.saved_address is not None:
            self.loadDeviceButton.config(state="normal")
        else:
            self.loadDeviceButton.config(state=DISABLED)
        
    def reset_elements(self):
        """Resets the UI elements to their initial state."""
        self.deviceNameText.set("Not Connected")
        self.deviceListbox.delete(0, tk.END)
        self.selected_device_name = None
        self.selected_device_address = None
        self.startScanButton.config(state="normal")
        self.startTrialButton.config(state=DISABLED)
        self.debugButton.config(state=DISABLED)
        self.calTorqueButton.config(state=DISABLED)
        self.connectButton.config(state=DISABLED)
        self.saveDeviceButton.config(state=DISABLED)
        self.loadDeviceAvailible()

    def disable_elements(self):
        """disable the UI elements."""
        self.deviceNameText.set("Not Connected")
        self.deviceListbox.delete(0, tk.END)
        self.selected_device_name = None
        self.selected_device_address = None
        self.startScanButton.config(state=DISABLED)
        self.startTrialButton.config(state=DISABLED)
        self.debugButton.config(state=DISABLED)
        self.calTorqueButton.config(state=DISABLED)
        self.connectButton.config(state=DISABLED)
        self.saveDeviceButton.config(state=DISABLED)
        self.loadDeviceButton.config(state=DISABLED)
    
    def show_waiting_popup(self):
        """Shows the parameter waiting popup overlay."""
        # Raise it above other widgets and grid it to cover everything
        self.parameter_waiting_frame.grid(row=0, column=0, rowspan=8, columnspan=2, sticky="nsew")
        self.parameter_waiting_frame.tkraise()  # Bring to front

    def hide_waiting_popup(self):
        """Hides the parameter waiting popup overlay."""
        self.parameter_waiting_frame.grid_remove()
        
    def show(self):
        """Resets elements and shows the frame."""
        self.reset_elements()  # Reset elements when showing the frame
