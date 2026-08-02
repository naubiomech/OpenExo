#include "ThIMU.h"
#include "Logger.h"

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)
/*
#if defineds(ARDUINO_TEENSY36)
    #define MY_WIRE Wire1
#elif defined(ARDUINO_TEENSY41)
    #define MY_WIRE Wire
#endif

class ThIMU
{
    public:
    ThIMU(bool is_left);
    bool init(float timeout);
    float read_data();

    private:
    bool _handshake();
    bool _is_left;
    bool _is_initialized;
};
*/

#include "Config.h"

//TODO: Integrate with Board.h
#if BOARD_VERSION == AK_Board_V0_3
    #define MY_WIRE Wire1
#else 
    #define MY_WIRE Wire
#endif

ThIMU::ThIMU(bool is_left)
{
    _is_left = is_left;
    _is_initialized = false;
}

bool ThIMU::init(float timeout)
{
    MY_WIRE.begin();

    //Handshake
    float start_time = millis();
    while (!_handshake()) 
    {
        float delta_time = millis() - start_time;
        
        //logger::print("Handshake failed: ");
        //logger::println(delta_time);
        if (delta_time > timeout) 
        {
            return false;
        }
    }
    _is_initialized = true;
}

float ThIMU::read_data()
{
    if (!_is_initialized) 
    {
        return 0;
    }

    uint8_t addr = (_is_left) ? i2c_cmds::thigh_imu::left_addr : i2c_cmds::thigh_imu::right_addr;

    uint8_t data;
    MY_WIRE.beginTransmission(addr);
    MY_WIRE.write(i2c_cmds::thigh_imu::get_angle::reg);
    MY_WIRE.endTransmission();
    MY_WIRE.requestFrom(addr, i2c_cmds::thigh_imu::get_angle::len, false);
    data = MY_WIRE.read();
    MY_WIRE.endTransmission();

    return data;
}

bool ThIMU::_handshake()
{
    uint8_t addr = (_is_left) ? i2c_cmds::thigh_imu::left_addr : i2c_cmds::thigh_imu::right_addr;

    //logger::print("Handshaking with IMU at address: ");
    //logger::println(addr);

    MY_WIRE.beginTransmission(addr);
    MY_WIRE.write(i2c_cmds::thigh_imu::handshake::reg);
    MY_WIRE.endTransmission();
    delay(10);
    MY_WIRE.requestFrom(addr, i2c_cmds::thigh_imu::handshake::len, false);
    uint8_t val = MY_WIRE.read();
    MY_WIRE.endTransmission();

    //logger::print("Handshake value: ");
    //logger::println(val);

    if (val == 0x01) 
    {
        return true;
    } else 
    {
        return false;
    }
    
}

#endif

