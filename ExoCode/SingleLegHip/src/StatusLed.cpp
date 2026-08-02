//Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)
#include "Arduino.h"
#include "StatusLed.h"
#include "Logger.h"
#include <math.h>

//#define STATUS_LED_DEBUG

/*
Constructors
*/

StatusLed::StatusLed(int r_pin, int g_pin, int b_pin)
{
  //See header file for information on what each variable is for.
  _r_pin = r_pin;
  _g_pin = g_pin;
  _b_pin = b_pin;
  
  _brightness =  2048; //Range 0 - 4095, off to full on.
  
  _current_message = status_defs::messages::trial_off;          //Initalize message to trial off
  _msg_idx = status_led_defs::status_led_idx[_current_message];
        
  //Configure the pin for the LED
  pinMode(_r_pin, OUTPUT);  //Sets the pin as output
  pinMode(_g_pin, OUTPUT);  //Sets the pin as output
  pinMode(_b_pin, OUTPUT);  //Sets the pin as output
  
  update(_current_message); //Set status
};

StatusLed::StatusLed(int r_pin, int g_pin, int b_pin, int brightness)
{
  //See header file for information on what each variable is for.
  _r_pin = r_pin;
  _g_pin = g_pin;
  _b_pin = b_pin;
  
  _brightness = brightness ; //Range 0 - 255, off to full on.
  
  _current_message = status_defs::messages::trial_off;          //Initalize message to trial off
  _msg_idx = status_led_defs::status_led_idx[_current_message];
        
  //Configure the pin for the LED
  pinMode(_r_pin, OUTPUT);  //Sets the pin as output
  pinMode(_g_pin, OUTPUT);  //Sets the pin as output
  pinMode(_b_pin, OUTPUT);  //Sets the pin as output
  
  update(_current_message); //Set status
};

/*
Public
*/

void StatusLed::update(uint16_t message)
{
    if (message != _current_message)
    {
        _current_message = message;                                     //Update _current_message 
        _msg_idx = status_led_defs::status_led_idx[_current_message];
        _pattern_start_timestamp = millis();                            //Restart the timer
        _period_ms = _message_pattern[_msg_idx][1];

        #ifdef STATUS_LED_DEBUG
            logger::print("StatusLed::update : Message updated to ");
            print_status_message(message);
            logger::print("\n");
        #endif
    }
    
    int red = _message_colors[_current_message][0];
    int green = _message_colors[_current_message][1];
    int blue = _message_colors[_current_message][2];
             
    _set_color(_message_colors[_current_message][0],_message_colors[_current_message][1],_message_colors[_current_message][2]);   //Set the LED state
};

/**
 * @brief Toggle the status LED on or off
 */
void StatusLed::toggle()
{
    // logger::println("StatusLed::toggle");
    // static bool led_on = false;
    // if (led_on)
    // {
    //     digitalWrite(_r_pin, LOW);
    //     digitalWrite(_g_pin, LOW);
    //     digitalWrite(_b_pin, LOW);
    //     led_on = false;
    // }
    // else
    // {
    //     _set_color(_message_colors[_current_message][0],_message_colors[_current_message][1],_message_colors[_current_message][2]);
    //     led_on = true;
    // }
}




void StatusLed::set_brightness(int brightness)
{
  _brightness = brightness;
};

/*
Protected
*/

void StatusLed::_set_color(int r_color, int g_color, int b_color)
{
    
//   logger::print(r_color);
//   logger::print("\t");
//   logger::print(g_color);
//   logger::print("\t");
//   logger::print(b_color);
//   logger::print("\n");
  
  if (status_led_defs::has_pwm)  //Using simple digital pins
  {
    //logger::println("StatusLed::_set_color : Using PWM");
    int r_color_scaled = floor(r_color * _brightness/4095); //Scale by brightness
    int g_color_scaled = floor(g_color * _brightness/4095); //Scale by brightness
    int b_color_scaled = floor(b_color * _brightness/4095); //Scale by brightness
    
    analogWrite(_r_pin, abs(status_led_defs::off_state - r_color_scaled)); //If 0 is the off state will set r_colorScaled value, if 255 is off state will set 255 - r_colorScaled effectively inverting the PWM signal
    analogWrite(_g_pin, abs(status_led_defs::off_state - g_color_scaled)); //If 0 is the off state will set g_colorScaled value, if 255 is off state will set 255 - g_colorScaled effectively inverting the PWM signal
    analogWrite(_b_pin, abs(status_led_defs::off_state - b_color_scaled)); //If 0 is the off state will set b_colorScaled value, if 255 is off state will set 255 - b_colorScaled effectively inverting the PWM signal
    
  }
  else
  {
    //logger::println("StatusLed::_set_color : Using digital pins");
    digitalWrite(_r_pin, (status_led_defs::off_state == 0) ? r_color >= 127 : r_color < 127);  //If the off state is low, LED turns on if color is >127 (1 on, 0 off), else the LED turns on if the >127 but the state is switched (0 on, 1 off)
    digitalWrite(_g_pin, (status_led_defs::off_state == 0) ? g_color >= 127 : g_color < 127);  //If the off state is low, LED turns on if color is >127 (1 on, 0 off), else the LED turns on if the >127 but the state is switched (0 on, 1 off)
    digitalWrite(_b_pin, (status_led_defs::off_state == 0) ? b_color >= 127 : b_color < 127);  //If the off state is low, LED turns on if color is >127 (1 on, 0 off), else the LED turns on if the >127 but the state is switched (0 on, 1 off)
  }
};

void StatusLed::_solid()
{
    _set_color(_message_colors[_msg_idx][0],_message_colors[_msg_idx][1],_message_colors[_msg_idx][2]);   //Set the LED state
    return;
};

void StatusLed::_pulse()
{
    if (status_led_defs::has_pwm)
    {
        int timestamp = millis();
        if (timestamp - _pattern_start_timestamp > _period_ms)
        {
            _pattern_start_timestamp = timestamp;
        }
        // int time_diff = (timestamp - _pattern_start_timestamp);
        // _pattern_brightness_percent =  time_diff < (_period_ms / 2) ? 100 * time_diff / (_period_ms / 2) : 100 * (_period_ms - time_diff) / (_period_ms / 2);
        
        float angle_deg = 360.0 * (timestamp - _pattern_start_timestamp) / _period_ms;
        _pattern_brightness_percent = 100 * sin (angle_deg * PI / 180);
        // logger::print(angle_deg);
        // logger::print("\n");
        
        _set_color(_pattern_brightness_percent * _message_colors[_msg_idx][0] / 100, _pattern_brightness_percent * _message_colors[_msg_idx][1]/100, _pattern_brightness_percent * _message_colors[_msg_idx][2]/100);   // Set the LED state
    }
    else
    {
        //The _pattern_start_timestamp will get overwritten in this which should be ok but may cause some weirdness as it is getting checked twice.
        _blink();
    }
    return;
};

void StatusLed::_blink()
{
    int timestamp = millis();
    if (timestamp - _pattern_start_timestamp > _period_ms)
    {
        _pattern_start_timestamp = timestamp;
    }
    
    bool on = (timestamp - _pattern_start_timestamp) < (_period_ms/2);
    _set_color(on * _message_colors[_msg_idx][0], on * _message_colors[_msg_idx][1], on * _message_colors[_msg_idx][2]);   //Set the LED state
    return;
};

void StatusLed::_rainbow_sin()
{
    //Reset the pattern if we have gone past the period.
    int timestamp = millis();
    if (timestamp - _pattern_start_timestamp > _period_ms)
    {
        _pattern_start_timestamp = timestamp;
    }
    
    //Rgb colors that will be set later on
    uint8_t colors [] = {0, 0, 0}; 

    //Angular value where each colors sin wave starts
    int start_angles_deg[] = {240, 0, 120};

    //Get angle based on the time.
    float angle_deg = 360.0 * (timestamp - _pattern_start_timestamp) / _period_ms;
    
    for(int i = 0; i < 3; i++)
    {
       //Find the positive angle from the start angle
       int angle_from_start_deg = angle_deg < start_angles_deg[i] ?  angle_deg + 360 - start_angles_deg[i] : angle_deg - start_angles_deg[i];     
       
       //Get the value of the color at the current angle.
       colors[i] = 255 * sin (angle_from_start_deg * PI / 180);
    }

    //Set the LED state
    _set_color(colors[0], colors[1], colors[2]);   
    return;
};

void StatusLed::_rainbow_hsv()
{
    int timestamp = millis();
    if (timestamp - _pattern_start_timestamp > _period_ms)
    {
        _pattern_start_timestamp = timestamp;
    }
    
    uint8_t colors [] = {0, 0, 0}; //rgb
    
    //Color counts up for 60 degrees from the start. Holds high for 120, then counts down for 60.
    int start_angles_deg[] = {240, 0, 120};
        
    float angle_deg = 360.0 * (timestamp - _pattern_start_timestamp) / _period_ms;
    int angle_from_start_deg = 0;
    
    
    for(int i = 0; i < 3; i++)
    {
       //Find the positive angle from the start angle
       angle_from_start_deg = angle_deg < start_angles_deg[i] ? 360 + angle_deg - start_angles_deg[i] : angle_deg - start_angles_deg[i]; 
       
       if (angle_from_start_deg < 60)
       {
           colors[i] = 255 * angle_from_start_deg / 60;
       }
       else if (angle_from_start_deg < 180)
       {
           colors[i] = 255;
       }
       else if (angle_from_start_deg < 240)
       {
           colors[i] = 255 * (240 - angle_from_start_deg) / 60;
       }
       else 
       {
           colors[i] = 0;
       }
    }
    
    _set_color(colors[0], colors[1], colors[2]);   //Set the LED state
    return;
};
#endif