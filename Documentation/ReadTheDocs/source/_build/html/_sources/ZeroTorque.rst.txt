Zero Torque Controller
======================

Description
-----------
This controller attempts to control the joint so that the output torque is zero. It can be operated with or without closed-loop control.

Parameters
----------
Parameter index order can be found in `ControllerData.h <https://github.com/naubiomech/OpenExo/blob/main/ExoCode/src/ControllerData.h>`_.

- **use_pid**  
  This flag turns PID on (1) or off (0).
- **p_gain**  
  Proportional gain for closed-loop control.
- **i_gain**  
  Integral gain for closed-loop control.
- **d_gain**  
  Derivative gain for closed-loop control.
