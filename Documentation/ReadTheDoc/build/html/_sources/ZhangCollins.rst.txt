Zhang Collins Controller
=========================

Description
-----------
This controller applies a spline defined as a percent of gait. It requires a heel FSR for proper operation. The torque profile implemented by this controller is based on the following publication:

Zhang, J., Fiers, P., Witte, K. A., Jackson, R. W., Poggensee, K. L., Atkeson, C. G., & Collins, S. H. (2017).  
[Human-in-the-loop optimization of exoskeleton assistance during walking.](https://www.science.org/doi/full/10.1126/science.aal5054)  
*Science, 356(6344), 1280-1284.*

Parameters
----------
Parameter index order can be found in :doc:`ControllerData <ExoCode/src/ControllerData.h>`.

- **torque**  
  Magnitude of the peak torque in Nm.
- **peak_time**  
  Time when the peak torque occurs (expressed as a percentage of the gait cycle).
- **rise_time**  
  Duration from zero torque to peak torque (expressed as a percentage of the gait cycle).
- **fall_time**  
  Duration from peak torque back to zero torque (expressed as a percentage of the gait cycle).
- **direction**  
  Flag to flip the torque from plantarflexion (PF) to dorsiflexion (DF); essentially an assistance/resistance flag.
- **use_pid**  
  Flag to enable PID control (1 = on, 0 = off).
- **p_gain**  
  Proportional gain for closed-loop control.
- **i_gain**  
  Integral gain for closed-loop control.
- **d_gain**  
  Derivative gain for closed-loop control.
