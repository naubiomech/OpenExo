Adding a New Joint
==================

Exocode.ini
-----------
1. **Line 118:**  
   Add the new joint to the debug statement that determines if the joint is used.
2. **Line 148:**  
   Add the new joint to the if statements that decide whether or not the joint is used.
   - Copy the if statements for the left and right sides from an existing joint (e.g., hip) and paste them below the last joint.
   - Modify the joint’s name to match your new joint.
3. **Line 398:**  
   Add code for calibrating the torque sensor and enabling motors for the new joint while in headless mode.
   - Copy the formatting from another joint—ensure you do this for both the left and right sides.
4. **Line 540:**  
   Update the section that sets the controller for each joint (while in headless mode).
   - Copy the formatting for one joint and rename it to your new joint.
   - Pull in the default controller information from the config and set the controller based on this configuration.

ParseIni.h
----------
1. Update **exo_name** in the `config_defs` namespace—ensure this encompasses any configuration for the new joint (e.g., bilateral, unilateral, multi-joint).
2. Update **JointType** in the `config_defs` namespace.
3. Update **joint_id** to utilize the proper byte for the new joint (refer to the comment in the code).
   - Also update the left and right joint information following the established format.
4. Add a new enum class for the joint's controllers following the format of existing joints.
   - Include any controllers associated with the joint (you can update further later via "AddNewController").
5. Update the static const int variables at the end of the `config_defs` namespace to account for the new joint.
6. In the `config_map` namespace, update the IniKeyCode for **exo_name** to include all configurations from the `exo_name` enum in `config_defs`.
7. In the `config_map` namespace, add a new IniKeyCode for the joint's controllers following the format used by the other joints.
   - Ensure the terminology in the section (after `config_defs::`) is updated accordingly.
8. Update the `ConfigData` struct at the end to include fields for the new joint.

ParseIni.cpp
------------
1. From line 174 onward, copy the template from each section to add the new joint so that it is formatted identically to the other joints.

ExoData.h
---------
1. In the function `for_each_joint`, add the new joint following the formatting of the existing joints.

ExoData.cpp
-----------
1. In the function `get_used_joints` (line 36), add the new joint following the format of the existing joints—make sure to include both the left and right sides.
2. In the function `get_joint_with` (line 59), add the new joint following the established format for both left and right sides.
3. In the function `print`, add print statements for the new joint (for each side) following the formatting used for the other joints.

UART_commands.h
---------------
1. In the function `get_config` (line 196), update the mapping with the new `config_defs` values you added in ParseIni.h.  
   - Refer to the added static const int values at the end of the `config_defs` namespace.
2. Similarly, update the function `update_config` (line 227) with the new `config_defs` values.
3. In the function `get_real_time_data` (line 306), add a new case to the switch statement corresponding to the bilateral configuration of the new joint and for any multi-joint instances.
   - You can send up to 10 data points at a time, following the established formatting.

Side.h
------
1. Create a Joint object specific to the new joint (refer to line 158 where other joint objects are specified; follow their formatting).

Side.cpp
--------
1. In the Side constructor, add the new joint object (created in the header) following the format of the other joints.
2. In the function `disable_motors` (line 58), add the new joint following the existing formatting.
3. In the function `read_data` (line 98), add an if statement at the end (around line 142) to read data specific to the new joint object (use other joints as templates).
4. In the function `check_calibration` (line 161), add an if statement at the end (around line 189) to check calibration for the new joint (follow the established formatting).
5. In the function `update_motor_cmds` (line 534), add an if statement at the end that executes functions for the new joint (use the formatting from other joints).

SideData.h
----------
1. At line 42, create a new instance of JointData and add an object for the new joint, following the format of existing joints.

SideData.cpp
------------
1. In the SideData constructor, add the new joint object (as defined in the header) following the established format.
2. On the last line, add a statement to reconfigure the new joint—model this on the formatting for other joints.

Joint.h
-------
1. Create a new class for the joint; ensure it inherits from the public `_joint` class.
   - Include a joint-specific constructor, destructor, and the two essential functions: `run_joint` and `set_controller` in the public section.
   - In the private section, declare instances of the controller class specific to this joint and create new objects as needed.
   - Essentially, copy the structure used for the other joints and change the names and controllers accordingly.

Joint.cpp
---------
1. In the `_Joint` constructor (around line 40), add the new joint to the Joint Debug statement.
2. In the function `get_torque_sensor_pin`, add a new case to the switch statement for your added joint, following the existing format.
3. In the function `get_motor_enable_pin`, add a new case to the switch statement for the new joint.
4. Add the new joint class following the format of the existing joints:
   - Include all controller instances in the initializer list along with the `_Joint` instance.
   - Update debug statements to reflect the new joint.
   - Update the switch case that creates an object for the specific motor type.
   - Update the call to `set_controller` at the end of the constructor.
   - In the function `run_joint`, update the debug statements (the remaining code is non-joint specific).
   - In `set_controller`, update the debug statement and the switch case for the controller ID.

JointData.cpp
-------------
1. Update the switch statement in the JointData initializer (line 31) to include the new joint.
   - Include the segment that sets the value for `is_used` and the if statement that flips the direction.
   - Follow the format used for other joints.
2. Repeat the process for the `reconfigure` function.

Motor.cpp
---------
1. Update the switch statement in the motor initializer (line 39) to include the new joint.

MotorData.cpp
-------------
1. Update the switch statement in the initializer (line 16) to incorporate the new joint.
   - Include both the gearing and the flip direction portions.
2. Repeat the process for the `reconfigure` function.

ControllerData.cpp
------------------
1. Update the switch statement in the constructor (line 16) to include the new joint.
2. Repeat the process for the `reconfigure` function (line 56).

Controller.cpp
--------------
1. Update the switch statement in the constructor of `_Controller` to incorporate the new joint (line 35).

ble_commands.h
--------------
1. In the function `new_trq`, update the joint_id mapping to include the new joint (ensure you update both left and right sides).
   - Adjust the joint id number to maintain a logical order (e.g., left_hip = 1, left_knee = 2, left_ankle = 3, left_elbow = 4, ...).
2. Update the if statement that associates controller data with the joint ID.

ParamsfromSD.h
--------------
1. In the `controller_parameter_filenames` namespace, add a new `ParamFilenameKey` for the new joint and associate it with the appropriate controllers.

ParamsfromSD.cpp
----------------
1. In the function `print_param_error_message`, update the switch statement to include the new joint.
   - Update the entire switch statement (around line 10) and add a case for the new joint.
2. In the function `set_controller_params`:
   - Copy a case from another joint (paste it below the last joint) and update any references to the specific joint (e.g., elbow, ankle, etc.) to your new joint.
   - Make sure no references to the existing joints are missed in this section.

RealTimeI2C.h
-------------
1. In the `rt_data` namespace, update the static int `RT_LEN` to include the new configurations added in `get_real_time_data` from `uart_commands.h`.

StatusDefs.h
------------
1. In the `messages` namespace, update the error messages to include the new joint:
   - Update for Torque Sensor, Motor, and Controller messages for both sides (left and right).
   - Make sure to update the numbering appropriately.
2. Update any remaining "error_to_be_used" nomenclature if necessary.

StatusDefs.cpp
--------------
1. Update the switch statement in `print_status_message` to include the new joint.
   - Include cases for Torque Sensor, Motor, and Controller for both sides.

StatusLED.h
-----------
1. Update the `IdxRemap status_led_idx` in the `status_led_defs` namespace to include the new joint, following the format of the other joints.
2. Remove any now invalid "error_to_be_used_#" sections.

Config.ini
----------
1. Copy the settings for one existing joint (e.g., `[bilateralHipAnkle]`) and paste them later in the file.
2. Rename the newly copied section to reflect your new joint.
3. Update the contents accordingly.

SD Card
-------
1. Create a folder named `jointControllers` and include the CSV files for the controllers specific to the new joint.

Python GUI - ActiveTrialSettings.py
-------------------------------------
1. Find the section titled **"jointMap"**.
   - Add the new joint name and increment the integer value following the format of the other joints.
2. In the `UpdateTorque` class, update the string list in the **"Joint Select"** section to include the new joint name (e.g., Left elbow, Right elbow).

Python GUI - exoDeviceManager.py
--------------------------------
1. Locate **"jointDictionary"** under the `ExoDeviceManager` class.
   - Update the dictionary with the new joint numbers you added in `ActiveTrialSettings.py` and associate them with the Motor ID number set via RLink and in the software.
   - Follow the format of the other joints (e.g., `1: 33, 0 = "Left Hip"` with an ID of 33).

Done
----
It should now be good to go.
