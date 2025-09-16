import datetime as dt
import tkinter as tk

import matplotlib.animation as animation
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg



class BasePlot:
    figure_size = (7, 2.5)
    def __init__(self, master, title):
        self.master = master
        self.title = title
        self.figure = plt.Figure(figsize=BasePlot.figure_size)
        self.ax = self.figure.add_subplot(1, 1, 1)
        self.xValues = []
        self.yValues = []
        self.secondY = []
        self.param_values = []  # Direct reference to parameter values
        self.param_index_cache = {}  # Direct reference to parameter cache

        # Plot initialization
        self.line1, = self.ax.plot([], [], label='Controller Value', color='blue')
        self.line2, = self.ax.plot([], [], label='Measurement Value', color='red')

        self.ax.set_xticks([])  # Hide x-ticks for simplicity
        self.ax.set_title(title)

        self.canvas = FigureCanvasTkAgg(self.figure, master=self.master)
        self.canvas.draw()
        self.canvas.get_tk_widget().grid()

    def animate(self):
        raise NotImplementedError("Subclasses should implement this method")
   
    @classmethod
    def set_figure_size(cls, size):
        """Set the figure size for all plots."""
        cls.figure_size = size

    def update_plot(self, xValues, yValues, secondY, bottomLim, topLim, title):
        max_points = 20  # Keep only the last 20 points
        if len(xValues) > max_points:
            xValues = xValues[-max_points:]
            yValues = yValues[-max_points:]
            secondY = secondY[-max_points:]
        # Update line data
        self.line1.set_data(xValues, yValues)
        self.line2.set_data(xValues, secondY)

        # Efficiently redraw only the updated parts
        self.ax.relim()  # Recalculate limits based on new data
        self.ax.autoscale(enable=True, axis='both', tight=False)

        # Draw without clearing axes
        self.canvas.draw_idle()
        self.canvas.flush_events()

    def refresh_figure(self):
        """Refresh the figure if the size is updated."""
        self.figure.set_size_inches(BasePlot.figure_size)
        self.canvas.draw()
        
    def clear_plot(self):
        """Clear the plot data and refresh the display."""
        self.xValues = []
        self.yValues = []
        self.secondY = []
        # Optionally clear the axes if you want to remove any lingering data
        self.ax.cla()
        self.ax.set_title(None)
        self.ax.set_xticks([])  # Restore x-ticks hidden state
        # Reinitialize the lines
        self.line1, = self.ax.plot([], [], label='Controller Value', color='blue')
        self.line2, = self.ax.plot([], [], label='Measurement Value', color='red')
        self.canvas.draw_idle()

class TopPlot(BasePlot):
    def __init__(self, master):
        super().__init__(master, "Top Plot")
        
    def animate(self, chartSelection):
        # Check the plotting mode from the master frame
        plotting_mode = getattr(self.master, 'plotting_mode', 'dropdowns')
        
        if plotting_mode == "indices_0_3":
            # Use the traditional index-based plotting for Data 0-3
            self._animate_indices_0_3_mode(chartSelection)
        elif plotting_mode == "indices_4_7":
            # Use the traditional index-based plotting for Data 4-7
            self._animate_indices_4_7_mode(chartSelection)
        else:
            # Use the new dropdown-based plotting
            self._animate_dropdown_mode()
    
    def _animate_indices_0_3_mode(self, chartSelection):
        """Traditional index-based plotting mode for Controller data (indices 0-3)"""
        try:
            # Get the parameter values from the real-time data
            param_values = self.master.controller.deviceManager._realTimeProcessor.param_values
            
            # Controller mode (indices 0-3)
            blue_value = param_values[0] if len(param_values) > 0 else 0.0      # Index 0
            orange_value = param_values[1] if len(param_values) > 1 else 0.0   # Index 1
            title = "Controller Mode"
            
            bottomLimit = -1
            topLimit = 1
            
        except (AttributeError, IndexError, ValueError):
            blue_value = 0.0
            orange_value = 0.0
            title = "Top Plot - Controller Mode"
            bottomLimit = -1
            topLimit = 1

        # Update the plot data
        self.xValues.append(dt.datetime.now())
        self.yValues.append(blue_value)
        self.secondY.append(orange_value)
        self.ax.set_title(title)

        self.update_plot(self.xValues, self.yValues, self.secondY, bottomLimit, topLimit, title)
    
    def _animate_indices_4_7_mode(self, chartSelection):
        """Traditional index-based plotting mode for Sensor data (indices 4-7)"""
        try:
            # Get the parameter values from the real-time data
            param_values = self.master.controller.deviceManager._realTimeProcessor.param_values
            
            # Sensor mode (indices 4-7)
            blue_value = param_values[4] if len(param_values) > 4 else 0.0      # Index 4
            orange_value = param_values[5] if len(param_values) > 5 else 0.0   # Index 5
            title = "Sensor Mode"
            
            bottomLimit = -1
            topLimit = 1
            
        except (AttributeError, IndexError, ValueError):
            blue_value = 0.0
            orange_value = 0.0
            title = "Top Plot - Sensor Mode"
            bottomLimit = -1
            topLimit = 1

        # Update the plot data
        self.xValues.append(dt.datetime.now())
        self.yValues.append(blue_value)
        self.secondY.append(orange_value)
        self.ax.set_title(title)

        self.update_plot(self.xValues, self.yValues, self.secondY, bottomLimit, topLimit, title)

    def _animate_dropdown_mode(self):
        """Dropdown-based plotting mode"""
        # Get the selected parameter indices from the dropdowns
        # Top right dropdown 1 controls blue line (line1)
        # Top right dropdown 2 controls orange line (line2)
        
        try:
            # Get the selected parameter names from the dropdowns (these are cleaned names)
            blue_param_name_clean = self.master.topRightDropdown1.get()
            orange_param_name_clean = self.master.topRightDropdown2.get()
            
            # Use cached indices if available, otherwise calculate them
            if not hasattr(self, '_param_index_cache') or self._param_index_cache is None:
                self._build_param_index_cache()
            
            # Get cached indices
            blue_index = self._param_index_cache.get(blue_param_name_clean, 0)
            orange_index = self._param_index_cache.get(orange_param_name_clean, 1)
            
            # Get the parameter values from the real-time data
            param_values = self.master.controller.deviceManager._realTimeProcessor.param_values
            
            # Extract the values for the selected parameters
            blue_value = param_values[blue_index] if blue_index < len(param_values) else 0.0
            orange_value = param_values[orange_index] if orange_index < len(param_values) else 0.0
            
            # Set appropriate limits based on the data type
            bottomLimit = -1
            topLimit = 1
            
            # Update the plot title to show what's being plotted (using cleaned names)
            title = f"Blue: {blue_param_name_clean}, Orange: {orange_param_name_clean}"
            
        except (AttributeError, IndexError, ValueError):
            # Fallback to default values if there's an error
            blue_value = 0.0
            orange_value = 0.0
            title = "Top Plot - Select Parameters"
            bottomLimit = -1
            topLimit = 1

        # Update the plot data
        self.xValues.append(dt.datetime.now())
        self.yValues.append(blue_value)      # Blue line (line1)
        self.secondY.append(orange_value)   # Orange line (line2)
        self.ax.set_title(title)

        self.update_plot(self.xValues, self.yValues, self.secondY, bottomLimit, topLimit, title)
    
    def _build_param_index_cache(self):
        """Build a cache of parameter name to index mappings for fast lookup"""
        try:
            param_names = self.master.controller.deviceManager._realTimeProcessor._chart_data.param_names
            self._param_index_cache = {}
            
            for i, original_name in enumerate(param_names):
                # Create cleaned version of original name for comparison
                if original_name.startswith("exo_data->"):
                    cleaned_original = original_name[10:]
                else:
                    cleaned_original = original_name
                
                # Store the mapping
                self._param_index_cache[cleaned_original] = i
        except (AttributeError, IndexError, ValueError):
            self._param_index_cache = {}


class BottomPlot(BasePlot):
    def __init__(self, master):
        super().__init__(master, "Bottom Plot")

    def animate(self, chartSelection):
        # Check the plotting mode from the master frame
        plotting_mode = getattr(self.master, 'plotting_mode', 'dropdowns')
        
        if plotting_mode == "indices_0_3":
            # Use the traditional index-based plotting for Data 0-3
            self._animate_indices_0_3_mode(chartSelection)
        elif plotting_mode == "indices_4_7":
            # Use the traditional index-based plotting for Data 4-7
            self._animate_indices_4_7_mode(chartSelection)
        else:
            # Use the new dropdown-based plotting
            self._animate_dropdown_mode()
    
    def _animate_indices_0_3_mode(self, chartSelection):
        """Traditional index-based plotting mode for Controller data (indices 0-3)"""
        try:
            # Get the parameter values from the real-time data
            param_values = self.master.controller.deviceManager._realTimeProcessor.param_values
            
            # Controller mode (indices 0-3)
            blue_value = param_values[2] if len(param_values) > 2 else 0.0      # Index 2
            orange_value = param_values[3] if len(param_values) > 3 else 0.0   # Index 3
            title = "Controller Mode"
            
            bottomLimit = -1
            topLimit = 1
            
        except (AttributeError, IndexError, ValueError):
            blue_value = 0.0
            orange_value = 0.0
            title = "Bottom Plot - Controller Mode"
            bottomLimit = -1
            topLimit = 1

        # Update the plot data
        self.xValues.append(dt.datetime.now())
        self.yValues.append(blue_value)
        self.secondY.append(orange_value)
        self.ax.set_title(title)

        self.update_plot(self.xValues, self.yValues, self.secondY, bottomLimit, topLimit, title)
    
    def _animate_indices_4_7_mode(self, chartSelection):
        """Traditional index-based plotting mode for Sensor data (indices 4-7)"""
        try:
            # Get the parameter values from the real-time data
            param_values = self.master.controller.deviceManager._realTimeProcessor.param_values
            
            # Sensor mode (indices 4-7)
            blue_value = param_values[6] if len(param_values) > 6 else 0.0      # Index 6
            orange_value = param_values[7] if len(param_values) > 7 else 0.0   # Index 7
            title = "Sensor Mode"
            
            bottomLimit = -1
            topLimit = 1
            
        except (AttributeError, IndexError, ValueError):
            blue_value = 0.0
            orange_value = 0.0
            title = "Bottom Plot - Sensor Mode"
            bottomLimit = -1
            topLimit = 1

        # Update the plot data
        self.xValues.append(dt.datetime.now())
        self.yValues.append(blue_value)
        self.secondY.append(orange_value)
        self.ax.set_title(title)

        self.update_plot(self.xValues, self.yValues, self.secondY, bottomLimit, topLimit, title)

    def _animate_dropdown_mode(self):
        """Dropdown-based plotting mode"""
        # Get the selected parameter indices from the dropdowns
        # Bottom right dropdown 1 controls blue line (line1)
        # Bottom right dropdown 2 controls orange line (line2)
        
        try:
            # Get the selected parameter names from the dropdowns (these are cleaned names)
            blue_param_name_clean = self.master.bottomRightDropdown1.get()
            orange_param_name_clean = self.master.bottomRightDropdown2.get()
            
            # Get the original parameter names list to find the indices
            param_names = self.master.controller.deviceManager._realTimeProcessor._chart_data.param_names
            
            # Use cached indices if available, otherwise calculate them
            if not hasattr(self, '_param_index_cache') or self._param_index_cache is None:
                self._build_param_index_cache()
            
            # Get cached indices
            blue_index = self._param_index_cache.get(blue_param_name_clean, 0)
            orange_index = self._param_index_cache.get(orange_param_name_clean, 1)
            
            # Get the parameter values from the real-time data
            param_values = self.master.controller.deviceManager._realTimeProcessor.param_values
            
            # Extract the values for the selected parameters
            blue_value = param_values[blue_index] if blue_index < len(param_values) else 0.0
            orange_value = param_values[orange_index] if orange_index < len(param_values) else 0.0
            
            # Set appropriate limits based on the data type
            bottomLimit = -1
            topLimit = 1
            
            # Update the plot title to show what's being plotted (using cleaned names)
            title = f"Blue: {blue_param_name_clean}, Orange: {orange_param_name_clean}"
            
        except (AttributeError, IndexError, ValueError):
            # Fallback to default values if there's an error
            blue_value = 0.0
            orange_value = 0.0
            title = "Bottom Plot - Select Parameters"
            bottomLimit = -1
            topLimit = 1

        # Update the plot data
        self.xValues.append(dt.datetime.now())
        self.yValues.append(blue_value)      # Blue line (line1)
        self.secondY.append(orange_value)   # Orange line (line2)
        self.ax.set_title(title)

        self.update_plot(self.xValues, self.yValues, self.secondY, bottomLimit, topLimit, title)
    
    def _build_param_index_cache(self):
        """Build a cache of parameter name to index mappings for fast lookup"""
        try:
            param_names = self.master.controller.deviceManager._realTimeProcessor._chart_data.param_names
            self._param_index_cache = {}
            
            for i, original_name in enumerate(param_names):
                # Create cleaned version of original name for comparison
                if original_name.startswith("exo_data->"):
                    cleaned_original = original_name[10:]
                else:
                    cleaned_original = original_name
                
                # Store the mapping
                self._param_index_cache[cleaned_original] = i
        except (AttributeError, IndexError, ValueError):
            self._param_index_cache = {}

class FSRPlot(BasePlot):
    def __init__(self, master, goal=None):
        super().__init__(master, "Left FSR")
        self.goal = None  # Store the goal value
        self.counter_above_goal = 0  # Initialize the counter
        self.above_goal = False  # Track if currently above the goal

        # Initialize plot lines for FSR and target
        self.line_fsr, = self.ax.plot([], [], label='FSR Value', color='blue')
        self.line_target, = self.ax.plot([], [], label='Target Value', color='red', linestyle='--')

    def set_goal(self, goal):
        self.goal = goal  # Set the new goal

    def animate(self, chartSelection):
        topMeasure  = None
        title = " "
        bottomLimit = 0
        topLimit = 1.1
        if chartSelection == "Right Leg":
            topMeasure = (
                self.master.controller.deviceManager._realTimeProcessor._chart_data.leftSet
            )
            title = "Right Leg"
        elif chartSelection == "Left Leg":
            topMeasure = (
                self.master.controller.deviceManager._realTimeProcessor._chart_data.leftFsr
            )
            title = "Left Leg"

        if topMeasure is None:
            topMeasure = 0

        self.xValues.append(dt.datetime.now())
        self.yValues.append(self.goal)
        self.secondY.append(topMeasure)
        self.ax.set_title(title)

        # Check if topMeasure crosses the goal
        if self.goal is not None:
            if topMeasure > self.goal and not self.above_goal:
                self.counter_above_goal += 1  # Increment the counter
                self.above_goal = True  # Set the flag to true
                self.master.update_counter_label()  # Notify BioFeedback to update counter
            elif topMeasure <= self.goal:
                self.above_goal = False  # Reset the flag when it goes below or equal to the goal

        self.update_plot(self.xValues, self.yValues, self.secondY, bottomLimit , topLimit, title)