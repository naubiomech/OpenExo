#include "ComsMCU.h"
#include "StatusLed.h"
#include "StatusDefs.h"
#include "Time_Helper.h"
#include "UARTHandler.h"
#include "uart_commands.h"
#include "UART_msg_t.h"
#include "Config.h"
#include "error_codes.h"
#include "Logger.h"
#include "ComsLed.h"

#if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)

#define COMSMCU_DEBUG 0

ComsMCU::ComsMCU(ExoData* data, uint8_t* config_to_send):_data{data}
{
    switch (config_to_send[config_defs::battery_idx])
    {
    case (uint8_t)config_defs::battery::smart:
        _battery = new SmartBattery();
        break;
    case (uint8_t)config_defs::battery::dumb:
        _battery = new RCBattery();
        break;
    default:
        //logger::println("ERROR: ComsMCU::ComsMCU->Unrecognized battery type!");
        _battery = new RCBattery();
        break;
    }

    _battery->init();
    _exo_ble = new ExoBLE(_data, config_to_send[config_defs::exo_name_idx]);
    _exo_ble->setup();

    uint8_t rt_data_len = 0;
    switch (config_to_send[config_defs::exo_name_idx])
    {
        case (uint8_t)config_defs::exo_name::bilateral_ankle:
            rt_data_len = rt_data::BILATERAL_ANKLE_RT_LEN;
            break;
        case (uint8_t)config_defs::exo_name::bilateral_hip:
            rt_data_len = rt_data::BILATERAL_HIP_RT_LEN;
            break;
        case (uint8_t)config_defs::exo_name::bilateral_hip_ankle:
            rt_data_len = rt_data::BILATERAL_HIP_ANKLE_RT_LEN;
            break;
        default:
            rt_data_len = 8;
            break;
    }

    //rt_data::msg_len = rt_data_len
    // logger::print("ComsMCU::ComsMCU->rt_data_len: "); logger::println(rt_data_len);
}

void ComsMCU::handle_ble()
{
    #if COMSMCU_DEBUG
        logger::println("ComsMCU::handle_ble->Start");
    #endif

    bool non_empty_ble_queue = _exo_ble->handle_updates();

    if (non_empty_ble_queue)
    {
        #if COMSMCU_DEBUG
            logger::println("ComsMCU::handle_ble->non_empty_ble_queue");
        #endif

        BleMessage msg = ble_queue::pop();
        _process_complete_gui_command(&msg);

        #if COMSMCU_DEBUG
            logger::println("ComsMCU::handle_ble->processed message");
        #endif
    }

    #if COMSMCU_DEBUG
        logger::println("ComsMCU::handle_ble->End");
    #endif
}

void ComsMCU::local_sample()
{
    #if COMSMCU_DEBUG
        logger::println("ComsMCU::local_sample->Start");
    #endif

    Time_Helper* t_helper = Time_Helper::get_instance();
    static const float context = t_helper->generate_new_context();
    static float del_t = 0;
    del_t += t_helper->tick(context);

    if (del_t > (BLE_times::_status_msg_delay/2)) 
    {
        static float filtered_value = _battery->get_parameter();
        float raw_battery_value = _battery->get_parameter();
        filtered_value = utils::ewma(raw_battery_value, filtered_value, k_battery_ewma_alpha);
        _data->battery_value = filtered_value;
        del_t = 0;
    }

    ComsLed::get_instance()->life_pulse();

    #if COMSMCU_DEBUG
        logger::println("ComsMCU::local_sample->End");
    #endif
}

void ComsMCU::update_UART()
{
    #if COMSMCU_DEBUG
        logger::println("ComsMCU::update_UART->Start");
    #endif

    static Time_Helper* t_helper = Time_Helper::get_instance();
    static const float _context = t_helper->generate_new_context();
    static float del_t = 0;
    del_t += t_helper->tick(_context);
    
    if (del_t > UART_times::UPDATE_PERIOD)
    {
        UARTHandler* handler = UARTHandler::get_instance();
        UART_msg_t msg = handler->poll(UART_times::COMS_MCU_TIMEOUT);

        if (msg.command)
        {
            UART_command_utils::handle_msg(handler, _data, msg);
        }

        del_t = 0;
    }

    #if COMSMCU_DEBUG
        logger::println("ComsMCU::update_UART->End");
    #endif
}


void ComsMCU::update_gui() 
{
    #if COMSMCU_DEBUG
        logger::println("ComsMCU::update_gui->Start");
    #endif
    
    static Time_Helper* t_helper = Time_Helper::get_instance();
    static float my_mark = _data->mark;
    static float* rt_floats = new float(rt_data::len);

    //Get real time data from ExoData and send to GUI
    const bool new_rt_data = real_time_i2c::poll(rt_floats);
    static float del_t_no_msg = millis();

    if(_data->first_message)
    {
        Serial.println("Initial Send");
        _exo_ble->sendInitialParameterNames();
        _data->first_message = false;
    }
    
    if ( _data->parameter_names_received && (new_rt_data || rt_data::new_rt_msg))
    {
        Serial.println("Now hitting data..");
        del_t_no_msg = millis();

        #if COMSMCU_DEBUG
            logger::println("ComsMCU::update_gui->new_rt_data");
        #endif

        _life_pulse();
        rt_data::new_rt_msg = false;

        BleMessage rt_data_msg = BleMessage();
        rt_data_msg.command = ble_names::send_real_time_data;
        rt_data_msg.expecting = rt_data::len;

        for (int i = 0; i < rt_data::len; i++)
        {   
            #if REAL_TIME_I2C
                rt_data_msg.data[i] = rt_floats[i];
            #else
                rt_data_msg.data[i] = rt_data::float_values[i];
            #endif
        }

        if (my_mark < _data->mark)
        {
            my_mark = _data->mark;
            rt_data_msg.data[_mark_index] = my_mark;
        }

        _exo_ble->send_message(rt_data_msg);

        Serial.println("Sending real time data..");

        #if COMSMCU_DEBUG
            logger::println("ComsMCU::update_gui->sent message");
        #endif
    } 
    else 
    {
        //If we should be getting messages and we dont for 1 second, spin on error
        uint16_t exo_status = _data->get_status();
        const bool correct_status = (exo_status == status_defs::messages::trial_on) || 
            (exo_status == status_defs::messages::fsr_calibration) || 
            (exo_status == status_defs::messages::fsr_refinement) ||
            (exo_status == status_defs::messages::error);

        if (correct_status)
        {
            // if (millis() - del_t_no_msg > 3000)
            // {
            //     #if COMSMCU_DEBUG
            //          logger::println("ComsMCU::update_gui->No message for 3 second");
            //     #endif
            //     while (true)
            //     {
            //         logger::println("ComsMCU::update_gui->No message for 3 second");
            //         delay(10000);
            //     }
            // }
        }
    }
    //Periodically send status information
    static float status_context = t_helper->generate_new_context(); 
    static float del_t_status = 0;
    del_t_status += t_helper->tick(status_context);
    if (_data->parameter_names_received && (del_t_status > BLE_times::_status_msg_delay))
    {
        Serial.println("Sending battery status..");
        #if COMSMCU_DEBUG
            logger::println("ComsMCU::update_gui->Sending status");
        #endif
        //Send status data
        BleMessage batt_msg = BleMessage();
        batt_msg.command = ble_names::send_batt;
        batt_msg.expecting = ble_command_helpers::get_length_for_command(batt_msg.command);
        batt_msg.data[0] = _data->battery_value;
        _exo_ble->send_message(batt_msg);
        del_t_status = 0;

        #if COMSMCU_DEBUG
            logger::println("ComsMCU::update_gui->sent message");
        #endif
    }
    

    #if COMSMCU_DEBUG
        logger::println("ComsMCU::update_gui->End");
    #endif
}

void ComsMCU::handle_errors()
{
    #if COMSMCU_DEBUG
        logger::println("ComsMCU::handle_errors->Start");
    #endif

    static ErrorCodes error_code = NO_ERROR;

    if (_data->error_code != static_cast<int>(error_code))
    {
        error_code = static_cast<ErrorCodes>(_data->error_code);
        _exo_ble->send_error(_data->error_code, _data->error_joint_id);
    }

    #if COMSMCU_DEBUG
        logger::println("ComsMCU::handle_errors->End");
    #endif
}

void ComsMCU::_process_complete_gui_command(BleMessage* msg) 
{
    #if COMSMCU_DEBUG
        logger::println("ComsMCU::_process_complete_gui_command->Start");
        BleMessage::print(*msg);
    #endif

    switch (msg->command)
    {
    case ble_names::start:
        ble_handlers::start(_data, msg);
        break;
    case ble_names::stop:
        ble_handlers::stop(_data, msg);
        break;
    case ble_names::cal_trq:
        ble_handlers::cal_trq(_data, msg);
        break;
    case ble_names::cal_fsr:
        ble_handlers::cal_fsr(_data, msg);
        break;
    case ble_names::assist:
        ble_handlers::assist(_data, msg);
        break;
    case ble_names::resist:
        ble_handlers::resist(_data, msg);
        break;
    case ble_names::motors_on:
        ble_handlers::motors_on(_data, msg);
        break;
    case ble_names::motors_off:
        ble_handlers::motors_off(_data, msg);
        break;
    case ble_names::mark:
        ble_handlers::mark(_data, msg);
        break;
    case ble_names::new_fsr:
        ble_handlers::new_fsr(_data, msg);
        break;
    case ble_names::new_trq:
        ble_handlers::new_trq(_data, msg);
        break;
    case ble_names::update_param:
        ble_handlers::update_param(_data, msg);
        break;
    case ble_names::send_param_recieved:
        ble_handlers::send_param_recieved(_data);
        break;
    case ble_names::send_param_not_recieved:
        Serial.println("Resend parameter names");
        _exo_ble->sendInitialParameterNames();
        break;
    default:
        logger::println("ComsMCU::_process_complete_gui_command->No case for command!", LogLevel::Error);
        break;
    }

    #if COMSMCU_DEBUG
        logger::println("ComsMCU::_process_complete_gui_command->End");
    #endif
}

void ComsMCU::_life_pulse()
{
    #if COMSMCU_DEBUG
        logger::println("ComsMCU::_life_pulse->Start");
    #endif

    static int count = 0;
    count++;

    if (count > k_pulse_count)
    {
        count = 0;
        digitalWrite(25, !digitalRead(25));
    }

    #if COMSMCU_DEBUG
        logger::println("ComsMCU::_life_pulse->End");
    #endif
}
#endif