# Step Controller

## Description
This controller attempts to apply a constant torque at the joint for a set duration and for a set number of times.

## Parameters
Parameter index order can be found in [ControllerData.h](/ExoCode/src/ControllerData.h).
- amplitude - Magnitude of the applied torque, in Nm
- duration - Duration of the applied torque
- repetitions - Number of times the torque is applied
- spacing - Time between each application of torque 
- pid_flag - This flag turns PID on(1) or off(0)
- p_gain - Proportional gain for closed loop control
- i_gain - Integral gain for closed loop control
- d_gain - Derivative gain for closed loop control
- alpha - Filtering term for exponentially wieghted moving average (EWMA) filter, used on torque sensor to cut down on noise.