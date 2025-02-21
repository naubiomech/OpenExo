/**
 * @file JointData.h
 *
 * @brief Declares a class used to store data for joint to access 
 * 
 * @author P. Stegall 
 * @date Jan. 2022
*/


#ifndef JointData_h
#define JointData_h

#include "Arduino.h"

class ExoData;

class HipIMUData{

    union {
            float estim_angle;
            byte angle_bytes[4];
    }data;

}



#endif