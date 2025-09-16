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
        # Get the selected parameter indices from the dropdowns
        # Top right dropdown 1 controls blue line (line1)
        # Top right dropdown 2 controls orange line (line2)
        
        try:
            # Get the selected parameter names from the dropdowns (these are cleaned names)
            blue_param_name_clean = self.master.topRightDropdown1.get()
            orange_param_name_clean = self.master.topRightDropdown2.get()
            
            # Get indices directly from dropdown selection (much faster)
            blue_index = self._get_param_index(blue_param_name_clean)
            orange_index = self._get_param_index(orange_param_name_clean)
            
            # Access data the same way as the original code using _chart_data
            chart_data = self.master.controller.deviceManager._realTimeProcessor._chart_data
            
            # Get the parameter values array
            param_values = chart_data.param_values if hasattr(chart_data, 'param_values') else []
            
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
    
    def _get_param_index(self, param_name_clean):
        """Get parameter index directly from dropdown selection - much faster than cache lookup"""
        try:
            # Get the current dropdown values to find the index
            dropdown_values = self.master.topRightDropdown1['values']
            if param_name_clean in dropdown_values:
                return dropdown_values.index(param_name_clean)
            return 0  # Default to first parameter
        except (AttributeError, ValueError):
            return 0


class BottomPlot(BasePlot):
    def __init__(self, master):
        super().__init__(master, "Bottom Plot")

    def animate(self, chartSelection):
        # Get the selected parameter indices from the dropdowns
        # Bottom right dropdown 1 controls blue line (line1)
        # Bottom right dropdown 2 controls orange line (line2)
        
        try:
            # Get the selected parameter names from the dropdowns (these are cleaned names)
            blue_param_name_clean = self.master.bottomRightDropdown1.get()
            orange_param_name_clean = self.master.bottomRightDropdown2.get()
            
            # Get indices directly from dropdown selection (much faster)
            blue_index = self._get_param_index(blue_param_name_clean)
            orange_index = self._get_param_index(orange_param_name_clean)
            
            # Access data the same way as the original code using _chart_data
            chart_data = self.master.controller.deviceManager._realTimeProcessor._chart_data
            
            # Get the parameter values array
            param_values = chart_data.param_values if hasattr(chart_data, 'param_values') else []
            
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
    
    def _get_param_index(self, param_name_clean):
        """Get parameter index directly from dropdown selection - much faster than cache lookup"""
        try:
            # Get the current dropdown values to find the index
            dropdown_values = self.master.bottomRightDropdown1['values']
            if param_name_clean in dropdown_values:
                return dropdown_values.index(param_name_clean)
            return 0  # Default to first parameter
        except (AttributeError, ValueError):
            return 0

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