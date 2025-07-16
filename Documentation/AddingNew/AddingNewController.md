# Adding a new controller

## parseIni.h
1. Go to the config_defs namespace and the joint(s) you are creating a controller for and add it to the enum class so we can reference it.
2. Go to the config_map namespace and add for the joint(s) you are creating a controller for and add the name that will be used in the config.ini file.

## ControllerData.h
1. Go to the controller_defs namespace and add a namespace for your controller following the template from the other controllers
    - Create idx values that will be used to access the different parameters (remember this is 0 indexed)
    - Add a num_parameter value that stores the number of parameters the controller needs.
    - if this number is larger than the max_parameters at the bottom of controller_defs, change max_parameters to your value. 

## Controller.h
1. Under Controller.h create a new controller that inherits the _Controller class.
    - this must have:
        - A constructor that takes in the joint id and exo_data pointer.
        - A destructor
        - And a calc_motor_cmd() function that returns the torque command in Nm.
    - Additional private functions or variables can be added as needed but the only calls to a controller will be to the three required functions.

## ControllerData.h
1. This section is optional, at the bottom of ControllerData.h you can add any variables for your controller that you would like to be able to plot, these would be in place of the variables defined in Controller.h. 
	- To assign value to these parameters in Controller.cpp, you need to use the following formatting:
		- controller_data->NAMEOFVARIABLE = VALUE;
		
## Controller.cpp
1. Define all the functions for the controller.  
    - An initializer list for _Controller will be needed when calling the constructor.  
    
## Joint.h
1. Under the joint(s) (hip, knee, ankle) the controller is valid for, add an object to the new controller.

## Joint.cpp
1. For the joint(s) the controller is valid for:
    1. Add the constructor for the object you just added to the initializer list (the same name as in Joint.h).
    2. In set_controller(), add the controller to the switch case, referencing the joint controller namespace in the config_defs namespace.  
        - Use the existing controllers for the template
        - This changes which controller object the _controller pointer is using.
        - Don't forget to add the 'break;' at the end of the switch case!
	- Note: The switch case references the config_defs name and the value passed to _controller references the object created in Joint.h

## Create parameter file
1. In the SDCard folder go to the joint your controller is for.
2. Create a new parameter csv file, I would recommend just copying one of the existing ones
3. The first cell in the first line is the number of lines in the header.  Parameters will start being read after this number of lines.
4. The first cell in the second line is the number of parameters to read, this should match what you put in ControllerData.h.  This was easier than trying to get the reading a full line of floats to be robust.
5. After the header lines, put the parameter sets you would like to use, one set per line. The parameters should be listed in the index order from ControllerData.h. The first set (row) will be used as the default values for the controller.
6. If the controller is multi joint copy to the other joint folders.

## ParamsFromSD.h
1. In the controller_parameter_filenames namespace go to the appropriate joint
2. Link the controller id from config_defs and the path on the SD card for the parameter file you just created.

## uart_commands.h
1. This section is optional, if you want to plot or save one on of the variables you created in ControllerData.h:
	- In get_real_time_data()
		- Find the case for the joint you are using the controller for (e.g., bilateral_hip)
		- Assign the variable to one of the data spots in the rx.msg (e.g., rx_msg.data[0])
			- If the variable is specific to the controller, use the following formatting:
				- rx_msg.data[#] = exo_data->NAME_side.JOINT.controller.VARIABLENAME;
					- Here: NAME_side should either be left_side or right_side, JOINT should be the joint you are working with, and VARIABLENAME is the name of the variable you want to plot.
			- You can also plot variables specific to a side (defined in SideData.h) using the following formatting:
				- rx_msg.data[#] = exo_data->NAME_side.VARIABLENAME;

## Done
It should now be good to go.  You will need to change the controller and update the parameters.