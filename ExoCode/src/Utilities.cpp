/*
 * Takes in the joint id and returns if the left indicator bit is set as a bool
 *
 */
#include "Utilities.h"
#include "Logger.h"
 
namespace utils
{
    bool get_is_left(config_defs::joint_id id)
    {
        return get_is_left((uint8_t) id);
    };
    
    bool get_is_left(uint8_t id)
    {
        return (id & (uint8_t)config_defs::joint_id::left) == (uint8_t)config_defs::joint_id::left;
    };

    uint8_t get_joint_type(config_defs::joint_id id)
    {
        return get_joint_type((uint8_t) id);
    };
    
    uint8_t get_joint_type(uint8_t id)
    {
        return id & (~(uint8_t)config_defs::joint_id::left & ~(uint8_t)config_defs::joint_id::right);  //Return the joint id with the left/right indicators masked out.  
    };
    
    bool schmitt_trigger(float value, bool is_high, float lower_threshold, float upper_threshold)
    {
        bool trigger = 0;
        if (is_high)
        {
            trigger = value > lower_threshold;  //One if true, Zero if false
        }
        else
        {
            trigger = value > upper_threshold;  //One if true, Zero if false
        }    
        return trigger;
    }
    
     //TODO: Add template so works with ints, floats, whatever.
     int rate_limit(int setpoint, int last_value, int* last_time, int rate_per_ms)
     {
        int time = millis();
        int step = (time - *last_time) * rate_per_ms;
        *last_time = time;
        
        return min(setpoint, last_value + step); 
     };
     
    uint8_t update_bit(uint8_t original, bool val, uint8_t loc)
    {
        uint8_t keep_bit = ~(1<<loc);  //Set a mask for the bits we aren't setting 
        
        return (original & keep_bit) | (val<<loc);
    };
    
    uint16_t update_bit(uint16_t original, bool val, uint8_t loc)
    {
        uint16_t keep_bit = ~(1<<loc);  //Set a mask for the bits we aren't setting 
        
        return (original & keep_bit) | (val<<loc);
    };
    
    bool get_bit(uint8_t original, uint8_t loc)
    {
        uint8_t bit_to_check = (1<<loc);  //Set a mask for the bits we are checking
        
        return (original & bit_to_check) == bit_to_check;
    };
    
    bool get_bit(uint16_t original, uint8_t loc)
    {
        uint8_t bit_to_check = (1<<loc);  //Set a mask for the bits we are checking
        
        return (original & bit_to_check) == bit_to_check;
    };
    
    float degrees_to_radians(float angle_deg)
    {
        return angle_deg * 2 * PI / 180;
    };
    
    float radians_to_degrees(float angle_rad)
    {
        return angle_rad * 180 / (2 * PI);
    };

    String remove_all_chars(String str, char rmv)
    {
        bool found = false;
        while(!found)
        {
            int index = str.indexOf(rmv);
            if (index == -1) 
            {
                found = true;
                continue;
            }
            str.remove(index, 1);
        }
        return str;
    };
    String remove_all_chars(char* arr, char rmv)
    {
        String str = String(arr);
        remove_all_chars(str, rmv);
    }

    /*
     * given an integer, return the number of characters in it
     */
    int get_char_length(int ofInt)
    {
        int len = 0;
        int localInt = ofInt;
        if (localInt < 0) {
            len += 1;
            
            //Quick abs(x)
            localInt = ((localInt < 0) ? -1 * localInt : localInt);
        }

        //Faster than loop
        if (localInt < 10) {
            len += 1;
        } else if (localInt < 100) {
            len += 2;
        } else if (localInt < 1000) {
            len += 3;
        } else if (localInt < 10000) {
            len += 4;
        } else if (localInt < 100000) {
            len += 5;
        } else if (localInt < 1000000) {
            len += 6;
        }
        return len;
    };
   
    SpeedCheck::SpeedCheck(int pin)
    {
        _pin = pin;
        _state = 0;
        
        pinMode(_pin, OUTPUT);
        digitalWrite(_pin, _state);
    };
    
    void SpeedCheck::toggle()
    {
        _state = _state ^ 1;
        
        digitalWrite(_pin, _state);
    };
    
    /*
     * Used to convert between floats and bytes 
     * 
     * !! NOT to be used outside of utils
     */
    union FloatByteUnion
    {
        float f;
        uint8_t b[sizeof(float)];
    };
    
    union ShortIntByteUnion
    {
        short int i;
        uint8_t b[sizeof(short int)];
    };
    
    bool is_little_endian()
    {
        FloatByteUnion val;
        switch (sizeof(float))
        {
            case 4: //32 bit
                val.f = 1.401298464324817e-45;  //Only works for 32 bit floats, arduino doesn't handle -0.
                return val.b[0] == 0x01;
                break;
            case 8: //64 bit
                val.f = 4.94065645841246544176568792868E-324;  //Only works for 64 bit floats, arduino doesn't handle -0.
                return val.b[0] == 0x01;
                break;
            default:
                //logger::println("Utilities :: is_little_endian() : System does not appear to be 32 or 64 bit");    
                break;
        }
        
    }
    
    void float_to_uint8(float num_to_convert, uint8_t *converted_bytes)
    {
        FloatByteUnion val;
        val.f = num_to_convert;
        int idx;
        // logger::println(val.f);
        for(uint8_t i = 0; i<sizeof(float); i++)
        {
            if (is_little_endian())
            {
                idx = i;
            }
            else
            {
                idx = sizeof(float)-i-1;
            }
            // logger::println(idx);
            // logger::println(val.b[0],HEX);
            converted_bytes[i] = val.b[idx];
            // logger::println(converted_bytes[i],HEX);
        }
        return;
    }
    
    void uint8_to_float(uint8_t *bytes_to_convert, float *converted_float)
    {
        FloatByteUnion val;
        int idx;
        // logger::println(bytes_to_convert[0],HEX);
        
        //Flip the idx if not little endian
        for(uint8_t i = 0; i<sizeof(float); i++)
        {
            if (is_little_endian())
            {
                idx = i;
            }
            else
            {
                idx = sizeof(float)-i-1;
            }
            // logger::println(idx);
            val.b[i] = bytes_to_convert[idx];
            // logger::println(val.b[i],HEX);
        }
        
        *converted_float = val.f;
        // logger::println(*converted_float);
         
        return;
    }
    
    void float_to_short_fixed_point_bytes(float num_to_convert, uint8_t *converted_bytes, uint8_t factor)
    {
        ShortIntByteUnion val;
        val.i = (short int) (num_to_convert * factor);
        int idx;
        // logger::println(val.f);
        for(uint8_t i = 0; i<sizeof(short int); i++)
        {
            if (is_little_endian())
            {
                idx = i;
            }
            else
            {
                idx = sizeof(short int)-i-1;
            }
            // logger::println(idx);
            // logger::println(val.b[0],HEX);
            converted_bytes[i] = val.b[idx];
            // logger::println(converted_bytes[i],HEX);
        }
        return;
    }
    
    void short_fixed_point_bytes_to_float(uint8_t *bytes_to_convert, float *converted_val, uint8_t factor)
    {
        ShortIntByteUnion val;
        int idx;
        // logger::println(bytes_to_convert[0],HEX);
        
        //Flip the idx if not little endian
        for(uint8_t i = 0; i<sizeof(short int); i++)
        {
            if (is_little_endian())
            {
                idx = i;
            }
            else
            {
                idx = sizeof(short int)-i-1;
            }
            // logger::println(idx);
            val.b[i] = bytes_to_convert[idx];
            // logger::println(val.b[i],HEX);
        }
        
        *converted_val = ((float)val.i/factor);
        // logger::println(*converted_float);
         
        return;
    }
    
    /* Exponentially weighted moving average filter. Takes in a new value, the current filtered value, and 
     * a filter parameter (alpha). The filter weights the new input. See the link below for more
     * information on this filter. 
     * 
     * https://en.wikipedia.org/wiki/Exponential_smoothing
     * https://towardsdatascience.com/time-series-from-scratch-exponentially-weighted-moving-averages-ewma-theory-and-implementation-607661d574fe 
     * 
     */
    float ewma(float new_value, float filter_value, float alpha)
    {
        return (filter_value + alpha*(new_value-filter_value));
    }
    
    /*
     * Takes in the in a byte and if it is 0xFF changes to 0xFE
     *
     */
    uint8_t ff_to_fe(uint8_t val)
    {
        if(0xFF==val)
        {
            return 0xFE;
        }
        return val;
    }

    void spin_on_error_with(String message)
    {
        for (;;)
        {
            logger::println(message);
            delay(500);
        }
    }

    bool is_close_to(float val1, float val2, float tolerance)
    {
        return (abs(val1-val2) < tolerance);
    }

    std::pair<float, float> online_std_dev(std::queue<float> set)
    {
        std::queue<float> set_copy = set;
        
        //Calculate the std dev
        float mean = 0;
        float M2 = 0;
        float delta = 0;
        float variance = 0;
        float std_dev = 0;
        int n = 0;
        while (!set_copy.empty())
        {
            n++;
            float x = set_copy.front();
            set_copy.pop();
            delta = x - mean;
            mean = mean + delta / n;
            M2 = M2 + delta * (x - mean);
        }

        if (n < 2)
        {
            variance = 0;
            std_dev = 0;
        }
        else
        {
            variance = M2 / (n - 1);
            std_dev = sqrt(variance);
        }

        return std::make_pair(mean, std_dev);
    }

    bool is_outside_range(float val, float min, float max)
    {
        return (val < min || val > max);
    }

    std::string trimControllerName(const std::string& input) {
    size_t pos = input.find('>');
    if (pos != std::string::npos && pos + 1 < input.size()) {
        // Return the substring starting after the '>' character
        return input.substr(pos + 1);
    }
    // Return an empty string if '>' is not found
    return input;
}
}