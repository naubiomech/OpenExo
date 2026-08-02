/**
 * @file ComsLed.h
 * @author Chance Cuddeback
 * @brief This class is used to control the LED on the coms board. It uses the singleton pattern.
 * 
 */

#ifndef ComsLed_h
#define ComsLed_h

#if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)

#include "Arduino.h"

class ComsLed 
{
    public:
    /**
     * @brief Get the instance object
     * 
     * @return ComsLed* A reference to the singleton
     */
    static ComsLed* get_instance();

    /**
     * @brief Set the LED to a specific color
     * 
     * @param r The red value
     * @param g The green value
     * @param b The blue value
     */
    void set_color(uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Get the color of the LED
     * 
     */
    void get_color(uint8_t* r, uint8_t* g, uint8_t* b);

    /**
     * @brief Pulse the LED at a specific frequency
     * 
     */
    void life_pulse();


    private:
    /**
     * @brief Class constructor
     * 
     */
    ComsLed();

    /**
     * @brief Handle active low pins
     * 
     * @param value The value to handle
     * @return uint8_t The value to write to the pin
     */
    uint8_t _handle_active_low(uint8_t value);

    /**
     * @brief The red value
     * 
     */
    uint8_t r;

    /**
     * @brief The green value
     * 
     */
    uint8_t g;

    /**
     * @brief The blue value
     * 
     */
    uint8_t b;

    /**
     * @brief The number of times the life pulse has been called
     * 
     */
    int life_pulse_counter = 0;

    /**
     * @brief The frequency of the life pulse
     * 
     */
    const int life_pulse_frequency = 100;
};

#endif
#endif