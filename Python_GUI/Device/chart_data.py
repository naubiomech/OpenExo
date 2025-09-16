class ChartData:
    def __init__(self):
        self.rightTorque = 0.0
        self.rightState = 0.0
        self.leftTorque = 0.0
        self.leftState = 0.0
        self.rightSet = 0.0
        self.leftSet = 0.0
        self.rightFsr = 0.0
        self.leftFsr = 0.0
        self.param_names = []  # List to store parameter names
        self.param_values = []  # List to store parameter values
        self.num_params = 0 # Number of parameters

    #function to update the parameter names and number of parameters
    def updateNames(
        self, 
        param_names, 
        num_params
    ):
        self.param_names = param_names
        self.num_params = num_params

    # function to update the parameter values using the parameter list data streaming protocol
    def updateParamValues(
        self,
        param_values
    ):
        self.param_values = param_values

    # function to update the values of the chart data
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
        data10, # minSA
        data11, # maxSA
        data12, # battery
        data13, # maxFSR
        data14, # stancetime
        data15, # swingtime
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
