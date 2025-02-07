/*
 * 
 * P. Stegall Jan. 2022
*/

#include "Exo.h"
#include "Time_Helper.h"
#include "UARTHandler.h"
#include "UART_msg_t.h"
#include "uart_commands.h"
#include "Logger.h"

//#define EXO_DEBUG  //Uncomment if you want the debug statements to print to serial monitor

//Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41) 
/*
 * Constructor for the Exo
 * Takes the exo_data
 * Uses initializer list for sides.
 * Only stores these objects, and exo_data pointer.
 */
Exo::Exo(ExoData* exo_data)
: left_side(true, exo_data)      //Constructor: uses initializer list for the sides
, right_side(false, exo_data)    //Constructor: uses initializer list for the sides
, sync_led(logic_micro_pins::sync_led_pin, sync_time::SYNC_START_STOP_HALF_PERIOD_US, sync_time::SYNC_HALF_PERIOD_US, logic_micro_pins::sync_led_on_state, logic_micro_pins::sync_default_pin)  //Create a sync LED object, the first and last arguments (pin) are found in Board.h, and the rest are in Config.h. If you do not have a digital input for the default state you can remove SYNC_DEFAULT_STATE_PIN.
, status_led(logic_micro_pins::status_led_r_pin, logic_micro_pins::status_led_g_pin, logic_micro_pins::status_led_b_pin)  //Create the status LED object.

#ifdef USE_SPEED_CHECK
    ,speed_check(logic_micro_pins::speed_check_pin)
#endif

{
    this->data = exo_data;
    
    #ifdef EXO_DEBUG
        logger::println("Exo :: Constructor : _data set");
    #endif

    pinMode(logic_micro_pins::motor_stop_pin,INPUT_PULLUP);
    
    #ifdef EXO_DEBUG
        logger::println("Exo :: Constructor : motor_stop_pin Mode set");
    #endif
};

/* 
 * Run the exo 
 */
bool Exo::run()
{
    //Check if we are within the system frequency we want.
    static UARTHandler* handler = UARTHandler::get_instance();
    static Time_Helper* t_helper = Time_Helper::get_instance();
    static float context = t_helper->generate_new_context();

    static float delta_t = 0;
    static uint16_t prev_status = data->get_status();
    delta_t += t_helper->tick(context);

    //Check if the real time data is ready to be sent.
    static float rt_context = t_helper->generate_new_context();
    static float rt_delta_t = 0;

    static const float lower_bound = (float) 1/LOOP_FREQ_HZ * 1000000 * (1 - LOOP_TIME_TOLERANCE);
    
    if (delta_t >= (lower_bound))
    {    
        #if USE_SPEED_CHECK
            logger::print(String(delta_t) + "\n");
            speed_check.toggle();
        #endif

        //Check if we should update the sync LED and record the LED on/off state.
        data->sync_led_state = sync_led.handler();
        bool trial_running = sync_led.get_is_blinking();

        //Check the estop
        data->estop = digitalRead(logic_micro_pins::motor_stop_pin);

        //If the estop is low, disable all of the motors
        if (data->estop)
        {
            data->for_each_joint([](JointData* j_data, float* args){j_data->motor.enabled = false;});
        }

        //Record the side data and send new commands to the motors.
        left_side.run_side();
        right_side.run_side();

        //Update status LED
        status_led.update(data->get_status());
        #ifdef EXO_DEBUG
            logger::println("Exo::Run:Time_OK");
            logger::println(delta_t);
            logger::println(((float)1 / LOOP_FREQ_HZ * 1000000 * (1 + LOOP_TIME_TOLERANCE)));
        #endif

        //Check for incoming UART messages
        UART_msg_t msg = handler->poll(UART_times::CONT_MCU_TIMEOUT);       //UART_times::CONT_MCU_TIMEOUT is in Config.h
        UART_command_utils::handle_msg(handler, data, msg);

        //Send the coms mcu the real time data every _real_time_msg_delay microseconds
        rt_delta_t += t_helper->tick(rt_context);
        uint16_t exo_status = data->get_status();
        const bool correct_status = (exo_status == status_defs::messages::trial_on) || (exo_status == status_defs::messages::fsr_calibration) || (exo_status == status_defs::messages::fsr_refinement) || (exo_status == status_defs::messages::error);
        
        if ((rt_delta_t >= BLE_times::_real_time_msg_delay) && (correct_status))
        {
            #ifdef EXO_DEBUG
                logger::print("Exo::run->Sending Real Time Message: ");
                logger::println(rt_delta_t);
            #endif
            
            UART_msg_t msg;
            UART_command_handlers::get_real_time_data(handler, data, msg, data->config);
            rt_delta_t = 0;
        }

        delta_t = 0;
        return true;
    }

    return false;
};



#endif