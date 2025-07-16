Proportional Hip Moment Controller
==================================

Description
-----------
Provides Hip assistance based on real-time estimate of the user's hip joint moment
Requires both toe and heel FSRs.
NOTE: THIS CONTROLLER IS STILL UNDER DEVELOPMENT (Stay tuned for future updates). 

Based on: 

S.S.P.A. Bishe, L. Liebelt, Y. Fang, & Z.F. Lerner (2022).
`A Low-Profile Hip Exoskeleton for Pathological Gait Assistance <https://ieeexplore.ieee.org/document/9812300>`__. 
2022 IEEE ICRA. 

Parameters
----------

Parameter index order can be found in `ControllerData.h <https://github.com/naubiomech/OpenExo/blob/main/ExoCode/src/ControllerData.h>`__. 

- **extension_setpoint**: Magnitude of peak extension torque (Nm)
- **flexion_setpoint**: Magnitude of peak flexion torque (Nm)

