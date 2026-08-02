// #include "Battery.h"
// #include "Arduino.h"
// #include <Wire.h>
// #include "Logger.h"
// #if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)

// #define BATTERY_DEBUG 0

//Note: Our Current Version of the PCB does not contain the necessary components to read battery voltage, future iterations will re-add this component

// void SmartBattery::init() {;}
// float SmartBattery::get_parameter()
// {
    // #if REAL_TIME_I2C
    // return 0;
    // #endif
     //I2C* instance = I2C::get_instance();
     //instance->read_i2c(data, i2c_cmds::smart::get_battery_voltage::addr, i2c_cmds::smart::get_battery_voltage::reg, i2c_cmds::smart::get_battery_voltage::len);
     //uint8_t voltage = (data[0] << 8) | data[1];
     //voltage = voltage >> 3;
     //return float(voltage);
// }

// void RCBattery::init() 
// {
    // #if REAL_TIME_I2C
    // return;
    // #endif
     //Wire.begin();
     //Wire.beginTransmission(i2c_cmds::rc::calibrate::addr);
     //Wire.write(i2c_cmds::rc::calibrate::reg);
     //Wire.write(i2c_cmds::rc::calibrate::val); // Calibration value: Cal = 0.04096/(CurrentLSB*ShuntResistance) 
     //Wire.endTransmission();

     //I2C* instance = I2C::get_instance();
     //instance->write_i2c(i2c_cmds::rc::calibrate::addr, i2c_cmds::rc::calibrate::reg, i2c_cmds::rc::calibrate::val);
// }
// float RCBattery::get_parameter()
// {
    // #if REAL_TIME_I2C
    // return 0;
    // #endif
     ////Battery Voltage, could get shunt voltage and calculate current for funsies
     //int data[i2c_cmds::rc::get_battery_voltage::len];
     //Wire.beginTransmission(i2c_cmds::rc::get_battery_voltage::addr);
     //Wire.write(i2c_cmds::rc::get_battery_voltage::reg);
     //Wire.endTransmission();
     //Wire.requestFrom(i2c_cmds::rc::get_battery_voltage::addr, i2c_cmds::rc::get_battery_voltage::len, true);
     //data[0] = Wire.read();
     //data[1] = Wire.read();
     //Wire.endTransmission();
     //I2C* instance = I2C::get_instance();
     //instance->read_i2c(data, i2c_cmds::rc::get_battery_voltage::addr, i2c_cmds::rc::get_battery_voltage::reg, i2c_cmds::rc::get_battery_voltage::len);
    
     //#if BATTERY_DEBUG
     //logger::print("RCBattery::get_parameter->Raw Data: ");
     //logger::print(data[0], HEX);
     //logger::print(" ");
     //logger::println(data[1], HEX);
     //#endif

     //uint8_t voltage = (data[0] << 8) | data[1];
     //voltage = voltage >> 3;
     //voltage = voltage * BusLSB * 10000;

     //#if BATTERY_DEBUG
     //logger::print("RCBattery::get_parameter->Battery Voltage: ");
     //logger::println(voltage);
     //#endif

     //return float(voltage);
// }
// #endif