/**
 * @file AnkleAngles.h
 * @author Chancelor Cuddeback
 * @brief Reads the ASD5600 angle sensor on the ankle joint.
 * @date 2023-07-18
 * 
 */

#ifndef ANKLE_ANGLE_H
#define ANKLE_ANGLE_H

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

#include <Arduino.h>
#include "Board.h"

/**
 * @brief Class to interface with a microcontroller that samples angle data.
 * 
 */
class AnkleAngles
{
    public:
    AnkleAngles() {}
    
    // AnkleAngles(AnkleAngles& other) = delete;
    // void operator=(const AnkleAngles&) = delete;
    // static AnkleAngles* GetInstance();
    
    /**
     * @brief Initialize the ankle sensor, and perform a handshake.
     * 
     * @param timeout Timeout before giving up on handshake in ms
     * @return true 
     * @return false 
     */
    bool init(bool is_left);
    
    /**
     * @brief Get the left or right angle. Returns a ratiometric value [0, 1]
     * 
     * @param return_normalized true to return normalized angle data, based on moving peak detection
     * @return float 
     */
    float get(bool is_left, bool return_normalized = false);

    private:
    float _update_population_statistics(const float new_value);
    float _max_average = 0.01;
    float _min_average = 0.99;
    float _mean = 0;
    const float _ema_alpha = 0.0001;
    const float _max_min_delta = 0.00001;

    bool _left = false;
    
    /**
     * @brief Check if the ankle sensor is initialized.
     * 
     */
    bool _is_initialized = false;
    
    /**
     * @brief Right analog pin
     * 
     */
    const int _right_pin = logic_micro_pins::right_ankle_angle_pin;
    
    /**
     * @brief Left analog pin
     * 
     */
    const int _left_pin = logic_micro_pins::left_ankle_angle_pin;
};


#endif
#endif