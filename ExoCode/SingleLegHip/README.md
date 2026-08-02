# Single-Leg Hip Exoskeleton (event-driven gait phase)

A standalone Arduino sketch for a ONE-SIDED (single-leg) hip exoskeleton on a
Teensy 4.1. It reuses the shared `src/` folder from the main `ExoCode` sketch
(copy it here to build, same as the `systemCheck` sketches — the folder is
gitignored).

## What is different from upstream

The gait phase is detected with an **event-driven state machine**
(`GaitPhaseDetector`) that watches the FSRs on **both feet** (exo heel+toe and
contralateral heel+toe), instead of extrapolating the phase forward from an
expected step duration.

### The key insight

During normal walking, **double support** happens twice per cycle, and during
each double support one foot's toe enters its falling edge at (almost) the same
time the other foot's heel enters its rising edge:

```
   L toe falling edge ─┐                 ┌── L heel rising edge
   R heel rising edge ─┘  <-- double --> └── R toe falling edge
                         support #1          support #2
```

So the supporting (contralateral) foot tells us exactly when the exo leg is
about to push off and swing, and when it is about to heel strike. No time
prediction needed.

| Contralateral (supporting) foot event | Meaning for the exo leg |
|---------------------------------------|-------------------------|
| Heel rising edge (lands) | About to push off and start swinging |
| Toe falling edge (leaves) | About to heel strike and end swinging |

The exo leg's own FSRs are used as a cross-check.

### Robustness

* **No time prediction** — swing start/end are anchored to real FSR edges, so
  sudden acceleration/deceleration cannot shift the torque curve.
* **Debounced edges** — every edge must hold for `CONTACT_DEBOUNCE_MS` (8 ms)
  before it is accepted (rejects shuffling/dragging).
* **Boot guard** — the first sample never emits an edge.
* **Min swing filter** — swings shorter than `MIN_SWING_MS` are rejected.
* **Percent progress** — within a swing, progress is normalized by the measured
  duration of the previous swing (event to event), not a moving average.

## Configuration

Set the side in `SingleLegHip.ino`:

```c
10,  //[3]  Exo name (left_hip)   -- change to 11 for right_hip
2,   //[4]  Exo side (left)       -- change to 3 for right
```

The default controller is `franksCollinsHip`, which now uses the event-driven
phase (`in_swing` / `ev_percent_stance` / `ev_percent_swing`). The torque curve
(extension during stance, flexion during swing, zero-torque region between) is
unchanged; only the phase input is different.

## Files changed vs upstream

| File | Change |
|------|--------|
| `src/GaitPhaseDetector.h/.cpp` | **New** — event-driven bilateral gait phase detector |
| `src/SideData.h/.cpp` | Added event-driven phase fields (`in_swing`, `ev_percent_swing`, ...) |
| `src/Side.h/.cpp` | Integrated the detector into each side |
| `src/Controller.cpp` | `FranksCollinsHip` now uses the event-driven phase |
| `src/Config.h` | Added `Single_Leg_Hip_Board` + `gait_phase_config` |
| `src/Board.h` | Added `Single_Leg_Hip_Board` pin map |
| `src/ParseIni.h` | Added `single_leg_hip` board version |
