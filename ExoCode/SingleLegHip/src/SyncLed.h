/**
 * @file SyncLed.h
 *
 * @brief Class to blink and LED in a specific pattern to syncronize the data from the microcontroler with optical tracking systems
 * There will be _numStartStopBlinks high pulses at the begining and end of the sequence with each pulse lasting _sync_start_stop_half_period_us.
 * In in between the start and stop it will blink with each high or low lasting _sync_half_period_us.
 * The sequence is started and stopped using trigger().
 * 
 * This setup requires an external timer since the Arduino ISR timers can't take in arguments like the class's self.
 * To change the LED's actual on/off state, updateLed must be called.
 * A sample wrapper for the ISR for the timer:
 * 
 *  void grossLedInteruptWrapper(void)
 *  {
 *    syncLed.syncLedHandler(); //Calculate the LED state based on the timer, but don't change the actual LED state.
 *    syncTimer.begin(grossLedInteruptWrapper, syncLed.currentSyncPeriod);  //Update the timer period ideally we would only do this if it changed, might add a flag to syncLed if needed
 *  }
 * 
 * Then in the main loop or when you are recording data:
 * int ledState = syncLed.updateLed();  //Actually change the led state, and record the state in the data
 *  
 * The static state (not flashing) can be set to either HIGH or LOW
 * 
 * If you need the pin low to be LED on (like with a P channel mosfet) you can change that in the defines below.
 * 
 * @author P. Stegall 
 * @date Sept. 2021
*/


#ifndef SyncLed_h
#define SyncLed_h

//Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)
#include "Arduino.h"
#include "Board.h"
#include "config.h"
//#include "IntervalTimer.h"

//Define the on and off state of the LED. This is handy for if you are using a P Channel MOSFET where low is on.
//#define SYNC_LED_ON_STATE HIGH
//#define SYNC_LED_OFF_STATE LOW

// const unsigned int SYNC_HALF_PERIOD_US = 125000;                             //Half blink period in micro seconds
// const unsigned int SYNC_START_STOP_HALF_PERIOD_US = 4 * SYNC_HALF_PERIOD_US; //Half blink period for the begining and end of the sequence. This is usually longer so it is easy to identify.


/**
 * @brief Class used to control a synchronization LED
 *
 */
class SyncLed
{
  public:
    //Constructors one you can set the default LED State
    SyncLed(int pin, int sync_start_stop_half_period_us, int sync_half_period_us);
  	SyncLed(int pin, int sync_start_stop_half_period_us, int sync_half_period_us, int default_led_state);
    SyncLed(int pin, int sync_start_stop_half_period_us, int sync_half_period_us, int default_led_state, int led_default_state_pin);  //This isn't a great way of overloading but with high and low being int instead of bool I don't have a great alternative
    
    /**
     * @brief Method call which starts and stops the blink sequence calculation, does not change LED state
     */    
  	void trigger();  
    
    /**
     * @brief updates the on/off state of the LED based on the pattern
     * Put the LED in the appropriate state and returns that state for recording
     */
  	void update_led(); //Changes the LED State to the current state
  	
    /**
     * @brief Change the period for the Sync pattern after initialization
     *
     * @param half period in us for the start stop pattern
     * @param half period in us for the main sync pattern
     */
  	void update_periods(int sync_start_stop_half_period_us, int sync_half_period_us);  //Used if you need to change the periods after initialization
  	
    /**
     * @brief Handler which calculates the LED state, and sets the current period the interupt should use.  
     * Calls the appropriate methods for setting the LED state based on flags.
     * @return state of the LED
     */
  	bool handler();  
    
    /**
     * @brief Allows you to change the default state on the fly. Primarily used with digital pin _led_default_state_pin.
     * 
     * @param default state to use 
     */
  	void set_default_state(int new_default);   
    
    /**
     * @brief returns if the led is on or off.
     *
     * @return on/off state of the LED
     */
  	bool get_led_is_on(); 
   
    /**
     * @brief returns if the LED is currently in a blink pattern.
     *
     * @return 1 if the LED is currently in a blink pattern, 0 otherwise.
     */
  	bool get_is_blinking();
  	
	
    
  private:
    
  	
    /**
     * @brief Set values based on start stop sequence. This is a separate section to allow for use with interrupts.
     */
  	void _blink_start_stop();   
  	
    /**
     * @brief  does the main blink sequence. This is a separate section to allow for use with interrupts.
     */
  	void _blink(); 
  	
    /**
     * @brief set LED to default state.
     */
  	void _default_state();  
   
    volatile int _led_state;                    /**< Records the current LED state. This may not actually be set and depending on the circuit low may be on */
    int _led_is_on;                             /**< Records if the led is set to on. This is what you should use to record the value. */
	volatile int _current_sync_period;          /**< The current period to use. Whenever syncLedHandler is called the interupt should have another begin call to make sure the period is correct. */
	volatile bool _do_blink ;                   /**< Flag which says if the blink sequence is active, use volatile for shared variables */
    volatile bool _do_start_stop_sequence;      /**< Flag for if we are in the start stop region of the sequence, use volatile for shared variables */
    bool _is_blinking;                          /**< True while the system is blinking. */
    
    int _pin;                                   /**< Pin the LED is on. */
  	int _default_led_state;                     /**< Default LED state */
    int _led_default_state_pin;                 /**< Pin for external switch for setting LED state -1 if not used. */
  	
  	int _sync_start_stop_half_period_us;        /**< The time to hold a state, for the start stop sequence */
  	int _sync_half_period_us;                   /**< The time to hold a state, for the main blink sequence */
  	volatile unsigned int _state_change_count ; /**< Store the number of state changes for the start stop sequnce, use volatile for shared variables */
  	unsigned int _num_start_stop_blinks;        /**< The number of times to have the LED be on during the start stop sequnce, I think it will always be 1, but there is the option to change it. */
	int _last_state_change_timestamp_us;        /**< Records the time of the last state change */
	
};
#endif
#endif
