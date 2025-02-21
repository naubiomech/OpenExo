#include "Arduino.h"
#include "SyncLed.h"
#include "Logger.h"
//#include <IntervalTimer.h>
//#include "IntervalTimerEx.h"
//Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)


SyncLed::SyncLed(int pin, int sync_start_stop_half_period_us, int sync_half_period_us)
{
  Serial.println("making sync led");
    //See header file for information on what each variable is for.
	_pin = pin;
	_current_sync_period = sync_half_period_us;
	_sync_start_stop_half_period_us = sync_start_stop_half_period_us;
    _last_state_change_timestamp_us = 0;
	_sync_half_period_us = sync_half_period_us;
	_default_led_state = logic_micro_pins::sync_led_on_state;
	_led_state = _default_led_state;
    _led_default_state_pin = -1;
    _led_is_on = _led_state == logic_micro_pins::sync_led_on_state;
    
	_state_change_count = 0; 
	_do_blink = false; 
	_do_start_stop_sequence = false; 
	_num_start_stop_blinks = sync_time::NUM_START_STOP_BLINKS;
	_is_blinking = false;
    
	//Configure the pin for the LED
	pinMode(_pin, OUTPUT);
	digitalWrite(_pin,_default_led_state);	
};

SyncLed::SyncLed(int pin, int sync_start_stop_half_period_us, int sync_half_period_us, int default_led_state)
{
	//See header file for information on what each variable is for.
	_pin = pin;
	_current_sync_period = sync_half_period_us;
	_sync_start_stop_half_period_us = sync_start_stop_half_period_us;
    _last_state_change_timestamp_us = 0;
	_sync_half_period_us = sync_half_period_us;
	_default_led_state = default_led_state;
    _led_default_state_pin = -1;
    _led_state = _default_led_state;
    _led_is_on = _led_state == logic_micro_pins::sync_led_on_state;
	
	_state_change_count = 0;                                    //Track how many 
	_do_blink = false;                                          //Use volatile for shared variables
	_do_start_stop_sequence = false;                            //Use volatile for shared variables
	_num_start_stop_blinks = sync_time::NUM_START_STOP_BLINKS;
	_is_blinking = false;
    
	//Configure the pin for the LED
	pinMode(_pin, OUTPUT);
	digitalWrite(_pin,_default_led_state);
};

SyncLed::SyncLed(int pin, int sync_start_stop_half_period_us, int sync_half_period_us, int default_led_state , int led_default_state_pin)
{
  //See header file for information on what each variable is for.
  _pin = pin;
  _current_sync_period = sync_half_period_us;
  _sync_start_stop_half_period_us = sync_start_stop_half_period_us;
  _last_state_change_timestamp_us = 0;
  _sync_half_period_us = sync_half_period_us;
  _default_led_state = default_led_state;
  _led_default_state_pin = led_default_state_pin;
  _led_state = _default_led_state;
  _led_is_on = _led_state == logic_micro_pins::sync_led_on_state;
  
  _state_change_count = 0;                                      //Track how many 
  _do_blink = false;                                            //Use volatile for shared variables
  _do_start_stop_sequence = false;                              //Use volatile for shared variables
  _num_start_stop_blinks = sync_time::NUM_START_STOP_BLINKS;
  _is_blinking = false;
  
  //Configure the pin for the LED
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin,_default_led_state);

  pinMode(_led_default_state_pin, INPUT_PULLUP);
  _default_led_state = digitalRead(_led_default_state_pin);
};

/* Public */

void SyncLed::trigger()
{
	_do_start_stop_sequence = _do_start_stop_sequence ^ true;   //xor with one to change boolean state
	_do_blink = _do_blink ^ true;                               //xor with one to change boolean state
	_state_change_count = 0;                                    //Reset the state change count
};


void SyncLed::update_led()
{
	int temp_led_state = _led_state;                            // Quickly record the state to minimize time without interrupts
	digitalWrite(_pin, temp_led_state);                         // Change the LED state
    _default_led_state = digitalRead(_led_default_state_pin);   // Technically this will update for the next call, but functionally this shouldn't really matter as it will change before you can use the sync.
  
    _led_is_on = temp_led_state == logic_micro_pins::sync_led_on_state;
};

void SyncLed::update_periods(int sync_start_stop_half_period_us, int sync_half_period_us)
{
	_sync_start_stop_half_period_us = sync_start_stop_half_period_us;
	_sync_half_period_us = sync_half_period_us;
};

bool SyncLed::handler()  
{
    int timestamp_us = micros();
    if ((timestamp_us - _last_state_change_timestamp_us) >= _current_sync_period)
    {
        _last_state_change_timestamp_us = timestamp_us;
        
        //Do start stop sequence
        if(_do_start_stop_sequence)
        {
            _blink_start_stop();
            _is_blinking = true;  //This way it will be recorded as blinking anytime not in the default state
        }
        
        //Do the main blinks
        else if (_do_blink)
        {
            _blink();
        }
        //Hold default state
        else
        {
            _is_blinking = false; //In the default state it is not blinking.
            _default_state();
        }
      
        update_led();
    }
  return _led_is_on;
  
};

void SyncLed::set_default_state(int new_default)
{
  _default_led_state = new_default;
};


/* Protected */

void SyncLed::_blink_start_stop(void)
{
  // logger::print("blinkStartStop: State Change Count : ");
  // logger::print(stateChangeCount);
  // logger::print("\n");

  //If the period is not correct it is the first call in the sequence
  if (_current_sync_period == _sync_half_period_us){
    _current_sync_period = _sync_start_stop_half_period_us;  //Set the correct period
    // logger::print("blinkStartStop: sync half period changed to  : ");
    // logger::print(current_sync_period);
    // logger::print("\n");
    _led_state = logic_micro_pins::sync_led_on_state; //Set the LED to on so that way end of the first call in the sequence will be off
  }

  //Toggle state
  if (_led_state == logic_micro_pins::sync_led_off_state) {
    _led_state = logic_micro_pins::sync_led_on_state;
  } 
  else {
    _led_state = logic_micro_pins::sync_led_off_state;
  }
 
  _state_change_count = _state_change_count + 1;   //Iterate the state change counter
  
  // logger::print("The LED is: ");
  // logger::print(_led_state);
  // logger::print("\n");

  //Once we have done the appropriate number of state change stop the start stop sequence.
  if (_state_change_count == (2*_num_start_stop_blinks+1*(_default_led_state==logic_micro_pins::sync_led_on_state))) //If the default state is 1 you need and extra one so make it low before it goes to the default state.
  {
    _do_start_stop_sequence = false;
  }
  
  //digitalWrite(syncLEDPin, _led_state);
};

void SyncLed::_blink(void)
{
  // logger::print("blinkLED: State Change Count : ");
  // logger::print(blinkCount);
  // logger::print("\n");

  //If the period is wrong change it to the correct one.
  if (_current_sync_period == _sync_start_stop_half_period_us ){
    _current_sync_period = _sync_half_period_us;
    // logger::print("blinkLED: sync half period changed to  : ");
    // logger::print(current_sync_period);
    // logger::print("\n");
  }

  //Toggle LED state
  if (_led_state == logic_micro_pins::sync_led_off_state) {
    _led_state = logic_micro_pins::sync_led_on_state;
    // _blinkCount = _blinkCount + 1;  //Increase when LED turns on
  } 
  else {
    _led_state = logic_micro_pins::sync_led_off_state;
  }
  //digitalWrite(syncLEDPin, _led_state);
};


void SyncLed::_default_state()
{
	_led_state = _default_led_state;
};

bool SyncLed::get_led_is_on()
{
    return _led_is_on;
}; 

bool SyncLed::get_is_blinking()
{
    return _is_blinking;
}; 

#endif