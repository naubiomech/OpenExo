Elbow Min Max Controller
========================

Description
-----------
This controller applies a torque to the elbow based on FSRs placed on the hand. It is meant to be used for weighted lifting. More details on this controller can be found in:

D Colley, C. D. Bowersock, and Z. F. Lerner,  
"[A Lightweight Powered Elbow Exoskeleton for Manual Handling Tasks]"  
in IEEE Transactions on Medical Robotics and Bionics.

Parameters
----------
Parameter index order can be found in :doc:`ControllerData <ExoCode/src/ControllerData.h>`. The parameters for this controller are:

- **FLEXamplitude**  
  Torque setpoint, in Nm, for flexion assistance.
- **DigitFSR_threshold**  
  Upper threshold for finger (grip) FSRs.
- **PalmFSR_threshold**  
  Upper threshold for palm FSR.
- **DigitFSR_LOWthreshold**  
  Lower threshold for finger (grip) FSRs.
- **PalmFSR_LOWthreshold**  
  Lower threshold for palm FSR.
- **CaliRequest**  
  Flag to request calibration of sensors.
- **TrqProfile**  
  Flag that toggles between torque states (1 = spring, 0 = constant).
- **p_gain**  
  Proportional gain for closed-loop controls.
- **i_gain**  
  Integral gain for closed-loop controls.
- **d_gain**  
  Derivative gain for closed-loop controls.
- **TorqueLimit**  
  Torque safety limit, in Nm; prevents the controller from prescribing a torque larger than this value.
- **SpringPkTorque**  
  Sets the maximum spring torque in Nm.
- **EXTamplitude**  
  Extension torque setpoint in Nm.
- **FiltStrength**  
  Sets the strength of the setpoint filter.

