/**
 * @file ComsMCU.h
 * @author Chance Cuddeback
 * @brief The top of the composite structure for the communication microcontroller. This class 
 * manages the bluetooth connection and data. The class also performs battery sampling.
 * @date 2022-08-22
 * 
 */
#ifndef COMSMCU_H
#define COMSMCU_H

#include "Arduino.h"
#include "ExoBLE.h"
// #include "Battery.h"
#include "BleMessage.h"
#include "ParseIni.h"
#include "ExoData.h"
#include "BleMessageQueue.h"

/**
 * @brief ComsMCU class. 
 * 
 */
 
#if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)
class ComsMCU
{
    public:
        /**
         * @brief Construct a new Coms M C U object
         * 
         * @param data a reference to the ExoData object
         * @param config_to_send a reference to the sd card configuration 
         */
        ComsMCU(ExoData* data, uint8_t* config_to_send);

        /**
         * @brief Check for Bluetooth Low Energy events, process data if available
         * 
         */
        void handle_ble();

        /**
         * @brief Sample any sensors that the communications microcontroller is responsible for
         * sampling
         */
        void local_sample();
        
        /**
         * @brief Send UART msg and update data or config based on response
         */
        void update_UART();

        /**
         * @brief Sends data to the GUI. If a trial is active, the real time data will be sent
         * every 1000000/(BLE_time::_real_time_msg_delay) Hz. The battery and error reset data are sent
         * every 1000000/(BLE_time::_status_msg_delay) Hz.
         */
        void update_gui();

        /**
         * @brief Check for errors and pass to the GUI
         * 
         */
        void handle_errors();
    private:
        /**
         * @brief Pulses the power LED to indicate a valid Real Time connection
         * 
         */
        void _life_pulse();
        const int k_pulse_count = 10;
        
        /**
         * @brief Private function responsible for calling the correct ble message handler
         * 
         * @param msg Complete BLE message
         */
        void _process_complete_gui_command(BleMessage* msg);

        //Reference to ExoBLE object, this is the next step down the composition heirarchy
        ExoBLE* _exo_ble;

        //Hold on to the last message from the GUI
        BleMessage _latest_gui_message = BleMessage();
        
        //Data
        ExoData* _data;
        
        //Battery
        // _Battery* _battery;

        const int _mark_index = 1;

        //Alpha value for the exponentially weighted moving average on the battery data
        // const float k_battery_ewma_alpha = 0.1;
        // const float k_time_threshold = 5000; //microseconds
        
};
#endif
#endif