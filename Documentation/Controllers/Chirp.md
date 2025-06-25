# Chirp Controller

## Description
This controller applies a user defined sine wave that increases in frequency at a set rate. This can be used to characterize the bandwidth of the system. 

## Parameters
Parameter index order can be found in [ControllerData.h](/ExoCode/src/ControllerData.h).
- amplitude - Amplitude, in Nm, of the torque sine wave
- start_frequency - Starting frequency for the chirp
- end_frequency - Ending frequency for the chirp
- duration - The duration that you want the chirp to be applied
- yshift - Shifts the center of the chirp if you want it to be something other than zero
- pid_flag - This flag turns PID on(1) or off(0)
- p_gain - Proportional gain for closed loop controls
- i_gain - Integral gain for closed loop controls
- d_gain - Derivative gain for closed loop controls