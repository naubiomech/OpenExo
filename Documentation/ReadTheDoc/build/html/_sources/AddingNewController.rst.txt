Adding a New Controller
=======================

This section provides instructions on how to add a new controller to the platform.

parseIni.h
----------
1. In the ``config_defs`` namespace for the joint(s) you are creating a controller for, add the new controller to the enum class so it can be referenced.
2. In the ``config_map`` namespace, add an entry for the joint(s) you are creating a controller for with the name that will be used in the ``config.ini`` file.

ControllerData.h (Controller Definition)
------------------------------------------
1. In the ``controller_defs`` namespace, add a namespace for your controller following the template from the existing controllers:
   - Create **idx** values that will be used to access the different parameters (remember, indexing starts at 0).
   - Add a **num_parameter** value that stores the number of parameters the controller needs.
   - If this number is larger than the current ``max_parameters`` in ``controller_defs``, update ``max_parameters`` accordingly.

Controller.h
------------
1. In ``Controller.h``, create a new controller that inherits from the ``_Controller`` class.
   - This new controller must include:
     - A **constructor** that takes in the joint ID and an ``exo_data`` pointer.
     - A **destructor**.
     - A ``calc_motor_cmd()`` function that returns the torque command in mNm.
   - Additional private functions or variables can be added as needed, but note that external calls will only invoke these three required functions.

ControllerData.h (Optional Additional Data)
---------------------------------------------
1. (Optional) At the bottom of ``ControllerData.h``, you can add any extra variables for your controller that you would like to plot. These would serve as replacements or supplements to the variables defined in ``Controller.h``.
   - To assign values to these parameters in ``Controller.cpp``, use the following formatting:

     .. code-block:: c++

        controller_data->NAMEOFVARIABLE = VALUE;

Controller.cpp
--------------
1. Define all the functions for your new controller.
   - When calling the constructor, make sure to use an initializer list for the base class ``_Controller``.

Joint.h
-------
1. Within the header file for the joint (hip, knee, ankle) where the controller applies, add an object instance of your new controller.

Joint.cpp
---------
1. For each joint that your controller is valid for:
   1. Add the constructor for the new controller object in the initializer list.
   2. In the ``set_controller()`` function, add a new case to the switch statement:
      - Reference the joint controller namespace in the ``config_defs`` namespace.
      - Use one of the existing controllers as a template.
      - This case will determine which controller object the ``_controller`` pointer uses.
      - **Remember:** End the case with a ``break;`` statement.
   - *Note:* The switch statement references the ``config_defs`` name, and the value passed to ``_controller`` should reference the object created in ``Joint.h``.

Create Parameter File
---------------------
1. In the SDCard folder corresponding to the joint your controller is for, create a new parameter CSV file. It is recommended to copy one of the existing files as a starting template.
2. The first cell in the first line should indicate the number of header lines. Parameter reading will begin after this number of lines.
3. The first cell in the second line should state the number of parameters to read; this must match the value specified in ``ControllerData.h``.
4. After the header, list the parameter sets you wish to use, one set per line. The parameters should be listed in the order defined by ``ControllerData.h``. The first set will be used as the default.
5. If the controller applies to multiple joints, copy the file into the corresponding joint folders.

ParamsFromSD.h
--------------
1. Within the ``controller_parameter_filenames`` namespace, navigate to the appropriate joint.
2. Link the controller ID from ``config_defs`` with the path on the SD card for the parameter file you just created.

uart_commands.h
---------------
1. (Optional) If you want to plot or save one of the variables defined in ``ControllerData.h``, proceed as follows in ``get_real_time_data()``:
   - Find the case corresponding to the joint where your controller is used (e.g., ``bilateral_hip``).
   - Assign the variable to one of the data spots in the ``rx_msg`` structure (e.g., ``rx_msg.data[0]``).
     - For controller-specific variables, format the assignment like this:

       .. code-block:: c++

          rx_msg.data[#] = exo_data->NAME_side.JOINT.controller.VARIABLENAME;

         where:
         - ``NAME_side`` should be either ``left_side`` or ``right_side``,
         - ``JOINT`` is the joint you are working with, and
         - ``VARIABLENAME`` is the variable to plot.
     - To plot variables specific to a side (defined in ``SideData.h``), you may use:

       .. code-block:: c++

          rx_msg.data[#] = exo_data->NAME_side.VARIABLENAME;

Done
----
Your new controller should now be integrated and ready to use!  
Remember to change the active controller and update the parameters as necessary.
