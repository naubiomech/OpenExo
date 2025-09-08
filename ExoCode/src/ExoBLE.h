/**
 * @file ExoBLE.h
 * @author Chance Cuddeback
 * @brief Class to handle all bluetooth work. This include initialization,
 * advertising, connection, and data transfer. 
 * @date 2022-08-22
 * 
 */


#ifndef EXOBLE_H
#define EXOBLE_H
#if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)

//#define EXOBLE_DEBUG      //Uncomment if you want to print debug statements to the serial monitor
#define MAX_PARSER_CHARACTERS       8
#define NAME_PREAMBLE               "EXOBLE_"
#define MAC_ADDRESS_TOTAL_LENGTH    17
#define MAC_ADDRESS_NAME_LENGTH     6

#include <Arduino.h>
#include <ArduinoBLE.h>

#include "BleParser.h"
#include "GattDb.h"
#include "BleMessage.h"
#include "BleMessageQueue.h"

class ExoBLE 
{
    public:
        /**
         * @brief Construct a new Exo BLE object
         * 
         * @param data A reference to the ExoData object
         */
         ExoBLE(ExoData* data, uint8_t* config_to_send);
        
        /**
         * @brief Sets GATT DB, device name, and begins advertising. 
         * 
         * @return true If the initialization succeeded
         * @return false If the initialization failed
         */
        bool setup();

        /**
         * @brief Starts and stops advertising.
         * 
         * @param onoff True to begin advertising, false to stop. 
         */
        void advertising_onoff(bool onoff);

        /**
         * @brief Checks for changes in the connection status and polls for BLE events
         * 
         * @return true If there is data waiting in the message queue
         * @return false False if there is no data in the message queue
         */
        bool handle_updates();

        /**
         * @brief Send a BLE message using the Nordic UART Service. The data is serialized with the parser object. 
         * 
         * @param msg The message that you would like to send.
         */
        void send_message(BleMessage &msg);

        /**
         * @brief Send an error code to the GUI, uses a seperate service and characteristic
         * 
         * @param error_code 
         */
        void send_error(int error_code, int joint_id);

    private:
        //BLE connection state
        int _connected = 0;

        // Testing
        
        ExoData* _data;
        uint8_t* _config_to_send;

        //The Gatt database which defines the services and characteristics
        GattDb _gatt_db = GattDb();

        //The parser used to serialize and deserialize the BLE data
        BleParser _ble_parser = BleParser();
};

/**
 * @brief Holds the callbacks for the data reception
 * 
 */
namespace ble_rx
{
    void on_rx_recieved(BLEDevice central, BLECharacteristic characteristic);
}

/**
 * @brief Holds the callbacks for the connection events, only used if using the Adafruit_Bluefruit_SPI_Friend
 * 
 */
namespace connection_callbacks
{
    static bool is_connected = false;
}

#endif // defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)


#endif