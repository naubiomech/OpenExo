/**
 * @file StatusLed.h
 *
 * @brief Class to set an RGB LED to different colors based on the state of the system
 * 
 * Constructor: StatusLed(int r_pin, int g_pin, int b_pin) or (int r_pin, int g_pin, int b_pin, int brightness)
 *   The pins are the RGB LED pins ideally they are PWM but can also handle simple digital pins.
 *   In the header set NO_PWM to true or false depending on if you have PWM or simple digital pins.
 *   Brightness sets the brightness from 255 to 0, this is ignored for simple digital pins
 *   
 * updateLed(int message) method sets the color of the LED, these messages can be found in the preprocessor part of the header.
 * setBrightness(int brightness) method is used to change the brightness after initialization.
 * 
 * @author P. Stegall 
 * @date Dec. 2021
*/

#ifndef StatusLed_h
#define StatusLed_h

#include "Arduino.h"
#include "Board.h"
#include "StatusDefs.h"

#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)
#include <map>

/**
 * @brief stores the mapping between error messages and the LED display
 */
namespace status_led_defs
{
    typedef std::map<uint16_t, uint16_t> IdxRemap; /**< Type to map an error message to a display type */
    
    /**< Maps messages to idx used for color and pattern. Mainly needed so errors and warnings will look the same.*/
    const IdxRemap status_led_idx = 
    { 
        {status_defs::messages::off, 0},
        {status_defs::messages::trial_off, 1},    
        {status_defs::messages::trial_on, 2},
        {status_defs::messages::test, 3},  
        {status_defs::messages::torque_calibration, 4},
        {status_defs::messages::fsr_calibration, 5},    
        {status_defs::messages::fsr_refinement, 6},   
        {status_defs::messages::motor_start_up, 7},
        
        {status_defs::messages::error, 8}, 
        {status_defs::messages::error_left_heel_fsr, 8},
        {status_defs::messages::error_left_toe, 8},    
        {status_defs::messages::error_right_heel_fsr, 8},
        {status_defs::messages::error_right_toe_fsr, 8},
        {status_defs::messages::error_left_hip_torque_sensor, 8},
        {status_defs::messages::error_left_knee_torque_sensor, 8},
        {status_defs::messages::error_left_ankle_torque_sensor, 8},
        {status_defs::messages::error_left_elbow_torque_sensor, 8},
        {status_defs::messages::error_right_hip_torque_sensor, 8},
        {status_defs::messages::error_right_knee_torque_sensor, 8},
        {status_defs::messages::error_right_ankle_torque_sensor, 8},
        {status_defs::messages::error_right_elbow_torque_sensor, 8},
        {status_defs::messages::error_left_hip_motor, 8},
        {status_defs::messages::error_left_knee_motor, 8},
        {status_defs::messages::error_left_ankle_motor, 8},
        {status_defs::messages::error_left_elbow_motor, 8},
        {status_defs::messages::error_right_hip_motor, 8},
        {status_defs::messages::error_right_knee_motor, 8},
        {status_defs::messages::error_right_ankle_motor, 8},
        {status_defs::messages::error_right_elbow_motor, 8},
        {status_defs::messages::error_left_hip_controller, 8},
        {status_defs::messages::error_left_knee_controller, 8},
        {status_defs::messages::error_left_ankle_controller, 8},
        {status_defs::messages::error_left_elbow_controller, 8},
        {status_defs::messages::error_right_hip_controller, 8},
        {status_defs::messages::error_right_knee_controller, 8},
        {status_defs::messages::error_right_ankle_controller, 8},
        {status_defs::messages::error_right_elbow_controller, 8},
        {status_defs::messages::error_to_be_used_1, 8},
        {status_defs::messages::error_to_be_used_2, 8},
        {status_defs::messages::error_to_be_used_3, 8},
        {status_defs::messages::error_to_be_used_4, 8},
        {status_defs::messages::error_to_be_used_5, 8},
        {status_defs::messages::error_to_be_used_6, 8},
        {status_defs::messages::error_to_be_used_7, 8},
        
        {status_defs::messages::warning, 9},
        {status_defs::messages::warning_exo_run_time, 9}, 
        {status_defs::messages::warning_to_be_used_1, 9}, 
        {status_defs::messages::warning_to_be_used_2, 9}, 
        {status_defs::messages::warning_to_be_used_3, 9}, 
        {status_defs::messages::warning_to_be_used_4, 9}, 
        {status_defs::messages::warning_to_be_used_5, 9}, 
        {status_defs::messages::warning_to_be_used_6, 9}, 
        {status_defs::messages::warning_to_be_used_7, 9}, 
        {status_defs::messages::warning_to_be_used_8, 9}, 
        {status_defs::messages::warning_to_be_used_9, 9}, 
        {status_defs::messages::warning_to_be_used_10, 9}, 
        {status_defs::messages::warning_to_be_used_11, 9}, 
        {status_defs::messages::warning_to_be_used_12, 9}, 
        {status_defs::messages::warning_to_be_used_13, 9}, 
        {status_defs::messages::warning_to_be_used_14, 9},
    }; 
}
#endif

//Define the on and off state of the LED. This is handy for if you are using a P Channel MOSFET where low is on.
//#define STATUS_LED_ON_STATE 0
//#define STATUS_LED_OFF_STATE 255

//Color assumes 255 is on. The code will use the on/off state above to compensate for the code.
//#define STATUS_MESSAGE_LED_OFF 0              //Set the message index
//#define STATUS_COLOR_LED_OFF {0, 0, 0}        //Set the color in {R, G, B} format 0-255

//#define STATUS_MESSAGE_TRIAL_OFF 1            //Set the message index
//#define STATUS_COLOR_TRIAL_OFF {0, 0, 255}    //Set the color in {R, G, B} format 0-255 
    
//#define STATUS_MESSAGE_TRIAL_ON 2             //Set the message index
//#define STATUS_COLOR_TRIAL_ON {0, 255, 0}     //Set the color in {R, G, B} format 0-255

//#define STATUS_MESSAGE_ERROR 3                //Set the message index
//#define STATUS_COLOR_ERROR {255, 0, 0}        //Set the color in {R, G, B} format 0-255

//#define NO_PWM true //True if using simple digital pins, false if using pwm pins

/**
 * @brief define how each message will be displayed.
 */
namespace status_led_defs
{   
    /**
     * @brief colors each message should display
     */
    namespace colors //Just used namespace due to the complex structure.
    {
        const int off[] =  {0, 0, 0};
        const int trial_off[] = {0, 0, 255};
        const int trial_on[] = {0, 255, 0};
        const int test[] = {0, 255, 0};
        const int torque_calibration[] = {255, 255, 0};
        const int fsr_calibration[] = {255, 0, 255};
        const int fsr_refinement[] = {255, 0, 255};
        const int motor_start_up[] = {255, 68, 0};
        const int error[] = {255, 0, 0};
        const int warning[] = {255, 68, 0};
    }
    
    /**
     * @brief patterns to use for each message
     */
    namespace patterns //Just used namespace due to the complex structure.
    {
        const uint8_t solid = 0;
        const uint8_t blink = 1;
        const uint8_t pulse = 2;
        const uint8_t rainbow = 3;
        
        //Format is pattern number and period in ms
        const int off[] =  {solid, 0};                  //Solid
        const int trial_off[] = {solid,0};              //Solid
        const int trial_on[] = {pulse, 500};            //Pulse
        const int test[] = {rainbow, 4000};             //Rainbow
        const int torque_calibration[] = {solid, 0};    //Solid
        const int fsr_calibration[] = {solid, 0};       //Solid
        const int fsr_refinement[] = {pulse, 250};      //Pulse
        const int motor_start_up[] = {pulse, 4000};     //Pulse
        const int error[] = {blink, 250};               //Blinking
        const int warning[] = {blink, 250};             //Blinking
    }
    
    const uint8_t on_state = logic_micro_pins::status_led_on_state;     /**< On state of the LED used to determine PWM ratio*/
    const uint8_t off_state = logic_micro_pins::status_led_off_state;   /**< Off state of the LED used to determine PWM ratio*/
    
    const bool has_pwm = logic_micro_pins::status_has_pwm;              /**< Records if the display should use PWM or simple digital output. */ 
}

//Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)
/**
 * @brief class used to set the status LED display based on the Status
 */
class StatusLed
{
  public:
    //Constructors one you can set the default LED State
    StatusLed(int r_pin, int g_pin, int b_pin);                 //Pins are the pins assocated with the different LED inputs
    StatusLed(int r_pin, int g_pin, int b_pin, int brightness); //Pins are the pins assocated with the different LED inputs, brightness is used to scale the colors that are sent: color * brightness/255
   
    /**
     * @brief Change the message and LED state
     *
     * @param status message
     */
    void update(uint16_t message); //Changes the LED State to the current state
    
    /**
     * @brief Change the brightness. Only used when logic_micro_pins::status_has_pwm is true.
     *
     * @param int used to set the brightness of the LED
     */
    void set_brightness(int brightness);  //Used if you need to change the brightness after initialization, brightness is used to scale the colors that are sent: color * brightness/255
    
    /**
     * @brief Toggle the status LED on or off
     */
    void toggle();

  private:
  
    /**
     * @brief Set LED state based on color values
     *
     * @param red value 0-255
     * @param green value 0-255
     * @param blue value 0-255
     */
    void _set_color(int R, int G, int B);  //Changes the color R,G,and B are 0-255 values to set the corresponding colors.
    
    /**
     * @brief  Displays whatever the current _message_colors values are continuously
     */
    void _solid();
    
    /**
     * Dims and brightens the LED based on current message
     */
    void _pulse();
    
    /**
     * @brief Blinks the LED with equal amounts, based on current message
     */
    void _blink();
    
    /**
     * @brief Brightens and dims each color as a sin wave where each color is phase shifted.
     */
    void _rainbow_sin();
    
    /**
     * @brief Brightens and dims each color as a trapezoidal wave where each color is phase shifted.
     */
    void _rainbow_hsv();
    
    int _r_pin;             /**< Pin used for the red LED */
    int _g_pin;             /**< Pin used for the green LED */
    int _b_pin;             /**< Pin used for the blue LED */
    int _brightness;        /**< Max brightness of LED this scales the RGB colors, color * brightness/255 */
    int _current_message;   /**< Index of the current message used to select the correct color. */
    uint16_t _msg_idx;      /**< Index the current message uses for display */
    
    int _pattern_start_timestamp;       /**< Keeps track of the time the current pattern has run. */
    int _period_ms;                     /**< Period of the pattern */
    int _pattern_brightness_percent;    /**< Percent of the  for the pattern brightness. */
    
    //Make sure to keep in index order from messages, this is an array of the colors to use _messageColors[_currentMessage][color] where color is 0 for r, 1 for g, and 2 for b.
    //This method of accessing array elements is bulky but works.

    /**< Mapping from _msg_idx to color */
    const int _message_colors[10][3] = {{status_led_defs::colors::off[0], status_led_defs::colors::off[1], status_led_defs::colors::off[2]}, 
                {status_led_defs::colors::trial_off[0], status_led_defs::colors::trial_off[1], status_led_defs::colors::trial_off[2]}, 
                {status_led_defs::colors::trial_on[0], status_led_defs::colors::trial_on[1], status_led_defs::colors::trial_on[2]}, 
                {status_led_defs::colors::test[0], status_led_defs::colors::test[1], status_led_defs::colors::test[2]}, 
                {status_led_defs::colors::torque_calibration[0], status_led_defs::colors::torque_calibration[1], status_led_defs::colors::torque_calibration[2]}, 
                {status_led_defs::colors::fsr_calibration[0], status_led_defs::colors::fsr_calibration[1], status_led_defs::colors::fsr_calibration[2]}, 
                {status_led_defs::colors::fsr_refinement[0], status_led_defs::colors::fsr_refinement[1], status_led_defs::colors::fsr_refinement[2]},
                {status_led_defs::colors::motor_start_up[0], status_led_defs::colors::motor_start_up[1], status_led_defs::colors::motor_start_up[2]},
                {status_led_defs::colors::error[0], status_led_defs::colors::error[1], status_led_defs::colors::error[2]}, 
                {status_led_defs::colors::warning[0], status_led_defs::colors::warning[1], status_led_defs::colors::warning[2]} 
                }; 

    /**< Mapping from _msg_idx to pattern */
    const int _message_pattern[10][2] = {{status_led_defs::patterns::off[0], status_led_defs::patterns::off[1]}, 
                {status_led_defs::patterns::trial_off[0], status_led_defs::patterns::trial_off[1]}, 
                {status_led_defs::patterns::trial_on[0], status_led_defs::patterns::trial_on[1]}, 
                {status_led_defs::patterns::test[0], status_led_defs::patterns::test[1]}, 
                {status_led_defs::patterns::torque_calibration[0], status_led_defs::patterns::torque_calibration[1]}, 
                {status_led_defs::patterns::fsr_calibration[0], status_led_defs::patterns::fsr_calibration[1]}, 
                {status_led_defs::patterns::fsr_refinement[0], status_led_defs::patterns::fsr_refinement[1]},
                {status_led_defs::patterns::motor_start_up[0], status_led_defs::patterns::motor_start_up[1]},
                {status_led_defs::patterns::error[0], status_led_defs::patterns::error[1]},
                {status_led_defs::patterns::warning[0], status_led_defs::patterns::warning[1]} 
                };
};
#endif
#endif
