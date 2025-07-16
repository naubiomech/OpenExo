# Calibration Manager Controller

## Description
This controller applies a small, constant torque while printing statements of relevance (command value/sign, torque sensor value/sign, angle sensor value/sign) to the Arduino Serial Monitor. 

## Parameters
There are no user defined parameters associated with this controller. 

## Purposes
After you have built an exo, one of the first things to check is the motor directions and the signs (positive/negative) of any of your external sensors (torque, angle, etc.). 
Having correct motor directions is important as it allows the exo controller designers to focus on the control itself, knowing that, if calibrated properly before use, the exo assistance will always be in the designed directions. 
The same principle applies for the sensors involved with control. Since PID control is implemented in many of our exo controllers, it is crucial to make sure that the directions match. 
This article will elaborate how to conduct the direction calibration using the CalibrManager controller. 
On each exo, this calibration process only needs to be done **once**, its results will need to be **manually** saved on the SD card and will be loaded **automatically** every time the exo starts up.

## Motor directions
When designing a controller, the values returned from the "calc_motor_cmd()" function determines the motor torque command (feedforward). To ease the controller design process, we propose that controller designers follow the following direction definitions for torque commands:
- At both ankle joints: Positive for Dorsiflexion, and negative for Plantarflexion
- At both knee joints: Positive for Extension, and negative for Flexion
- At both hip joints: Positve for Flexion, and negative for Extension
- At both shoulder joints: Positive for Flexion, and negative for Extension
- At both elbow joints: Positive for Flexion, and negative for Extension

In summary, this follows the right-hand rule.

## Torque sensor signs (positive/negative)
Following the motor direction definitions, we need to verify that, when the motor supplies a positive torque command, the torque sensors, when zeroed properly, also returns a positive reading when the motor shaft is not spinning.

# How to calibrate
1. Set Calibration Manager as the default controller for the current exo. 
2. Connect the Teensy to a computer through USB and open up the Arduino Serial monitor.
3. Connect the exo to the python GUI.
4. Power on the exo and start a new session.
5. Make sure you are securly holding the joint of interest (either with your hand or worn on your body) so that it can not freely move. If left unsecured the joint will actuate upon start which could cause injury.
6. Upon start, the Calibration Manager will send a positive torque command (3.5 Nm by default) to each motor.
7. Feel the direction that the torque is trying to actuate. 
8. If the torque is not rotating in the positive direction (see above for the specific directions), power off the device, remove the SD Card, and flip the motor direction to its correct direction by modifying the appropriate "JointFlipMotorDir" in "Config.ini".
9. After modifying all motor directions (if needed), start the test over again [once again making sure the device is secured], and this time focus on the sensor readings (e.g., torque, angle) shown on the Arduino Serial Monitor. 
10. Again feel the direction that the torque is trying to actuate (it should now match the definitions above).
11. Now try to manually move the joint back to its neutral position (e.g., move the hip upright away from a flexed position into a neutral position). 
12. If the corresponding sensor does not return a postive value, you will need to modify the SD card to flip the sensor direction.
13. Again, power off the device, remove the SD card, and flip the sensor direction by modifying the approriate "JointFlipSensorDir" in "Config.ini". 