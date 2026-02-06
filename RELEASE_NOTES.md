# Release Notes

## Arm Configuration and arm_1/arm_2 Joints

### Added
- New arm exoskeleton configuration `bilateralArm` that supports bilateral `arm_1` and `arm_2` joints.
- New joint types `arm_1` and `arm_2` with full data/model integration across parsing, configuration, and runtime.
- New controller mappings for arm joints with `constantTorque` and `spline` (and `disabled`).
- New arm controller parameter CSVs under `SDCard/arm1Controllers` and `SDCard/arm2Controllers`.

### Configuration
- `SDCard/config.ini` now supports arm joints and settings:
  - `arm_1`, `arm_2`
  - `arm_1GearRatio`, `arm_2GearRatio`
  - `arm_1DefaultController`, `arm_2DefaultController`
  - `arm_1UseTorqueSensor`, `arm_2UseTorqueSensor`
  - `arm_1FlipMotorDir`, `arm_2FlipMotorDir`
  - `arm_1FlipTorqueDir`, `arm_2FlipTorqueDir`
  - `arm_1FlipAngleDir`, `arm_2FlipAngleDir`
  - `leftArm1RoM`, `rightArm1RoM`, `leftArm2RoM`, `rightArm2RoM`
  - `leftArm1TorqueOffset`, `rightArm1TorqueOffset`, `leftArm2TorqueOffset`, `rightArm2TorqueOffset`

### CAN IDs
- `left_arm_1` = 80
- `right_arm_1` = 48
- `left_arm_2` = 192
- `right_arm_2` = 160

### Notes
- Arm joints use the same motor types and CAN motor selection as existing joints.
- If your hardware limits the number of motors per side, ensure board pin availability and update any board limits as needed.
