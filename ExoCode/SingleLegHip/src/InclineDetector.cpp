#if defined(ARDUINO_ARDUINO_NANO33BLE)

#include "InclineDetector.h"
#include "Utilities.h"
#include "Logger.h"

#define INCLINE_DETECTOR_DEBUG 0

InclineDetector::InclineDetector(float alpha, float threshold)
{
    #if INCLINE_DETECTOR_DEBUG
        logger::println("InclineDetector::InclineDetector()");
        logger::print("alpha: ");
        logger::println(alpha);
        logger::print("threshold: ");
        logger::println(threshold);
    #endif

    _alpha = alpha;
    _threshold = threshold;
    _avgPressure = -1;
    _state = Level;
    _smooth_demeanedPressure = 0;
}

incline_state_t InclineDetector::run(float pressure)
{
    if (_avgPressure == -1)
    {
        _avgPressure = pressure;
    }
    else
    {
        _avgPressure = utils::ewma(pressure, _avgPressure, _alpha);
    }
    float demeanedPressure = pressure - _avgPressure;

    _smooth_demeanedPressure = utils::ewma(demeanedPressure, _smooth_demeanedPressure, _alpha);

    if (_smooth_demeanedPressure > _threshold)
    {
        _state = Decline;
    }
    else if (_smooth_demeanedPressure < -_threshold)
    {
        _state = Incline;
    }
    else
    {
        _state = Level;
    }

    #if INCLINE_DETECTOR_DEBUG
        logger::print("Pressure: ");
        logger::print(_smooth_demeanedPressure);
        logger::print("\t");
        logger::print("High: ");
        logger::print(_threshold);
        logger::print("\t");
        logger::print("Low: ");
        logger::println(-_threshold);
    #endif

    return _state;
}

incline_state_t InclineDetector::getInclineState()
{
    return _state;
}

void InclineDetector::setAlpha(float alpha)
{
    _alpha = alpha;
}

void InclineDetector::setThreshold(float threshold)
{
    _threshold = threshold;
}

#endif