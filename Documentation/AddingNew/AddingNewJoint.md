# Adding a new joint

## Exocode.ini
1. Line 118: Add joint to the debug statement determining if it is used.
2. Line 148: Add joint to if statements relating to whether or not the joint is used
	- Copy the if statements for the left and right side for one joint (e.g., hip) and paste below last joint
	- Modify the joint to be named after the new joint you are adding. 
3. Line 398: Add in calibrate torque sensor and enable motors for the joint while in headless mode
	- Copy the formatting from another joint, make sure to do both left and right side.
4. Line 540: This section sets the controller for each joint (while in headless mode), copy the formatting for one joint and rename it to your new joint
	- Need to pull in the default controller information from the config
	- Then set the controller based on this configuration information

## ParseIni.h
1. Update exo_name in the config_defs namespace, make sure to encompass any configuration you would want with that joint (e.g., bilateral, unilateral, multi-joint)
2. Update JointType in the config_defs namespace
3. Update joint_id to utilize the proper byte for the new joint (see comment in code)
	- make sure to update the left and right joint information following the format as well. 
4. Add a new enum class for the joint's controllers following the other joints' formatting
	- Add in any controllers associated with the joint if there are any ready, you can always come back to this following "AddNewController"
5. Update the static const int variables at the end of the config_defs namespace to account for the new joint.
6. In config_map namespace, update the IniKeyCode exo_name to account for all the configurations made in the exo_name enum class in config_defs
7. In config_map namespace, add in a new IniKeyCode for the joint's controllers (following the format for the other joints)
	- Make sure to update the terminology of the entire section (i.e., the section after config_defs:: in the main text of it
8. Update the ConfigData struct at the end to match 
 
## ParseIni.cpp
1. ~Line 180 -> Onward: Copy template in each section to add in new joint so that it is formatted same as other joints.

## ExoData.h
1. In the functions "for_each_joint", add in the new joint following the format of the previous joints
    
## ExoData.cpp
1. In the function "get_used_joints" (~ Line 36), add in the new joint following the format of the other joints already there, make sure to do both the left and right sides
2. In the function "get_joint_with" (~ Line 59), add in the new joint following the format of the other joints already there, make sure to do both the left and right sides
3. Similarly, in the function "print" add in the prints of the new joint for each side (left and right) following the establish formatting.

## UART_commands.h
1. Update the function "get_config" (~Line 196) with the config_defs you added in ParseIni.h, refer to that file and look at the added "static const int" at the end of the config_defs namespace
2. Similarly, update the function "update_config" (~Line 227) with the config_defs you added in ParseIni.h, refer to that file and look at the added "static const int" at the end of the config_defs namespace
3. In "get_real_time_data" (~Line 306), add in a new case in the switch statement corresponding to the bilateral configuration of the joint and for any multi-joint instances. You can send up to 10 data points at a time using the established formmating.

## Side.h
1. Create Joint object specific to the joint that you are adding (~Line 158, is where the other joint objects are specificed, follow their formmating)

## Side.cpp
1. Add in the new joint object you just created in the .h file into the constructer of the side following the format of the other joints in the constructer 
2. In the "disable_motors" function (~Line 58), add in the new joint following the formatting of the other joints.
3. In the "read_data" function (~Line 98), add in an if statment at the end (~Line 142) that reads the data specific to the joint object (see other joints for formatting)
4. In the "check_calibration" function (~Line 161), add in a if statment at the end of the end of the function (Line 189) that checks the calibration specific to the new joint object (see other joints for formatting)
5. In the "update_motor_cmds" function (~Line 534), add in a new if statement at the end that runs the new joint functions (see other joints for formatting)

## SideData.h
1. ~Line 42: Create a new instance of jointdata and create an object for it for your new joint (see other joints for formatting)

## SideData.cpp
1. Add in the new joint object you just created in the .h file into the constructer of the sidedata following the format of the other joints in the constructer 
2. On the last line add in a new statement reconfiguring the joint you just added (see other joints for formatting). 

## Joint.h
1. Create a class specific to the joint, make sure it inherits the public _joint class
	- Make sure to include a joint specific constructor and destructor and the two essential functions "run_joint" and "set_controller" in the public section.
	- In the private section call instances of the controller class for those specific to the joint and create new objects for those classes
	- Aka copy what the other joints did and change the names and controllers

## Joint.cpp
1.	Add the new joint to the Joint Debug statement (~Line 40) in the _Joint constructor at the begining
2. 	In the function "get_torque_sensor_pin" add a new case to the switch statment specific to your added joint following the format of the other joints
3.	Similarly, in the function "get_motor_enable_pin", add a new case to the switch statement specific to your added joint.
4. 	Add in the new joint class following the format of the other joints
	- Make sure to include all the controller instances in the initializer list in addition to the _Joint instance 
	- Update the debug statements to refelct the newly added joint
	- Update the Switch Case that creates an object for the specific motor type 
	- Update the call of the "set_controller" function at the end of the constructor
	- Update the "run_joint" function 
		- update the debug statements, the rest is non-joint specific 
	-Update the "set_controller" function 
		- Update debug statement
		- Update switch case for controller ID

## JointData.cpp
1. Update the switch statement in the JointData initializer to include the new joint (~Line 31) 
	- Include the portion that sets the value for "is_used" and the if statment to flip the direction 
	- Follow the format of the other joints in this section 
2. Repeat this process for the "reconfigure" function 

## Motor.cpp
1. Update the switch statment in the motor initializer (~Line 39) to include the new joint

## MotorData.cpp
1.	Update the switch statement in the initalizer (~Line 16) to include the new joint
	- Include both the gearing and the flip direction portions
2. 	Repeat this process for the "reconfigure" function 

## ControllerData.cpp
1.	Update the switch statement in the constructor (~Line 16) to include the new joint
2. 	Repeat process for the "reconfigure" function (~Line 56)

## Controller.cpp
1.	Update the switch statement in the constructor of _Controller to incorporate the new joint (~Line 35)

## ble_commands.h
1. In the function "new_trq" update the joint_id mapping to encompass the new joint (make sure to do both left and right side)
	- Make sure you update the joint id number in these statments to follow a logical order (e.g., left_hip = 1, left knee = 2, left ankle = 3, left elbow = 4, ....)
2. Update the if statement associating controller data with the joint ID

## ParamsfromSD.h
1.	In the controller_parameter_filenames namespace, add in a ParamFilenameKey for the new joint and associate the appropriate controllers

## ParamsfromSD.cpp
1.	In the "print_param_error_message" function, update the switch statment to include the new joint
	- Make sure to update the actual switch statment itself during this process (~Line 10) in addition to adding the new case 
2. In the "set_controller_params" function 
	- Copy a case of the switch statement from another joint below the last added joint, update any reference to the specific joint (e.g., elbow, ankle,...) to your new joint
	- There is a lot going on here so make sure you did not miss any reference to the specific joint in this section. 
## RealTimeI2C.h
1.	In the rt_data namespace update the static int RT_LEN to include the configurations you added to "get_real_time_data" in uart_commands.h

## StatusDefs.h
1. In the messages namespace, update the error messages with the joint you are adding
	- Torque Sensor, Motor, Controller portions for both side (left, right)
	- Make sure to go back and update the numbering 
2. Update the nomenclature of the remaining "error_to_be_used" if there are any 

## StatusDefs.cpp
1.	Update the switch statment in "print_status_message" to include the new joint
	- Torque Sensor, Motor, Controller for both sides (left, right)

## StatusLED.h
1.	Update IdxRemap status_led_idx in the status_led_defs namespace with the new joint, follow the other joints for formatting
2. 	Remove the no longer valid "error_to_be_used_#" sections 

## Config.ini
1.	Copy the information for one joint setting (e.g., bilateralHipAnkle) and add paste it later on 
2. 	Rename the newly added formatting for your newly added joint
3. 	Update the contained information appropriatly

## SD Card
1. Make a jointControllers folder with the .csv for the controllers specific to the joint

## Python GUI - ActiveTrialSettings.py
1. Find the section titled "jointMap"
	- Add in the new joint name and incrament the int value in the section following the format of the other joints.
2. In the UpdateTorque Class, update the string list in "values" under the section labeled "Joint Select" with the joint name (e.g., Left elbow, right elbow)

## Python GUI - exoDeviceManager.py
1. Find "jointDictionary" under the ExoDeviceManager Class
	- Update the directory with the joint numbers you added in "ActiveTrialSettings" and associate with the Motor ID number you set the motor to via the RLink and in the software
	- Follow the format of the other joints (e.g., 1: 33. 0 = "Left Hip with an ID of 33")

## Done
It should now be good to go.