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

    # old function to update the values of the chart data
    # this is not used anymore, but is kept for reference
    def updateValues(
        self,
        rightTorque,
        rightState,
        leftTorque,
        leftState,
        rightSet,
        leftSet,
        rightFsr,
        leftFsr
    ):           
        self.rightTorque = rightTorque
        self.rightState = rightState
        self.leftTorque = leftTorque
        self.leftState = leftState
        self.rightSet = rightSet
        self.leftSet = leftSet
        self.rightFsr = rightFsr
        self.leftFsr = leftFsr
