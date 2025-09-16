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
        self.blue_index = 0
        self.orange_index = 1
        

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
        
    def animate(self, chart_selection):
        top_controller = None
        top_measure = None
        bottom_limit = -1
        top_limit = 1
        
        try:
            param_values = self.master.controller.deviceManager._realTimeProcessor.param_values
            if self.blue_index < len(param_values):
                top_controller = param_values[self.blue_index]
            if self.orange_index < len(param_values):
                top_measure = param_values[self.orange_index]
            title = f"Blue: {self.blue_index}, Orange: {self.orange_index}"
        except (AttributeError, IndexError, ValueError):
            pass

        if top_controller is None or top_measure is None:
            top_controller = 0
            top_measure = 0

        x_time = self.master.controller.deviceManager._realTimeProcessor.x_time
        self.xValues.append(x_time)
        self.yValues.append(top_controller)
        self.secondY.append(top_measure)
        self.ax.set_title(title)

        self.update_plot(self.xValues, self.yValues, self.secondY, bottom_limit, top_limit, title)
    
    def update_indices(self):
        try:
            dropdown_values = self.master.topRightDropdown1['values']
            blue_name = self.master.topRightDropdown1.get()
            orange_name = self.master.topRightDropdown2.get()
            
            if blue_name in dropdown_values:
                self.blue_index = dropdown_values.index(blue_name)
            if orange_name in dropdown_values:
                self.orange_index = dropdown_values.index(orange_name)
        except (AttributeError, ValueError):
            pass


class BottomPlot(BasePlot):
    def __init__(self, master):
        super().__init__(master, "Bottom Plot")

    def animate(self, chart_selection):
        top_controller = None
        top_measure = None
        bottom_limit = -1
        top_limit = 1
        
        try:
            param_values = self.master.controller.deviceManager._realTimeProcessor.param_values
            if self.blue_index < len(param_values):
                top_controller = param_values[self.blue_index]
            if self.orange_index < len(param_values):
                top_measure = param_values[self.orange_index]
            title = f"Blue: {self.blue_index}, Orange: {self.orange_index}"
        except (AttributeError, IndexError, ValueError):
            pass

        if top_controller is None or top_measure is None:
            top_controller = 0
            top_measure = 0

        x_time = self.master.controller.deviceManager._realTimeProcessor.x_time
        self.xValues.append(x_time)
        self.yValues.append(top_controller)
        self.secondY.append(top_measure)
        self.ax.set_title(title)

        self.update_plot(self.xValues, self.yValues, self.secondY, bottom_limit, top_limit, title)
    
    def update_indices(self):
        try:
            dropdown_values = self.master.bottomRightDropdown1['values']
            blue_name = self.master.bottomRightDropdown1.get()
            orange_name = self.master.bottomRightDropdown2.get()
            
            if blue_name in dropdown_values:
                self.blue_index = dropdown_values.index(blue_name)
            if orange_name in dropdown_values:
                self.orange_index = dropdown_values.index(orange_name)
        except (AttributeError, ValueError):
            pass

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

        x_time = self.master.controller.deviceManager._realTimeProcessor.x_time
        self.xValues.append(x_time)
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