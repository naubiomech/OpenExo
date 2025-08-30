from tkinter import StringVar
from datetime import datetime

class ExoData:
    def __init__(self):
        self.tStep = []
        self.rTorque = []
        self.epochTime = []  # New list to store epoch timestamps
        # self.rSetP = []
        # self.rState = []
        # self.lTorque = []
        # self.lSetP = []
        # self.lState = []
        # self.lFsr = []
        # self.rFsr = []
        self.paramNames =[] # List to store parameter names
        self.paramValues = [[]]  # List to store parameter values this will be a 2D list
        self.numParams = 0
        #record our features
        self.MinShankVel=[]
        self.MaxShankVel=[]
        self.MinShankAng=[]
        self.MaxShankAng=[]
        self.MaxFSR=[]
        self.StanceTime=[]
        self.SwingTime=[]
        #and the predicted task/state
        self.Task=[]
        self.BatteryPercent=StringVar()
        self.BatteryPercent.set("Battery Percent: ?")
        self.Mark=[] #mark our Trials
        self.MarkVal=0
        self.MarkLabel=StringVar()
        self.MarkLabel.set("Mark: " +str(self.MarkVal))

#working on updating this data stucture to match new data streaming protocol will need another function to creat parameter names list
    def addDataPoints(
        self,
        x_Time,
        param_values,
        MinSV,
        MaxSV,
        MinSA,
        MaxSA,
        maxFSR,
        stanceTime,
        swingTime,
        Task,
        Battery, 
    ):
        timestamp = int(datetime.now().timestamp())  # Current epoch time
        self.epochTime.append(timestamp)  # New list for epoch time
        self.tStep.append(x_Time)
        self.paramValues.append(param_values)  # Append the parameter values
        # self.rTorque.append(rightToque)
        # self.rSetP.append(rightSet)
        # self.rState.append(rightState)
        # self.lTorque.append(leftTorque)
        # self.rSetP.append(leftSet)
        # self.lState.append(leftState)
        # self.lFsr.append(leftFsr)
        # self.rFsr.append(rightFsr)
        self.MinShankVel.append(MinSV)
        self.MaxShankVel.append(MaxSV)
        self.MinShankAng.append(MinSA)
        self.MaxShankAng.append(MaxSA)
        self.MaxFSR.append(maxFSR)
        self.StanceTime.append(stanceTime)
        self.SwingTime.append(swingTime)
        self.Task.append(Task)
        self.BatteryPercent.set("Battery: " + str(round(Battery))+"%")
        self.Mark.append(self.MarkVal)
    
    def setParameterNames(self, param_names):
        """Set the parameter names for the data structure"""
        self.paramNames = param_names
        self.numParams = len(param_names)
    
    def initializeParamValues(self):
        """Initialize the paramValues structure properly"""
        if not self.paramValues or len(self.paramValues) == 0:
            self.paramValues = [] 
        