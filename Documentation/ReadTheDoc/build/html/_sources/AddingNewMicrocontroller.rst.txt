Adding a New Microcontroller
============================

This file assumes you are still using Arduino. If you are moving away from Arduino, you will need to remove Arduino-specific items (e.g., ``logger::prints``, pin definitions using ``A#``, etc.). Most of the code should run with little modification, but some reformatting may be necessary. If you are migrating away from Arduino, it is assumed that you have sufficient knowledge to handle those changes without further direction.

Selecting Microcontroller
-------------------------
You should confirm that the microcontroller includes the required elements. As of 20220517, these include:

**Controller:**
- SPI
- CAN
- 8 Analog Inputs
- 9 Digital outputs  
  - *Ideally with 3 PWM outputs for LED*
- 2 Digital inputs
- Floating Point Unit

**Coms:**
- I2C
- SPI
- Bluetooth Low Energy

**IMU:**
- Floating Point Unit

It is also advisable to check that there are existing Arduino libraries for the modules, ideally with identical interfaces. Note that the SPI peripheral in slave mode can be particularly challenging, as most SPI libraries assume the microcontroller is the controller (master).

Design Board
------------
- When designing the board, start from the current board as a template.
- In the schematic, place components close to where they are needed on the board—this is particularly important for capacitors and resistors, where proximity matters.
- For the board layout file, ensure those capacitors and resistors are placed correctly.
- For the CAN bus, place a restriction (or shield) above the traces to prevent capacitive coupling on the lines.

Board.h
-------
1. Create a new ``#define`` for your board, where you define both the board's name and an associated number.
2. Create a new ``#elif`` for the new board, defining the pins for your board.
   - Add a preprocessor condition such as ``#if defined([Microcontroller])`` so you can use pin names like ``A19`` without compiling errors when building for a different board.
3. Assign the pins based on your board design—use existing boards as a template.

config.ini
----------
1. Add your board and its version to the file.

ParseIni.h
----------
1. In the ``config_defs`` namespace, add your board name and board version.
2. For the controller board, ensure that the ``std::map`` is available.
3. In the ``config_map`` namespace, add the mapping for your board and version.

Other Files
-----------
This section gets more involved. You will need to update multiple files with preprocessor directives and specific includes to ensure that the code compiles correctly for your microcontroller.

1. **Preprocessor Defines:**  
   Go through each file and add preprocessor defines around the areas that should compile for your microcontroller.
   - For the **controller micro**, add your micro in a condition similar to:
     
     .. code-block:: c++
     
        #if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)
     
   - For the **coms micro**, add your micro in a condition like:
     
     .. code-block:: c++
     
        #if defined(ARDUINO_ARDUINO_NANO33BLE)
     
   - In some cases, you may need to add specific includes (for example, include SPI in ``ExoCode.ino``).

2. **Determining the Correct Microcontroller Name for Defines:**  
   - Navigate to your Arduino directory: ``arduino/hardware/[type]/boards.txt``.
   - Locate your microcontroller and look for an entry like ``*.board.name=[name]``.
   - In your define, prepend the obtained board name with ``ARDUINO_``.
     - For example, if the board name is ``TEENSY41`` (for Teensy 4.1), then you will use ``ARDUINO_TEENSY41``.
     - Note: The Nano 33 appears as ``ARDUINO_NANO33BLE``, so the corresponding define will be ``ARDUINO_ARDUINO_NANO33BLE``.  
       **Be careful** to follow the naming conventions exactly to ensure consistency.
