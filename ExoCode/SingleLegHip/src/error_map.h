/**
 * @file error_map.h
 * @author Chancelor Cuddeback
 * @brief Maps error codes to error types

 */

#ifndef ERROR_MAPS_H
#define ERROR_MAPS_H
#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)

#include "error_codes.h"
#include "error_types.h"
#include <map>

const std::map<ErrorCodes, ErrorType*> error_map = {
    {TEST_ERROR, new TestError()},
    {POOR_STATE_VARIANCE_ERROR, new PoorStateVarianceError()},
    {POOR_TRANSMISSION_EFFICIENCY_ERROR, new PoorTransmissionEfficiencyError()},
    {TORQUE_OUT_OF_BOUNDS_ERROR, new TorqueOutOfBoundsError()},
    {TORQUE_VARIANCE_ERROR, new TorqueVarianceError()},
    {FORCE_VARIANCE_ERROR, new ForceVarianceError()},
    {TRACKING_ERROR, new TrackingError()},
    {MOTOR_TIMEOUT_ERROR, new MotorTimeoutError()}
};

#endif
#endif // ERROR_MAPS_H