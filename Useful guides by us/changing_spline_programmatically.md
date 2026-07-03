# Changing the Spline Programmatically

How to update the ankle Spline controller's nodes from outside the normal
"type-in-the-GUI" workflow. Three layers are covered:

1. What a "spline change" actually is at runtime (node **values** vs node **count**).
2. The live-update command the GUI already uses (`update_param` over BLE), including
   the exact wire framing, the IDs for the ankle Spline, and what the Teensy validates.
3. **The recommended integration for your case:** adding a small UDP listener to the
   GUI so an outside program can hand the GUI a new spline (in the same layout
   `spline.csv` uses) and let the GUI's existing pipeline push it to the exo.

This is a companion to `Using_spline_controller.md` (start there for setup, the CSV
format, and the firmware gotchas).

---

## 1. What you can change at runtime

A spline node value — an x-position (% gait) or a y-torque (Nm) — is just one
**indexed parameter** in the controller's parameter array, and it can be overwritten
in place while the controller runs. What you **cannot** change at runtime is the
**number** of nodes: the count is compiled into the firmware (see
`ADDING_SPLINE_NODES.md`). So "programmatically change the spline" means "overwrite the
10 node values (and optionally the 6 flags/gains) of the currently-running 5-node
Spline," not "add nodes on the fly."

Two properties matter for any programmatic approach:

- **Updates are RAM-only and ephemeral.** Re-selecting the controller reloads
  `spline.csv` from the SD card and wipes your live edits (issue #9 in the Using doc).
  To persist a profile, write it into the CSV.
- **x must stay strictly increasing.** The interpolator returns 0 Nm if the x values
  aren't strictly increasing at the instant it evaluates (`Controller.cpp:903–909`).
  Because values are applied one at a time, an in-progress reshape that momentarily
  puts x out of order can cause a transient zero-torque glitch. Update torques (y)
  freely; when you move x-positions, send them in an order that never breaks
  monotonicity, or update with motors stopped.

---

## 2. The live-update command (`update_param` / `'f'`)

The exo's BLE MCU exposes a **Nordic UART Service** (service `6E400001-…`, write
characteristic `6e400002-…`, notify characteristic `6e400003-…`). A live
single-parameter update is the `update_param` command (`'f'`,
`ble_commands.h:53` / handler at `:430`). Framing, exactly as the GUI sends it
(`QtExoDeviceManager.updateTorqueValues`, ~line 794): write the ASCII byte `b"f"`,
then write **four** separate 8-byte little-endian IEEE-754 doubles
(`struct.pack("<d", v)`) in this order:

```
f  →  [ joint_id, controller_id, param_index, value ]
```

**IDs for the ankle Spline:**

| Field | Value | Source |
|---|---|---|
| `joint_id` | left ankle = **68**, right ankle = **36** (side bits: left `0x40`, right `0x20`) | `ParseIni.h:137/145` |
| `controller_id` | ankle spline = **12** | `config_defs::ankle_controllers::spline`, `ParseIni.h` |
| `param_index` | `Node1_x=0, Node1_y=1, … Node5_x=8, Node5_y=9`, then `sim_gait=10, use_percent_gait=11, use_pid=12, P=13, I=14, D=15` | `controller_defs::spline`, `ControllerData.h` |
| `value` | the number (% gait for an x index, Nm for a y index) | — |

So to change node 3's torque, write index 5; to move node 4's x-position, write
index 6. A full 5-node reshape is 10 writes (indices 0–9). Note the 16 parameter
indices (0–15) line up **1:1** with the 16 columns of `spline.csv`'s value row — that
correspondence is what makes the UDP bridge in §3 trivial.

**What the Teensy enforces** (so a bad message is rejected, not blindly applied —
`ParamUpdateValidation.h`). An update is NAK'd if: the joint isn't in use / the side
doesn't match (`joint_mismatch`); **Spline isn't the joint's currently selected
controller** (`controller_mismatch`); the index is past the parameter count
(`invalid_index`); or the value is outside the parameter's configured min/max
(`out_of_bounds`). Consequences:

- **Select Ankle → Spline first** (in the GUI, or via the controller-select command
  `new_trq` / `'F'` with `[joint_id, controller_id, set_num]`). You can't push node
  values into a controller that isn't the active one.
- The firmware sends an ACK/NAK back on the notify characteristic (`6e400003-…`);
  rejection reasons are surfaced in the GUI via `RtBridge` → `MainWindow._on_param_update_ack`.

### Sending it directly — two quick options

**Option A — reuse the GUI's device manager (if the GUI is running).** The GUI owns
the single BLE connection (only one BLE central can hold the link at a time), and
`QtExoDeviceManager.updateTorqueValues([bilateral, joint_id, controller_id,
param_index, value])` does the framing and optional bilateral mirroring for you
(`build_parameter_updates`, `QtExoDeviceManager.py:654`):

```python
dm = ...  # the running QtExoDeviceManager instance
LEFT_ANKLE, SPLINE = 68, 12
flat = [v for xy in nodes for v in xy]       # nodes: [(x1,y1)...(x5,y5)] -> [x1,y1,...]
for param_index, value in enumerate(flat):   # indices 0..9
    dm.updateTorqueValues([True, LEFT_ANKLE, SPLINE, param_index, value])
```

**Option B — direct `bleak`, headless (GUI not running).** Only one central can be
connected, so run this **instead of** the GUI, not alongside it:

```python
import struct, asyncio
from bleak import BleakClient
TX = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"

async def set_param(client, joint_id, controller_id, index, value):
    await client.write_gatt_char(TX, b"f", response=False)
    for v in (joint_id, controller_id, index, value):
        await client.write_gatt_char(TX, struct.pack("<d", float(v)), response=False)
    await asyncio.sleep(0.01)   # small gap between updates, as the GUI does
```

---

## 3. Recommended: a UDP listener inside the GUI

Your outside program computes a new spline and hands it to the GUI **in the exact
layout `spline.csv` already uses**; the GUI parses it and reuses its built-in
parameter-update pipeline. This is clean because (a) the GUI keeps sole ownership of
the BLE link, (b) you write no BLE code in your outside program — it just sends a
datagram, and (c) the 16 CSV columns map 1:1 onto parameter indices 0–15, so parsing
is a `split(",")`.

**Yes, this is possible**, and the GUI's architecture makes it straightforward:

- `GUI.py` runs a normal Qt event loop (`QApplication.exec()`); `MainWindow` creates
  `self.qt_dev = QtExoDeviceManager(self)`.
- `QtExoDeviceManager` already runs `bleak` on a **background asyncio thread**, and
  `updateTorqueValues → _submit → asyncio.run_coroutine_threadsafe` is **thread-safe**
  (`QtExoDeviceManager.py:621`). So a command originating on another thread is fine.
- The existing "apply a parameter" entry point is `MainWindow._on_apply_settings(payload)`
  with `payload = [isBilateral, joint, controller, parameter, value]` — it validates,
  queues the pending update for ACK tracking, and calls `qt_dev.updateTorqueValues(payload)`
  (`MainWindow.py:792`). **This is the pipeline you reuse.**

### Wire format assumption

Assume each UDP datagram is one line matching **row 6 of `spline.csv`** — the 16
comma-separated values in column order:

```
Node1_x,Node1_y,Node2_x,Node2_y,Node3_x,Node3_y,Node4_x,Node4_y,Node5_x,Node5_y,sim_gait,use_percent_gait,use_pid,P,I,D
```

e.g. `0,0,25,1,50,3,75,-3,100,0,0,1,0,0,0,0`. (If your program sends the whole
multi-row CSV instead, split on newlines and take the first line that parses as 16
numbers; the parser below tolerates a trailing newline and surrounding whitespace.)

### Threading model

Run the UDP socket on its own thread (blocking `recvfrom`), and hand each parsed
spline to the GUI thread via a **queued Qt signal**. The actual dispatch then happens
on the GUI thread — important because `_on_apply_settings` also touches Qt widgets
(status labels, page navigation), which must not be called from a worker thread.

### Edit 1 — a UDP listener object (`Python_GUI/services/UdpSplineListener.py`, new file)

```python
import socket
import logging
from PySide6 import QtCore


class UdpSplineListener(QtCore.QObject):
    """Listens for UDP datagrams carrying a spline.csv-format value row and
    emits the 16 parsed parameters on the GUI thread."""

    # Emitted with the 16 floats [Node1_x, Node1_y, ... D] on each valid datagram.
    splineReceived = QtCore.Signal(list)

    def __init__(self, host="127.0.0.1", port=34567, parent=None):
        super().__init__(parent)
        self._host = host
        self._port = port
        self._running = False
        self._sock = None
        self._thread = None
        self.logger = logging.getLogger("OpenExo").getChild("UdpSplineListener")

    def start(self):
        if self._running:
            return
        self._running = True
        self._thread = QtCore.QThread()
        self.moveToThread(self._thread)
        self._thread.started.connect(self._run)
        self._thread.start()

    def stop(self):
        self._running = False
        try:
            if self._sock:
                self._sock.close()  # unblocks recvfrom
        except OSError:
            pass
        if self._thread:
            self._thread.quit()
            self._thread.wait(1000)

    def _run(self):
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._sock.bind((self._host, self._port))
        self.logger.info("UDP spline listener bound to %s:%d", self._host, self._port)
        while self._running:
            try:
                data, _addr = self._sock.recvfrom(4096)
            except OSError:
                break  # socket closed by stop()
            params = self._parse(data)
            if params is not None:
                self.splineReceived.emit(params)

    @staticmethod
    def _parse(data: bytes):
        """Return a list of 16 floats from a spline.csv value row, or None."""
        try:
            text = data.decode("utf-8", errors="replace")
        except Exception:
            return None
        for line in text.splitlines():
            cells = [c.strip() for c in line.split(",") if c.strip() != ""]
            if len(cells) < 16:
                continue
            try:
                return [float(c) for c in cells[:16]]
            except ValueError:
                continue
        return None
```

### Edit 2 — wire it into `MainWindow`

In `MainWindow.__init__`, after `self.qt_dev = QtExoDeviceManager(self)` is created,
start the listener and connect its signal to a new slot (the `QueuedConnection` makes
the slot run on the GUI thread):

```python
from services.UdpSplineListener import UdpSplineListener

# ... after self.qt_dev is created ...
self.udp_spline = UdpSplineListener(host="127.0.0.1", port=34567, parent=self)
self.udp_spline.splineReceived.connect(
    self._on_udp_spline, QtCore.Qt.QueuedConnection)
self.udp_spline.start()
```

Add the slot. It reuses `_on_apply_settings` exactly the way the GUI's own
"Update Controller" button does — one call per parameter index:

```python
LEFT_ANKLE_ID = 68     # config_defs::joint_id::left_ankle
ANKLE_SPLINE_ID = 12   # config_defs::ankle_controllers::spline

@QtCore.Slot(list)
def _on_udp_spline(self, params: list):
    """params = 16 floats in spline.csv column order (indices 0..15)."""
    if len(params) < 16:
        self.logger.warning("UDP spline ignored: expected 16 values, got %d", len(params))
        return
    self.logger.info("Applying spline from UDP: %s", params)
    for param_index, value in enumerate(params):
        # bilateral=True mirrors to both ankles; drop to a single side if desired.
        self._on_apply_settings([True, LEFT_ANKLE_ID, ANKLE_SPLINE_ID,
                                 param_index, float(value)])
```

Be sure `self.udp_spline.stop()` is called on shutdown (e.g. in `MainWindow.closeEvent`).

### Behavior and caveats

- **Preconditions still apply.** Ankle → Spline must be the active controller, or the
  Teensy NAKs every write (`controller_mismatch`). Have the operator select it once, or
  extend the UDP protocol to also send the controller-select (`new_trq`/`'F'`) first.
- **Send order for x-positions.** `_on_apply_settings` dispatches immediately per index,
  so the loop above walks indices 0→15 in order. Sending a *complete* new profile whose
  x column is already strictly increasing is fine; if you send partial x-position changes
  that transiently break monotonicity, expect a brief zero-torque blip (§1).
- **Only local traffic by default.** Binding to `127.0.0.1` accepts datagrams from
  programs on the same machine. Bind to `0.0.0.0` to accept from other hosts — do that
  only on a trusted network, since there is no authentication and each datagram directly
  commands the exo. (The Teensy still bounds every value, but access control is on you.)
- **Persistence.** These are live edits; they vanish on controller re-select. If you want
  a UDP-sent profile to survive, also have the GUI write it to `spline.csv`.

---

## 4. Making it persistent: remote SD-card writes (firmware work, not yet supported)

Everything above is **runtime-only** — it never touches the SD card. As of today there
is **no way to edit files on the SD card remotely**, so you cannot persist a profile by
rewriting `spline.csv` over the link. The firmware only ever *reads* the card: every
access is `FILE_READ` (`ParamsFromSD.cpp`, and the default-mode `SD.open` in
`ListCtrlParams.cpp`), nothing writes it, and there is no data logging to SD. The BLE
command set (`ble_commands.h`, received commands `start, stop, cal_trq, cal_fsr,
new_trq, new_fsr, assist, resist, motors_on/off, mark, update_param, reset_system`) has
**no file-transfer command** — you can't push a file to the card or read one back. So
`spline.csv` remains the source of truth and can only be changed by physically pulling
the card and editing it on a computer; the live `update_param` path is for within-session
tuning that gets overwritten on the next controller re-select.

To support remote persistence you'd add a firmware command that follows exactly the
"write these numbers/bytes into the corresponding file" model. Concretely: (1) define a
new BLE/UART command (e.g. `write_param_file`) whose payload is a **target file id** (an
enum for `ankleControllers/spline.csv`, `hipControllers/spline.csv`, etc. — never a
free-form path, to avoid letting a datagram write anywhere on the card) plus the **new
value row** (the 16 spline numbers, which the firmware formats back into the CSV's row-6
layout). (2) Add a handler that opens that file with `FILE_WRITE`/`O_CREAT|O_TRUNC` via
SdFat (the write capability exists — `IniFile` already models a `FILE_WRITE` mode that
`ParseIni` simply never uses), writes the formatted rows, closes/flushes, and returns an
ACK/NAK just like `update_param` does. (3) On the GUI/UDP side, send that command instead
of (or alongside) the live per-parameter writes. For just the 16 spline numbers this is a
single small message; the only real complication is if you ever want to write **arbitrary
larger files**, which would need a chunked transfer protocol layered on the fixed-size
command framing plus reassembly on-device (BLE throughput is low, so a multi-KB file
arrives as many packets). For the spline case none of that is needed — a fixed
"here are the 16 values for file X, write them" command is enough, and it's the clean way
to make a UDP-pushed profile survive a controller re-select or a power cycle.
