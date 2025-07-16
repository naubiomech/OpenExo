Build And Flash
===============

Downloads
---------
1. **Download Arduino IDE:**

   - Available on the Arduino website: `Arduino Software <https://www.arduino.cc/en/software>`_  

2. **Install Teensyduino:**

   - Download it on the PJRC website: `Teensyduino <https://www.pjrc.com/teensy/td_download.html>`_  

3. **Install the Arduino Nano 33 BLE:**

   - This can be done through the Arduino Boards Manager.

4. **Install the Arduino libraries:**

   - Follow the instructions provided in the :ref:`libraries` section below.

Flashing
--------
Once you have all the tools installed, follow these steps to flash your device:

1. Connect your computer to the Teensy using a micro-USB cable.
2. Change the Board to **Teensy 4.1**.
3. Change the Arduino port to use the Teensyduino port.  
   *The correct port will display the board name next to the COM port (Windows only).*
4. Ensure the Board Version is correctly set in the **Config.h** file.
5. Press the **Arrow** button on the Arduino IDE to upload the code to the Teensy.
6. Change the Board to **Arduino Nano 33 BLE**.
7. Change the Arduino port as described in step 3.
8. Press the **Upload** button.  
   *Note: The Nano will take significantly longer to build than the Teensy.*

If this is your first time, we recommend our `First Time Startup Guide <https://theopenexo.readthedocs.io/en/latest/FirstTimeStartUp.html`__,
which, in detail, walks you through downloading the relevant software, setting up the system, and running the system for the first time.
It also outlines potential error handling approaches to aid in troubleshooting should the system not work.

.. _libraries:

Libraries
---------
All files within the **libraries** directory of this codebase should be copied to:

.. code-block:: python

   C:\Users\[USER]\Documents\Arduino\libraries\

This will "install" the libraries on your system.  
Details on the libraries can be found in the `Libraries Folder <https://github.com/naubiomech/OpenExo/tree/main/Libraries>`_.

Arduino Boards Manager
----------------------
To open the boards manager, press:

.. code-block:: python

   Tools -> Board -> Boards Manager
