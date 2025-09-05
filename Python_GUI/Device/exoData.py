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
        data0,  # rightTorque
        data1,  # rightState
        data2,  # rightSet
        data3,  # leftTorque
        data4,  # leftState
        data5,  # leftSet
        data6,  # rightFsr
        data7,  # leftFsr
        data8,  # MinSV
        data9,  # MaxSV
        data10,  # MinSA
        data11,  # MaxSA
        data13,  # maxFSR
        data14,  # stanceTime
        data15,  # swingTime
        Task,
        data12,  # Battery
    ):
        timestamp = int(datetime.now().timestamp())  # Current epoch time
        self.epochTime.append(timestamp)  # New list for epoch time
        self.tStep.append(x_Time)
        self.rTorque.append(data0)  # rightTorque
        self.rSetP.append(data2)  # rightSet
        self.rState.append(data1)  # rightState
        self.lTorque.append(data3)  # leftTorque
        self.lSetP.append(data5)  # leftSet
        self.lState.append(data4)  # leftState
        self.lFsr.append(data7)  # leftFsr
        self.rFsr.append(data6)  # rightFsr
        self.MinShankVel.append(data8)  # MinSV
        self.MaxShankVel.append(data9)  # MaxSV
        self.MinShankAng.append(data10)  # MinSA
        self.MaxShankAng.append(data11)  # MaxSA
        self.MaxFSR.append(data13)  # maxFSR
        self.StanceTime.append(data14)  # stanceTime
        self.SwingTime.append(data15)  # swingTime
        self.Task.append(Task)
        self.BatteryPercent.set("Battery: " + str(round(data12))+"%")  # Battery
        self.Mark.append(self.MarkVal)
        