# Elbow Min Max Controller

## Description
This controller applies a torque to the elbow based on FSRs placed on the hand. It is meant to be used for weighted lifting. 
More details on this controller can be found in:

D Colley, C. D. Bowersock, and Z. F. Lerner (2024). 
[A Lightweight Powered Elbow Exoskeleton for Manual Handling Tasks](https://ieeexplore.ieee.org/document/10684723)
in IEEE Transactions on Medical Robotics and Bionics, vol. 6, no. 4, pp. 1627-1636, doi: 10.1109/TMRB.2024.3464690.

## Parameters
Parameter index order can be found in [ControllerData.h](/ExoCode/src/ControllerData.h).
- FLEXamplitude - Torque setpoint, in Nm, for flexion assistance 
- DigitFSR_threshold - Upper threshold for finger (grip) FSRs
- PalmFSR_threshold - Upper threshold for palm FSR
- DigitFSR_LOWthreshold - Lower threshold for finger (grip) FSRs
- PalmFSR_LOWthreshold - Lower threshold for palm FSR
- CaliRequest - Flag to request calibration of sensors
- TrqProfile - Flag that toggles between torque states (1 = spring, 0 = constant)
- p_gain - Proportional gain for closed loop control
- i_gain - Integral gain for closed loop control
- d_gain - Derivative gain for closed loop control
- TorqueLimit - Torque saftey limit, in Nm; prevents controller from prescribing a torque larger than this value
- SpringPkTorque - Sets the maximum spring torque in Nm
- EXTamplitude - Extension torque setpoint in Nm
- FiltStrength - Sets the strength of the setpoint filter