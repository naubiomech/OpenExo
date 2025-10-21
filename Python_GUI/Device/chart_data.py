class ChartData:
    def __init__(self):
        self.data0 = 0.0  # rightTorque
        self.data1 = 0.0  # rightState
        self.data2 = 0.0  # rightSet
        self.data3 = 0.0  # leftTorque
        self.data4 = 0.0  # leftState
        self.data5 = 0.0  # leftSet
        self.data6 = 0.0  # rightFsr
        self.data7 = 0.0  # leftFsr
        self.data8 = 0.0  # minSV
        self.data9 = 0.0  # maxSV
        self.data10 = 0.0  # battery
        self.data11 = 0.0  # maxSA
        self.data12 = 0.0  # battery
        self.data13 = 0.0  # maxFSR
        self.data14 = 0.0  # stancetime
        self.data15 = 0.0  # swingtime

    def updateValues(
        self,
        data0,  # rightTorque
        data1,  # rightState
        data2,  # rightSet
        data3,  # leftTorque
        data4,  # leftState
        data5,  # leftSet
        data6,  # rightFsr
        data7,  # leftFsr
        data8,  # minSV
        data9,  # maxSV
        data10,  # battery
        data11,  # maxSA
        data12,  # minSA
        data13,  # maxFSR
        data14,  # stancetime
        data15,  # swingtime
    ):
        self.data0 = data0
        self.data1 = data1
        self.data2 = data2
        self.data3 = data3
        self.data4 = data4
        self.data5 = data5
        self.data6 = data6
        self.data7 = data7
        self.data8 = data8
        self.data9 = data9
        self.data10 = data10
        self.data11 = data11
        self.data12 = data12
        self.data13 = data13
        self.data14 = data14
        self.data15 = data15
