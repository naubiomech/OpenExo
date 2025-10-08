#ifndef REAL_TIME_I2C_H
#define REAL_TIME_I2C_H
#include "Arduino.h"

namespace rt_data 
{
    static int BILATERAL_HIP_ANKLE_RT_LEN = 11;
    static int BILATERAL_ANKLE_RT_LEN = 11;
    static int BILATERAL_HIP_RT_LEN = 11;
    static int BILATERAL_ELBOW_RT_LEN = 11;
    static int BILATERAL_HIP_ELBOW_RT_LEN = 11;
    static int BILATERAL_ANKLE_ELBOW_RT_LEN = 11;
    static const uint8_t len = BILATERAL_HIP_ANKLE_RT_LEN;
    static float* float_values = new float(len);

    static bool new_rt_msg = false;
};

namespace real_time_i2c
{
    void msg(float* data, int len);
    bool poll(float* pack_array);
    void init();
};

#endif