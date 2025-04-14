Controllers
===========

Overview of the controller process.

.. toctree::
   :maxdepth: 2
   :caption: Joint-Specific Controllers:

   ankle
   elbow
   hip


Calibration Manager Controller
------------------------------
**Description**  
This controller applies a small, constant torque while printing statements of relevance (command value/sign, torque sensor value/sign, angle sensor value/sign) to the Arduino Serial Monitor.

**Parameters**  
There are no user-defined parameters associated with this controller.

**Purposes**  
After you have built an exo, one of the first things to check is the motor directions and the signs (positive/negative) of any of your external sensors (torque, angle, etc.). Having correct motor directions is important as it allows the exo controller designers to focus solely on the control logic—knowing that if the system is calibrated properly, the exo assistance will always be in the correct direction. The same principle applies to the sensors: since PID control is implemented in many of our controllers, matching the sign of sensor readings to motor commands is crucial.  
This article elaborates how to conduct the direction calibration using the Calibration Manager Controller. On each exo, this calibration process needs to be run **once**; its results are then **manually** saved on the SD card and loaded **automatically** at startup.

**Motor Directions**  
When designing a controller, the output of the `calc_motor_cmd()` function determines the motor torque command (feedforward). To simplify controller design, we propose the following directional conventions for torque commands:
- **Ankle joints:** Positive for Dorsiflexion, negative for Plantarflexion.
- **Knee joints:** Positive for Extension, negative for Flexion.
- **Hip joints:** Positive for Flexion, negative for Extension.
- **Shoulder joints:** Positive for Flexion, negative for Extension.
- **Elbow joints:** Positive for Flexion, negative for Extension.

This overall approach follows the **right-hand rule**.

**Torque Sensor Signs (Positive/Negative)**  
Ensure that when the motor outputs a positive torque command, the torque sensors—when zeroed properly—also return a positive value when the motor shaft is stationary.

**How to Calibrate**  
1. Set the Direction Calibration Manager as the default controller for the current exo. Connect the Teensy via USB, open the Arduino Serial Monitor, connect the exo (Arduino Nano) with the Python app, power on the exo, and start a new session. The Calibration Manager will send a positive torque command (3.5 Nm by default) to each motor.
2. Verify the torque by feeling the force (e.g., via the foot plates on an ankle exo). If a motor (e.g., on the left side) does not produce the expected positive rotation, modify the `config.ini` on the SD card to flip the motor direction for that side.
3. Restart the process and, this time, focus on sensor readings displayed on the Serial Monitor. Manually return the foot plate (or another joint element) to its neutral position (for example, 0°). If the sensor does not report a positive value, adjust the sensor direction in the `config.ini`. Repeat this calibration for all relevant sensors.

Chirp Controller
----------------
**Description**  
This controller applies a user-defined sine wave that increases in frequency at a set rate. It is designed to help characterize the system's bandwidth.

**Parameters**  
Parameter index order can be found in :doc:`ControllerData <ExoCode/src/ControllerData.h>`.
- **amplitude**: Amplitude (in Nm) of the torque sine wave.
- **start_frequency**: Starting frequency for the chirp.
- **end_frequency**: Ending frequency for the chirp.
- **duration**: How long the chirp is applied.
- **yshift**: Shifts the center of the chirp if it is to be non-zero.
- **use_pid**: Flag to enable (1) or disable (0) PID control.
- **p_gain**: Proportional gain for closed-loop control.
- **i_gain**: Integral gain for closed-loop control.
- **d_gain**: Derivative gain for closed-loop control.

Constant Torque Controller
--------------------------
**Description**  
This controller attempts to apply a constant torque at the joint. It can be operated with or without closed-loop control.

**Parameters**  
Parameter index order can be found in :doc:`ControllerData <ExoCode/src/ControllerData.h>`.
- **amplitude**: Magnitude of the applied torque (in Nm).
- **direction**: Flag to flip the direction of the applied torque.
- **alpha**: Filtering term for the exponentially weighted moving average (EWMA) filter applied to the torque sensor to reduce noise. Lower values increase delay.
- **use_pid**: Flag to enable (1) or disable (0) PID control.
- **p_gain**: Proportional gain.
- **i_gain**: Integral gain.
- **d_gain**: Derivative gain.

Step Controller
---------------
**Description**  
This controller applies a constant torque at the joint repeatedly for a set duration and number of times.

**Parameters**  
Parameter index order can be found in :doc:`ControllerData <ExoCode/src/ControllerData.h>`.
- **amplitude**: Magnitude of the applied torque (in Nm).
- **duration**: Duration of each torque application.
- **repetitions**: Number of times the torque is applied.
- **spacing**: Time between torque applications.
- **use_pid**: Flag to enable (1) or disable (0) PID control.
- **p_gain**: Proportional gain.
- **i_gain**: Integral gain.
- **d_gain**: Derivative gain.
- **alpha**: Filtering term for the EWMA filter.

Zero Torque Controller
----------------------
**Description**  
This controller attempts to regulate the joint so that the output torque is zero. It can be operated with or without closed-loop control.

**Parameters**  
Parameter index order can be found in :doc:`ControllerData <ExoCode/src/ControllerData.h>`.
- **use_pid**: Flag to enable (1) or disable (0) PID control.
- **p_gain**: Proportional gain.
- **i_gain**: Integral gain.
- **d_gain**: Derivative gain.

Controller Subfolders by Joint
------------------------------
For implementation details specific to individual joints, refer to the subfolders for:
- **Ankle Controllers**
- **Elbow Controllers**
- **Hip Controllers**

The subfolders (each with its own ``index.rst`` file) are linked above in the toctree. They provide additional documentation and examples for configuring and tuning controllers tailored to the specific dynamics of each joint.
