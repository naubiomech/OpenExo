// *
 // * @file Battery.h
 // * @author Chancelor Cuddeback
 // * @brief Defines the battery interface and implements the child battery classes.
 // * @date 2023-07-18
 // * 


// #ifndef RCBATTERY_H
// #define RCBATTERY_H

// #include "I2CHandler.h"
// #if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)

// *
 // * @brief Abstract class that defines the battery interface
 // * 

// class _Battery 
// {
    // public:
        // Initialize any variables or peripherals
        // virtual void init() = 0;
        // Collect and return the parameter of interest
        // virtual float get_parameter() = 0;
// };

// *
 // * @brief Class that implements the smart battery interface. Based on the Inspired Energy batteries
 // * 

// class SmartBattery: public _Battery
// {
    // public:
        // void init();
        // float get_parameter();
    
    // private:
        // uint8_t data[i2c_cmds::smart::get_battery_voltage::len];
// };

// *
 // * @brief Class that implements the RC battery interface. Based on the basic LiPo batteries
 // * 

// class RCBattery: public _Battery
// {
    // public:
        // void init();
        // float get_parameter();

    // private:
        // uint8_t data[i2c_cmds::rc::get_battery_voltage::len];
        // const int BusLSB = 4; 
// };
// #endif
// #endif
