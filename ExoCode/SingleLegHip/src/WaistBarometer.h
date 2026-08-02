#ifndef WAISTBAROMETER_H
#define WAISTBAROMETER_H

#if defined(ARDUINO_ARDUINO_NANO33BLE)

/**
 * @file WaistBarometer.h
 * @author Chance Cuddeback
 * @brief Uses the Arduino Nano 33 BLE Barometer to read pressure
 * @date 2022-10-26
 * 
 */

#include "Arduino.h"

typedef enum
{
    unit_PSI = 0,
    unit_MBAR = 1,
    unit_kPa = 2,
    unit_Pa = 3,
    pressure_unit_LENGTH
} pressure_unit_t;

class WaistBarometer
{
    public:
        /**
         * @brief Construct a new Waist Barometer object
         * 
         */
        WaistBarometer(pressure_unit_t desired_unit = unit_Pa);

        /**
         * @brief Get the Pressure object
         * 
         * @return float The pressure, units defined in Constructor
         */
        float getPressure();

    private:

        /**
         * @brief The unit of pressure to return
         * 
         */
        pressure_unit_t _unit;
};

#endif // ARDUINO_ARDUINO_NANO33BLE
#endif // WAISTBAROMETER_H