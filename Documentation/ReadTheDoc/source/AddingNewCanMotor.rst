Adding a New CAN Motor
======================

config.ini
----------
1. Go to the individual exoskeleton sections and add the name of your new motor in the **values comments** above the joints.
   Make sure you add it to *all* the exoskeleton configurations.

parseIni.h
----------
1. In the ``config_defs`` namespace, add a new enumeration value to the **motor enum**.
2. In the ``config_map`` namespace, add a case for your new motor in the motor mapping.  
   Use the **string** you added to ``config.ini``.

Motor.h
-------
1. Define a new **CAN motor** class that inherits ``_CANMotor`` (see ``class AK60`` as an example).
   - To define a motor class, copy one of the existing CAN motor classes at the bottom of the file and rename it.
   - Ensure the class name is **unique** and descriptive.

Motor.cpp
---------
1. Define the **constructor** for your new class.
2. Following the other CAN motor classes, define your ``_T_MAX`` and ``_V_MAX`` to match the motor’s maximum torque and velocity:
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
