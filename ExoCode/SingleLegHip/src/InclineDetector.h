#ifndef INCLINEDETECTOR_H
#define INCLINEDETECTOR_H

#if defined(ARDUINO_ARDUINO_NANO33BLE)
/**
 * @file InclineDetector.h
 * @author Chance Cuddeback
 * @brief Uses pressure readings to estimate the incline of the terraint
 * @date 2022-10-26
 * 
 */

#include "Arduino.h"

//Average stair ascent should be ~0.006 Pa/sample at 500 Hz
#define DEFAULT_ALPHA 0.01f
#define SMOOTH_ALPHA 0.025f
#define DEFAULT_THRESHOLD 1.5f
#define INCLINE_DELTA_MS 10 // ms

typedef enum
{
    Incline = 0,
    Decline = 1,
    Level = 2,
    incline_state_LENGTH
} incline_state_t;

class InclineDetector
{
    public:
        /**
         * @brief Construct a new Incline Detector object
         * 
         * @param float Alpha for the moving average filter
         */
        InclineDetector(float alpha = DEFAULT_ALPHA, float threshold = DEFAULT_THRESHOLD);

        /**
         * @brief Handle a new pressure reading, and return the current state
         * 
         * @param pressure Units may vary, currently tuned for Pa TODO: Handle various units in tuning
         * @return incline_state_t 
         */
        incline_state_t run(float pressure);

        /**
         * @brief Get the Incline State object
         * 
         * @return incline_state_t The state of the incline
         */
        incline_state_t getInclineState();

        void setAlpha(float alpha);
        void setThreshold(float threshold);

    private:

        /**
         * @brief The current state of the incline
         * 
         */
        incline_state_t _state;

        /**
         * @brief Average pressure reading
         * 
         */
        float _avgPressure;

        /**
         * @brief Alpha for moving average filter
         * 
         */
        float _alpha;
        /**
         * @brief Threshold for determining if the incline is level. Symetric about average
         * 
         */
        float _threshold;

        /**
         * @brief Signal used to determine incline
         * 
         */
        float _smooth_demeanedPressure;

        float _smooth_alpha;
};

#endif // ARDUINO_ARDUINO_NANO33BLE
#endif // INCLINEDETECTOR_H