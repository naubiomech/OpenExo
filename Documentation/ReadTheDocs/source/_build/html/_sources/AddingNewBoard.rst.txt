Adding a New Board
==================

Config.h
--------
1. Add the name of your new board using a ``#define`` at the top of the file:

   .. code-block:: c

      #define AK_Board_V0_4 3

2. Change the current board to your new board if you intend to use it immediately.  
   This helps ensure all changes function properly.

ParseIni.h
----------
.. note::
   This file is not currently used but **should be updated for future-proofing**.

1. In ``config_def::board_name``, add your board’s name if it’s different from existing ones.
2. In ``config_def::board_version``, add your board version if it’s not currently present.
3. In ``config_map::board_name``, specify how the new board name should be referenced in the INI file.
4. In ``config_map::board_version``, specify how the new version should be referenced in the INI file.

Board.h
-------
1. Use an ``#if defined(YOUR_BOARD_NAME_AND_VERSION)`` block to define the connections on your board.
2. Following the format of existing boards, assign pins and behavior for your microcontroller components:

   - You should have both a ``logic_micro_pins`` namespace and a ``coms_micro_pins`` namespace.
   - Avoid straying too far from the existing structure to prevent potential issues.

Done!!
------
Once you've updated these files, your new board should be recognized and configurable within the system.
