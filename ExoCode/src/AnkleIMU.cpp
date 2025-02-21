#include "AnkleIMU.h"
#include <Arduino.h>
#include <utility/imumaths.h>
#include "Logger.h"

#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41) 

#if BOARD_VERSION == AK_Board_V0_3 
#define IMU_WIRE Wire1
#endif
static const int left_address = 0x29;
static const int right_address = 0x28;
static const int BNO055_EULER_R_LSB_ADDR = 0X1C;
static const int BNO055_EULER_R_MSB_ADDR = 0x1D;

/**
 * @brief Construct a new Ankle I M U:: Ankle I M U object
 * 
 * @param is_left 
 */
AnkleIMU::AnkleIMU(bool is_left) : _is_left{is_left}
{
    const int addr = (is_left)?(left_address):(right_address);
    _addr = addr;

	#if BOARD_VERSION == AK_Board_V0_3 
        //_imu = Adafruit_BNO055(-1, addr, &IMU_WIRE);
	#endif

    if(!_imu.begin())
    {
        const String dir_str = (is_left)?("Left"):("Right"); 
        logger::print(dir_str + " AnkleIMU cannot initialize!", LogLevel::Error);
        return;
    }

    _initialized = true;
    _imu.setExtCrystalUse(true);
}

float AnkleIMU::get_global_angle()
{
    if (!_initialized)
    {
        logger::print("AnkleIMU unitialized!", LogLevel::Error);
        return 0;
    }

    //Get raw data
    uint8_t raw_data[2];
	
#if BOARD_VERSION == AK_Board_V0_3 
        IMU_WIRE.beginTransmission(_addr);
        //IMU_WIRE.write(BNO055_EULER_R_LSB_ADDR);
        IMU_WIRE.endTransmission();
        IMU_WIRE.requestFrom(_addr, 2, false);
        raw_data[0] = IMU_WIRE.read();
        raw_data[1] = IMU_WIRE.read();
        IMU_WIRE.endTransmission();
	#endif

    //Get float data
    const int16_t int_data = ((int16_t)raw_data[0]) | (((int16_t)raw_data[1]) << 8);
    const float y_angle = ((float)int_data) / 16.0f;
    return y_angle;

    // imu::Vector<3> euler = _imu.getVector(Adafruit_BNO055::VECTOR_EULER);
    // return euler.y();
}

#endif