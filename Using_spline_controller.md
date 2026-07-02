Issues / gotchas you'll hit

  1. The default config.ini is a Hip exo — the ankle is disabled.
  SDCard/config.ini currently has:
  - name = Hip, hip = NullMotor, ankle = 0 (ankle not used)
  - ankleDefaultController = 0

  So the ankle joint isn't even instantiated. You must reconfigure it for an
  ankle (details below).

  2. The GUI won't even show ankle controllers unless the ankle default
  controller is set. (This is the subtle one.)
  In ListCtrlParams.cpp:80, with the default build flags
  (LIST_CTRL_PARAMS_SEND_MAX = 0), the firmware only streams a joint's controller
  list to the GUI if that joint's default controller is > 1 (i.e., not
  "disabled"). ankleDefaultController = 0 maps to disabled = 1, which fails the >
  1 gate → the GUI receives zero ankle controllers, so "Spline" never appears in
  the dropdown. Setting ankleDefaultController = spline (or any real controller)
  fixes this and the GUI then lists all ankle controllers including Spline.

  3. The default spline.csv runs in simulated gait.
  SDCard/ankleControllers/spline.csv data row is:
  0,0, 25,1, 50,3, 75,-3, 100,0, 1, 0, 0, 0,0,0
  The 11th value (sim_gait_idx) is 1. In Spline::calc_motor_cmd
  (Controller.cpp:825), sim_gait=1 makes the controller ignore your real gait and
  sweep %gait on a fixed 1-second timer (_get_percent_gait(true),
  Controller.cpp:281). Great for bench-testing a motor, useless for walking
  assistance. Set this to 0 for real walking.

  4. use_percent_gait flag changes the x-axis meaning.
  The 12th value (use_percent_gait_idx, default 0):
  - 0 → uses percent_stance (legacy): your node x-axis (0–100) is mapped over the
  stance phase only, and torque is y[0] during swing.
  - 1 → uses percent_gait: x-axis is the full gait cycle heel-strike to
  heel-strike.
  Decide which you want. For most ankle push-off profiles defined over the whole
  cycle, you want 1.

  5. Real gait detection requires working, calibrated FSRs.
  Both percent_gait and percent_stance come from FSR heel/toe strike timing
  (Side.cpp:302–337). Until a few steps establish expected_step_duration,
  percent_gait returns -1, which is ≤ x[0], so the spline outputs y[0]
  (Controller.cpp:902). Keep node 1 at (0, 0) so you get zero torque while
  standing / before gait is established, and you must Calibrate FSR from the GUI
  before walking.

  6. Hard ±15 Nm clamp. Controller.cpp:856–863 clamps the spline output to ±15 Nm
  regardless of your node values. Don't expect more.

  7. Torque direction & sign — do a direction calibration first (safety).
  The spline output goes straight to the motor as a torque command
  (Joint.cpp:1108/1145: setpoint / gearing → _motor->transaction). The sign of
  your node y values picks plantarflexion vs dorsiflexion, and that depends on
  ankleFlipMotorDir / ankleFlipTorqueDir. Before running a real profile, verify
  direction with the calibrManager controller / small constant torque, per
  Documentation/Direction Calibration.md. A flipped sign on a closed-loop term
  can be unsafe.

  8. Open-loop vs PID. With use_pid = 0 (default) the controller is pure
  feed-forward — it commands torque via motor current and does not need the
  torque sensor. The torque sensor (and ankleUseTorqueSensor = yes) only matters
  if you set use_pid = 1. Either is fine; just know the torque-sensor reading
  isn't used for the command unless PID is on.

  9. Re-selecting the controller in the GUI reloads the CSV. When you pick a
  controller (BLE new_trq → set_controller_params, ble_commands.h:321), the
  firmware re-reads spline.csv from the SD card, overwriting any live edits. So
  fix the defaults in the CSV; use live GUI edits only for on-the-fly tuning
  within a session.

  10. Live GUI tuning of nodes works for negatives/decimals. I checked the BLE
  path end-to-end: values are sent as 8-byte doubles → float
  (BleParser.cpp:62–70) and applied as parameters[index] = value
  (uart_commands.h:595). So you can tune node torques to negative/fractional Nm
  live. (The byte-truncation concern doesn't apply here.)

  11. Shared layout caveat (only relevant if you change node count). The Spline
  class and controller_defs::spline layout are shared by hip/ankle/arm_1/arm_2.
  Adding nodes (your ADDING_SPLINE_NODES.md plan) changes all of them and
  requires updating every spline.csv. For a stock 5-node ankle-only run this is a
  non-issue. Your doc's 7-node math checks out (2·7+6=20 ≤ max_parameters=22,
  and ≤ MAX_COLUMNS=30).

  ---
  What to set

  ExoCode/src/Config.h
  - BOARD_VERSION → match your actual board (currently AK_Board_V0_5_1).
  - USE_ANGLE_SENSORS / BATTERY_SENSOR → match your hardware (not used by the
  spline math, but the firmware initializes them).
  - LOOP_FREQ_HZ 500 is fine.

  SDCard/config.ini (ankle-only)
  - name = Ankle (bilateral) — or leftAnkle / rightAnkle for one side
  - sides = bilateral (or left/right)
  - ankle = your motor, exactly as spelled in the map (AK60, AK60v1.1, AK60v3,
  AK70, AK80, AK45_36, AK45_10, MaxonMotor). Set hip = 0.
  - ankleGearRatio = your transmission ratio
  - ankleDefaultController = spline  ← required so the GUI lists Spline (issue
  #2). Use exactly spline (note the comment list in the file has a typo "splin" —
  ignore it).
  - ankleUseTorqueSensor = yes only if you'll use PID; otherwise 0 is fine
  - ankleFlipMotorDir / ankleFlipTorqueDir / ankleFlipAngleDir → per your
  direction calibration
  - leftAnkleRoM / rightAnkleRoM, torque offsets → as appropriate (255 = use
  calibration value)

  SDCard/ankleControllers/spline.csv (row 6 = the 16 values, in order)
  Node1_x, Node1_y, … Node5_x, Node5_y, sim_gait, use_percent_gait, use_pid, P,
  I, D
  - Node 1 = (0, 0); nodes with strictly increasing x (0→100); |y| ≤ 15 Nm; last
  node (100, 0)
  - sim_gait = 0  ← real walking (was 1)
  - use_percent_gait = 1 (full cycle) or 0 (stance only) — your choice
  - use_pid = 0 to start (open loop); P/I/D = 0

  GUI workflow
  1. Flash firmware; SD card in place with the edited config.ini + spline.csv.
  2. Connect; Calibrate Torque (if using PID) and Calibrate FSR.
  3. Verify torque direction with a small/constant torque before trusting the
  profile.
  4. Select Ankle → Spline, pick the parameter set, enable motors / Start.
  5. Take several steps so gait timing establishes, then live-tune node torques
  as needed.