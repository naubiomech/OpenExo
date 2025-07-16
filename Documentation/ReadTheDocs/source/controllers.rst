Controllers
===========

Overview of Controllers. Here we present the controllers capable of use by multiple joints.

.. toctree::
   :maxdepth: 2
   :caption: Joint-Specific Controllers:

   ankle
   hip
   elbow


Calibration Manager Controller
------------------------------
**Description**  
This controller applies a small, constant torque while printing statements of relevance (command value/sign, torque sensor value/sign, angle sensor value/sign) to the Arduino Serial Monitor. 

**Parameters**  
There are no user-defined parameters associated with this controller.

**Purposes**  
After you have built an exo, one of the first things to check is the motor directions and the signs (positive/negative) of any of your external sensors (torque, angle, etc.). 
Having correct motor directions is important as it allows the exo controller designers to focus on the control itself, knowing that, if calibrated properly before use, the exo assistance will always be in the designed directions.   
The same principle applies for the sensors involved with control. Since PID control is implemented in many of our exo controllers, it is **crucial** to make sure that the directions match.
This article will elaborate how to conduct the direction calibration using the CalibrManager controller.  
On each exo, this calibration process only needs to be done **once**, its results will need to be **manually** saved on the SD card and will be loaded **automatically** every time the exo starts up.

**Motor Directions**  
When designing a controller, the values returned from the "calc_motor_cmd()" function determines the motor torque command (feedforward). To ease the controller design process, we propose that controller designers follow the following direction definitions for torque commands:

- **Ankle Joints:** Positive for Dorsiflexion, and negative for Plantarflexion
- **Knee Joints:** Positive for Extension, and negative for Flexion
- **Hip Joints:** Positve for Flexion, and negative for Extension
- **Elbow joints:** Positive for Flexion, and negative for Extension

In summary, this **follows the right-hand rule**.

**Torque Sensor Signs (Positive/Negative)**  
Following the motor direction definitions, we need to verify that, when the motor supplies a positive torque command, the torque sensors, when zeroed properly, also returns a positive reading when the motor shaft is not spinning.

**How to Calibrate**  

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

Chirp Controller
----------------
**Description**  
This controller applies a user defined sine wave that increases in frequency at a set rate. This can be used to characterize the bandwidth of the system. 

**Parameters**  
Parameter index order can be found in `ControllerData.h <https://github.com/naubiomech/OpenExo/blob/main/ExoCode/src/ControllerData.h>`_.

- **amplitude**: Amplitude, in Nm, of the torque sine wave.
- **start_frequency**: Starting frequency for the chirp
- **end_frequency**: Ending frequency for the chirp
- **duration**: The duration that you want the chirp to be applied
- **yshift**: Shifts the center of the chirp if you want it to be something other than zero
- **use_pid**: This flag turns PID on(1) or off(0)
- **p_gain**: Proportional gain for closed loop control
- **i_gain**: Integral gain for closed loop control
- **d_gain**: Derivative gain for closed loop control

Constant Torque Controller
--------------------------
**Description**  
This controller attempts to apply a constant torque to the joint. This can be done with or without closed loop control.

**Parameters**  
Parameter index order can be found in `ControllerData.h <https://github.com/naubiomech/OpenExo/blob/main/ExoCode/src/ControllerData.h>`_.

- **amplitude**: Magnitude of the applied torque, in Nm
- **direction**: Flag to flip the direction of the applied torque
- **alpha**: Filtering term for exponentially wieghted moving average (EWMA) filter, used on torque sensor to cut down on noise. The lower this value the higher the delay caused by the filtering. 
- **use_pid**: This flag turns PID on(1) or off(0)
- **p_gain**: Proportional gain for closed loop control
- **i_gain**: Integral gain for closed loop control
- **d_gain**: Derivative gain for closed loop control

Step Controller
---------------
**Description**  
This controller attempts to apply a constant torque at the joint for a set duration and for a set number of times.

**Parameters**  
Parameter index order can be found in `ControllerData.h <https://github.com/naubiomech/OpenExo/blob/main/ExoCode/src/ControllerData.h>`_.

- **amplitude**: Magnitude of the applied torque, in Nm
- **duration**: Duration of the applied torque
- **repetitions**: Number of times the torque is applied
- **spacing**: Time between each application of torque
- **use_pid**: This flag turns PID on(1) or off(0)
- **p_gain**: Proportional gain for closed loop control
- **i_gain**: Integral gain for closed loop control
- **d_gain**: Derivative gain for closed loop control
- **alpha**: Filtering term for exponentially wieghted moving average (EWMA) filter, used on torque sensor to cut down on noise.

Zero Torque Controller
----------------------
**Description**  
This controller attempts to regulate the joint so that the output torque is zero. It can be operated with or without closed-loop control.

**Parameters**  
Parameter index order can be found in `ControllerData.h <https://github.com/naubiomech/OpenExo/blob/main/ExoCode/src/ControllerData.h>`_.

- **use_pid**: This flag turns PID on(1) or off(0)
- **p_gain**: Proportional gain for closed loop control
- **i_gain**: Integral gain for closed loop control
- **d_gain**: Derivative gain for closed loop control

Controller Subfolders by Joint
------------------------------
For implementation details specific to individual joints, refer to the subfolders for:

- **Ankle Controllers**
- **Hip Controllers**
- **Elbow Controllers**

The subfolders (each with its own ``index.rst`` file) are linked above in the toctree. They provide additional documentation and examples for configuring and tuning controllers tailored to the specific dynamics of each joint.
