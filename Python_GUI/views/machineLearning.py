import tkinter as tk
import matplotlib.animation as animation
import matplotlib.pyplot as plt
import Data.SaveModelData as saveModelData
from tkinter import (BOTTOM, CENTER, LEFT, RIGHT, TOP, E, N, S, IntVar, StringVar, W, X, Y, ttk, simpledialog)
from async_tkinter_loop import async_handler
from Widgets.Charts.chart import BasePlot,BottomPlot, TopPlot
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from PIL import ImageTk, Image, ImageEnhance

# Biofeedback Frame
class MachineLearning(tk.Frame):
    # Constructor for frame
    def __init__(self, parent, controller):
        super().__init__(parent)
        # Controller object to switch frames
        self.controller = controller
        self.is_plotting = False  # Flag to control if plotting should happen

        # Set the disconnection callback
        self.controller.deviceManager.on_disconnect = self.MachineLearning_on_device_disconnected

        # Variables to manage states and UI elements
        self.var = IntVar()
        self.chartVar = StringVar()
        self.chartVar.set("Controller")
        self.graphVar = StringVar()
        self.graphVar.set("Both Graphs")  # Default to "Both Graphs"

        # Create StringVars for button names
        self.levelButtonName = StringVar()
        self.descendButtonName = StringVar()
        self.ascendButtonName = StringVar()
        self.modelButtonName = StringVar()
        self.deleteModelButtonName = StringVar()
        
        # Confirmation flag for deleting model
        self.confirmation = 0

        #create our model controll names, frequently changed
        self.levelButtonName=StringVar()
        self.descendButtonName=StringVar()
        self.ascendButtonName=StringVar()
        self.modelButtonName=StringVar()
        self.deleteModelButtonName=StringVar()
        #self.controlModeLabel=StringVar()
        #self.controlMode=0
        self.confirmation=0 #used as a flag to request second confirmation form user to delete model
        self.modelDataWriter = saveModelData.CsvWritter()
        
        # Target stiffness variable
        self.target_var = StringVar(value="Update Stiffness")  # Display the target value

        #UI Styling
        self.fontstyle = 'Segoe UI'
        
        # Create the UI elements
        self.create_widgets()

    # Frame UI elements
    def create_widgets(self):

        # Load and place the smaller image behind the timer and battery
        small_image = Image.open("./Resources/Images/OpenExo.png").convert("RGBA")
        small_image = small_image.resize((80, 40))  # Resize the image to a smaller size
        self.small_bg_image = ImageTk.PhotoImage(small_image)

        # Create a Canvas for the smaller image
        small_canvas = tk.Canvas(self, width=80, height=50, highlightthickness=0)
        small_canvas.create_image(0, 0, image=self.small_bg_image, anchor="nw")
        small_canvas.grid(row=0, column=8, sticky="ne", padx=5, pady=10)  # Top-right corner

        # Dropdown for chart selection
        self.chartDropdown = ttk.Combobox(
            self,
            textvariable=self.chartVar,
            state="readonly",
            values=["Controller", "Sensor"],
        )
        self.chartDropdown.grid(row=4,columnspan=5, sticky="NSEW", pady=5, padx=5)

        # # Graph selection dropdown
        # self.graphDropdown = ttk.Combobox(
        #     dropdown_frame,
        #     textvariable=self.graphVar,
        #     state="readonly",
        #     values=["Both Graphs", "Top Graph", "Bottom Graph"],
        # )
        #self.graphDropdown.pack(side=LEFT, padx=5)
        # Back button to return to the Active Trial frame
        backButton = ttk.Button(self, text="Back", command=self.handle_back_button)
        backButton.grid(row=0, column=0, pady=10)


        # Title label for the frame
        calibrationMenuLabel = tk.Label(self, text="Machine learning", font=(self.fontstyle, 40))
        calibrationMenuLabel.grid(row=0, column=0, columnspan=10, pady=20)

        # Battery status label
        batteryPercentLabel = tk.Label(
            self, 
            textvariable=self.controller.deviceManager._realTimeProcessor._exo_data.BatteryPercent, 
            font=(self.fontstyle, 12)
        )
        batteryPercentLabel.grid(row=0, column=8,sticky="E") 

        BasePlot.set_figure_size((3, 2)) 

        # Create and place the top plot
        self.topPlot = TopPlot(self)
        self.topPlot.canvas.get_tk_widget().grid(row=1, column=0, columnspan=5, sticky="NSEW", pady=5, padx=5)

        # Create and place the bottom plot
        self.bottomPlot = BottomPlot(self)
        self.bottomPlot.canvas.get_tk_widget().grid(row=2, column=0, columnspan=5, sticky="NSEW", pady=5, padx=5)
            
        # Bind dropdown selections to their respective methods
        self.chartDropdown.bind("<<ComboboxSelected>>", self.newSelection)
        #self.graphDropdown.bind("<<ComboboxSelected>>", self.newSelection)

        # Store current plots for updating
        self.currentPlots = [self.topPlot, self.bottomPlot]
        self.plot_update_job = None  # Store the job reference

        # End Trial Button
        endTrialButton = ttk.Button(
            self,
            text="End Trial",
            command=async_handler(self.on_end_trial_button_clicked),
        )
        endTrialButton.grid(row=10, column=0, pady=20, sticky="w")

        # Update Torque Button
        updateTorqueButton = ttk.Button(
            self,
            text="Update Torque",
            command=self.go_to_update_torque,
        )
        updateTorqueButton.grid(row=1, column=7, pady=10)

        # Mark Trial Button
        markButton = ttk.Button(
            self,
            textvariable=self.controller.deviceManager._realTimeProcessor._exo_data.MarkLabel,
            command=async_handler(self.on_mark_button_clicked),
        )
        markButton.grid(row=1, column=8, pady=10)
        
        # Recalibrate FSRs Button
        self.recalibrateFSRButton = ttk.Button(
            self,
            text="Recalibrate FSRs",
            command=async_handler(self.on_recal_FSR_button_clicked),
        )
        self.recalibrateFSRButton.grid(row=1, column=7, pady=(110,0))

        # Model Status Label
        modelStatusLabel = tk.Label(
            self, 
            textvariable=self.controller.deviceManager._realTimeProcessor._predictor.modelStatus, 
            font=(self.fontstyle, 12)
        )
        modelStatusLabel.grid(row=2, column=7, sticky="n")
        
        # Level Trial Button
        self.levelButtonName.set("Collect Level Data")
        levelTrialButton = ttk.Button(
            self,
            textvariable=self.levelButtonName,
            command=async_handler(self.on_level_trial_button_clicked),
        )
        levelTrialButton.grid(row=2, column=7, pady=(0,140))
        
        # Display Level Step Count
        lvlstepsLabel = tk.Label(
            self, 
            textvariable=self.controller.deviceManager._realTimeProcessor._predictor.levelStepsLabel, 
            font=(self.fontstyle, 12)
        )
        lvlstepsLabel.grid(row=2, column=7,pady=(0,140), padx=(0,220))

        # Descend Trial Button
        self.descendButtonName.set("Collect Descend Data")
        descendTrialButton = ttk.Button(
            self,
            textvariable=self.descendButtonName,
            command=async_handler(self.on_descend_trial_button_clicked),
        )
        descendTrialButton.grid(row=2, column=7, pady=(0,30))

        # Display Descend Step Count
        desstepsLabel = tk.Label(
            self, 
            textvariable=self.controller.deviceManager._realTimeProcessor._predictor.descendStepsLabel, 
            font=(self.fontstyle, 12)
        )
        desstepsLabel.grid(row=2, column=7, pady=(0,30), padx=(0,220))

        # Ascend Trial Button
        self.ascendButtonName.set("Collect Ascend Data")
        ascendTrialButton = ttk.Button(
            self,
            textvariable=self.ascendButtonName,
            command=async_handler(self.on_ascend_trial_button_clicked),
        )
        ascendTrialButton.grid(row=2, column=7, pady=(80,0))

        # Display Ascend Step Count
        ascstepsLabel = tk.Label(
            self, 
            textvariable=self.controller.deviceManager._realTimeProcessor._predictor.ascendStepsLabel, 
            font=(self.fontstyle, 12)
        )
        ascstepsLabel.grid(row=2, column=7, pady=(80,0), padx=(0,220))

        # Create Model Button
        if self.controller.deviceManager._realTimeProcessor._predictor.modelExists:
            self.modelButtonName.set("Stair Model Active " + str(self.controller.deviceManager._realTimeProcessor._predictor.optimizedscore) + "% Acc")
        else:
            self.modelButtonName.set("Create Stair Model")

        createModelButton = ttk.Button(
            self,
            textvariable=self.modelButtonName,
            command=async_handler(self.on_model_button_clicked),
        )
        createModelButton.grid(row=2, column=7, pady=(190,0))

        # Delete Model Button
        self.deleteModelButtonName.set("Delete Model")
        deleteModelButton = ttk.Button(
            self,
            textvariable=self.deleteModelButtonName,
            command=async_handler(self.on_delete_model_button_clicked),
        )
        deleteModelButton.grid(row=10, column=0, pady=20, padx=(100,0), sticky="e")


        # Button to set target stiffness
        self.target_button = ttk.Button(self, textvariable=self.target_var, 
            command=async_handler(self.ask_target_value))
        self.target_button.grid(row=2, column=8, pady=(0,140))



        self.controller.deviceManager._realTimeProcessor._predictor.controlModeLabel.set("Control Mode: Manual")
        
        toggleControlButton = ttk.Button(
            self,
            textvariable=self.controller.deviceManager._realTimeProcessor._predictor.controlModeLabel,
            command=async_handler(self.on_toggle_control_button_clicked),
        )
        toggleControlButton.grid(row=2, column=8, pady=(0,30))


        # Configure grid weights for centering
        for i in range(10):
            self.grid_rowconfigure(i, weight=1)
        for j in range(10):
            self.grid_columnconfigure(j, weight=1)

    async def ask_target_value(self):
        # Prompt the user for a target value
        user_input = simpledialog.askstring("Input", "Please enter a target Stiffness:")
        
        if user_input is not None:
            try:
                # Attempt to convert the input to a float
                stiffness_value  = float(user_input)
                self.target_var.set(f"Stiffness: {stiffness_value}")  # Properly update the StringVar
                # Call the async handler to update stiffness
                await self.controller.deviceManager.newStiffness(stiffness_value)
            except ValueError:
                print("Invalid input. Please enter a numeric value.")

        
    # Navigate to the Update Torque frame
    def go_to_update_torque(self):
        self.controller.frames["UpdateTorque"].previous_frame = "MachineLearning"
        self.controller.show_frame("UpdateTorque")

    # Handle back button press
    def handle_back_button(self):
        self.stop_plot_updates()  # Stop ongoing plot updates
        self.controller.show_frame("ActiveTrial")  # Switch to ActiveTrial frame
        active_trial_frame = self.controller.frames["ActiveTrial"]
        active_trial_frame.newSelection(self)  # Start plotting in the active trial

    # Show frame and update plots
    def show(self):
        # Show the frame and update plots
        self.is_plotting = True
        self.newSelection()

    def hide(self):
        # This method is called when switching away from this frame
        self.stop_plot_updates()

    # Handle Level Trial Button click
    async def on_level_trial_button_clicked(self):
        '''
        If not currently recording data, 
            record and label data as level.
        If recording, end the recording.
        '''
        if self.controller.deviceManager._realTimeProcessor._predictor.state == 0:  # If not recording
            self.controller.deviceManager._realTimeProcessor._predictor.state = 1  # Record as level
            self.levelButtonName.set("End Level Collection")
        elif self.controller.deviceManager._realTimeProcessor._predictor.state == 1:  # If recording
            self.controller.deviceManager._realTimeProcessor._predictor.state = 0  # Stop recording
            self.levelButtonName.set("Collect Level Data")

    # Handle Descend Trial Button click
    async def on_descend_trial_button_clicked(self):
        if self.controller.deviceManager._realTimeProcessor._predictor.state == 0:  # If not recording
            self.controller.deviceManager._realTimeProcessor._predictor.state = 2  # Record as descend
            self.descendButtonName.set("End Descend Collection")
        elif self.controller.deviceManager._realTimeProcessor._predictor.state == 2:  # If recording
            self.controller.deviceManager._realTimeProcessor._predictor.state = 0  # Stop recording
            self.descendButtonName.set("Collect Descend Data")

    # Handle Ascend Trial Button click
    async def on_ascend_trial_button_clicked(self):
        if self.controller.deviceManager._realTimeProcessor._predictor.state == 0:  # If not recording
            self.controller.deviceManager._realTimeProcessor._predictor.state = 3  # Record as ascend
            self.ascendButtonName.set("End Ascend Collection")
        elif self.controller.deviceManager._realTimeProcessor._predictor.state == 3:  # If recording
            self.controller.deviceManager._realTimeProcessor._predictor.state = 0  # Stop recording
            self.ascendButtonName.set("Collect Ascend Data")

    # Handle Model Button click
    async def on_model_button_clicked(self):
        if not self.controller.deviceManager._realTimeProcessor._predictor.modelExists:  # If there is no model
            if len(self.controller.deviceManager._realTimeProcessor._predictor.database):  # If there is data
                self.controller.deviceManager._realTimeProcessor._predictor.createModel()  # Create the model
                self.modelButtonName.set("Stair Model Active " + str(self.controller.deviceManager._realTimeProcessor._predictor.optimizedscore) + "% Acc")
                # Save the data for troubleshooting or future use
                if self.controller.deviceManager._realTimeProcessor._predictor.database:  
                    self.modelDataWriter.writeToCsv(self.controller.deviceManager._realTimeProcessor._exo_data, self.controller.deviceManager._realTimeProcessor._predictor)
            else:
                self.modelButtonName.set("Collect Level, Descend, Ascend Data First")  # Prompt to collect data first
        else:
            self.modelButtonName.set("Stair Model Active " + str(self.controller.deviceManager._realTimeProcessor._predictor.optimizedscore) + "% Acc")

    # Handle Delete Model Button click
    async def on_delete_model_button_clicked(self):
        if self.confirmation == 0:  # Flag for confirmation
            self.deleteModelButtonName.set("Are you Sure?")  # Ask for confirmation
            self.confirmation += 1 
        else:
            self.controller.deviceManager._realTimeProcessor._predictor.deleteModel()  # Delete model
            self.modelButtonName.set("Create Stair Model")  # Reset labels and flags
            self.deleteModelButtonName.set("Delete Model")
            self.confirmation=0
    
    async def toggleControl(self):
        self.controller.deviceManager._realTimeProcessor._predictor.controlMode+=1 #toggle to the next mode
        if self.controller.deviceManager._realTimeProcessor._predictor.controlMode==1: #check new mode
            self.controller.deviceManager._realTimeProcessor._predictor.controlModeLabel.set("Control Mode: Machine Learner") 
        else: #if we exceed total number of modes, return to default
            self.controller.deviceManager._realTimeProcessor._predictor.controlMode=0
            self.controller.deviceManager._realTimeProcessor._predictor.controlModeLabel.set("Control Mode: Manual") 
            

    async def on_toggle_control_button_clicked(self):
        await self.toggleControl()

    # Handle Mark Button click
    async def on_mark_button_clicked(self):
        self.controller.deviceManager._realTimeProcessor._exo_data.MarkVal += 1
        self.controller.deviceManager._realTimeProcessor._exo_data.MarkLabel.set("Mark: " + str(self.controller.deviceManager._realTimeProcessor._exo_data.MarkVal))

    # Handle Recalibrate FSRs Button click
    async def on_recal_FSR_button_clicked(self):
        await self.recalibrateFSR()

    # Recalibrate FSRs
    async def recalibrateFSR(self):
        await self.controller.deviceManager.calibrateFSRs()

    # Handle End Trial Button click
    async def on_end_trial_button_clicked(self):
        await self.endTrialButtonClicked()

    # End Trial Button click functionality
    async def endTrialButtonClicked(self):
        await self.ShutdownExo()
        self.controller.show_frame("ScanWindow")

    # Update plots based on selection
    def update_plots(self, selection):
        # Cancel the previous update job if it exists
        if self.plot_update_job:
            self.after_cancel(self.plot_update_job)
            self.plot_update_job = None

        # Only continue updating plots if the flag is set to True
        if not self.is_plotting:
            return

        # Determine which plots to animate based on the graph selection
        graph_selection = self.graphVar.get()
        plots_to_update = []
        if graph_selection == "Both Graphs":
            plots_to_update = [self.topPlot, self.bottomPlot]
        elif graph_selection == "Top Graph":
            plots_to_update = [self.topPlot]
        elif graph_selection == "Bottom Graph":
            plots_to_update = [self.bottomPlot]

        # Animate the selected plots
        for plot in plots_to_update:
            plot.animate(selection)

        # Schedule the next update
        self.plot_update_job = self.after(
            20, self.update_plots, selection
        )  # Schedule with a delay

        # Enable interactions after the first plot update is complete
        self.after(20, self.enable_interactions)

    # Handle new selection in dropdown
    def newSelection(self, event=None):
        # Disable buttons and dropdown until process completes
        self.disable_interactions()

        # Determine which plots to show
        selection = self.chartVar.get()
        self.update_plots(selection)

    # Disable interactions for buttons and dropdown
    def disable_interactions(self):
        self.chartDropdown.config(state='disabled')
        for widget in self.winfo_children():
            if isinstance(widget, ttk.Button) or isinstance(widget, ttk.Combobox):
                widget.config(state='disabled')

    # Enable interactions for buttons and dropdown
    def enable_interactions(self):
        self.chartDropdown.config(state='normal')
        for widget in self.winfo_children():
            if isinstance(widget, ttk.Button) or isinstance(widget, ttk.Combobox):
                widget.config(state='normal')

    # Stop plot updates
    def stop_plot_updates(self):
        if self.plot_update_job:
            self.after_cancel(self.plot_update_job)
            self.plot_update_job = None
        self.is_plotting = False

    # Shutdown the exoskeleton
    async def ShutdownExo(self):
        await self.controller.deviceManager.motorOff()  # Turn off motors
        await self.controller.deviceManager.stopTrial()  # End trial
        # Load data from Exo into CSV
        self.controller.trial.loadDataToCSV(self.controller.deviceManager)

    def MachineLearning_on_device_disconnected(self):
        tk.messagebox.showwarning("Device Disconnected", "Please Reconnect")
        
        self.controller.trial.loadDataToCSV(
            self.controller.deviceManager, True
        )  # Load data from Exo into CSV
        self.controller.show_frame("ScanWindow")# Navigate back to the scan page
        self.controller.frames["ScanWindow"].show()  # Call show method to reset elements
            
