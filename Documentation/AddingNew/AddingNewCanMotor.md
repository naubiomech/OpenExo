# Adding a new CAN motor
## config.ini
1. Go to the individual exoskeletons and add the name of the motor to the values comments above the joints.  Make sure to add it to all of the exoskeleton configurations.

## parseIni.h
1. Go to the config_defs namespace and add an enumeration to the "enum class motor : uint8_t" section.
2. Go to the config_map namespace and add a case for your new motor in the "const IniKeyCode motor" section.  The string is the value you added to config.ini.

## Motor.h
1. Define a new CAN motor class, these are the motors that inherit _CANMotor, see class AK60 as and example.
    - To define a motor class, copy and paste the other CAN motor classes at the bottom of the file and change the name. 
    - Make sure the class name is unique and descriptive.

## Motor.cpp
1. Define the constructor for your new class. Following the other CAN classes, define your _T_MAX and _V_MAX to reflect the motors maximal torque
  and velocity respectively. You can find these values on the motors datasheet. 
    - _T_MAX is the peak torque in Nm
    - _V_MAX is the Max Speed (rpm) converted to rad/s: value [rev/min] / (60 [s/min]) * (2 * PI [rad/rev])

## Joint.cpp
1. For each Joint constructor, add a case to the switch statement for your motor. 

## Done 
Modify the config.ini file to use your new motors!