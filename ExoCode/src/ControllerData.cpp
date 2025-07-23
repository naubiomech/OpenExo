/*
 * 
 * P. Stegall Jan. 2022
*/

#include "ControllerData.h"
#include <string>

/*
 * Constructor for the controller data.
 * Takes the joint id and the array from the INI parser.
 * Stores the id, sets the controller to the default controller for the appropriate joint, and records the joint type to check we are using appropriate controllers. 
 */
ControllerData::ControllerData(config_defs::joint_id id, uint8_t* config_to_send)
{
    
    switch ((uint8_t)id & (~(uint8_t)config_defs::joint_id::left & ~(uint8_t)config_defs::joint_id::right))  //Use the id with the side masked out.
    {
        case (uint8_t)config_defs::joint_id::hip:
        {
            controller = config_to_send[config_defs::exo_hip_default_controller_idx];
            joint = config_defs::JointType::hip;
            break;
        }
        case (uint8_t)config_defs::joint_id::knee:
        {
            controller = config_to_send[config_defs::exo_knee_default_controller_idx];
            joint = config_defs::JointType::knee;
            break;
        }
        case (uint8_t)config_defs::joint_id::ankle:
        {
            controller = config_to_send[config_defs::exo_ankle_default_controller_idx];
            joint = config_defs::JointType::ankle;
            break;
        }
        case (uint8_t)config_defs::joint_id::elbow:
        {
            controller = config_to_send[config_defs::exo_elbow_default_controller_idx];
            joint = config_defs::JointType::elbow;
            break;
        }
    }
    
    setpoint = 0;
    parameter_set = 0;

    for (int i=0; i < controller_defs::max_parameters; i++)
    {    
        parameters[i] = 0;
    }

    filtered_cmd = 0;
    filtered_torque_reading = 0;

};

void ControllerData::reconfigure(uint8_t* config_to_send) 
{
    //Just reset controller
    switch ((uint8_t)joint)  //Use the id with the side masked out.
    {
        case (uint8_t)config_defs::joint_id::hip:
        {
            controller = config_to_send[config_defs::exo_hip_default_controller_idx];
            break;
        }
        case (uint8_t)config_defs::joint_id::knee:
        {
            controller = config_to_send[config_defs::exo_knee_default_controller_idx];
            break;
        }
        case (uint8_t)config_defs::joint_id::ankle:
        {
            controller = config_to_send[config_defs::exo_ankle_default_controller_idx];
            break;
        }
        case (uint8_t)config_defs::joint_id::elbow:
        {
            controller = config_to_send[config_defs::exo_elbow_default_controller_idx];
            break;
        }
    }
    
    setpoint = 0;

    for (int i=0; i < controller_defs::max_parameters; i++)
    {    
        parameters[i] = 0;
    }
};


uint8_t ControllerData::get_parameter_length()
{
    uint8_t length = 0;
    return length;
}
