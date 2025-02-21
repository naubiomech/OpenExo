/**
 * @file HipIMU.h
 *
 * @brief Declares a class for controlling and reading hip IMU
 * 
 * @author C. Gaudette 
 * @date Feb. 2024
*/

#ifndef HipIMU_h
#define HipIMU_h

//Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)

#include "Arduino.h"

#include "ExoData.h"
#include "Controller.h"
#include "HipIMUData.h"

/**
 * @brief Class used to define the interface for the hip imu.
 * All joints should have a:
 * 
 * 
 */
class _Joint
{

    public:
    _HipIMU(config_defs::joint_id id, ExoData* exo_data);
    virtual ~_HipIMU(){};

    read_imu(int pin);

    protected:
    HipIMUData* _hip_imu_data;

}

#endif
#endif