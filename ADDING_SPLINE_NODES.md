# Adding Nodes to the Spline Controller

This guide explains how the OpenExo `Spline` controller works and exactly what to
change to increase the number of spline nodes. It uses a **5 → 7 node** upgrade as a
worked example.

> **Scope / status:** This is a planning document. No source code is modified by this
> file. Apply the edits below yourself, then recompile and re-flash the Teensy.

---

## 1. Background: how the spline is computed

The interpolation lives in `ExoCode/src/Controller.cpp` → `Spline::_spline_interpolate`.
It is a **natural cubic spline** (the classic *Numerical Recipes* `spline`/`splint`
pair fused into one function).

- **Inputs:** `x[]` = node positions on the **% gait** axis, `y[]` = node **torque**
  values in Nm, and `percent_gait` = the single point to evaluate right now.
- The controller does **not** pre-render a curve. Every control loop it computes the
  current `percent_gait` (`Spline::calc_motor_cmd`) and evaluates the spline once to get
  the instantaneous torque setpoint. The curve is traced out in real time as
  `percent_gait` sweeps 0 → 100.

What the routine does, step by step:

1. **Safety / boundaries** – if the `x` values are not strictly increasing it returns 0.
   If `percent_gait` is before the first node or after the last node, the torque is
   **clamped** to the endpoint value (`y[0]` or `y[n-1]`) — flat, not extrapolated.
2. **Solve second derivatives** – a cubic spline fits a separate cubic on each interval
   such that the curve passes through every node and has matching first *and* second
   derivatives at the interior nodes (smooth, C² continuous). Those constraints form a
   **tridiagonal system** for the second derivatives `y2[i]`, solved with the Thomas
   algorithm (forward sweep + back substitution).
3. **Natural boundary condition** – `y2[0] = 0` and `y2[n-1] = 0`, i.e. zero curvature
   at both ends (this is what makes it a *natural* spline).
4. **Evaluate** – find the interval containing `percent_gait` and evaluate the `splint`
   formula (linear interpolation between the two node torques plus a cubic curvature
   correction from the second derivatives).

The final command is clamped to **±15 Nm** in `Spline::calc_motor_cmd`.

The interpolation math is already written generically in terms of a single count `n`,
so **the algorithm itself does not change** when you add nodes — only the count and the
parameter plumbing around it.

---

## 2. Key concept: node *count* is compile-time, node *values* are runtime

| What you want to change | What it takes |
|---|---|
| Node **positions / torque values** (move a point, change its Nm) | **CSV only** (or live per-index via the GUI). No recompile. |
| **Number** of nodes (5 → 7) | **Firmware edit + recompile + flash**, *and then* the CSV must match. |

The node count is baked into the firmware: fixed parameter indices, a fixed
`num_parameter`, and fixed-size stack arrays (`const int n = 5; float y2[n];`).
You **cannot** add a node by editing the CSV alone — the CSV defines *where* the nodes
are, the firmware defines *how many* exist. They must agree.

### Limits to respect

- **`max_parameters`** (`ExoCode/src/ControllerData.h`) is currently `22`
  (`= spv2::num_parameter`). The spline parameter array must fit inside it.
  - Parameter count for `N` nodes = `2*N + 6` (two values per node + 6 flags/gains).
  - 5 nodes = 16, **7 nodes = 20**, 8 nodes = 22 (fits). **Only above 8 nodes** must you
    raise `max_parameters` (and note it costs RAM on every joint/side).
- **GUI handshake** `MAX_COLUMNS` (`ExoCode/src/ListCtrlParams.h`, currently `30`) leaves
  ~26 data columns, so up to ~10 nodes stream to the app with no change.
- **Shared controller caveat:** the `Spline` class and the `controller_defs::spline`
  parameter layout are shared by **hip, ankle, arm_1, and arm_2**. Changing the node
  count changes the layout for *all* of them. Update every `spline.csv`
  (`SDCard/ankleControllers/`, `hipControllers/`, `arm1Controllers/`, `arm2Controllers/`)
  so none load mismatched values.

For a **7-node** spline: `2*7 + 6 = 20` parameters → fits within `max_parameters = 22`
and within `MAX_COLUMNS = 30`, so **no limit changes are required**. 

---

## 3. Worked example: upgrade 5 → 7 nodes

There are **5 edits** (4 firmware + the CSVs). All snippets are illustrative — match the
surrounding style/formatting in the actual files.

### Edit 1 — Parameter indices (`ExoCode/src/ControllerData.h`)

In `namespace spline`, add the two new nodes and **shift the trailing flag/gain indices
up by 4** (two new nodes × two values), then update `num_parameter`.

**Before (5 nodes, `num_parameter = 16`):**
```cpp
namespace spline
{
    const uint8_t node1_x_idx = 0;
    const uint8_t node1_y_idx = 1;
    const uint8_t node2_x_idx = 2;
    const uint8_t node2_y_idx = 3;
    const uint8_t node3_x_idx = 4;
    const uint8_t node3_y_idx = 5;
    const uint8_t node4_x_idx = 6;
    const uint8_t node4_y_idx = 7;
    const uint8_t node5_x_idx = 8;
    const uint8_t node5_y_idx = 9;
    const uint8_t sim_gait_idx = 10;
    const uint8_t use_percent_gait_idx = 11;
    const uint8_t use_pid_idx = 12;
    const uint8_t p_gain_idx = 13;
    const uint8_t i_gain_idx = 14;
    const uint8_t d_gain_idx = 15;
    const uint8_t num_parameter = 16;
}
```

**After (7 nodes, `num_parameter = 20`):**
```cpp
namespace spline
{
    const uint8_t node1_x_idx = 0;
    const uint8_t node1_y_idx = 1;
    const uint8_t node2_x_idx = 2;
    const uint8_t node2_y_idx = 3;
    const uint8_t node3_x_idx = 4;
    const uint8_t node3_y_idx = 5;
    const uint8_t node4_x_idx = 6;
    const uint8_t node4_y_idx = 7;
    const uint8_t node5_x_idx = 8;
    const uint8_t node5_y_idx = 9;
    const uint8_t node6_x_idx = 10;   // NEW
    const uint8_t node6_y_idx = 11;   // NEW
    const uint8_t node7_x_idx = 12;   // NEW
    const uint8_t node7_y_idx = 13;   // NEW
    const uint8_t sim_gait_idx = 14;          // shifted +4
    const uint8_t use_percent_gait_idx = 15;  // shifted +4
    const uint8_t use_pid_idx = 16;           // shifted +4
    const uint8_t p_gain_idx = 17;            // shifted +4
    const uint8_t i_gain_idx = 18;            // shifted +4
    const uint8_t d_gain_idx = 19;            // shifted +4
    const uint8_t num_parameter = 20;         // was 16
}
```

> The flag/gain reads in `calc_motor_cmd` use these **named** constants, so once you
> renumber them here those reads follow automatically.

### Edit 2 — `max_parameters` (`ExoCode/src/ControllerData.h`)

```cpp
const uint8_t max_parameters = spv2::num_parameter;   // = 22
```

For **7 nodes (20 params)**, `20 ≤ 22`, so **leave this unchanged**.
Only edit it if you ever exceed 8 nodes — e.g. make it the max of all controllers, or
set it to your new `spline::num_parameter`.

### Edit 3 — Build the node arrays (`ExoCode/src/Controller.cpp`, `Spline::calc_motor_cmd`)

Grow the `x` / `y` arrays from 5 to 7 entries and add reads for the new node indices.

**Before:**
```cpp
float x[5] =
{
    _controller_data->parameters[controller_defs::spline::node1_x_idx],
    _controller_data->parameters[controller_defs::spline::node2_x_idx],
    _controller_data->parameters[controller_defs::spline::node3_x_idx],
    _controller_data->parameters[controller_defs::spline::node4_x_idx],
    _controller_data->parameters[controller_defs::spline::node5_x_idx],
};

float y[5] =
{
    _controller_data->parameters[controller_defs::spline::node1_y_idx],
    _controller_data->parameters[controller_defs::spline::node2_y_idx],
    _controller_data->parameters[controller_defs::spline::node3_y_idx],
    _controller_data->parameters[controller_defs::spline::node4_y_idx],
    _controller_data->parameters[controller_defs::spline::node5_y_idx],
};
```

**After:**
```cpp
float x[7] =
{
    _controller_data->parameters[controller_defs::spline::node1_x_idx],
    _controller_data->parameters[controller_defs::spline::node2_x_idx],
    _controller_data->parameters[controller_defs::spline::node3_x_idx],
    _controller_data->parameters[controller_defs::spline::node4_x_idx],
    _controller_data->parameters[controller_defs::spline::node5_x_idx],
    _controller_data->parameters[controller_defs::spline::node6_x_idx],   // NEW
    _controller_data->parameters[controller_defs::spline::node7_x_idx],   // NEW
};

float y[7] =
{
    _controller_data->parameters[controller_defs::spline::node1_y_idx],
    _controller_data->parameters[controller_defs::spline::node2_y_idx],
    _controller_data->parameters[controller_defs::spline::node3_y_idx],
    _controller_data->parameters[controller_defs::spline::node4_y_idx],
    _controller_data->parameters[controller_defs::spline::node5_y_idx],
    _controller_data->parameters[controller_defs::spline::node6_y_idx],   // NEW
    _controller_data->parameters[controller_defs::spline::node7_y_idx],   // NEW
};
```

### Edit 4 — Node count in the interpolator (`ExoCode/src/Controller.cpp`, `Spline::_spline_interpolate`)

```cpp
const int n = 5;   // change to: const int n = 7;
```

This is the **only** change inside `_spline_interpolate` — the rest of the function is
written in terms of `n` and needs no other edits.

### Edit 5 — Parameter CSV file(s) (`SDCard/.../spline.csv`)

Update the parameter-count cell and the column layout. For an ankle-only build, edit
`SDCard/ankleControllers/spline.csv` (and the hip/arm copies if those joints are ever
used — see the shared-controller caveat).

- **Row 1, cell 1:** header line count — keep at `5` (unchanged).
- **Row 2, cell 1:** parameter count — change `16` → **`20`**.
- **Row 5:** the parameter-name header. Add `Node6_x, Node6_y, Node7_x, Node7_y` before
  the flag columns.
- **Row 6 (and any extra rows):** the values — must now have **20** comma-separated
  values in node order, then the 6 flags/gains.

**Example 7-node `spline.csv`:**
```
5,"header Size, the first N rows will be ignored, except for this first cells in the first two rows",,,,,,,,,,,,,,,,,,
20,"parameter number, the number of parameters to read per line",,,,,,,,,,,,,,,,,,
,Parameter list for the spline controller,,,,,,,,,,,,,,,,,,
,Parameter order:,,,,,,,,,,,,,,,,,,
Node1_x,Node1_y,Node2_x,Node2_y,Node3_x,Node3_y,Node4_x,Node4_y,Node5_x,Node5_y,Node6_x,Node6_y,Node7_x,Node7_y,1=sim %gait,1=%gait 0=%stance,PID Flag,P Gain,I Gain,D Gain
0,0,15,1,30,2,50,3,65,1,80,-2,100,0,1,0,0,0,0,0
```

In this example data row the seven `(x, y)` nodes are:
`(0,0) (15,1) (30,2) (50,3) (65,1) (80,-2) (100,0)`, followed by
`sim=1, use_%gait=0, PID=0, P=0, I=0, D=0`.

> **Reminder:** `x` values must be **strictly increasing**, or the interpolator returns
> 0 as a safety fallback.

---

## 4. After the edits

1. **Recompile and flash** the Teensy (the node count is compiled in).
2. Confirm the CSV's parameter-count cell (`20`) matches `spline::num_parameter` (`20`).
3. Once flashed, you can tune the node **positions/torques** freely via the CSV or live
   from the GUI (per-index `update_param`) **without** recompiling. Only the **count**
   requires another firmware change.

---

## 5. Quick checklist (5 → 7 nodes)

- [ ] `ControllerData.h` — add `node6_*`/`node7_*` indices, shift flags +4, `num_parameter = 20`
- [ ] `ControllerData.h` — `max_parameters` unchanged (20 ≤ 22)
- [ ] `Controller.cpp` `calc_motor_cmd` — `x[7]` / `y[7]` with node6/node7 reads
- [ ] `Controller.cpp` `_spline_interpolate` — `const int n = 7;`
- [ ] `SDCard/ankleControllers/spline.csv` — count cell `20`, 20 columns in name + value rows
- [ ] (If hip/arm spline ever used) update their `spline.csv` to match
- [ ] Recompile + flash

---

## 6. Optional: make node count CSV-driven (no recompile)

If you want to change node count from the CSV alone in the future, that's a larger
refactor: add a `num_nodes` parameter, size arrays to a `MAX_NODES` constant instead of a
fixed `n`, and have `_spline_interpolate` take the count as an argument and loop to
`num_nodes`. The count would then be CSV-driven up to `MAX_NODES`, still bounded by
`max_parameters` (`2*MAX_NODES + 6 ≤ 22`) and the GUI's `MAX_COLUMNS`. This is the cleaner
long-term design if you expect to experiment with node counts often.
