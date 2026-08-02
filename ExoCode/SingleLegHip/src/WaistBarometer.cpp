#if defined(ARDUINO_ARDUINO_NANO33BLE)

#include "WaistBarometer.h"
#include "Logger.h"
#include <Arduino_LPS22HB.h>


WaistBarometer::WaistBarometer(pressure_unit_t desired_unit)
{
    _unit = desired_unit;
    if (!BARO.begin()) {
        //logger::println("Failed to initialize pressure sensor!");
    }
}

float WaistBarometer::getPressure()
{
    float pressure = BARO.readPressure();
    if (_unit == unit_Pa)
    {
        pressure *= 1000;
    }
    //TODO: Conversion for other units
    return pressure;
}

#endif