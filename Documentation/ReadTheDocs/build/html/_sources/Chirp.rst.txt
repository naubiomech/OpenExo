Chirp Controller
================

Description
-----------
This controller applies a user-defined sine wave that increases in frequency at a set rate. It can be used to characterize the bandwidth of the system.

Parameters
----------
Parameter index order can be found in `ControllerData.h <https://github.com/naubiomech/OpenExo/blob/main/ExoCode/src/ControllerData.h>`_.

- **amplitude**  
  Amplitude, in Nm, of the torque sine wave.
- **start_frequency**  
  Starting frequency for the chirp.
- **end_frequency**  
  Ending frequency for the chirp.
- **duration**  
  The duration for which the chirp is applied.
- **yshift**  
  Shifts the center of the chirp if a value other than zero is desired.
- **use_pid**  
  This flag turns PID control on (1) or off (0).
- **p_gain**  
  Proportional gain for closed-loop control.
- **i_gain**  
  Integral gain for closed-loop control.
- **d_gain**  
  Derivative gain for closed-loop control.
