/**
 * @file ParamsFromSD.h
 *
 * @brief Declares the functions to pull controller parameters from the SD card and defines the mapping to the parameter files.
 * 
 * @author P. Stegall 
 * @date Jan. 2022
*/

#ifndef ParamsFromSD_h
#define ParamsFromSD_h

#include "ExoData.h"
#include "ParseIni.h"
#include "Utilities.h"

#include <SD.h>
#include <SPI.h>
#include <map>
#include <string>

//Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)
    #ifndef SD_SELECT
        #define SD_SELECT BUILTIN_SDCARD
    #endif
    
    typedef std::map<uint8_t, std::string> ParamFilenameKey;
    
    /**
     * @brief Types of errors when reading the SD card
     */
    namespace param_error
    {
        const uint8_t num_joint_ids = 3;                            /**< Number of bits the joint type ids need */
        const uint8_t SD_not_found_idx = num_joint_ids;             /**< Error when SD card isn't present */
        const uint8_t file_not_found_idx = SD_not_found_idx + 1;    /**< Error when file is not found on the SD card */
    }
    
    /**
     * @brief Namespace with map to between controller and file location
     */
    namespace controller_parameter_filenames
    {
        const ParamFilenameKey hip
        {
            {(uint8_t)config_defs::hip_controllers::disabled,"hipControllers/zeroTorque.csv"},
            {(uint8_t)config_defs::hip_controllers::zero_torque,"hipControllers/zeroTorque.csv"},
            {(uint8_t)config_defs::hip_controllers::franks_collins_hip, "hipControllers/franksCollinsHip.csv"},
            {(uint8_t)config_defs::hip_controllers::constant_torque, "hipControllers/constantTorque.csv"},
            {(uint8_t)config_defs::hip_controllers::chirp,"hipControllers/chirp.csv"},
            {(uint8_t)config_defs::hip_controllers::step,"hipControllers/step.csv"},
            {(uint8_t)config_defs::hip_controllers::phmc,"hipControllers/PHMC.csv"},
        };
        
        const ParamFilenameKey knee
        {
            {(uint8_t)config_defs::knee_controllers::disabled,"kneeControllers/zeroTorque.csv"},
            {(uint8_t)config_defs::knee_controllers::zero_torque,"kneeControllers/zeroTorque.csv"},
            {(uint8_t)config_defs::knee_controllers::constant_torque, "kneeControllers/constantTorque.csv"},
            {(uint8_t)config_defs::knee_controllers::chirp,"kneeControllers/chirp.csv"},
            {(uint8_t)config_defs::knee_controllers::step,"kneeControllers/step.csv"},
        };
        
        const ParamFilenameKey ankle
        {
            {(uint8_t)config_defs::ankle_controllers::disabled,"ankleControllers/zeroTorque.csv"},
            {(uint8_t)config_defs::ankle_controllers::zero_torque,"ankleControllers/zeroTorque.csv"},
            {(uint8_t)config_defs::ankle_controllers::pjmc,"ankleControllers/PJMC.csv"},
            {(uint8_t)config_defs::ankle_controllers::zhang_collins,"ankleControllers/zhangCollins.csv"},
            {(uint8_t)config_defs::ankle_controllers::constant_torque, "ankleControllers/constantTorque.csv"},
            {(uint8_t)config_defs::ankle_controllers::trec,"ankleControllers/trec.csv"},
            {(uint8_t)config_defs::ankle_controllers::chirp,"ankleControllers/chirp.csv"},
            {(uint8_t)config_defs::ankle_controllers::step,"ankleControllers/step.csv"},
			{(uint8_t)config_defs::ankle_controllers::spv2,"ankleControllers/spv2.csv"},
        };

        const ParamFilenameKey elbow
        {
            {(uint8_t)config_defs::elbow_controllers::disabled,"elbowControllers/zeroTorque.csv"},
            {(uint8_t)config_defs::elbow_controllers::zero_torque,"elbowControllers/zeroTorque.csv"},
            {(uint8_t)config_defs::elbow_controllers::elbow_min_max, "elbowControllers/elbowMinMax.csv"},
            {(uint8_t)config_defs::elbow_controllers::chirp,"elbowControllers/chirp.csv"},
            {(uint8_t)config_defs::elbow_controllers::step,"elbowControllers/step.csv"},
        };

    };
    
    /**
     * @brief Prints name of error message
     *
     * @param error identifier
     */
    void print_param_error_message(uint8_t error_type);
    
    /**
     * @brief Reads files from SD card and sets them to the appropriate controller parameters in the exo_data object
     * see ParseIni for details on inputs
     * 
     * @param joint_id : the joint id 
     * @param controller_id : the controller id 
     * @param set_num : parameter set to read from the SD card
     * @param exo_data : location to put the data 
     * 
     * @return : Error int.
     */
    uint8_t set_controller_params(uint8_t joint_id, uint8_t controller_id, uint8_t set_num, ExoData* exo_data);


#endif
#endif