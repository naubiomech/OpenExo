#if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)

#include "ExoBLE.h"
#include "Utilities.h"
#include "Time_Helper.h"
#include "ComsLed.h"
#include "Config.h"
#include "error_codes.h"
#include "Logger.h"

#define EXOBLE_DEBUG 0

ExoBLE::ExoBLE(ExoData* data, uint8_t config_to_send)
{
    _data = data;
    _config_to_send = config_to_send;  // Fix: was assigning to parameter instead of member
}

bool ExoBLE::setup()
{
    if (!BLE.begin())
    {
        utils::spin_on_error_with("BLE.begin() failed");
        return false;
    }

    //Setup name and initialize data
    String name = utils::remove_all_chars(BLE.address(), ':');
    name.remove(name.length() - MAC_ADDRESS_NAME_LENGTH);
    name = NAME_PREAMBLE + name;

    //Using exo_info namespace defined in Config.h
    String FirmwareVersion = exo_info::FirmwareVersion; //String to add to firmware char
    String PCBVersion = exo_info::PCBVersion;           //String to add to pcb char
    String DeviceName = exo_info::DeviceName;           //String to add to device char

    //Check if the name is null, if it is use the name above, if not check for preamble
    if (DeviceName == "NULL")
    {
        DeviceName = name;
    }
    else
    {
        //Check if the name has the preamble, if not add it
        if (!DeviceName.startsWith(NAME_PREAMBLE))
        {
            DeviceName = NAME_PREAMBLE + DeviceName;
        }
    }

    //Initialize char arrays
    char name_char[name.length()];
    char firmware_char[FirmwareVersion.length()];
    char pcb_char[PCBVersion.length()];
    char device_char[DeviceName.length()];

    //Add data to array
    name.toCharArray(name_char, name.length() + 1);
    FirmwareVersion.toCharArray(firmware_char, FirmwareVersion.length() + 1);
    PCBVersion.toCharArray(pcb_char, PCBVersion.length() + 1);
    DeviceName.toCharArray(device_char, DeviceName.length() + 1);

    //Create pointer that pointes to array
    const char *k_name_pointer = name_char;
    const char *firmware_pointer = firmware_char;
    const char *pcb_pointer = pcb_char;
    const char *device_pointer = device_char;

    //Set name for device
    BLE.setLocalName(k_name_pointer);
    BLE.setDeviceName(k_name_pointer);

    //Initialize GATT DB
    _gatt_db.FirmwareChar.writeValue(firmware_char);
    _gatt_db.PCBChar.writeValue(pcb_char);
    _gatt_db.DeviceChar.writeValue(device_char);
    send_error(0, 0);

    //Configure services and advertising data
    BLE.setAdvertisedService(_gatt_db.UARTService);

    //UART Chars
    _gatt_db.UARTService.addCharacteristic(_gatt_db.TXChar);
    _gatt_db.UARTService.addCharacteristic(_gatt_db.RXChar);

    //Device Info Chars
    _gatt_db.UARTServiceDeviceInfo.addCharacteristic(_gatt_db.PCBChar);
    _gatt_db.UARTServiceDeviceInfo.addCharacteristic(_gatt_db.FirmwareChar);
    _gatt_db.UARTServiceDeviceInfo.addCharacteristic(_gatt_db.DeviceChar);

    //Error Char
    _gatt_db.ErrorService.addCharacteristic(_gatt_db.ErrorChar);

    BLE.addService(_gatt_db.UARTService);
    BLE.addService(_gatt_db.UARTServiceDeviceInfo);
    BLE.addService(_gatt_db.ErrorService);

    _gatt_db.RXChar.setEventHandler(BLEWritten, ble_rx::on_rx_recieved);
    BLE.setConnectionInterval(6, 6);
    advertising_onoff(true);

    return true;
}

void ExoBLE::advertising_onoff(bool onoff)
{
    if (onoff)
    {
        //Start Advertising
        // logger::println("Start Advertising");
        BLE.advertise();

        //Turn the blue led off
        ComsLed *led = ComsLed::get_instance();
        uint8_t r, g, b;
        led->get_color(&r, &g, &b);
        led->set_color(r, g, 0);
    }
    else
    {
        //Stop Advertising
        // logger::println("Stop Advertising");
        BLE.stopAdvertise();

        //Turn the blue led on
        ComsLed *led = ComsLed::get_instance();
        uint8_t r, g, b;
        led->get_color(&r, &g, &b);
        led->set_color(r, g, 255);
    }
}

bool ExoBLE::handle_updates()
{
    #if EXOBLE_DEBUG
        logger::print("ExoBLE::handle_updates:Start");
        logger::print("\n");
    #endif

    static Time_Helper *t_helper = Time_Helper::get_instance();
    static float update_context = t_helper->generate_new_context();
    static float del_t = 0;
    del_t += t_helper->tick(update_context);

    if (del_t > BLE_times::_update_delay)
    {
        del_t = 0;
        #if EXOBLE_DEBUG
            static float poll_context = t_helper->generate_new_context();
            static float poll_time = 0;
            static float connected_context = t_helper->generate_new_context();
            static float connected_time = 0;
        #endif

        //Poll for updates and check connection status
        #if EXOBLE_DEBUG
            logger::print("Poll for updates and check connection status");
            logger::print("\n");
        #endif

        BLE.poll();
        int32_t current_status = BLE.connected();

        if (_connected == current_status)
        {
            #if EXOBLE_DEBUG
                logger::print("ExoBLE::handle_updates:queue size:");
                logger::print(ble_queue::size());
                logger::print("\n");
            #endif

            return ble_queue::size();
        }

        //The BLE connection status changed
        if (current_status < _connected)
        {
            //Disconnection
            #if EXOBLE_DEBUG
                logger::print("Disconnection");
                logger::print("\n");
            #endif
        }
        else if (current_status > _connected)
        {
            //Connection
            #if EXOBLE_DEBUG
                logger::print("Connection");
                logger::print("\n");
            #endif
        }

        advertising_onoff(current_status == 0);
        _connected = current_status;
    }

    #if EXOBLE_DEBUG
        logger::print("ExoBLE::handle_updates:queue size:");
        logger::print(ble_queue::size());
        logger::print("\n");
    #endif

    return ble_queue::size();
}

void ExoBLE::send_message(BleMessage &msg)
{
    if (!this->_connected)
    {
        return; /* Don't bother sending anything if no one is listening */
    }

    #if EXOBLE_DEBUG
        BleMessage::print(msg);
    #endif

    static const int k_preamble_length = 3;
    int max_payload_length = ((k_preamble_length + msg.expecting) * (MAX_PARSER_CHARACTERS + 1));
    byte buffer[max_payload_length];

    int bytes_to_send = _ble_parser.package_raw_data(buffer, msg);

    _gatt_db.TXChar.writeValue(buffer, bytes_to_send);
}

void ExoBLE::send_message_w_string(BleMessage &msg, const char* msg_text)
{
    if (!this->_connected)
    {
        return; /* Don't bother sending anything if no one is listening */
    }
    #if EXOBLE_DEBUG
        BleMessage::print(msg);
    #endif

    static const int k_preamble_length = 3;
    int max_payload_length = ((k_preamble_length + msg.expecting) * (MAX_PARSER_CHARACTERS + 1));
    byte buffer[max_payload_length];

    int bytes_to_send = _ble_parser.package_raw_data(buffer, msg);

    _gatt_db.TXChar.writeValue(msg_text, false);
}

void ExoBLE::sendInitialParameterNames()
{
        // Add timer logic here
    static unsigned long first_connect_start_time = 0;
    static bool timer_started = false;
    
    Serial.println("first connect");
    UART_msg_t msg; // make a message  so we can properly call get_real_time_data, this msg is currently blank
    Serial.println("getting real time data");
    UART_command_handlers::initialize_parameter_names(_data, _data->config);
    static const int num_entries = UART_command_handlers::num_entries;
    Serial.println("got param names arr");

    //for each entry in the param_names_arr, send the name to the GUI individually
    for (int i = 0; i < num_entries; i++)
    {
        Serial.print(i);
        Serial.print(": ");
        std::string param_name = UART_command_handlers::param_names_arr[i];
        Serial.println(param_name.c_str());
        int success = _gatt_db.TXChar.writeValue(param_name.c_str(), true);
    }
    std::string end_str = "END"; //marks the end of the parameter names list
    _gatt_db.TXChar.writeValue(end_str.c_str(), true);
}

void ExoBLE::send_error(int error_code, int joint_id)
{
    if (!this->_connected)
    {
        return; /* Don't bother sending anything if no one is listening */
    }

    #if EXOBLE_DEBUG
        logger::print("Exoble::send_error->Sending: ", LogLevel::Error);
        logger::print(joint_id, LogLevel::Error);
        logger::print(", ", LogLevel::Error);
        logger::print(error_code, LogLevel::Error);
        logger::print("\n");
    #endif

    String error_string = String(error_code) + ":" + String(joint_id);
    
    //Convert to char array
    char error_char[error_string.length() + 1];
    error_string.toCharArray(error_char, error_string.length() + 1);

    _gatt_db.ErrorChar.writeValue(error_char);
}

void ble_rx::on_rx_recieved(BLEDevice central, BLECharacteristic characteristic)
{
    static BleMessage *empty_msg = new BleMessage();
    static BleParser *parser = new BleParser();
    static BleMessage *msg = new BleMessage();

    //Must reset message to avoid duplicate data
    (*msg) = *empty_msg;

    char data[32] = {0};
    int len = characteristic.valueLength();
    characteristic.readValue(data, len);

        #if EXOBLE_DEBUG
            logger::print("On Rx Recieved: ");
            for (int i=0; i<len;i++)
            {
                logger::print(data[i]);
                logger::print(", ");
            }
            logger::print("\n");
        #endif

    msg = parser->handle_raw_data(data, len);
    
    if (msg->is_complete)
    {
        #if EXOBLE_DEBUG
            logger::print("on_rx_recieved->Command: ");
            BleMessage::print(*msg);
        #endif

        ble_queue::push(msg);
    }

    #if EXOBLE_DEBUG
        logger::print("on_rx_recieved->End\n");
    #endif
}

#endif // defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)