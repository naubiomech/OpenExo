/**
 * @file error_codes.h
 * @author your name (you@domain.com)
 * @brief Error codes for the exoskeleton. These are used to index the error map.
 * 
 */

#ifndef ERROR_CODES_H
#define ERROR_CODES_H

enum ErrorCodes : int
{
    NO_ERROR = 0, //This must be the first element
    TEST_ERROR,
    POOR_STATE_VARIANCE_ERROR,
    POOR_TRANSMISSION_EFFICIENCY_ERROR,
    TORQUE_OUT_OF_BOUNDS_ERROR,
    TORQUE_VARIANCE_ERROR,
    FORCE_VARIANCE_ERROR,
    TRACKING_ERROR,
    MOTOR_TIMEOUT_ERROR,

    ERROR_CODE_LENGTH //This must be the last element
};

#endif //ERROR_CODES_H