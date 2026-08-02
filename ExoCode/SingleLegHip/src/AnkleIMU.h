/**
 * @file AnkleIMU.h
 * @author Chancelor Cuddeback
 * @brief 
 * @version 0.1
 * @date 2023-07-18
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef ANKLEIMU_H
#define ANKLEIMU_H
#include <Adafruit_BNO055.h>

/**
 * @brief Class for interfacing with the ankle IMU
 * 
 */
class AnkleIMU
{
    public:
    
     /**
     * @brief Construct a new Ankle I M U object
     * 
     * @param is_left 
     */
    AnkleIMU(bool is_left);
    
    /**
     * @brief Get the global angle object
     * 
     * @return float 
     */
    float get_global_angle();

    private:
    Adafruit_BNO055 _imu;
    int _addr;
    const bool _is_left;
    bool _initialized{false};
    
};


#endif