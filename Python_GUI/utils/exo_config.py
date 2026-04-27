"""
Parses ``SDCard/config.ini`` so the GUI can mirror the firmware's view of the
exoskeleton without the user having to re-enter the same information twice.

Two pieces of information are exposed:

* :func:`allowed_joint_types` – the set of joint families (``hip``/``knee``/
  ``ankle``/``elbow``/``arm_1``/``arm_2``) that the configured ``name`` is
  expected to expose.  Used to filter out spurious joints in the handshake –
  e.g. a ``Hip`` device that still has ``hipDefaultController`` left at the
  default ``step`` will broadcast hip controllers even when the SD card says
  ``name = Ankle``.
* :func:`default_controller_for` – the controller name (matching the strings
  the firmware emits in the handshake) that should be pre-selected for a
  given joint on the Update Parameters page.

The parser is intentionally lenient: an unreadable / missing config.ini
returns ``None`` from the high-level helpers so callers can fall back to the
existing handshake-only behaviour.
"""

from __future__ import annotations

import configparser
import os
from typing import Dict, Optional, Set


# ---------------------------------------------------------------------------
# Path resolution
# ---------------------------------------------------------------------------

# Repository layout:
#   <repo>/Python_GUI/utils/exo_config.py   <- this file
#   <repo>/SDCard/config.ini                <- firmware config
_DEFAULT_CONFIG_PATH = os.path.normpath(
    os.path.join(os.path.dirname(__file__), "..", "..", "SDCard", "config.ini")
)


def get_config_path() -> str:
    """Return the absolute path to ``SDCard/config.ini``."""
    return _DEFAULT_CONFIG_PATH


# ---------------------------------------------------------------------------
# Mapping tables (mirror ExoCode/src/ParseIni.h)
# ---------------------------------------------------------------------------

# Each `name` value in [Exo] maps to the set of joint families it covers.
_NAME_TO_JOINT_TYPES: Dict[str, Set[str]] = {
    # Bilateral configurations
    "ankle":             {"ankle"},
    "hip":               {"hip"},
    "knee":              {"knee"},
    "elbow":             {"elbow"},
    "hipankle":          {"hip", "ankle"},
    "hipelbow":          {"hip", "elbow"},
    "ankleelbow":        {"ankle", "elbow"},
    "arm":               {"arm_1", "arm_2"},

    # Backward-compatible explicit-bilateral spellings
    "bilateralankle":    {"ankle"},
    "bilateralhip":      {"hip"},
    "bilateralknee":     {"knee"},
    "bilateralelbow":    {"elbow"},
    "bilateralhipankle": {"hip", "ankle"},
    "bilateralhipelbow": {"hip", "elbow"},
    "bilateralankleelbow": {"ankle", "elbow"},
    "bilateralarm":      {"arm_1", "arm_2"},

    # Unilateral configurations
    "leftankle":         {"ankle"},
    "rightankle":        {"ankle"},
    "lefthip":           {"hip"},
    "righthip":          {"hip"},
    "leftknee":          {"knee"},
    "rightknee":         {"knee"},
    "leftelbow":         {"elbow"},
    "rightelbow":        {"elbow"},
    "lefthipankle":      {"hip", "ankle"},
    "righthipankle":     {"hip", "ankle"},
    "lefthipelbow":      {"hip", "elbow"},
    "righthipelbow":     {"hip", "elbow"},
    "leftankleelbow":    {"ankle", "elbow"},
    "rightankleelbow":   {"ankle", "elbow"},
}

# The handshake transmits joint names like "Ankle(L)", "Hip(R)", "Arm1(L)".
# Map the leading token (case-insensitive) back to a joint family.
_JOINT_NAME_PREFIX_TO_TYPE: Dict[str, str] = {
    "ankle": "ankle",
    "hip":   "hip",
    "knee":  "knee",
    "elbow": "elbow",
    "arm1":  "arm_1",
    "arm2":  "arm_2",
}


# ---------------------------------------------------------------------------
# Parsing
# ---------------------------------------------------------------------------


def _read_config(path: Optional[str] = None) -> Optional[configparser.ConfigParser]:
    cfg_path = path or _DEFAULT_CONFIG_PATH
    if not os.path.isfile(cfg_path):
        return None
    parser = configparser.ConfigParser(
        # The firmware allows ; and # for comments; configparser supports both
        # via inline_comment_prefixes.  Keys/values may also have leading
        # tabs; configparser handles that natively.
        inline_comment_prefixes=(";", "#"),
        comment_prefixes=(";", "#"),
        strict=False,
    )
    try:
        parser.read(cfg_path, encoding="utf-8")
    except (configparser.Error, OSError):
        return None
    return parser


def get_exo_name(path: Optional[str] = None) -> Optional[str]:
    """Return the raw ``name`` value from ``[Exo]``, or ``None`` if missing."""
    parser = _read_config(path)
    if parser is None or not parser.has_option("Exo", "name"):
        return None
    return parser.get("Exo", "name").strip()


def allowed_joint_types(path: Optional[str] = None) -> Optional[Set[str]]:
    """Return the set of joint families the configured ``name`` exposes.

    Returns ``None`` when ``config.ini`` is missing or ``name`` is unknown so
    callers can leave the handshake unfiltered in that case.
    """
    raw = get_exo_name(path)
    if raw is None:
        return None
    return _NAME_TO_JOINT_TYPES.get(raw.strip().lower())


def default_controllers(path: Optional[str] = None) -> Dict[str, str]:
    """Return the per-family default-controller names from ``[Exo]``.

    Keys are joint families (``hip``/``knee``/``ankle``/``elbow``/
    ``arm_1``/``arm_2``).  Values are the literal strings from config.ini
    (e.g. ``"step"``, ``"PJMC"``) – the same casing the firmware uses in the
    handshake.  Disabled entries (``"0"``) are omitted.
    """
    parser = _read_config(path)
    if parser is None or not parser.has_section("Exo"):
        return {}

    result: Dict[str, str] = {}
    for family, key in (
        ("hip",   "hipDefaultController"),
        ("knee",  "kneeDefaultController"),
        ("ankle", "ankleDefaultController"),
        ("elbow", "elbowDefaultController"),
        ("arm_1", "arm_1DefaultController"),
        ("arm_2", "arm_2DefaultController"),
    ):
        if parser.has_option("Exo", key):
            value = parser.get("Exo", key).strip()
            if value and value != "0":
                result[family] = value
    return result


# ---------------------------------------------------------------------------
# Helpers used by the GUI
# ---------------------------------------------------------------------------


def joint_type_for(handshake_name: str) -> Optional[str]:
    """Return the joint family for a handshake joint name (e.g. "Ankle(L)")."""
    if not handshake_name:
        return None
    # Drop any "(L)"/"(R)" side suffix, then normalise.
    base = handshake_name.split("(", 1)[0].strip().lower()
    return _JOINT_NAME_PREFIX_TO_TYPE.get(base)


def is_joint_allowed(handshake_name: str, allowed: Optional[Set[str]]) -> bool:
    """Return whether a handshake joint should be shown for the given allow-set."""
    if not allowed:
        # Either we couldn't read config.ini or the name is unrecognised – be
        # permissive and show whatever the firmware reported.
        return True
    family = joint_type_for(handshake_name)
    if family is None:
        return True
    return family in allowed


def default_controller_for(handshake_name: str,
                           defaults: Optional[Dict[str, str]] = None,
                           path: Optional[str] = None) -> Optional[str]:
    """Return the default controller for a handshake joint, or ``None``."""
    if defaults is None:
        defaults = default_controllers(path)
    family = joint_type_for(handshake_name)
    if family is None:
        return None
    return defaults.get(family)
