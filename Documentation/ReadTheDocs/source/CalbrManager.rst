Calibration Manager Controller
==============================

Description
-----------
This controller applies a small, constant torque while printing statements of relevance (command value/sign, torque sensor value/sign, angle sensor value/sign) to the Arduino Serial Monitor.

Parameters
----------
There are no user-defined parameters associated with this controller.

Purposes
--------
After you have built an exo, one of the first things to check is the motor directions and the signs (positive/negative) of any external sensors (such as torque or angle sensors). Having correct motor directions is important because it allows the exo controller designers to concentrate solely on control logic—knowing that if the system is calibrated properly before use, the exo assistance will always operate in the intended directions. The same principle applies to the sensors involved in control; since PID control is implemented in many of our controllers, it is crucial that sensor readings match the motor command signs.

This document explains how to perform the calibration using the Calibration Manager Controller. Note that on each exo, this calibration process needs to be done **once**; its results must then be **manually** saved to the SD card and will be loaded **automatically** each time the exo starts up.

Motor Directions
----------------
When designing a controller, the values returned by the ``calc_motor_cmd()`` function determine the motor torque command (feedforward). To simplify the controller design process, we propose that designers adhere to the following directional conventions for torque commands:

- **Ankle joints:**  
  Positive for Dorsiflexion, negative for Plantarflexion.
- **Knee joints:**  
  Positive for Extension, negative for Flexion.
- **Hip joints:**  
  Positive for Flexion, negative for Extension.
- **Shoulder joints:**  
  Positive for Flexion, negative for Extension.
- **Elbow joints:**  
  Positive for Flexion, negative for Extension.

In summary, this approach follows the **right-hand rule**.

Torque Sensor Signs (Positive/Negative)
-----------------------------------------
Based on the motor direction definitions above, verify that when the motor supplies a positive torque command, the torque sensors (when properly zeroed) also return a positive value when the motor shaft is not spinning.

How to Calibrate
----------------
1. **Set Up the Controller:**  
   Set the Direction Calibration Manager as the default controller for the current exo. Connect the Teensy to a computer via USB and open the Arduino Serial Monitor. Also, connect the exo (Arduino Nano) with the Python application. Power on the exo and start a new session. The Calibration Manager will send a positive torque command (3.5 Nm by default) to each motor.

2. **Check Motor Direction:**  
   Feel each torque—e.g., by using the foot plates on an ankle exo. If the torque applied (for example, to the left foot plate) does not cause it to rotate in the expected positive direction (refer to the directional definitions above), flip the motor direction for that side. Flipping the motor direction is achieved by modifying the ``config.ini`` file on the SD card.

3. **Check Sensor Readings:**  
   After making any necessary changes to motor directions, restart the calibration process. This time, focus on the sensor readings (e.g., torque, angle) displayed on the Arduino Serial Monitor. For example, feel the motor torque on an ankle exo's foot plate, then manually move the foot plate back to its neutral position (e.g., 0°). If the corresponding sensor does not return a positive value, flip the sensor direction by modifying the ``config.ini`` on the microSD card. Repeat this process for all torque sensors as needed.
