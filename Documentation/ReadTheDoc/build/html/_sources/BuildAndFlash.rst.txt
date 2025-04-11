Build And Flash
===============

Downloads
---------
1. **Download Arduino IDE 1.8.19:**  
   - Available `here <https://www.arduino.cc/en/software>`_  
   - or `here <https://drive.google.com/drive/folders/1IRxJFNm2gxUtCeU8Dcavg_Mv2ubANOHK?usp=drive_link>`_.

2. **Install Teensyduino:**  
   - Download it `here <https://www.pjrc.com/teensy/td_download.html>`_  
   - or `here <https://drive.google.com/drive/folders/1IRxJFNm2gxUtCeU8Dcavg_Mv2ubANOHK?usp=drive_link>`_.
   - **Note:** Install **version 1.56** for your system.

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
