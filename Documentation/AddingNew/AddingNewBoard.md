# Adding a New Board

## Config.h
1. Add a the name of your new board using a define at the top of the file
```
#define AK_Board_V0_4 3
```
2. Change the current board to your board if you are wanting to use it now.  This is a good idea to make sure all your changes work.

```
#define BOARD_VERSION AK_Board_V0_4 3
```

## ParseIni.h
Note: this is not currently used but should be added for future proofing.
1. In config_def::board_name add your board name if different from the existing ones
2. In config_def::board_version add your board version if not currently present
3. In config_map::board_name add the link to how the new board name should be referenced in the ini file.
4. In config_map::board_version add the link to how the new version should be referenced in the ini file.

## Board.h
1. Use the ```#if defined(YOUR_BOARD_NAME_AND_VERSION)``` to define the connections on your board
2. Following the format of the existing boards assign the pins and behavior of different components of the micro controller
    - There should be a logic_micro_pins and a coms_micro_pins namespace.
    - Do not stray to far from the existing structure as this may cause things to break.
    
## Done!!