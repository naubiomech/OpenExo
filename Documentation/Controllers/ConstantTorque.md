# Constant Torque Controller

## Description
This controller attempts to apply a constant torque to the joint.  
This can be done with or without closed loop control.

## Parameters
Parameter index order can be found in [ControllerData.h](/ExoCode/src/ControllerData.h).
- amplitude - Magnitude of the applied torque, in Nm
- direction - Flag to flip the direction of the applied torque
- alpha - Filtering term for exponentially wieghted moving average (EWMA) filter, used on torque sensor to cut down on noise. The lower this value the higher the delay caused by the filtering. 
- use_pid - This flag turns PID on(1) or off(0)
- p_gain - Proportional gain for closed loop control
- i_gain - Integral gain for closed loop control
- d_gain - Derivative gain for closed loop control