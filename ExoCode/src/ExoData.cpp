#include "ExoData.h"
#include "error_codes.h"
#include "Logger.h"
#include "ParamsFromSD.h"

/*
 * Constructor for the exo data.
 * Takes the array from the INI parser.
 * Stores the exo status, and the sync LED state.
 * Uses an initializer list for the side data. 
 */
ExoData::ExoData(uint8_t* config_to_send) 
: left_side(true, config_to_send)            //Using initializer list for member objects.
, right_side(false, config_to_send)
{
    this->_status = status_defs::messages::trial_off;
    this->sync_led_state = false;
    this->estop = false;

    this->config = config_to_send;
    this->config_len = ini_config::number_of_keys;

    this->mark = 10;  

    this->error_code = static_cast<int>(NO_ERROR);
    this->error_joint_id = 0;
    this->user_paused = false;

    //If Statement that determines if torque sensor is used for that joint (See Board.h for available torque sensor pins)
    if ((config_to_send[config_defs::hip_use_torque_sensor_idx] == (uint8_t)config_defs::use_torque_sensor::yes))
    {
        hip_torque_flag = 1;
    }

    if ((config_to_send[config_defs::knee_use_torque_sensor_idx] == (uint8_t)config_defs::use_torque_sensor::yes))
    {
        knee_torque_flag = 1;
    }

    if ((config_to_send[config_defs::ankle_use_torque_sensor_idx] == (uint8_t)config_defs::use_torque_sensor::yes))
    {
        ankle_torque_flag = 1;
    }

    if ((config_to_send[config_defs::elbow_use_torque_sensor_idx] == (uint8_t)config_defs::use_torque_sensor::yes))
    {
        elbow_torque_flag = 1;
    }
};

void ExoData::reconfigure(uint8_t* config_to_send) 
{
    left_side.reconfigure(config_to_send);
    right_side.reconfigure(config_to_send);
};

uint8_t ExoData::get_used_joints(uint8_t* used_joints)
{
    uint8_t len = 0;

    used_joints[len] = ((left_side.hip.is_used) ? (1) : (0));
    len += left_side.hip.is_used;
    used_joints[len] = ((left_side.knee.is_used) ? (1) : (0));
    len += left_side.knee.is_used;
    used_joints[len] = ((left_side.ankle.is_used) ? (1) : (0));
    len += left_side.ankle.is_used;
    used_joints[len] = ((left_side.elbow.is_used) ? (1) : (0));
    len += left_side.elbow.is_used;
    used_joints[len] = ((right_side.hip.is_used) ? (1) : (0));
    len += right_side.hip.is_used;
    used_joints[len] = ((right_side.knee.is_used) ? (1) : (0));
    len += right_side.knee.is_used;
    used_joints[len] = ((right_side.ankle.is_used) ? (1) : (0));
    len += right_side.ankle.is_used;
    used_joints[len] = ((right_side.elbow.is_used) ? (1) : (0));
    len += right_side.elbow.is_used;
    return len;
};

JointData* ExoData::get_joint_with(uint8_t id)
{
    JointData* j_data = NULL;
    switch (id)
    {
    case (uint8_t)config_defs::joint_id::left_hip:
        j_data = &left_side.hip;
        break;
    case (uint8_t)config_defs::joint_id::left_knee:
        j_data = &left_side.knee;
        break;
    case (uint8_t)config_defs::joint_id::left_ankle:
        j_data = &left_side.ankle;
        break;
    case (uint8_t)config_defs::joint_id::left_elbow:
        j_data = &left_side.elbow;
        break;
    case (uint8_t)config_defs::joint_id::right_hip:
        j_data = &right_side.hip;
        break;
    case (uint8_t)config_defs::joint_id::right_knee:
        j_data = &right_side.knee;
        break;
    case (uint8_t)config_defs::joint_id::right_ankle:
        j_data = &right_side.ankle;
        break; 
    case (uint8_t)config_defs::joint_id::right_elbow:
        j_data = &right_side.elbow;
        break;
    default:
        // logger::print("ExoData::get_joint_with->No joint with ");
        // logger::print(id);
        // logger::println(" was found.");
        break;
    }
    return j_data;
};

void ExoData::set_status(uint16_t status_to_set)
{
    //If the status is already error, don't change it
    if (this->_status == status_defs::messages::error)
    {
        return;
    }
    this->_status = status_to_set;
}

uint16_t ExoData::get_status(void)
{
    return this->_status;
}

void ExoData::set_default_parameters()
{
#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)
    this->for_each_joint([this](JointData* j_data, float* args)
        {
            if (j_data->is_used)
            {
                set_controller_params((uint8_t)j_data->id, j_data->controller.controller, 0, this);
            }
        }
    );
#endif
}

void ExoData::set_default_parameters(uint8_t id)
{
    #if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)
    float f_id = static_cast<float>(id);
    this->for_each_joint(
        [this](JointData* j_data, float* args) 
        {
            if (j_data->is_used && (uint8_t)j_data->id == static_cast<uint8_t>(args[0]))
            {
                set_controller_params((uint8_t)j_data->id, j_data->controller.controller, 0, this);
            }
        },
        &f_id
    );
    #endif
}

void ExoData::start_pretrial_cal()
{
    //Calibrate the Torque Sensors
    this->for_each_joint([](JointData* j_data, float* args) {j_data->calibrate_torque_sensor = j_data->is_used;});
}

void ExoData::print()
{
    logger::print("\t Status : ");
    logger::println(_status);
    logger::print("\t Sync LED : ");
    logger::println(sync_led_state);
    
    if (left_side.is_used)
    {
        logger::print("\tLeft :: FSR Calibration : ");
        logger::print(left_side.do_calibration_heel_fsr);
        logger::println(left_side.do_calibration_toe_fsr);
        logger::print("\tLeft :: FSR Refinement : ");
        logger::print(left_side.do_calibration_refinement_heel_fsr);
        logger::println(left_side.do_calibration_refinement_toe_fsr);
        logger::print("\tLeft :: Percent Gait : ");
        logger::println(left_side.percent_gait);
        logger::print("\tLeft :: Heel FSR : ");
        logger::println(left_side.heel_fsr);
        logger::print("\tLeft :: Toe FSR : ");
        logger::println(left_side.toe_fsr);
        
        if(left_side.hip.is_used)
        {
            logger::println("\tLeft :: Hip");
            logger::print("\t\tcalibrate_torque_sensor : ");
            logger::println(left_side.hip.calibrate_torque_sensor);
            logger::print("\t\ttorque_reading : ");
            logger::println(left_side.hip.torque_reading);
            logger::print("\t\tMotor :: p : ");
            logger::println(left_side.hip.motor.p);
            logger::print("\t\tMotor :: v : ");
            logger::println(left_side.hip.motor.v);
            logger::print("\t\tMotor :: i : ");
            logger::println(left_side.hip.motor.i);
            logger::print("\t\tMotor :: p_des : ");
            logger::println(left_side.hip.motor.p_des);
            logger::print("\t\tMotor :: v_des : ");
            logger::println(left_side.hip.motor.v_des);
            logger::print("\t\tMotor :: t_ff : ");
            logger::println(left_side.hip.motor.t_ff);
            logger::print("\t\tController :: controller : ");
            logger::println(left_side.hip.controller.controller);
            logger::print("\t\tController :: setpoint : ");
            logger::println(left_side.hip.controller.setpoint);
            logger::print("\t\tController :: parameter_set : ");
            logger::println(left_side.hip.controller.parameter_set);
        }
        
        if(left_side.knee.is_used)
        {
            logger::println("\tLeft :: Knee");
            logger::print("\t\tcalibrate_torque_sensor : ");
            logger::println(left_side.knee.calibrate_torque_sensor);
            logger::print("\t\ttorque_reading : ");
            logger::println(left_side.knee.torque_reading);
            logger::print("\t\tMotor :: p : ");
            logger::println(left_side.knee.motor.p);
            logger::print("\t\tMotor :: v : ");
            logger::println(left_side.knee.motor.v);
            logger::print("\t\tMotor :: i : ");
            logger::println(left_side.knee.motor.i);
            logger::print("\t\tMotor :: p_des : ");
            logger::println(left_side.knee.motor.p_des);
            logger::print("\t\tMotor :: v_des : ");
            logger::println(left_side.knee.motor.v_des);
            logger::print("\t\tMotor :: t_ff : ");
            logger::println(left_side.knee.motor.t_ff);
            logger::print("\t\tController :: controller : ");
            logger::println(left_side.knee.controller.controller);
            logger::print("\t\tController :: setpoint : ");
            logger::println(left_side.knee.controller.setpoint);
            logger::print("\t\tController :: parameter_set : ");
            logger::println(left_side.knee.controller.parameter_set);
        }
        if(left_side.ankle.is_used)
        {
            logger::println("\tLeft :: Ankle");
            logger::print("\t\tcalibrate_torque_sensor : ");
            logger::println(left_side.ankle.calibrate_torque_sensor);
            logger::print("\t\ttorque_reading : ");
            logger::println(left_side.ankle.torque_reading);
            logger::print("\t\tMotor :: p : ");
            logger::println(left_side.ankle.motor.p);
            logger::print("\t\tMotor :: v : ");
            logger::println(left_side.ankle.motor.v);
            logger::print("\t\tMotor :: i : ");
            logger::println(left_side.ankle.motor.i);
            logger::print("\t\tMotor :: p_des : ");
            logger::println(left_side.ankle.motor.p_des);
            logger::print("\t\tMotor :: v_des : ");
            logger::println(left_side.ankle.motor.v_des);
            logger::print("\t\tMotor :: t_ff : ");
            logger::println(left_side.ankle.motor.t_ff);
            logger::print("\t\tController :: controller : ");
            logger::println(left_side.ankle.controller.controller);
            logger::print("\t\tController :: setpoint : ");
            logger::println(left_side.ankle.controller.setpoint);
            logger::print("\t\tController :: parameter_set : ");
            logger::println(left_side.ankle.controller.parameter_set);
        }

        if (left_side.elbow.is_used)
        {
            logger::println("\tLeft :: Elbow");
            logger::print("\t\tcalibrate_torque_sensor : ");
            logger::println(left_side.elbow.calibrate_torque_sensor);
            logger::print("\t\ttorque_reading : ");
            logger::println(left_side.elbow.torque_reading);
            logger::print("\t\tMotor :: p : ");
            logger::println(left_side.elbow.motor.p);
            logger::print("\t\tMotor :: v : ");
            logger::println(left_side.elbow.motor.v);
            logger::print("\t\tMotor :: i : ");
            logger::println(left_side.elbow.motor.i);
            logger::print("\t\tMotor :: p_des : ");
            logger::println(left_side.elbow.motor.p_des);
            logger::print("\t\tMotor :: v_des : ");
            logger::println(left_side.elbow.motor.v_des);
            logger::print("\t\tMotor :: t_ff : ");
            logger::println(left_side.elbow.motor.t_ff);
            logger::print("\t\tController :: controller : ");
            logger::println(left_side.elbow.controller.controller);
            logger::print("\t\tController :: setpoint : ");
            logger::println(left_side.elbow.controller.setpoint);
            logger::print("\t\tController :: parameter_set : ");
            logger::println(left_side.elbow.controller.parameter_set);
        }
    }
    
    if (right_side.is_used)
    {
        logger::print("\tRight :: FSR Calibration : ");
        logger::print(right_side.do_calibration_heel_fsr);
        logger::println(right_side.do_calibration_toe_fsr);
        logger::print("\tRight :: FSR Refinement : ");
        logger::print(right_side.do_calibration_refinement_heel_fsr);
        logger::println(right_side.do_calibration_refinement_toe_fsr);
        logger::print("\tRight :: Percent Gait : ");
        logger::println(right_side.percent_gait);
        logger::print("\tLeft :: Heel FSR : ");
        logger::println(right_side.heel_fsr);
        logger::print("\tLeft :: Toe FSR : ");
        logger::println(right_side.toe_fsr);
        
        if(right_side.hip.is_used)
        {
            logger::println("\tRight :: Hip");
            logger::print("\t\tcalibrate_torque_sensor : ");
            logger::println(right_side.hip.calibrate_torque_sensor);
            logger::print("\t\ttorque_reading : ");
            logger::println(right_side.hip.torque_reading);
            logger::print("\t\tMotor :: p : ");
            logger::println(right_side.hip.motor.p);
            logger::print("\t\tMotor :: v : ");
            logger::println(right_side.hip.motor.v);
            logger::print("\t\tMotor :: i : ");
            logger::println(right_side.hip.motor.i);
            logger::print("\t\tMotor :: p_des : ");
            logger::println(right_side.hip.motor.p_des);
            logger::print("\t\tMotor :: v_des : ");
            logger::println(right_side.hip.motor.v_des);
            logger::print("\t\tMotor :: t_ff : ");
            logger::println(right_side.hip.motor.t_ff);
            logger::print("\t\tController :: controller : ");
            logger::println(right_side.hip.controller.controller);
            logger::print("\t\tController :: setpoint : ");
            logger::println(right_side.hip.controller.setpoint);
            logger::print("\t\tController :: parameter_set : ");
            logger::println(right_side.hip.controller.parameter_set);
            
        }
        
        if(right_side.knee.is_used)
        {
            logger::println("\tRight :: Knee");
            logger::print("\t\tcalibrate_torque_sensor : ");
            logger::println(right_side.knee.calibrate_torque_sensor);
            logger::print("\t\ttorque_reading : ");
            logger::println(right_side.knee.torque_reading);
            logger::print("\t\tMotor :: p : ");
            logger::println(right_side.knee.motor.p);
            logger::print("\t\tMotor :: v : ");
            logger::println(right_side.knee.motor.v);
            logger::print("\t\tMotor :: i : ");
            logger::println(right_side.knee.motor.i);
            logger::print("\t\tMotor :: p_des : ");
            logger::println(right_side.knee.motor.p_des);
            logger::print("\t\tMotor :: v_des : ");
            logger::println(right_side.knee.motor.v_des);
            logger::print("\t\tMotor :: t_ff : ");
            logger::println(right_side.knee.motor.t_ff);
            logger::print("\t\tController :: controller : ");
            logger::println(right_side.knee.controller.controller);
            logger::print("\t\tController :: setpoint : ");
            logger::println(right_side.knee.controller.setpoint);
            logger::print("\t\tController :: parameter_set : ");
            logger::println(right_side.knee.controller.parameter_set);
        }
        
        if(right_side.ankle.is_used)
        {
            logger::println("\tRight :: Ankle");
            logger::print("\t\tcalibrate_torque_sensor : ");
            logger::println(right_side.ankle.calibrate_torque_sensor);
            logger::print("\t\ttorque_reading : ");
            logger::println(right_side.ankle.torque_reading);
            logger::print("\t\tMotor :: p : ");
            logger::println(right_side.ankle.motor.p);
            logger::print("\t\tMotor :: v : ");
            logger::println(right_side.ankle.motor.v);
            logger::print("\t\tMotor :: i : ");
            logger::println(right_side.ankle.motor.i);
            logger::print("\t\tMotor :: p_des : ");
            logger::println(right_side.ankle.motor.p_des);
            logger::print("\t\tMotor :: v_des : ");
            logger::println(right_side.ankle.motor.v_des);
            logger::print("\t\tMotor :: t_ff : ");
            logger::println(right_side.ankle.motor.t_ff);
            logger::print("\t\tController :: controller : ");
            logger::println(right_side.ankle.controller.controller);
            logger::print("\t\tController :: setpoint : ");
            logger::println(right_side.ankle.controller.setpoint);
            logger::print("\t\tController :: parameter_set : ");
            logger::println(right_side.ankle.controller.parameter_set);
        }

        if (right_side.elbow.is_used)
        {
            logger::println("\tRight :: Elbow");
            logger::print("\t\tcalibrate_torque_sensor : ");
            logger::println(right_side.elbow.calibrate_torque_sensor);
            logger::print("\t\ttorque_reading : ");
            logger::println(right_side.elbow.torque_reading);
            logger::print("\t\tMotor :: p : ");
            logger::println(right_side.elbow.motor.p);
            logger::print("\t\tMotor :: v : ");
            logger::println(right_side.elbow.motor.v);
            logger::print("\t\tMotor :: i : ");
            logger::println(right_side.elbow.motor.i);
            logger::print("\t\tMotor :: p_des : ");
            logger::println(right_side.elbow.motor.p_des);
            logger::print("\t\tMotor :: v_des : ");
            logger::println(right_side.elbow.motor.v_des);
            logger::print("\t\tMotor :: t_ff : ");
            logger::println(right_side.elbow.motor.t_ff);
            logger::print("\t\tController :: controller : ");
            logger::println(right_side.elbow.controller.controller);
            logger::print("\t\tController :: setpoint : ");
            logger::println(right_side.elbow.controller.setpoint);
            logger::print("\t\tController :: parameter_set : ");
            logger::println(right_side.elbow.controller.parameter_set);
        }
    }
   
};

