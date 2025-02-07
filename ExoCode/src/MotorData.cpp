#include "MotorData.h"
#include "ParseIni.h"
#include "Logger.h"

/*
 * Constructor for the motor data.
 * Takes the joint id and the array from the INI parser.
 * Stores the id, if it is on the left side (for convenience), and the motor type
 * It also has the info for the motor CAN packages.
 */
MotorData::MotorData(config_defs::joint_id id, uint8_t* config_to_send)
{
    this->id = id;
    this->is_left = ((uint8_t)this->id & (uint8_t)config_defs::joint_id::left) == (uint8_t)config_defs::joint_id::left;
    
    switch ((uint8_t)this->id & (~(uint8_t)config_defs::joint_id::left & ~(uint8_t)config_defs::joint_id::right))  //Use the id with the side masked out.
    {
        case (uint8_t)config_defs::joint_id::hip:
        {
            motor_type = config_to_send[config_defs::hip_idx];
            
            switch (config_to_send[config_defs::hip_gear_idx])
            {
                case (uint8_t)config_defs::gearing::gearing_1_1:
                {
                    gearing = 1;
                    break;
                }
                case (uint8_t)config_defs::gearing::gearing_2_1:
                {
                    gearing = 2;
                    break;
                }                
                case (uint8_t)config_defs::gearing::gearing_3_1:
                {
                    gearing = 3;
                    break;
                }
                case (uint8_t)config_defs::gearing::gearing_4_5_1:
                {
                    gearing = 4.5;
                    break;
                }
                default:
                {
                    gearing = 1;
                    break;
                }
            }  
            if ((config_to_send[config_defs::hip_flip_motor_dir_idx] == (uint8_t)config_defs::flip_motor_dir::both) || ((config_to_send[config_defs::hip_flip_motor_dir_idx] == (uint8_t)config_defs::flip_motor_dir::left) && this->is_left) || ((config_to_send[config_defs::hip_flip_motor_dir_idx] == (uint8_t)config_defs::flip_motor_dir::right) && (!this->is_left)))
            {
                this->flip_direction = 1;
            }
            else
            {
                this->flip_direction = 0;
            }
            break;
        }
        case (uint8_t)config_defs::joint_id::knee:
        {
            motor_type = config_to_send[config_defs::knee_idx];
            
            switch (config_to_send[config_defs::knee_gear_idx])
            {
                case (uint8_t)config_defs::gearing::gearing_1_1:
                {
                    gearing = 1;
                    break;
                }
                case (uint8_t)config_defs::gearing::gearing_2_1:
                {
                    gearing = 2;
                    break;
                } 
                case (uint8_t)config_defs::gearing::gearing_3_1:
                {
                    gearing = 3;
                    break;
                }
                case (uint8_t)config_defs::gearing::gearing_4_5_1:
                {
                    gearing = 4.5;
                    break;
                }
                default:
                {
                    gearing = 1;
                    break;
                }
            }  
            
            if ((config_to_send[config_defs::knee_flip_motor_dir_idx] == (uint8_t)config_defs::flip_motor_dir::both) || ((config_to_send[config_defs::knee_flip_motor_dir_idx] == (uint8_t)config_defs::flip_motor_dir::left) && this->is_left) || ((config_to_send[config_defs::knee_flip_motor_dir_idx] == (uint8_t)config_defs::flip_motor_dir::right) && (!this->is_left)))
            {
                this->flip_direction = 1;
            }
            else
            {
                this->flip_direction = 0;
            }
            break;
        }
        case (uint8_t)config_defs::joint_id::ankle:
        {
            motor_type = config_to_send[config_defs::ankle_idx];
            
            switch (config_to_send[config_defs::ankle_gear_idx])
            {
                case (uint8_t)config_defs::gearing::gearing_1_1:
                {
                    gearing = 1;
                    break;
                }
                case (uint8_t)config_defs::gearing::gearing_2_1:
                {
                    gearing = 2;
                    break;
                } 
                case (uint8_t)config_defs::gearing::gearing_3_1:
                {
                    gearing = 3;
                    break;
                }
                case (uint8_t)config_defs::gearing::gearing_4_5_1:
                {
                    gearing = 4.5;
                    break;
                }
                default:
                {
                    gearing = 1;
                    break;
                }
            }  
            
            if ((config_to_send[config_defs::ankle_flip_motor_dir_idx] == (uint8_t)config_defs::flip_motor_dir::both) || ((config_to_send[config_defs::ankle_flip_motor_dir_idx] == (uint8_t)config_defs::flip_motor_dir::left) && this->is_left) || ((config_to_send[config_defs::ankle_flip_motor_dir_idx] == (uint8_t)config_defs::flip_motor_dir::right) && (!this->is_left)))
            {
                this->flip_direction = 1;
            }
            else
            {
                this->flip_direction = 0;
            }
            break;
        }
        case (uint8_t)config_defs::joint_id::elbow:
        {
            motor_type = config_to_send[config_defs::elbow_idx];

            switch (config_to_send[config_defs::elbow_gear_idx])
            {
            case (uint8_t)config_defs::gearing::gearing_1_1:
            {
                gearing = 1;
                break;
            }
            case (uint8_t)config_defs::gearing::gearing_2_1:
            {
                gearing = 2;
                break;
            }
            case (uint8_t)config_defs::gearing::gearing_3_1:
            {
                gearing = 3;
                break;
            }
            case (uint8_t)config_defs::gearing::gearing_4_5_1:
            {
                gearing = 4.5;
                break;
            }
            default:
            {
                gearing = 1;
                break;
            }
            }

            if ((config_to_send[config_defs::elbow_flip_motor_dir_idx] == (uint8_t)config_defs::flip_motor_dir::both) || ((config_to_send[config_defs::elbow_flip_motor_dir_idx] == (uint8_t)config_defs::flip_motor_dir::left) && this->is_left) || ((config_to_send[config_defs::elbow_flip_motor_dir_idx] == (uint8_t)config_defs::flip_motor_dir::right) && (!this->is_left)))
            {
                this->flip_direction = 1;
            }
            else
            {
                this->flip_direction = 0;
            }
            break;
        }
    }
    
    
    //For AK-Series Motors Only
    p = 0;      //Read position
    v = 0;      //Read velocity
    i = 0;      //Read current
    p_des = 0;  
    v_des = 0;
    kp = 0;
    kd = 0;
    t_ff = 0;
    kt = 0;
    
};

void MotorData::reconfigure(uint8_t* config_to_send) 
{
    switch ((uint8_t)this->id & (~(uint8_t)config_defs::joint_id::left & ~(uint8_t)config_defs::joint_id::right))  //Use the id with the side masked out.
    {
        case (uint8_t)config_defs::joint_id::hip:
        {
            motor_type = config_to_send[config_defs::hip_idx];
            
            switch (config_to_send[config_defs::hip_gear_idx])
            {
                case (uint8_t)config_defs::gearing::gearing_1_1:
                {
                    gearing = 1;
                    break;
                }
                case (uint8_t)config_defs::gearing::gearing_2_1:
                {
                    gearing = 2;
                    break;
                }                
                case (uint8_t)config_defs::gearing::gearing_3_1:
                {
                    gearing = 3;
                    break;
                }
                case (uint8_t)config_defs::gearing::gearing_4_5_1:
                {
                    gearing = 4.5;
                    break;
                }
                default:
                {
                    gearing = 1;
                    break;
                }
            }  
            if ((config_to_send[config_defs::hip_flip_motor_dir_idx] == (uint8_t)config_defs::flip_motor_dir::both) || ((config_to_send[config_defs::hip_flip_motor_dir_idx] == (uint8_t)config_defs::flip_motor_dir::left) && this->is_left) || ((config_to_send[config_defs::hip_flip_motor_dir_idx] == (uint8_t)config_defs::flip_motor_dir::right) && (!this->is_left)))
            {
                this->flip_direction = 1;
            }
            else
            {
                this->flip_direction = 0;
            }
            break;
        }
        case (uint8_t)config_defs::joint_id::knee:
        {
            motor_type = config_to_send[config_defs::knee_idx];
            
            switch (config_to_send[config_defs::knee_gear_idx])
            {
                case (uint8_t)config_defs::gearing::gearing_1_1:
                {
                    gearing = 1;
                    break;
                }
                case (uint8_t)config_defs::gearing::gearing_2_1:
                {
                    gearing = 2;
                    break;
                } 
                case (uint8_t)config_defs::gearing::gearing_3_1:
                {
                    gearing = 3;
                    break;
                }
                case (uint8_t)config_defs::gearing::gearing_4_5_1:
                {
                    gearing = 4.5;
                    break;
                }
                default:
                {
                    gearing = 1;
                    break;
                }
            }  
            
            if ((config_to_send[config_defs::knee_flip_motor_dir_idx] == (uint8_t)config_defs::flip_motor_dir::both) || ((config_to_send[config_defs::knee_flip_motor_dir_idx] == (uint8_t)config_defs::flip_motor_dir::left) && this->is_left) || ((config_to_send[config_defs::knee_flip_motor_dir_idx] == (uint8_t)config_defs::flip_motor_dir::right) && (!this->is_left)))
            {
                this->flip_direction = 1;
            }
            else
            {
                this->flip_direction = 0;
            }
            break;
        }
        case (uint8_t)config_defs::joint_id::ankle:
        {
            motor_type = config_to_send[config_defs::ankle_idx];
            
            switch (config_to_send[config_defs::ankle_gear_idx])
            {
                case (uint8_t)config_defs::gearing::gearing_1_1:
                {
                    gearing = 1;
                    break;
                }
                case (uint8_t)config_defs::gearing::gearing_2_1:
                {
                    gearing = 2;
                    break;
                } 
                case (uint8_t)config_defs::gearing::gearing_3_1:
                {
                    gearing = 3;
                    break;
                }
                case (uint8_t)config_defs::gearing::gearing_4_5_1:
                {
                    gearing = 4.5;
                    break;
                }
                default:
                {
                    gearing = 1;
                    break;
                }
            }  
            
            if ((config_to_send[config_defs::ankle_flip_motor_dir_idx] == (uint8_t)config_defs::flip_motor_dir::both) || ((config_to_send[config_defs::ankle_flip_motor_dir_idx] == (uint8_t)config_defs::flip_motor_dir::left) && this->is_left) || ((config_to_send[config_defs::ankle_flip_motor_dir_idx] == (uint8_t)config_defs::flip_motor_dir::right) && (!this->is_left)))
            {
                this->flip_direction = 1;
            }
            else
            {
                this->flip_direction = 0;
            }
            break;
        }
        case (uint8_t)config_defs::joint_id::elbow:
        {
            motor_type = config_to_send[config_defs::elbow_idx];

            switch (config_to_send[config_defs::elbow_gear_idx])
            {
            case (uint8_t)config_defs::gearing::gearing_1_1:
            {
                gearing = 1;
                break;
            }
            case (uint8_t)config_defs::gearing::gearing_2_1:
            {
                gearing = 2;
                break;
            }
            case (uint8_t)config_defs::gearing::gearing_3_1:
            {
                gearing = 3;
                break;
            }
            case (uint8_t)config_defs::gearing::gearing_4_5_1:
            {
                gearing = 4.5;
                break;
            }
            default:
            {
                gearing = 1;
                break;
            }
            }

            if ((config_to_send[config_defs::elbow_flip_motor_dir_idx] == (uint8_t)config_defs::flip_motor_dir::both) || ((config_to_send[config_defs::elbow_flip_motor_dir_idx] == (uint8_t)config_defs::flip_motor_dir::left) && this->is_left) || ((config_to_send[config_defs::elbow_flip_motor_dir_idx] == (uint8_t)config_defs::flip_motor_dir::right) && (!this->is_left)))
            {
                this->flip_direction = 1;
            }
            else
            {
                this->flip_direction = 0;
            }
            break;
        }
    }
    
    //For AK-Series Motors Only
    p = 0;      //Read position
    v = 0;      //Read velocity
    i = 0;      //Read current
    p_des = 0;  
    v_des = 0;
    kp = 0;
    kd = 0;
    t_ff = 0;
    last_command = 0;
};