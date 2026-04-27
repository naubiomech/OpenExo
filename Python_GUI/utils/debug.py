"""
Centralised debug-print helper.

Every diagnostic ``print()`` in the GUI now goes through :func:`dprint`,
so they can be silenced (default) or re-enabled in one place by flipping
:data:`DEBUG_PRINTS` to ``True``.

This is intentionally separate from the ``logging`` module: ``logger``
calls inside the codebase already route to the rotating log file via
``OpenExo.<module>`` — :func:`dprint` exists for the chatty stdout
traces we used during development that we want quiet by default but
trivial to bring back.
"""

from __future__ import annotations

import sys
from typing import Any

# Set to ``True`` to re-enable verbose stdout traces from the GUI
# (handshake parsing, controller-matrix info, scan/connect progress, …).
# Errors and data-rate stats are *not* gated by this flag — errors still
# go to the logger, and data-rate stats have been removed entirely.
DEBUG_PRINTS: bool = False


def dprint(*args: Any, **kwargs: Any) -> None:
    """``print``-compatible wrapper that no-ops when :data:`DEBUG_PRINTS` is False."""
    if DEBUG_PRINTS:
        print(*args, **kwargs)
        # Make traces show up promptly even when stdout is piped/buffered.
        try:
            sys.stdout.flush()
        except Exception:
            pass
