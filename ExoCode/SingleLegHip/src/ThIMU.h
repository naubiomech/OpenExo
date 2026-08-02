#ifndef THIMU_H
#define THIMU_H

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

#include <Arduino.h>
#include <Wire.h>
#include "I2CHandler.h" //Just used for register and address definitions




class ThIMU
{
    public:
    ThIMU(bool is_left);
    bool init(float timeout = 500); //Timeout in ms
    float read_data();

    private:
    bool _handshake();
    bool _is_left;
    bool _is_initialized;
};

#endif
#endif