import tkinter as tk
from tkinter import (BOTH, BOTTOM, DISABLED, StringVar, X, Y, ttk, font)
from async_tkinter_loop import async_handler
from PIL import ImageTk, Image, ImageEnhance
import asyncio

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

        self.selected_devices = []   # list of (name, address) tuples (max 2)

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
        titleLabel = ttk.Label(self, text="OpenExo GUI DUAL", font=(self.fontstyle, 30))
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
        self.deviceListbox = tk.Listbox(self, font=(self.fontstyle, 14), width=50, height=5, selectmode=tk.MULTIPLE)
        self.deviceListbox.grid(row=4, column=0, columnspan=2, pady=5)  # Center Listbox
        self.deviceListbox.bind("<<ListboxSelect>>", self.on_device_selected)  # Bind selection event

        # Action buttons
        action_button_frame = ttk.Frame(self)
        action_button_frame.grid(row=6, column=0, columnspan=2, pady=10, sticky="n")  # Center action button frame

        # Removed Capture 5s button


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
        self.saveDeviceButton.grid(row=0, column=4, padx=5)

        # Configure grid weights for centering
        for i in range(8):  # Assuming there are 7 rows
            self.grid_rowconfigure(i, weight=1)
        for j in range(2):  # Assuming 2 columns
            self.grid_columnconfigure(j, weight=1)

    # Callback for device disconnection
    def ScanWindow_on_device_disconnected(self):
        """Handles disconnection of the device."""
        self.deviceNameText.set("Disconnected After Scan Try Again")  # Update device name text
        self.stop_scanning_animation()  # Stop animation on disconnect
        self.reset_elements()

    async def on_save_device_button_clicked(self):
        # ... (disable buttons, disconnect, etc.)
        if len(self.selected_devices) == 2:
            (_, addr1), (_, addr2) = self.selected_devices
            os.makedirs(os.path.dirname(self.SETTINGS_FILE), exist_ok=True)
            with open(self.SETTINGS_FILE, "w") as f:
                f.write(addr1.strip() + "\n" + addr2.strip() + "\n")
            self.saved_address = (addr1.strip(), addr2.strip())
            self.deviceNameText.set(f"Saved pair:\n{addr1}\n{addr2}")
        else:
            self.deviceNameText.set("Select exactly 2 devices to Save")
            return

        # Auto-connect after save (optional)
        self.controller.deviceManager.set_deviceAddresses(list(self.saved_address))
        self.deviceNameText.set(f"Connecting to saved pair…")
        ok = await self.controller.deviceManager.scanAndConnectMulti()
        if ok:
            self.startTrialButton.config(state="normal")
            self.debugButton.config(state="normal")
            self.calTorqueButton.config(state="normal")
            self.deviceNameText.set(f"Connected:\n{self.saved_address[0]}\n{self.saved_address[1]}")
        else:
            self.deviceNameText.set("Connection Failed, Please Restart Devices")

    def load_device_available(self):
        if os.path.exists(self.SETTINGS_FILE):
            with open(self.SETTINGS_FILE, "r") as f:
                lines = [ln.strip() for ln in f.readlines() if ln.strip()]
            if len(lines) >= 2:
                self.saved_address = (lines[0], lines[1])
                self.deviceNameText.set(f"Saved pair:\n{lines[0]}\n{lines[1]}")
                self.loadDeviceButton.config(state="normal")
            else:
                self.loadDeviceButton.config(state=DISABLED)
        else:
            self.loadDeviceButton.config(state=DISABLED)

    async def on_quick_dual_test_clicked(self):
        if len(self.selected_devices) != 2:
            self.deviceNameText.set("Select exactly 2 devices for Dual Test")
            return
        (_, addr1), (_, addr2) = self.selected_devices
        dm = self.controller.deviceManager
        if not (hasattr(dm, "set_deviceAddresses") and hasattr(dm, "scanAndConnectMulti")):
            self.deviceNameText.set("Dual APIs missing in deviceManager")
            return
        dm.set_deviceAddresses([addr1, addr2])
        self.deviceNameText.set(f"Dual Test: connecting {addr1} + {addr2} ...")
        ok = await dm.scanAndConnectMulti()
        self.deviceNameText.set(f"Dual connect: {'OK' if ok else 'Failed'}")
        if ok and hasattr(dm, "motorOn_both"):
            await dm.motorOn_both()
            self.deviceNameText.set("Dual Test: motorOn sent to both")
            self.testBothBtn.config(state=("normal" if ok else DISABLED))

    async def on_test_both_clicked(self):
        dm = self.controller.deviceManager
        # sanity
        if not dm.connections or not any(c.get("is_connected") for c in dm.connections.values()):
            self.deviceNameText.set("Pair not connected")
            return

        self.testBothBtn.config(state=DISABLED)
        try:
            # 1) Motors ON (both)
            self.deviceNameText.set("Test: Motors ON (both)…")
            res_on = await dm.motorOn_both_status()
            self._report_fanout("ON", res_on)

            # small dwell so you can hear/feel ON
            await asyncio.sleep(0.8)

            # 2) Motors OFF (both)
            self.deviceNameText.set("Test: Motors OFF (both)…")
            res_off = await dm.motorOff_both_status()
            self._report_fanout("OFF", res_off)

            # 3) Disconnect both
            self.deviceNameText.set("Test: Disconnecting both…")
            await dm.disconnect_all()
            self.deviceNameText.set("Test: Done — disconnected ✅")
        finally:
            self.testBothBtn.config(state="normal")

    def _report_fanout(self, label: str, results: dict):
        """Summarize per-device results in UI + console."""
        oks = [a for a, v in results.items() if v is True]
        errs = {a: v for a, v in results.items() if v is not True}
        # brief UI summary
        if errs:
            self.deviceNameText.set(f"{label}: OK {len(oks)}, ERR {len(errs)} (see console)")
        else:
            self.deviceNameText.set(f"{label}: OK on {len(oks)} device(s)")
        # detailed console log
        for addr in oks:
            print(f"{label} [{addr}] OK")
        for addr, err in errs.items():
            print(f"{label} [{addr}] ERROR: {err}")
    
    # Removed on_capture_clicked method
        
    async def on_load_device_button_clicked(self):
        if not self.saved_address or len(self.saved_address) != 2:
            self.deviceNameText.set("No saved pair")
            return
        self.loadDeviceButton.config(state=DISABLED)
        self.startScanButton.config(state=DISABLED)
        await self.controller.deviceManager.disconnect_all()
        self.deviceListbox.delete(0, tk.END)
        self.deviceNameText.set(f"Connecting to saved pair…")
        dm = self.controller.deviceManager
        dm.set_deviceAddresses(list(self.saved_address))
        ok = await dm.scanAndConnectMulti()
        if ok:
            self.deviceNameText.set(f"Connected:\n{self.saved_address[0]}\n{self.saved_address[1]}")
            self.startTrialButton.config(state="normal")
            self.debugButton.config(state="normal")
            self.calTorqueButton.config(state="normal")
            # Removed captureBtn reference
            if hasattr(self, "testBothBtn"): self.testBothBtn.config(state="normal")
        else:
            self.deviceNameText.set("Connection Failed, Please Restart Devices")
            self.loadDeviceButton.config(state="normal")
        self.startScanButton.config(state="normal")

    async def on_calibrate_torque_button_clicked(self):
        self.deviceNameText.set("Calibrating torque on both devices…")
        res = await self.controller.deviceManager.calibrateTorque_both()
        ok = sum(1 for v in res.values() if v)
        self.deviceNameText.set(f"Calibrate Torque: OK {ok}, ERR {len(res)-ok}")


    async def on_connect_button_clicked(self):
        self.startScanButton.config(state=DISABLED)
        self.connectButton.config(state=DISABLED)
        self.saveDeviceButton.config(state=DISABLED)
        self.loadDeviceButton.config(state=DISABLED)

        if len(self.selected_devices) != 2:
            self.deviceNameText.set("Select exactly 2 devices")
            self.startScanButton.config(state="normal")
            return

        (name1, addr1), (name2, addr2) = self.selected_devices
        self.deviceNameText.set(f"Connecting to: {name1} {addr1}  +  {name2} {addr2}")

        dm = self.controller.deviceManager
        try:
            dm.set_deviceAddresses([addr1, addr2])
            ok = await dm.scanAndConnectMulti()
        except Exception as e:
            print("Dual connect error:", e)
            ok = False

        if ok:
            self.deviceNameText.set(f"Connected: {name1} {addr1}  +  {name2} {addr2}")
            self.startTrialButton.config(state="normal")
            self.debugButton.config(state="normal")
            self.calTorqueButton.config(state="normal")
            # Removed captureBtn reference
            if hasattr(self, "testBothBtn"): self.testBothBtn.config(state="normal")
        else:
            self.deviceNameText.set("Connection Failed, Please Restart Devices")
            self.connectButton.config(state="normal")

        self.startScanButton.config(state="normal")

    async def on_start_trial_button_clicked(self):
        """Handles the Start Trial button click."""
        await self.startTrialButtonClicked()  # Initiate the trial process

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
        

    def on_device_selected(self, event):
        indices = list(self.deviceListbox.curselection())
        # Enforce exactly two
        if len(indices) > 2:
            for idx in indices[2:]:
                self.deviceListbox.selection_clear(idx)
            indices = indices[:2]

        self.selected_devices = []
        for idx in indices:
            name, addr = self._parse_listbox_item(self.deviceListbox.get(idx))
            if addr:
                self.selected_devices.append((name, addr))

        # Dual-only: enable connect ONLY when exactly two
        ready = (len(self.selected_devices) == 2)
        self.connectButton.config(state=("normal" if ready else DISABLED))
        self.saveDeviceButton.config(state=("normal" if ready else DISABLED))

        # Dual test button (optional)
        if hasattr(self, "dualTestButton"):
            self.dualTestButton.config(state=("normal" if ready else DISABLED))

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
        active_trial_frame.disable_interactions()  # Disable buttons in ActiveTrial frame
        active_trial_frame.clear_both_plot()

        # Show ActiveTrial frame
        self.controller.show_frame("ActiveTrial")
        await self.controller.trial.calibrate(self.controller.deviceManager)  # Calibrate devices
        await self.controller.trial.beginTrial(self.controller.deviceManager)  # Begin the trial

        # Starts new selection once Active trial has started
        active_trial_frame.newSelection(self)
        active_trial_frame.startClock()

    def _parse_listbox_item(self, item: str):
        """
        Expects 'Name - AA:BB:CC:DD:EE:FF'. Returns (name, address).
        """
        try:
            name, addr = item.split(" - ", 1)
            return name.strip(), addr.strip()
        except ValueError:
            return item.strip(), None
        
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
        self.load_device_available()
        if hasattr(self, "dualTestButton"):
            self.dualTestButton.config(state=DISABLED)

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
        if hasattr(self, "dualTestButton"):
            self.dualTestButton.config(state=DISABLED)
    
    def show(self):
        """Resets elements and shows the frame."""
        self.reset_elements()  # Reset elements when showing the frame
