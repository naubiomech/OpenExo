from tkinter import StringVar
from datetime import datetime

class ExoData:
    def __init__(self):
        self.tStep = []
        self.rTorque = []
        self.epochTime = []  # New list to store epoch timestamps
        self.rSetP = []
        self.rState = []
        self.lTorque = []
        self.lSetP = []
        self.lState = []
        self.lFsr = []
        self.rFsr = []
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

    def addDataPoints(
        self,
        x_Time,
        rightToque,
        rightState,
        rightSet,
        leftTorque,
        leftState,
        leftSet,
        rightFsr,
        leftFsr,
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
        self.rTorque.append(rightToque)
        self.rSetP.append(rightSet)
        self.rState.append(rightState)
        self.lTorque.append(leftTorque)
        self.lSetP.append(leftSet)
        self.lState.append(leftState)
        self.lFsr.append(leftFsr)
        self.rFsr.append(rightFsr)
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
        