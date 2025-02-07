#include "JointData.h"
#include "Logger.h"

/*
 * Constructor for the joint data.
 * Takes the joint id and the array from the INI parser.
 * Stores the id, torque sensor reading, if it is on the left side (for convenience), and if the joint is used.
 * Uses an initializer list for the motor and controller data. 
 */
JointData::JointData(config_defs::joint_id id, uint8_t* config_to_send, float joint_RoM, bool do_flip_angle, float torque_offset)
: motor(id, config_to_send)
, controller(id, config_to_send)
, joint_RoM(joint_RoM)
, do_flip_angle(do_flip_angle)
, torque_offset(torque_offset)
{
    
    //Set all the data based on the id and configuration
    this->id = id;
    
    this->torque_reading = 0;
    this->is_left = ((uint8_t)this->id & (uint8_t)config_defs::joint_id::left) == (uint8_t)config_defs::joint_id::left;
    
    this->position = 0;
    this->velocity = 0;
    this->calibrate_torque_sensor = 0;
    this->joint_position = 0;
    this->joint_global_angle = 0;
    this->prev_joint_position = 0;
    this->joint_velocity = 0;
    
    switch ((uint8_t)this->id & (~(uint8_t)config_defs::joint_id::left & ~(uint8_t)config_defs::joint_id::right))  //Use the id with the side masked out.
    {
        case (uint8_t)config_defs::joint_id::hip:
        {
            //Check if joint and side is used
            is_used = (config_to_send[config_defs::hip_idx] != (uint8_t)config_defs::motor::not_used) && ((static_cast<uint8_t>(config_defs::exo_side::bilateral) == config_to_send[config_defs::exo_side_idx]) 
                || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]) && this->is_left)
                || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]) && !this->is_left));

            // logger::print("Hip\n");
            
            //Check if the sensor directions should be flipped
            if ((config_to_send[config_defs::hip_flip_torque_dir_idx] == (uint8_t)config_defs::flip_torque_dir::both) || ((config_to_send[config_defs::hip_flip_torque_dir_idx] == (uint8_t)config_defs::flip_torque_dir::left) && this->is_left) || ((config_to_send[config_defs::hip_flip_torque_dir_idx] == (uint8_t)config_defs::flip_torque_dir::right) && (!this->is_left)))
            {
                this->flip_direction = 1;
            }
            else
            {
                this->flip_direction = 0;
            }
			
			if ((config_to_send[config_defs::hip_flip_angle_dir_idx] == (uint8_t)config_defs::flip_angle_dir::both) || ((config_to_send[config_defs::hip_flip_angle_dir_idx] == (uint8_t)config_defs::flip_angle_dir::left) && this->is_left) || ((config_to_send[config_defs::hip_flip_angle_dir_idx] == (uint8_t)config_defs::flip_angle_dir::right) && (!this->is_left)))
            {
                this->do_flip_angle = 1;
            }
            else
            {
                this->do_flip_angle = 0;
            }
			
            break;
        }
        case (uint8_t)config_defs::joint_id::knee:
        {
            //Check if joint and side is used
            is_used = (config_to_send[config_defs::knee_idx] != (uint8_t)config_defs::motor::not_used) && ((static_cast<uint8_t>(config_defs::exo_side::bilateral) == config_to_send[config_defs::exo_side_idx]) 
                || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]) && this->is_left)
                || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]) && !this->is_left));

            // logger::print("Knee\n");
             
            //Check if the sensor directions should be flipped
            if ((config_to_send[config_defs::knee_flip_torque_dir_idx] == (uint8_t)config_defs::flip_torque_dir::both) || ((config_to_send[config_defs::knee_flip_torque_dir_idx] == (uint8_t)config_defs::flip_torque_dir::left) && this->is_left) || ((config_to_send[config_defs::knee_flip_torque_dir_idx] == (uint8_t)config_defs::flip_torque_dir::right) && (!this->is_left)))
            {
                this->flip_direction = 1;
            }
            else
            {
                this->flip_direction = 0;
            }
			
			if ((config_to_send[config_defs::knee_flip_angle_dir_idx] == (uint8_t)config_defs::flip_angle_dir::both) || ((config_to_send[config_defs::knee_flip_angle_dir_idx] == (uint8_t)config_defs::flip_angle_dir::left) && this->is_left) || ((config_to_send[config_defs::knee_flip_angle_dir_idx] == (uint8_t)config_defs::flip_angle_dir::right) && (!this->is_left)))
            {
                this->do_flip_angle = 1;
            }
            else
            {
                this->do_flip_angle = 0;
            }
			
            break;
        }
        case (uint8_t)config_defs::joint_id::ankle:
        {
            //Check if joint and side is used
            is_used = (config_to_send[config_defs::ankle_idx] != (uint8_t)config_defs::motor::not_used) && ((static_cast<uint8_t>(config_defs::exo_side::bilateral) == config_to_send[config_defs::exo_side_idx]) 
                || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]) && this->is_left)
                || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]) && !this->is_left));

            // logger::print("Ankle\n");
            
            //Check if the sensor directions should be flipped
            if ((config_to_send[config_defs::ankle_flip_torque_dir_idx] == (uint8_t)config_defs::flip_torque_dir::both) || ((config_to_send[config_defs::ankle_flip_torque_dir_idx] == (uint8_t)config_defs::flip_torque_dir::left) && this->is_left) || ((config_to_send[config_defs::ankle_flip_torque_dir_idx] == (uint8_t)config_defs::flip_torque_dir::right) && (!this->is_left)))
            {
                this->flip_direction = 1;
            }
            else
            {
                this->flip_direction = 0;
            }
			
			if ((config_to_send[config_defs::ankle_flip_angle_dir_idx] == (uint8_t)config_defs::flip_angle_dir::both) || ((config_to_send[config_defs::ankle_flip_angle_dir_idx] == (uint8_t)config_defs::flip_angle_dir::left) && this->is_left) || ((config_to_send[config_defs::ankle_flip_angle_dir_idx] == (uint8_t)config_defs::flip_angle_dir::right) && (!this->is_left)))
            {
                this->do_flip_angle = 1;
            }
            else
            {
                this->do_flip_angle = 0;
            }
			
            break;
        }
        case (uint8_t)config_defs::joint_id::elbow:
        {
            //Check if joint and side is used
            is_used = (config_to_send[config_defs::elbow_idx] != (uint8_t)config_defs::motor::not_used) && ((static_cast<uint8_t>(config_defs::exo_side::bilateral) == config_to_send[config_defs::exo_side_idx])
                || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]) && this->is_left)
                || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]) && !this->is_left));

            // logger::print("Elbow\n");

            //Check if the sensor directions should be flipped
            if ((config_to_send[config_defs::elbow_flip_torque_dir_idx] == (uint8_t)config_defs::flip_torque_dir::both) || ((config_to_send[config_defs::elbow_flip_torque_dir_idx] == (uint8_t)config_defs::flip_torque_dir::left) && this->is_left) || ((config_to_send[config_defs::elbow_flip_torque_dir_idx] == (uint8_t)config_defs::flip_torque_dir::right) && (!this->is_left)))
            {
                this->flip_direction = 1;
            }
            else
            {
                this->flip_direction = 0;
            }

            if ((config_to_send[config_defs::elbow_flip_angle_dir_idx] == (uint8_t)config_defs::flip_angle_dir::both) || ((config_to_send[config_defs::elbow_flip_angle_dir_idx] == (uint8_t)config_defs::flip_angle_dir::left) && this->is_left) || ((config_to_send[config_defs::elbow_flip_angle_dir_idx] == (uint8_t)config_defs::flip_angle_dir::right) && (!this->is_left)))
            {
                this->do_flip_angle = 1;
            }
            else
            {
                this->do_flip_angle = 0;
            }

            break;
        }
    }
       
};

void JointData::reconfigure(uint8_t* config_to_send) 
{
    switch ((uint8_t)this->id & (~(uint8_t)config_defs::joint_id::left & ~(uint8_t)config_defs::joint_id::right))  //Use the id with the side masked out.
    {
        case (uint8_t)config_defs::joint_id::hip:
        {
            //Check if joint and side is used
            is_used = (config_to_send[config_defs::hip_idx] != (uint8_t)config_defs::motor::not_used) && ((static_cast<uint8_t>(config_defs::exo_side::bilateral) == config_to_send[config_defs::exo_side_idx]) 
                || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]) && this->is_left)
                || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]) && !this->is_left));

            // logger::print("Hip\n");
            
            //Check if the sensor directions should be flipped
            if ((config_to_send[config_defs::hip_flip_torque_dir_idx] == (uint8_t)config_defs::flip_torque_dir::both) || ((config_to_send[config_defs::hip_flip_torque_dir_idx] == (uint8_t)config_defs::flip_torque_dir::left) && this->is_left) || ((config_to_send[config_defs::hip_flip_torque_dir_idx] == (uint8_t)config_defs::flip_torque_dir::right) && (!this->is_left)))
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
            //Check if joint and side is used
            is_used = (config_to_send[config_defs::knee_idx] != (uint8_t)config_defs::motor::not_used) && ((static_cast<uint8_t>(config_defs::exo_side::bilateral) == config_to_send[config_defs::exo_side_idx]) 
                || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]) && this->is_left)
                || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]) && !this->is_left));
            
            // logger::print("Knee\n");
            
            //Check if the sensor directions should be flipped
            if ((config_to_send[config_defs::knee_flip_torque_dir_idx] == (uint8_t)config_defs::flip_torque_dir::both) || ((config_to_send[config_defs::knee_flip_torque_dir_idx] == (uint8_t)config_defs::flip_torque_dir::left) && this->is_left) || ((config_to_send[config_defs::knee_flip_torque_dir_idx] == (uint8_t)config_defs::flip_torque_dir::right) && (!this->is_left)))
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
            //Check if joint and side is used
            is_used = (config_to_send[config_defs::ankle_idx] != (uint8_t)config_defs::motor::not_used) && ((static_cast<uint8_t>(config_defs::exo_side::bilateral) == config_to_send[config_defs::exo_side_idx]) 
                || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]) && this->is_left)
                || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]) && !this->is_left));
            
            // logger::print("Ankle\n");
            
            //Check if the sensor directions should be flipped
            if ((config_to_send[config_defs::ankle_flip_torque_dir_idx] == (uint8_t)config_defs::flip_torque_dir::both) || ((config_to_send[config_defs::ankle_flip_torque_dir_idx] == (uint8_t)config_defs::flip_torque_dir::left) && this->is_left) || ((config_to_send[config_defs::ankle_flip_torque_dir_idx] == (uint8_t)config_defs::flip_torque_dir::right) && (!this->is_left)))
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
            //Check if joint and side is used
            is_used = (config_to_send[config_defs::elbow_idx] != (uint8_t)config_defs::motor::not_used) && ((static_cast<uint8_t>(config_defs::exo_side::bilateral) == config_to_send[config_defs::exo_side_idx])
                || (((uint8_t)config_defs::exo_side::left == config_to_send[config_defs::exo_side_idx]) && this->is_left)
                || (((uint8_t)config_defs::exo_side::right == config_to_send[config_defs::exo_side_idx]) && !this->is_left));
            
            // logger::print("Elbow\n");

            //Check if the sensor directions should be flipped
            if ((config_to_send[config_defs::elbow_flip_torque_dir_idx] == (uint8_t)config_defs::flip_torque_dir::both) || ((config_to_send[config_defs::elbow_flip_torque_dir_idx] == (uint8_t)config_defs::flip_torque_dir::left) && this->is_left) || ((config_to_send[config_defs::elbow_flip_torque_dir_idx] == (uint8_t)config_defs::flip_torque_dir::right) && (!this->is_left)))
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
    
    //Reconfigure the contained objects
    motor.reconfigure(config_to_send);
    controller.reconfigure(config_to_send);
};

