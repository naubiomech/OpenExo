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
        
class TopPlot(BasePlot):
    def __init__(self, master):
        super().__init__(master, "Left Torque")
    def animate(self, chartSelection):
        topController = None
        title = " "
        bottomLimit = -1
        topLimit = 1
        if chartSelection == "Controller":
            topController = (
                self.master.controller.deviceManager._realTimeProcessor._chart_data.leftTorque
            )
            topMeasure = (
                self.master.controller.deviceManager._realTimeProcessor._chart_data.leftSet
            )
            title = "Controller"
        elif chartSelection == "Sensor":
            topController = (
                self.master.controller.deviceManager._realTimeProcessor._chart_data.leftState
            )
            topMeasure = (
                self.master.controller.deviceManager._realTimeProcessor._chart_data.leftFsr
            )
            title = "Sensor"
            bottomLimit = 0
            topLimit = 1.1

        if topController is None or topMeasure is None:
            topController = 0
            topMeasure = 0

        self.xValues.append(dt.datetime.now())
        self.yValues.append(topController)
        self.secondY.append(topMeasure)

        self.update_plot(self.xValues, self.yValues, self.secondY, bottomLimit,topLimit,title)


class BottomPlot(BasePlot):
    def __init__(self, master):
        super().__init__(master, "Right Torque")

    def animate(self, chartSelection):
        topController = None
        bottomLimit = -1
        topLimit = 1
        if chartSelection == "Controller":
            topController = (
                self.master.controller.deviceManager._realTimeProcessor._chart_data.rightTorque
            )
            topMeasure = (
                self.master.controller.deviceManager._realTimeProcessor._chart_data.rightSet
            )
            title = "Controller"
        elif chartSelection == "Sensor":
            topController = (
                self.master.controller.deviceManager._realTimeProcessor._chart_data.rightState
            )
            topMeasure = (
                self.master.controller.deviceManager._realTimeProcessor._chart_data.rightFsr
            )
            bottomLimit = 0
            topLimit = 1.1
            title = "Sensor"

        if topController is None or topMeasure is None:
            topController = 0
            topMeasure = 0

        self.xValues.append(dt.datetime.now())
        self.yValues.append(topController)
        self.secondY.append(topMeasure)

        self.update_plot(self.xValues, self.yValues, self.secondY, bottomLimit,topLimit,title)

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
        if chartSelection == "Left Leg":
            topMeasure = (
                self.master.controller.deviceManager._realTimeProcessor._chart_data.leftFsr
            )
            title = "Left Leg"
        elif chartSelection == "Right Leg":
            topMeasure = (
                self.master.controller.deviceManager._realTimeProcessor._chart_data.rightFsr
            )
            title = "Right Leg"

        if topMeasure is None:
            topMeasure = 0

        self.xValues.append(dt.datetime.now())
        self.yValues.append(self.goal)
        self.secondY.append(topMeasure)

        # Check if topMeasure crosses the goal
        if self.goal is not None:
            if topMeasure > self.goal and not self.above_goal:
                self.counter_above_goal += 1  # Increment the counter
                self.above_goal = True  # Set the flag to true
                self.master.update_counter_label()  # Notify BioFeedback to update counter
            elif topMeasure <= self.goal:
                self.above_goal = False  # Reset the flag when it goes below or equal to the goal

        self.update_plot(self.xValues, self.yValues, self.secondY, bottomLimit , topLimit, title)
