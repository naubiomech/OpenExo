Step Controller
===============

Description
-----------
This controller attempts to apply a constant torque at the joint for a set duration and for a set number of times.

Parameters
----------
Parameter index order can be found in :doc:`ControllerData <ExoCode/src/ControllerData.h>`.

- **amplitude**  
  Magnitude of the applied torque, in Nm.
- **duration**  
  Duration for which the torque is applied.
- **repetitions**  
  Number of times the torque is applied.
- **spacing**  
  Time between each application of torque.
- **use_pid**  
  Flag to turn PID on (1) or off (0).
- **p_gain**  
  Proportional gain for closed-loop control.
- **i_gain**  
  Integral gain for closed-loop control.
- **d_gain**  
  Derivative gain for closed-loop control.
- **alpha**  
  Filtering term for the exponentially weighted moving average (EWMA) filter used on the torque sensor to reduce noise.
