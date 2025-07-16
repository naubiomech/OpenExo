# Franks Collins Hip Controller

## Description
Provides Hip assistance based on a series of splines that have an extension and flexion portion.
Uses percent gait, requiring heel FSR.
Based on: 

P.W. Franks, G.M. Bryan, R.M. Martin, R. Reyes, A.C. Lakmazaheri, & S.H. Collins (2021). 
[Comparing optimized exoskeleton assistance of the hip, knee, and ankle in single and multi-joint configurations](https://www.cambridge.org/core/journals/wearable-technologies/article/comparing-optimized-exoskeleton-assistance-of-the-hip-knee-and-ankle-in-single-and-multijoint-configurations/9FBC1580F11614B388BE621D716800AD). 
in Wearable Technologies, vol. 2, pp. e16, doi: 10.1017/wtc.2021.14.

## Parameters
Parameter index order can be found in [ControllerData.h](/ExoCode/src/ControllerData.h).
- mass - User mass in kg used for denormalizing torque, currently not used in the controller but easy to implement if desired, defaults to zero as a result. 
- trough_normalized_torque_Nm_kg - Largest extension torque (Nm). Note: Currently not noramlized by user mass.
- peak_normalized_torque_Nm_kg - Largest flexion torque (Nm). Note: Currently not noramlized by user mass.
- start_percent_gait - Percent gait to start the torque pattern so it doesn't have a discontinuity at heel strike.
- trough_onset_percent_gait - Percent gait to start applying extension torque.
- trough_percent_gait - Percent gait to apply the largest extension torque.
- mid_time - Transition point between extension and flexion to apply 0 torque.
- mid_duration - The duration of the transition pause as a percent of the gait cycle.
- peak_percent_gait - Percent gait to apply the largest flexion torque.
- peak_offset_percent_gait - Percent gait to stop applying flexion torque.
- use_pid - This flag turns PID on(1) or off(0)
- p_gain - Proportional gain for closed loop control
- i_gain - Integral gain for closed loop control
- d_gain - Derivative gain for closed loop control