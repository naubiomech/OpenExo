Constant Torque Controller
==========================

Description
-----------
This controller attempts to apply a constant torque to the joint.  
It can be operated with or without closed-loop control.

Parameters
----------
Parameter index order can be found in :doc:`ControllerData <ExoCode/src/ControllerData.h>`.

- **amplitude**  
  Magnitude of the applied torque, in Nm.
- **direction**  
  Flag to flip the direction of the applied torque.
- **alpha**  
  Filtering term for the exponentially weighted moving average (EWMA) filter used on the torque sensor to reduce noise. Lower values result in increased delay.
- **use_pid**  
  Flag to turn PID on (1) or off (0).
- **p_gain**  
  Proportional gain for closed-loop control.
- **i_gain**  
  Integral gain for closed-loop control.
- **d_gain**  
  Derivative gain for closed-loop control.
