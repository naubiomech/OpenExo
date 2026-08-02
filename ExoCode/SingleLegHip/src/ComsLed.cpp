#include "ComsLed.h"
#include "Board.h"

#if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)

ComsLed::ComsLed()
{
    //Set the rgb pins to output
    pinMode(coms_micro_pins::red, OUTPUT);
    pinMode(coms_micro_pins::green, OUTPUT);
    pinMode(coms_micro_pins::blue, OUTPUT);
    
    //Set the initial color to 0
    this->r = 0;
    this->g = 0;
    this->b = 0;
    digitalWrite(coms_micro_pins::red, (bool)r);
    digitalWrite(coms_micro_pins::green, (bool)g);
    digitalWrite(coms_micro_pins::blue, (bool)b);
}

ComsLed* ComsLed::get_instance()
{
    static ComsLed* instance = new ComsLed();
    return instance;
}

void ComsLed::set_color(uint8_t r, uint8_t g, uint8_t b)
{
    this->r = r;
    this->g = g;
    this->b = b;
    digitalWrite(coms_micro_pins::red, (bool)_handle_active_low(r));
    digitalWrite(coms_micro_pins::green, (bool)_handle_active_low(g));
    digitalWrite(coms_micro_pins::blue, (bool)_handle_active_low(b));
}

void ComsLed::get_color(uint8_t* r, uint8_t* g, uint8_t* b)
{
    *r = this->r;
    *g = this->g;
    *b = this->b;
}

void ComsLed::life_pulse()
{
    life_pulse_counter++;
    if (life_pulse_counter > life_pulse_frequency)
    {
        life_pulse_counter = 0;
        
        //Flip the state of the green and blue pins
        digitalWrite(coms_micro_pins::green, !digitalRead(coms_micro_pins::green));
        digitalWrite(coms_micro_pins::blue, !digitalRead(coms_micro_pins::blue));
    }
}

uint8_t ComsLed::_handle_active_low(uint8_t value)
{
    if (coms_micro_pins::led_active_low)
    {
        return 255 - value;
    }
    else
    {
        return value;
    }
}

#endif