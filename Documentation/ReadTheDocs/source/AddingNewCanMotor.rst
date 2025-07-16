.. _AddingNewCanMotor:

Adding a New CAN Motor
======================

config.ini
----------
1. Go to the individual exoskeleton sections and add the name of your new motor in the **values comments** above the joints.
   Make sure you add it to *all* the exoskeleton configurations.

parseIni.h
----------
1. In the ``config_defs`` namespace, add a new enumeration value to the **enum class motor : uint8_t** section.
2. In the ``config_map`` namespace, add a case for your new motor in the **const IniKeyCode motor** section.  
   Use the ** same string** you added to ``config.ini``.

Motor.h
-------
1. Define a new **CAN motor** class that inherits ``_CANMotor`` (see ``class AK60`` as an example).

   - To define a motor class, copy one of the existing CAN motor classes at the bottom of the file and rename it.
   - Ensure the class name is **unique** and descriptive.

Motor.cpp
---------
1. Define the **constructor** for your new class. Follow the general format fo the other motor classes, such as:

.. code-block:: c++

   AK60::AK60(config_defs::joint_id id, ExoData* exo_data, int enable_pin): //Constructor: type is the motor type
   _CANMotor(id, exo_data, enable_pin)
   {
       _I_MAX = 22.0f;
       _V_MAX = 41.87f;
    
       float kt = 0.068 * 6;
       set_Kt(kt);
       exo_data->get_joint_with(static_cast<uint8_t>(id))->motor.kt = kt;

       #ifdef MOTOR_DEBUG
           logger::println("AK60::AK60 : Leaving Constructor");
       #endif
   };


2. Following the other CAN motor classes, define your ``_T_MAX`` and ``_V_MAX`` (found on motor's datasheet) to match the motor’s maximum torque and velocity:

   - ``_T_MAX`` is the peak torque in Nm.
   - ``_V_MAX`` is the max speed (rpm) converted to rad/s:

     .. code-block:: none

        [rev/min] / 60 [s/min] * (2 * PI) [rad/rev]

Joint.cpp
---------
1. In each **Joint** constructor, add a case to the ``switch`` statement to handle your new motor.

Done
----
Update the ``config.ini`` file to use your new motors, and you're all set!
