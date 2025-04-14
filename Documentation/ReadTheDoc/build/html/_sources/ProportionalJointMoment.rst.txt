Proportional Joint Moment Controller
======================================

Description
-----------
This controller applies a torque to the ankle proportional to the normalized FSR reading of the toe.
More details on this controller can be found in:

G. M. Gasparri, J. Luque and Z. F. Lerner,  
"[Proportional Joint-Moment Control for Instantaneously Adaptive Ankle Exoskeleton Assistance](https://ieeexplore.ieee.org/abstract/document/8669971),"  
in IEEE Transactions on Neural Systems and Rehabilitation Engineering, vol. 27, no. 4, pp. 751-759, April 2019, doi: 10.1109/TNSRE.2019.2905979.

Parameters
----------
Parameter index order can be found in :doc:`ControllerData <ExoCode/src/ControllerData.h>`.

- **stance_max**  
  The peak torque (in Nm) during stance. When the normalized FSR is 1, this will be the torque applied to the ankle.
- **swing_max**  
  The constant torque (in Nm) during swing. This value is rarely used.
- **is_assitance**  
  When set to 1 (assistive), the system applies the torque in the plantar flexion direction; when set to 0 (resistive), the torque is applied in the dorsiflexion direction.
- **use_pid**  
  This flag turns PID control on (1) or off (0).
- **p_gain**  
  Proportional gain for closed-loop control.
- **i_gain**  
  Integral gain for closed-loop control.
- **d_gain**  
  Derivative gain for closed-loop control.
- **torque_alpha**  
  Filtering term for the exponentially weighted moving average (EWMA) filter used on the torque sensor to reduce noise. Lower values increase the delay due to filtering.
