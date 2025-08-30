#include "RealTimeI2C.h"

#include "Config.h"
#include "Utilities.h"
#include "Logger.h"
#include <Wire.h>

//#define RT_I2C_DEBUG 1

#define FIXED_POINT_FACTOR 100

#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)
    #define HOST 1
    #if BOARD_VERSION == AK_Board_V0_3
    #define MY_WIRE Wire1
    #else 
    #define MY_WIRE Wire
    #endif
#else if defined(ARDUINO_ARDUINO_NANO33BLE)
    #define HOST 0
    #define MY_WIRE Wire
#endif

#define RT_I2C_ADDR 0x02
#define RT_I2C_REG 0x02

static volatile bool new_bytes = false;
static const int byte_buffer_len = rt_data::len * sizeof(float)/sizeof(short int) + 2;
static uint8_t* const byte_buffer = new uint8_t(byte_buffer_len);
static float* float_values = new float(rt_data::len);

static uint8_t _packed_len(uint8_t len)
{
    uint8_t packed_len = 0;
    // Calculate bytes needed for data: len * sizeof(float) / sizeof(short int)
    // Since sizeof(float) = 4 and sizeof(short int) = 2, this is len * 2
    packed_len += len * 2;  // 2 bytes per float (converted to short int)
    packed_len += 2; //Preamble 
    return packed_len;
}

static void _pack(uint8_t msg_id, uint8_t len, float *data, uint8_t *data_to_pack)
{
    //Pack metadata
    data_to_pack[0] = msg_id;
    data_to_pack[1] = len; // Deleted +2 for preamble, moved to _packed_len
    
    //Convert float array to short int array
    uint8_t _num_bytes = sizeof(float)/sizeof(short int);
    uint8_t buf[_num_bytes];
    for (int i=0; i<len; i++)
    {
        utils::float_to_short_fixed_point_bytes(data[i], buf, FIXED_POINT_FACTOR);
        uint8_t _offset = (2) + _num_bytes*i;
        memcpy((data_to_pack + _offset), buf, _num_bytes);
    }
}

void real_time_i2c::msg(float* data, int len)
{
    const uint8_t packed_len = _packed_len(len);
    uint8_t bytes[packed_len];
    _pack((uint8_t)RT_I2C_REG, len, data, bytes);

    #if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)
        MY_WIRE.beginTransmission(RT_I2C_ADDR);
        MY_WIRE.send(bytes, packed_len);
        MY_WIRE.endTransmission();
    #endif
}

#if defined(ARDUINO_ARDUINO_NANO33BLE)
//Warning: This is an interrupt and needs to be kept as short as possible. (NO SERIAL PRINTS)
//Also, any variables that are stored outside of its scope must be marked volatile and need to have their concurrency managed
static void on_receive(int byte_len)
{
    if (byte_len != byte_buffer_len) 
    {
        return;
    }
    for (int i=0; i<byte_len; i++)
    {
        byte_buffer[i] = MY_WIRE.read();
    }
    new_bytes = true;
}
#endif

void real_time_i2c::init()
{
    #if HOST
        MY_WIRE.begin();
    #else
        MY_WIRE.begin(RT_I2C_ADDR);
        MY_WIRE.onReceive(on_receive);
    #endif
}

bool real_time_i2c::poll(float* pack_array) 
{
    #if RT_I2C_DEBUG
        logger::println("real_time_i2c::poll()->Start");
    #endif

    if (!new_bytes)
    {
        #if RT_I2C_DEBUG
            logger::println("real_time_i2c::poll()->End (no new bytes)");
        #endif
        return false;
    }

    noInterrupts(); 
    const uint8_t len = byte_buffer[1];
    uint8_t buff[byte_buffer_len];
    memcpy(buff, byte_buffer, byte_buffer_len);
    new_bytes = false;
    interrupts();

    #if RT_I2C_DEBUG
        logger::print("real_time_i2c::poll()->Done copying bytes, len: ");
        logger::print(len);
        logger::println();
    #endif

    for (int i=0; i<(len); i++)
    {
        uint8_t data_offset = (2) + (i*2); //Preamble plus i * sizeof(float)/sizeof(short int)
        float tmp = 0;
        utils::short_fixed_point_bytes_to_float((uint8_t*)(buff+data_offset), &tmp, FIXED_POINT_FACTOR);
        pack_array[i] =  tmp;

        #if RT_I2C_DEBUG
            logger::print("real_time_i2c::poll()->i: ");
            logger::print(i);
            logger::print(" data_offset: ");
            logger::print(data_offset);
            logger::print(" buff[data_offset]: ");
            logger::print(buff[data_offset]);
            logger::print(" buff[data_offset+1]: ");
            logger::print(buff[data_offset+1]);
            logger::print(" tmp: ");
            logger::print(tmp);
            logger::print("\n");
        #endif
    }
    

    #if RT_I2C_DEBUG
        logger::println("real_time_i2c::poll()->End");
    #endif
    return true;
}