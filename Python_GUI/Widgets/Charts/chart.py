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
        self.x_values = []
        self.y_values = []
        self.second_y = []

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

    def update_plot(self, x_values, y_values, second_y, bottom_lim, top_lim, title):
        max_points = 20  # Keep only the last 20 points
        if len(x_values) > max_points:
            x_values = x_values[-max_points:]
            y_values = y_values[-max_points:]
            second_y = second_y[-max_points:]
        # Update line data
        self.line1.set_data(x_values, y_values)
        self.line2.set_data(x_values, second_y)

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
        self.x_values = []
        self.y_values = []
        self.second_y = []
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
        super().__init__(master, "Left Torque")
    def animate(self, chart_selection):
        top_controller = None
        title = " "
        bottom_limit = -1
        top_limit = 1
        if chart_selection == "Data 0-3":
            top_controller = (
                self.master.controller.deviceManager._realTimeProcessor._chart_data.data0
            )
            top_measure = (
                self.master.controller.deviceManager._realTimeProcessor._chart_data.data1
            )
            title = "Data 0 and 1"
        elif chart_selection == "Data 4-7":
            top_controller = (
                self.master.controller.deviceManager._realTimeProcessor._chart_data.data4
            )
            top_measure = (
                self.master.controller.deviceManager._realTimeProcessor._chart_data.data5
            )
            title = "Data 4 and 5"
            bottom_limit = 0
            top_limit = 1.1

        if top_controller is None or top_measure is None:
            top_controller = 0
            top_measure = 0

        self.x_values.append(dt.datetime.now())
        self.y_values.append(top_controller)
        self.second_y.append(top_measure)
        self.ax.set_title(title)

        self.update_plot(self.x_values, self.y_values, self.second_y, bottom_limit, top_limit, title)


class BottomPlot(BasePlot):
    def __init__(self, master):
        super().__init__(master, "Right Torque")

    def animate(self, chart_selection):
        top_controller = None
        bottom_limit = -1
        top_limit = 1
        if chart_selection == "Data 0-3":
            top_controller = (
                self.master.controller.deviceManager._realTimeProcessor._chart_data.data2
            )
            top_measure = (
                self.master.controller.deviceManager._realTimeProcessor._chart_data.data3
            )
            title = "Data 2 and 3"
        elif chart_selection == "Data 4-7":
            top_controller = (
                self.master.controller.deviceManager._realTimeProcessor._chart_data.data6
            )
            top_measure = (
                self.master.controller.deviceManager._realTimeProcessor._chart_data.data7
            )
            bottom_limit = 0
            top_limit = 1.1
            title = "Data 6 and 7"

        if top_controller is None or top_measure is None:
            top_controller = 0
            top_measure = 0

        self.x_values.append(dt.datetime.now())
        self.y_values.append(top_controller)
        self.second_y.append(top_measure)
        self.ax.set_title(title)

        self.update_plot(self.x_values, self.y_values, self.second_y, bottom_limit, top_limit, title)

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

    def animate(self, chart_selection):
        top_measure = None
        title = " "
        bottom_limit = 0
        top_limit = 1.1
        if chart_selection == "Right Leg":
            top_measure = (
                self.master.controller.deviceManager._realTimeProcessor._chart_data.data5
            )
            title = "Right Leg"
        elif chart_selection == "Left Leg":
            top_measure = (
                self.master.controller.deviceManager._realTimeProcessor._chart_data.data7
            )
            title = "Left Leg"

        if top_measure is None:
            top_measure = 0

        self.x_values.append(dt.datetime.now())
        self.y_values.append(self.goal)
        self.second_y.append(top_measure)
        self.ax.set_title(title)

        # Check if top_measure crosses the goal
        if self.goal is not None:
            if top_measure > self.goal and not self.above_goal:
                self.counter_above_goal += 1  # Increment the counter
                self.above_goal = True  # Set the flag to true
                self.master.update_counter_label()  # Notify BioFeedback to update counter
            elif top_measure <= self.goal:
                self.above_goal = False  # Reset the flag when it goes below or equal to the goal

        self.update_plot(self.x_values, self.y_values, self.second_y, bottom_limit, top_limit, title)
