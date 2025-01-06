# Adding New Sensors

## Overview
In our codebase, sensor integration varies depending on the sensor. Below, we list some of the sensors we've tested or integrated that are compatible with our codebase:
- **Analog input**
    - Force sensitive resistor
    - Torque sensor (strain gauge)
    - Angle sensor (AS5600; hall effect)
- **Bus protocol**
    - I2C (Teensy-Nano communication)
    - UART (Teensy-Nano communication)
    - CAN (Teensy-Motor communication)
    - SPI (Teensy-SD Card interface)

Here, we'll demonstrate how the Teensy communicates with sensors using an example of an analog input sensor and another example with the I2C protocol.

---

## Analog input
For an analog sensor, such as the torque sensor we used for exoskeleton torque feedback control, the fundamental operation involves using `analogRead()` on the signal pin to get the sensor reading (or its amplified value):

```
float current_reading =  analogRead(_pin)*torque_calibration::AI_CNT_TO_V;
```

### Notes from PJRC:
- None of the pins on the Teensy are 5-volt tolerate, and the input voltage range is fixed at 0 to 3.3 volts.
- `analogReference()` has no effect on Teensy 4.1

*Sidenote*: If you need to use `digitalRead()`, make sure to set the appropriate pin mode using `pinMode()` with values such as `INPUT` or `INPUT_PULLUP`. Refer to the user manual of the sensor or peripheral in use.

### More examples on `analogRead()`
The following `.cpp` files utilize `analogRead()`:
- `AnkleAngles.cpp`
- `FSR.cpp`
- `TorqueSensor.cpp`

### Available analog pins
If you're using our reference printed circuit boards (PCB), check the PCB design documentation for the list of available analog pins. You can always use stacking headers for Teensy 4.1 to make certain pins easier to access.

---

## Bus protocols
Unlike data links that use analog signals, a bus acts as a highway for digital signals carrying encoded messages, enabling much faster data transmissions at higher frequencies. You can find a comparison of some of the bus protocols used in our codebase [here](https://www.totalphase.com/blog/2021/12/i2c-vs-spi-vs-uart-introduction-and-comparison-similarities-differences/ "I2C vs SPI vs UART").

### I2C in our codebase—A detailed example
Noah Enlow put together some great documentation detailing how I2C works in our codebase, and we're excited to share it here.

<details open>

  <summary><strong>Click to fold</strong></summary>

- I2C works by sending a series of bytes that represent the information you're wanting to send. The Wire library in arduino does not automatically interpret the bytes being sent/recieved into the intended data type (e.g. float or int). Therefore, in order to send data over I2C with the wire library, the data must be converted into its respective representation in bytes. The simplest and most efficient way to do this is to use a union. 

- Unions in arduino (and C++) function similar to structs. However, with unions, the data stored within is stored in a single location in memory and can be accessed as any data type desired (such as float or int). Thus, a union allows you to access the same raw data in multiple ways. 

- This is useful for sending data over I2C because now we can collect our data from a sensor (say float data from an accelerometer), store it in a union, and then access that same data as bytes when we're ready to send it via I2C. On the receiving device, we essentially do the opposite. We receive the incoming bytes into a union, and then access the data as floats again.
The scheme is:
```
    float -> convert to bytes -> send over I2C -> receive as bytes -> convert back to float
```
- Below is a demonstration of how to send float data over I2C using a union (the process is identical for other data types such as ints or doubles):




    1. First define the union: 
    ```
        union Data                      # Data is the name of the union
        {
            float data_as_float;        # Representation of the data as a float
            byte data_as_bytes[4];      # Represntation of the data as a vector of bytes (must specify the size, floats are 4 bytes)
        };
    ```

    2. Now store the data you'll be sending:
    ```
        Data.data_as_float = data_to_be_sent;	# data_to_be_sent is a dummy variable representing the data you want to send
    ```

    3. Send via I2C using the Arduino Wire library:
    ```
        void sendEvent()                         # sendEvent() is called in your main loop whenever you are ready to send the data
        {													
            Wire.write(Data.data_as_bytes, sizeof(Data.data_as_bytes));
        }
    ```

- Now to recieve the data that has been sent. We'll create an identical union on the device requesting the data (primary) to make it more intelligible, however the union need not be identical.
    1. Define a union on the device receiving the data:
    ```
        union Data                      # Data is the name of the union
        {                	
            float data_as_float;        # Representation of the data as a float
            byte data_as_bytes[4];      # Representation of the data as a vector of bytes (must specify the size, floats are 4 bytes)
        };
    ```

    2. Now request the data from the peripheral (secondary) device, which in this case is the one doing the sending. Because the data is coming in as individual bytes, we'll need a for loop to iterate through those bytes and append them to the vector of bytes contained in the union we just defined.
    ```
        Wire.requestFrom(PERIPHERAL_ADDRESS, 4);
        for(int i=0; i<4; i++)
        {
            Data.data_as_bytes[i] = Wire.read();
        }
    ```
    3. Now to access that data as a float again:
    ```
        float data_recieved = Data.data_as_float;
    ```

- In summary, a union was created on our peripheral device (the device sending) to store the float data and send it as bytes over I2C. Another identical union was created on our primary device, which in this case is the one we want to recieve the data. A for loop is used on the primary to iteratively append the bytes being recieved to the byte-type variable in the union. Once this is done, the data recieved can then be accesed as a float again through the union.


</details>


---

### More examples on bus protocols
The following `.cpp` or `.h` files utilize I2C:
- `Battery.cpp`
- `I2CHandler.h`
- `RealTimeI2C.cpp`
- `ThIMU.h`

The following `.cpp` file utilize SPI:
- `ParamsFromSD.cpp`

The following `.cpp` or `.h` files utilize UART:
- `ble_commands.h`
- `ComsMCU.cpp` and `ComsMCU.h`
- `ErrorReporter.h`
- `Exo.cpp`
- `uart_commands.h`
- `UARTHandler.cpp`

The following `.cpp` or `.h` files utilize CAN:
- `CAN.h`
- `Motor.cpp`

### Available pins for bus protocols
If you're using our reference printed circuit boards (PCB), check the PCB design documentation for the list of available pins for bus protocols. You can always use stacking headers with Teensy 4.1 to make certain pins easier to access.

---
To ensure a sensor readings are accessible from various locations, we've created the following procedures to help you embed your new sensor into the codebase.
## Location—Where should I write the sensor code?
Because the sensors don't have a common interface it is a bit more straight forward to add new ones.
The main things you will need to decide is what part of the system makes the most sense for the part to live, e.g. exo, side, joint.

## Create the Sensor
The sensor should be its own class.
You will need to create a .h and .cpp file in the src folder.
The sensor class will have all the functions that you need to use the class, below you can find an example to build from.
This example is an outline of what would be in the header(.h) file.

```
class CLASS_NAME
{
    /*
     * public functions are ones that can be called from outside the class.
     */
    public: 
        /*
         * Constuctor for the class
         */
        CLASS_NAME(int pin);// you may not need the pin if it doesn't use an analog input.
        
        /*
         * Checks if the sensor needs to be calibrated and returns if the calibration is done.
         * This should be called every time the containing system is run.
         * Depending on your system you may not need to calibrate.
         */
        bool calibrate(bool do_calibrate);
        
        /*
         * Used to read the sensor and returns the value.
         * If you need multiple returns you will need to pass the place the date will go by pointer: 
         * void read(float* val0, float* val1);
         * Where it will be called using:
         * read(&place_to_store_val0, &place_to_store_val1);
         */
         float read();
    /*
     * Private functions and variables can only be called from within the class.
     */
    private:
        
        /* 
         * Sample private function this will be very dependent on what the system needs.
         */
        void _sample_function();
        
        // The pin the sensor is connected to.
        int _pin;
        
        // sample private variable for the sensor.
        float _sample_var;
};
```

## Migrating to the Main Code.

### Board.h
For each board that will use the sensor add the pin that each sensor will use.
If there are four sensors you will need to define 4 pins.
This assumes it is an analog sensor, if you are using a different type of sensor you can define the things that sensor needs here.
Make sure that that pin is free, this should be easier if you design the PCB first.

### System Containing Sensor Data .h
Create a variable to store the the data from the sensor.
If the sensor will be at the side level this will be in the SideData class, if it is the joint the JointData class, etc.
Additionally if the sensor will need to be calibrated make a variable to store that state;
```
float sensor_reading;
bool sensor_calibrate;
```

### System Containing Sensor Data .cpp
In the constructor related the the .h file you just modified, initialize the value to to what you want it to start at, this will often be 0.

```
sensor_reading = 0;
sensor_calibrate = false;
```

### System Containing Sensor.h
This will be Side.h if the sensor is at the side level or Joint.h if it is at the joint level, etc.
Add the include for the sensor, ```#include Sensor.h```, that you created.
Within the class; Side, Joint, etc.; declare an instance of the class ```SensorClass new_instance_of_sensor;```
Create as many instances as you need.

### System Containing Sensor.cpp
1. Within the .cpp file related to the .h file you just changed you need to call the constructor for the instances of the class you just declared.
This will be done in the initializer list.
For example for a joint level sensor:
```
Joint::Joint(config_defs::joint_id id, uint8_t* config_to_send)
: motor(id, config_to_send)
, controller(id, config_to_send)
, new_instance_of_sensor(pin_to_use)
{
```

2. Add a call to the sensor.read() in the containing classes read referencing the sensor data we already made.
For a joint level sensor:
``` 
_joint_data->sensor_reading = new_instance_of_sensor.read();
```

3. Add a call to the sensor.calibrate(do_calibrate) in the containing classes read referencing the sensor data we already made.
For a joint level sensor:
``` 
_joint_data->sensor_calibrate = new_instance_of_sensor.calibrate(_joint_data->sensor_calibrate);
```

This will get called every time the joint is run and if the calibrate variable is set to true will calibrate the sensor.
