Adding New Sensors
==================

Overview
--------
In our codebase, sensor integration varies depending on the sensor. Below are examples of sensors that have been tested or integrated and are compatible with our platform:

- **Analog input**
  - Force sensitive resistor
  - Torque sensor (strain gauge)
  - Angle sensor (AS5600; hall effect)

- **Bus protocol**
  - I2C (Teensy-Nano communication)
  - UART (Teensy-Nano communication)
  - CAN (Teensy-Motor communication)
  - SPI (Teensy-SD Card interface)

Here, we demonstrate how the Teensy communicates with sensors using an example for an analog input sensor and another for the I2C protocol.

--------------------------------------------------

Analog Input
------------
For an analog sensor—such as the torque sensor used for exoskeleton torque feedback control—the fundamental operation uses ``analogRead()`` on the sensor’s signal pin to retrieve its value (or an amplified version):

.. code-block:: c++

   float current_reading = analogRead(_pin) * torque_calibration::AI_CNT_TO_V;

**Notes from PJRC:**
- None of the pins on the Teensy are 5-volt tolerant; the input voltage range is fixed at 0 to 3.3 volts.
- ``analogReference()`` has no effect on the Teensy 4.1.

*Sidenote:*  
If you need to use ``digitalRead()``, ensure you set the proper pin mode using ``pinMode()``, such as ``INPUT`` or ``INPUT_PULLUP``. Refer to the sensor or peripheral user manual for guidance.

**More examples on ``analogRead()``:**  
The following files utilize ``analogRead()``:
- ``AnkleAngles.cpp``
- ``FSR.cpp``
- ``TorqueSensor.cpp``

**Available Analog Pins:**  
If you are using our reference PCB, consult the PCB design documentation for the list of available analog pins. Stacking headers for Teensy 4.1 can also be used to improve access.

--------------------------------------------------

Bus Protocols
-------------
Unlike analog signals, bus protocols provide a highway for digital signals, enabling faster data transmission at higher frequencies.  
A comparison of the protocols used in our codebase can be found here:  
`I2C vs SPI vs UART <https://www.totalphase.com/blog/2021/12/i2c-vs-spi-vs-uart-introduction-and-comparison-similarities-differences/>`_

### I2C in Our Codebase—A Detailed Example
Noah Enlow has put together excellent documentation on I2C in our codebase. Below is a step-by-step example of sending float data over I2C using a union.

1. **Define the Union on the Sending Device**  
   This union allows you to access the same raw data as a float or as bytes.
   
   .. code-block:: c++

      union Data {
          float data_as_float;        // Float representation
          byte data_as_bytes[4];      // Byte vector (floats are 4 bytes)
      };

2. **Store the Data to Send**
   
   .. code-block:: c++

      Data.data_as_float = data_to_be_sent;

3. **Send Data via I2C Using the Wire Library**

   .. code-block:: c++

      void sendEvent() {
          Wire.write(Data.data_as_bytes, sizeof(Data.data_as_bytes));
      }

4. **Receiving the Data on the Primary Device**
   
   1. Define an identical union:
   
      .. code-block:: c++

         union Data {
             float data_as_float;
             byte data_as_bytes[4];
         };

   2. Request the data from the peripheral:
   
      .. code-block:: c++

         Wire.requestFrom(PERIPHERAL_ADDRESS, 4);
         for (int i = 0; i < 4; i++) {
             Data.data_as_bytes[i] = Wire.read();
         }

   3. Convert the received bytes back to a float:
   
      .. code-block:: c++

         float data_received = Data.data_as_float;

In summary, the process is:

float -> convert to bytes -> send over I2C -> receive as bytes -> convert back to float


**More Examples on Bus Protocols:**

- Files using **I2C**:
  - ``Battery.cpp``
  - ``I2CHandler.h``
  - ``RealTimeI2C.cpp``
  - ``ThIMU.h``

- Files using **SPI**:
  - ``ParamsFromSD.cpp``

- Files using **UART**:
  - ``ble_commands.h``
  - ``ComsMCU.cpp`` and ``ComsMCU.h``
  - ``ErrorReporter.h``
  - ``Exo.cpp``
  - ``uart_commands.h``
  - ``UARTHandler.cpp``

- Files using **CAN**:
  - ``CAN.h``
  - ``Motor.cpp``

**Available Pins for Bus Protocols:**  
Check your reference PCB documentation for the list of available pins. Stacking headers on the Teensy 4.1 can also improve access.

--------------------------------------------------

Integrating New Sensors into the Codebase
------------------------------------------
To ensure that sensor readings are accessible across the system, follow these procedures to embed your new sensor.

### Location – Where Should I Write the Sensor Code?
Due to the lack of a common sensor interface, decide where the sensor code best fits within the system architecture (e.g., exo, side, or joint level).

### Create the Sensor
Develop a sensor as its own class by creating corresponding ``.h`` and ``.cpp`` files in the **src** folder. Below is an outline for the sensor header file:

.. code-block:: c++

   class CLASS_NAME {
       public:
           // Constructor: may include a pin parameter if using an analog input
           CLASS_NAME(int pin);
           
           // Checks if the sensor needs calibration and returns whether calibration is complete
           bool calibrate(bool do_calibrate);
           
           // Reads the sensor and returns its value; for multiple returns, use pointers
           float read();
           
       private:
           // A sample private function (implementation will vary)
           void _sample_function();
           
           // The pin to which the sensor is connected
           int _pin;
           
           // A private variable for sensor data (example)
           float _sample_var;
   };

### Migrating to the Main Code

#### Board.h
For each board that will use the sensor, define the pin(s) that each sensor will use. For analog sensors, define four pins if four sensors are employed (adapt for other types as needed). Ensure that the chosen pin is free; planning your PCB layout beforehand facilitates this.

#### System Containing Sensor Data (.h)
Create variables in the appropriate data-holding class (e.g., ``SideData`` or ``JointData``) to store sensor readings and calibration status. For example:

.. code-block:: c++

   float sensor_reading;
   bool sensor_calibrate;

#### System Containing Sensor Data (.cpp)
In the constructor for the corresponding data class, initialize these variables (often to 0 or false):

.. code-block:: c++

   sensor_reading = 0;
   sensor_calibrate = false;

#### System Containing Sensor.h
If the sensor is at the side level, include the sensor header in ``Side.h``; if at the joint level, in ``Joint.h``. Then, declare an instance of your sensor class:

.. code-block:: c++

   #include "Sensor.h"
   // Within the class declaration:
   SensorClass new_instance_of_sensor;

#### System Containing Sensor.cpp
Within the corresponding ``.cpp`` file:
1. In the class constructor (using the initializer list), instantiate your sensor object. For example, for a joint level sensor:

   .. code-block:: c++

      Joint::Joint(config_defs::joint_id id, uint8_t* config_to_send)
         : motor(id, config_to_send)
         , controller(id, config_to_send)
         , new_instance_of_sensor(pin_to_use)  // Initialize the sensor instance
      {
         // Other initialization code...
      }

2. In the method that reads sensor data, call the sensor’s ``read()`` function to update the system data:

   .. code-block:: c++

      _joint_data->sensor_reading = new_instance_of_sensor.read();

3. Similarly, call the sensor’s ``calibrate()`` method to update the calibration state:

   .. code-block:: c++

      _joint_data->sensor_calibrate = new_instance_of_sensor.calibrate(_joint_data->sensor_calibrate);

This ensures that every time the joint (or side) runs, the sensor data is updated and, if needed, calibrated.

--------------------------------------------------

By following the above steps, you can integrate new sensors into the system. Adjust the code examples as necessary to match the specific requirements of your sensor and target hardware.
