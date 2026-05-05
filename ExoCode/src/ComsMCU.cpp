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
#include "SystemReset.h"

#if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)

#define COMSMCU_DEBUG 1

ComsMCU::ComsMCU(ExoData* data, uint8_t* config_to_send):_data{data}
{
    /* switch (config_to_send[config_defs::battery_idx])
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
    } */

    // _battery->init();
    _exo_ble = new ExoBLE();
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
        case (uint8_t)config_defs::exo_name::bilateral_arm:
            rt_data_len = rt_data::BILATERAL_ARM_RT_LEN;
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
    // NOTE: Per-loop "Start"/"End" tracing removed -- the loop runs at
    // hundreds of Hz and printing a dozen lines every iteration at
    // 115200 baud was eating ~50 ms per loop, which made the BLE link
    // appear hung even though it was healthy.  We now only print on
    // genuine events (queue pop, command processed, errors).
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
}

void ComsMCU::local_sample()
{
    // Per-loop tracing removed; this fires every iteration.
    Time_Helper* t_helper = Time_Helper::get_instance();
    static const float context = t_helper->generate_new_context();
    static float del_t = 0;
    del_t += t_helper->tick(context);

    if (del_t > (BLE_times::_status_msg_delay/2)) 
    {
        /* static float filtered_value = _battery->get_parameter();
        float raw_battery_value = _battery->get_parameter();
        filtered_value = utils::ewma(raw_battery_value, filtered_value, k_battery_ewma_alpha);
        _data->battery_value = filtered_value; */
        del_t = 0;
    }

    ComsLed::get_instance()->life_pulse();
    _maybe_system_reset();
}

void ComsMCU::update_UART()
{
    // Per-loop tracing removed.
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
}


void ComsMCU::update_gui() 
{
    // Per-loop tracing removed; this fires every iteration.
    static Time_Helper* t_helper = Time_Helper::get_instance();
    static float my_mark = _data->mark;
    static float* rt_floats = new float(rt_data::len);

    //Get real time data from ExoData and send to GUI
    const bool new_rt_data = real_time_i2c::poll(rt_floats);
    static float del_t_no_msg = millis();

    // 1 Hz chart-data heartbeat counters.  We aggregate over a full second
    // and print one summary line, instead of printing per-sample.  Per-sample
    // prints at the chart rate were starving the BLE stack and making the
    // GUI think the Nano had dropped the link.
    static uint32_t rt_sent_in_window = 0;
    static uint32_t rt_skipped_in_window = 0;
    static unsigned long last_rt_heartbeat_ms = millis();
    static float last_rt_sample0 = 0.0f;
    static float last_rt_sample1 = 0.0f;
    static float last_rt_sample2 = 0.0f;

    if (new_rt_data || rt_data::new_rt_msg)
    {
        del_t_no_msg = millis();

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

        rt_sent_in_window++;
        if (rt_data::len > 0) last_rt_sample0 = rt_data_msg.data[0];
        if (rt_data::len > 1) last_rt_sample1 = rt_data_msg.data[1];
        if (rt_data::len > 2) last_rt_sample2 = rt_data_msg.data[2];
    } 
    else 
    {
        rt_skipped_in_window++;

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

    // Print a single throttled summary every ~1 second so the user can
    // watch the chart-data pipeline at a glance: how many RT samples were
    // pushed to BLE in the last second, the most recent values, and whether
    // a central is currently subscribed.
    #if COMSMCU_DEBUG
        unsigned long now_ms = millis();
        if (now_ms - last_rt_heartbeat_ms >= 1000)
        {
            logger::print("ComsMCU::update_gui->rt_chart sent=");
            logger::print(rt_sent_in_window);
            logger::print(" skipped=");
            logger::print(rt_skipped_in_window);
            logger::print(" /sec  ble=");
            logger::print(_exo_ble->is_connected() ? "yes" : "no");
            logger::print("  status=");
            logger::print(_data->get_status());
            logger::print("  sample[0..2]=");
            logger::print(last_rt_sample0);
            logger::print(",");
            logger::print(last_rt_sample1);
            logger::print(",");
            logger::print(last_rt_sample2);
            logger::print("\n");

            rt_sent_in_window = 0;
            rt_skipped_in_window = 0;
            last_rt_heartbeat_ms = now_ms;
        }
    #endif

    //Periodically send status information
    static float status_context = t_helper->generate_new_context(); 
    static float del_t_status = 0;
    del_t_status += t_helper->tick(status_context);
    if (del_t_status > BLE_times::_status_msg_delay)
    {
        //Send status data
        /* BleMessage batt_msg = BleMessage();
        batt_msg.command = ble_names::send_batt;
        batt_msg.expecting = ble_command_helpers::get_length_for_command(batt_msg.command);
        batt_msg.data[0] = _data->battery_value;
        _exo_ble->send_message(batt_msg); */

        del_t_status = 0;

        #if COMSMCU_DEBUG
            logger::println("ComsMCU::update_gui->status_tick");
        #endif
    }
}

void ComsMCU::handle_errors()
{
    // Per-loop tracing removed; this fires every iteration.
    static ErrorCodes error_code = NO_ERROR;

    if (_data->error_code != static_cast<int>(error_code))
    {
        error_code = static_cast<ErrorCodes>(_data->error_code);
        _exo_ble->send_error(_data->error_code, _data->error_joint_id);

        #if COMSMCU_DEBUG
            logger::print("ComsMCU::handle_errors->error_code changed to: ");
            logger::println(_data->error_code);
        #endif
    }
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
    case ble_names::reset_system:
        _schedule_system_reset();
        break;
    default:
        logger::println("ComsMCU::_process_complete_gui_command->No case for command!", LogLevel::Error);
        break;
    }

    #if COMSMCU_DEBUG
        logger::println("ComsMCU::_process_complete_gui_command->End");
    #endif
}

void ComsMCU::_schedule_system_reset()
{
    _reset_pending = true;
    _reset_start_ms = millis();
}

void ComsMCU::_maybe_system_reset()
{
    if (!_reset_pending)
    {
        return;
    }
    if ((millis() - _reset_start_ms) < _reset_delay_ms)
    {
        return;
    }

    UARTHandler* uart_handler = UARTHandler::get_instance();
    UART_msg_t tx_msg;
    tx_msg.command = UART_command_names::get_system_reset;
    tx_msg.joint_id = 0;
    tx_msg.len = 0;
    uart_handler->UART_msg(tx_msg);
    delay(10);

    exo_system_reset();
}

void ComsMCU::_life_pulse()
{
    // NOTE: This runs on every RT data send, so any logging here multiplies
    // by the chart sample rate (potentially hundreds of Hz).  Kept silent
    // intentionally -- the 1 Hz heartbeat in update_gui() already shows
    // whether RT sends are happening.
    static int count = 0;
    count++;

    if (count > k_pulse_count)
    {
        count = 0;
        digitalWrite(25, !digitalRead(25));
    }
}
#endif
