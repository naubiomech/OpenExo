#include "StatusDefs.h"
#include "Logger.h"

void print_status_message(uint16_t message)
{
    switch (message)
    {
        case status_defs::messages::off :
            logger::print("Off");
            break;    
        case status_defs::messages::trial_off :
            logger::print("Trial Off");
            break;
        case status_defs::messages::trial_on :
            logger::print("Trial On");
            break;
        case status_defs::messages::test :
            logger::print("Test");
            break;    
        case status_defs::messages::torque_calibration :
            logger::print("Torque Calibration");
            break;
        case status_defs::messages::fsr_calibration :
            logger::print("FSR Calibration");
            break;
        case status_defs::messages::fsr_refinement :
            logger::print("FSR Refinement");
            break;  
        case status_defs::messages::motor_start_up :
            logger::print("Motor Start Up");
            break;
        case status_defs::messages::error :
            logger::print("General Error");
            break;
        case status_defs::messages::error_left_heel_fsr :
            logger::print("Error :: Left Heel FSR");
            break;
        case status_defs::messages::error_left_toe :
            logger::print("Error :: Left Toe FSR");
            break;
        case status_defs::messages::error_right_heel_fsr :
            logger::print("Error :: Right Heel FSR");
            break;
        case status_defs::messages::error_right_toe_fsr :
            logger::print("Error :: Right Toe FSR");
            break;    
        case status_defs::messages::error_left_hip_torque_sensor :
            logger::print("Error :: Left Hip Torque Sensor");
            break;
        case status_defs::messages::error_left_knee_torque_sensor :
            logger::print("Error :: Left Knee Torque Sensor");
            break;
        case status_defs::messages::error_left_ankle_torque_sensor :
            logger::print("Error :: Left Ankle Torque Sensor");
            break;
        case status_defs::messages::error_left_elbow_torque_sensor:
            logger::print("Error :: Left Elbow Torque Sensor");
            break;
        case status_defs::messages::error_right_hip_torque_sensor :
            logger::print("Error :: Right Hip Torque Sensor");
            break;
        case status_defs::messages::error_right_knee_torque_sensor :
            logger::print("Error :: Right Knee Torque Sensor");
            break;
        case status_defs::messages::error_right_ankle_torque_sensor :
            logger::print("Error :: Right Ankle Torque Sensor");
            break;
        case status_defs::messages::error_right_elbow_torque_sensor:
            logger::print("Error :: Right Elbow Torque Sensor");
            break;
        case status_defs::messages::error_left_hip_motor :
            logger::print("Error :: Left Hip Motor");
            break;
        case status_defs::messages::error_left_knee_motor :
            logger::print("Error :: Left Knee Motor");
            break;    
        case status_defs::messages::error_left_ankle_motor :
            logger::print("Error :: Left Ankle Motor");
            break;
        case status_defs::messages::error_left_elbow_motor:
            logger::print("Error :: Left Elbow Motor");
            break;
        case status_defs::messages::error_right_hip_motor :
            logger::print("Error :: Right Hip Motor");
            break;
        case status_defs::messages::error_right_knee_motor :
            logger::print("Error :: Right Knee Motor");
            break; 
        case status_defs::messages::error_right_ankle_motor :
            logger::print("Error :: Right Ankle Motor");
            break;
        case status_defs::messages::error_right_elbow_motor:
            logger::print("Error :: Right Elbow Motor");
            break;
        case status_defs::messages::error_left_hip_controller :
            logger::print("Error :: Left Hip Controller");
            break;
        case status_defs::messages::error_left_knee_controller :
            logger::print("Error :: Left Knee Controller");
            break;
        case status_defs::messages::error_left_ankle_controller :
            logger::print("Error :: Left Ankle Controller");
            break;
        case status_defs::messages::error_left_elbow_controller:
            logger::print("Error :: Left Elbow Controller");
            break;
        case status_defs::messages::error_right_hip_controller :
            logger::print("Error :: Right Hip Controller");
            break;    
        case status_defs::messages::error_right_knee_controller :
            logger::print("Error :: Right Knee Controller");
            break;
        case status_defs::messages::error_right_ankle_controller :
            logger::print("Error :: Right Ankle Controller");
            break;
        case status_defs::messages::error_right_elbow_controller:
            logger::print("Error :: Right Elbow Controller");
            break;
    }
};
