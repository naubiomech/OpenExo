/**
 * @file ParseIni.h
 *
 * @brief Declares the functions needed and defines mapping between the INI keys and the exo components 
 * 
 * @author P. Stegall 
 * @date Jan. 2022
*/


#ifndef ParseIni_h
#define ParseIni_h

//Used for uint8_t
#include <stdint.h>

/**
 * @brief define the constants to use for the various arrays.
 */
namespace ini_config
{
    const int buffer_length = 500;  /**< length of the buffer for reading the file. */
    const int key_length = 25;      /**< Max length of the key name */
    const int section_length = 10;  /**< Max length of the section name */
    const int number_of_keys = 41;  /**< Number of keys to be parsed. */
}

//Includes for reading the ini file from the SD card.
//1 is the lowest value to confirm that data is present for sending over SPI

/**
 * @brief Namespace that defines numeric coding for different keys values. These are used throughout the code.
 */
namespace config_defs
{
    enum class board_name : uint8_t
    {
        AK_board = 1,
    };
    
    enum class board_version : uint8_t
    { 
        zero_one = 1,
        zero_three = 2,
        zero_four = 3,
        zero_five_one = 4,
    };
    
    enum class battery : uint8_t
    { 
        smart = 1,
        dumb = 2,
    };

    enum class exo_name : uint8_t
    { 
        bilateral_ankle = 1, 
        bilateral_hip = 2, 
        bilateral_knee = 3,
        bilateral_elbow = 4,
        bilateral_hip_ankle = 5,
        bilateral_hip_elbow = 6,
        bilateral_ankle_elbow = 7,
        left_ankle = 8,
        right_ankle = 9,
        left_hip = 10,
        right_hip = 11,
        left_knee = 12,
        right_knee = 13,
        left_elbow = 14,
        right_elbow = 15,
        left_hip_ankle = 16,
        right_hip_ankle = 17,
        left_hip_elbow = 18,
        right_hip_elbow = 19,
        left_ankle_elbow = 20,
        right_ankle_elbow = 21,
        test = 22,
    };
    
    enum class exo_side : uint8_t
    { 
        bilateral = 1, 
        left = 2, 
        right = 3,
    };
    
    enum class JointType
    {
        hip = 1,
        knee = 2,
        ankle = 3,
        elbow = 4,
    };
    
    enum class motor : uint8_t
    { 
        not_used = 1, 
        AK60 = 2, 
        AK80 = 3,
        AK60_v1_1 = 4,
        AK70 = 5,
    };
    
    enum class gearing : uint8_t
    { 
        gearing_1_1 = 1,
        gearing_2_1 = 2,
        gearing_3_1 = 3,
        gearing_4_5_1 = 4,
    };
    
    
    enum class joint_id : uint8_t
    {
        //Byte format : [0, is_left, !is_left, unused_joint, is_elbow, is_ankle, is_knee, is_hip]
        left = 0b01000000,
        right = 0b00100000,
        
        hip = 0b00000001,
        knee = 0b00000010,
        ankle = 0b00000100,
        elbow = 0b00001000,
                
        left_hip = left|hip,                //Set Motor ID to: 65
        left_knee = left|knee,              //Set Motor ID to: 66
        left_ankle = left|ankle,            //Set Motor ID to: 68
        left_elbow = left|elbow,            //Set Motor ID to: 72
                                            //Unused Joint ID: 80
        
        right_hip = right|hip,              //Set Motor ID to: 33
        right_knee = right|knee,            //Set Motor ID to: 34
        right_ankle = right|ankle,          //Set Motor ID to: 36
        right_elbow = right|elbow,          //Set Motor ID to: 40
                                            //Unused Joint ID: 48
    };
        
    enum class  hip_controllers : uint8_t
    {
        disabled = 1,
        zero_torque = 2,
        franks_collins_hip = 3,
        constant_torque = 4,
        chirp = 5,
        step = 6,
        phmc = 7,
        angle_based = 8,
    };
    
    enum class knee_controllers : uint8_t
    {
        disabled = 1,
        zero_torque = 2,
        constant_torque = 3,
        elbow_min_max = 4,
        chirp = 5,
        step = 6,
    };
        
    enum class ankle_controllers : uint8_t
    {
        disabled = 1, 
        zero_torque = 2, 
        pjmc = 3,
        zhang_collins = 4,
        constant_torque = 5,
        trec = 6,
		elbow_min_max = 7,
		calibr_manager = 8,
        chirp = 9,
        step = 10,
    };

    enum class elbow_controllers : uint8_t
    {
        disabled = 1,
        zero_torque = 2,
        elbow_min_max = 3,
        calibr_manager = 4,
        chirp = 5,
        step = 6,
    };
    
    enum class use_torque_sensor : uint8_t
    {
        no = 1, 
        yes = 2, 
    };

    enum class flip_motor_dir : uint8_t
    {
        neither = 1,
        left = 2,
        right = 3,
        both = 4,
    };

    enum class flip_torque_dir : uint8_t
    {
        neither = 1,
        left = 2,
        right = 3,
        both = 4,
    };

    enum class flip_angle_dir : uint8_t
    {
        neither = 1,
        left = 2,
        right = 3,
        both = 4,
    };
    
    static const int board_name_idx = 0;
    static const int board_version_idx = 1;
    
    static const int battery_idx = 2;
    
    static const int exo_name_idx = 3;
    static const int exo_side_idx = 4;
    
    static const int hip_idx = 5;
    static const int knee_idx = 6;
    static const int ankle_idx = 7;
    static const int elbow_idx = 8;
    
    static const int hip_gear_idx = 9;
    static const int knee_gear_idx = 10;
    static const int ankle_gear_idx = 11;
    static const int elbow_gear_idx = 12;
    
    static const int exo_hip_default_controller_idx = 13;
    static const int exo_knee_default_controller_idx = 14;
    static const int exo_ankle_default_controller_idx = 15;
    static const int exo_elbow_default_controller_idx = 16;
    
    static const int hip_use_torque_sensor_idx = 17;
    static const int knee_use_torque_sensor_idx = 18;
    static const int ankle_use_torque_sensor_idx = 19;
    static const int elbow_use_torque_sensor_idx = 20;
	
	static const int hip_flip_motor_dir_idx = 21;
	static const int knee_flip_motor_dir_idx = 22;
	static const int ankle_flip_motor_dir_idx = 23;
    static const int elbow_flip_motor_dir_idx = 24;
	
	static const int hip_flip_torque_dir_idx = 25;
	static const int knee_flip_torque_dir_idx = 26;
	static const int ankle_flip_torque_dir_idx = 27;
    static const int elbow_flip_torque_dir_idx = 28;
	
	static const int hip_flip_angle_dir_idx = 29;
	static const int knee_flip_angle_dir_idx = 30;
	static const int ankle_flip_angle_dir_idx = 31;
    static const int elbow_flip_angle_dir_idx = 32;
	
	static const int left_hip_RoM_idx = 33;
	static const int right_hip_RoM_idx = 34;
	static const int left_knee_RoM_idx = 35;
	static const int right_knee_RoM_idx = 36;
	static const int left_ankle_RoM_idx = 37;
	static const int right_ankle_RoM_idx = 38;
    static const int left_elbow_RoM_idx = 39;
    static const int right_elbow_RoM_idx = 40;
}

#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41) 
    #include <SD.h>
    #include <SPI.h>
    #include "IniFile.h"

    //Need to install ArduinoSTL in the library manager to use map
    #include <map>
    #include <string>

    //The select pin used for the SD card
    #ifndef SD_SELECT
        #define SD_SELECT BUILTIN_SDCARD
    #endif

    /**
     * @brief Parses the config.ini file in the root folder of the SD card and puts the parsed data to the provided array
     * 
     * @param pointer to the uint8_t array to be updated with the encoded parameter info. Array should be ini_config::number_of_keys in length
     */
    void ini_parser(uint8_t* config_to_send); // uses default filename
    
    /**
     * @brief Parses the specified filename from the SD card and puts the parsed data to the array provided
     * 
     * @param pointer to the character array that contains a nonstandard filename to parse.
     * @param pointer to the uint8_t array to be updated with the encoded parameter info. Array should be ini_config::number_of_keys in length
     */
    void ini_parser(char* filename, uint8_t* config_to_send); //uses sent filename
    
    /**
     * @brief retrieve the key values and get the print the output
     *
     * @param pointer to char array that contains the section containing the key
     * @param pointer to char array that contains the key name
     * @param pointer to char array to store the key value
     * @param length of the buffer for the key value
     */
    void get_section_key(IniFile ini, const char* section, const char* key, char* buffer, size_t buffer_len);  
    
    /**
     * @brief Prints the error messages of IniFile object.
     * e is the error message, and eol true means a println will be used at the end.
     * Requires that Serial is defined.
     * 
     * @param error id
     * @param if true will print and end of line character
     */
    void ini_print_error_message(uint8_t e, bool eol = true); 

    /**
     * @brief Mappings of config names to uint8_t that will be sent to the nano
     * "0" is mapped to one to help with debug as we should never send all zeros in the bytes.
     * If you see a uint8_t that is zero it indicates the field didn't exist.
    */
    namespace config_map
    {  
        
        //Define our own type so we don't have to type so much
        typedef std::map<std::string, uint8_t> IniKeyCode;
        
        const IniKeyCode board_name = 
        {
            {"AK_Board", (uint8_t)config_defs::board_name::AK_board},
        };
        
        const IniKeyCode board_version = 
        { 
            {"0.1", (uint8_t)config_defs::board_version::zero_one},
            {"0.3", (uint8_t)config_defs::board_version::zero_three},
            {"0.5.1", (uint8_t)config_defs::board_version::zero_five_one},
        };
        
        const IniKeyCode battery = 
        { 
            {"smart", (uint8_t)config_defs::battery::smart},
            {"dumb", (uint8_t)config_defs::battery::dumb},        
        };

        const IniKeyCode exo_name 
        { 
            {"bilateralAnkle", (uint8_t)config_defs::exo_name::bilateral_ankle}, 
            {"bilateralHip", (uint8_t)config_defs::exo_name::bilateral_hip},
            {"bilateralKnee", (uint8_t)config_defs::exo_name::bilateral_knee},
            {"bilateralElbow", (uint8_t)config_defs::exo_name::bilateral_elbow},
            {"bilateralHipAnkle", (uint8_t)config_defs::exo_name::bilateral_hip_ankle},
            {"bilateralHipElbow", (uint8_t)config_defs::exo_name::bilateral_hip_elbow},
            {"bilateralAnkleElbow", (uint8_t)config_defs::exo_name::bilateral_ankle_elbow},
            {"leftAnkle", (uint8_t)config_defs::exo_name::left_ankle},
            {"rightAnkle", (uint8_t)config_defs::exo_name::right_ankle},
            {"leftHip", (uint8_t)config_defs::exo_name::left_hip},
            {"rightHip", (uint8_t)config_defs::exo_name::right_hip},
            {"leftKnee", (uint8_t)config_defs::exo_name::left_knee},
            {"rightKnee", (uint8_t)config_defs::exo_name::right_knee},
            {"leftElbow", (uint8_t)config_defs::exo_name::left_elbow},
            {"rightElbow", (uint8_t)config_defs::exo_name::right_elbow},
            {"leftHipAnkle", (uint8_t)config_defs::exo_name::left_hip_ankle},
            {"rightHipAnkle", (uint8_t)config_defs::exo_name::right_hip_ankle},
            {"leftHipElbow", (uint8_t)config_defs::exo_name::left_hip_elbow},
            {"rightHipElbow", (uint8_t)config_defs::exo_name::right_hip_elbow},
            {"leftAnkleElbow", (uint8_t)config_defs::exo_name::left_ankle_elbow},
            {"rightAnkleElbow", (uint8_t)config_defs::exo_name::right_ankle_elbow},
            {"test", (uint8_t)config_defs::exo_name::test},
        };
        
        const IniKeyCode exo_side 
        { 
            {"bilateral", (uint8_t)config_defs::exo_side::bilateral}, 
            {"left", (uint8_t)config_defs::exo_side::left}, 
            {"right", (uint8_t)config_defs::exo_side::right},
        };
        
        const IniKeyCode motor 
        { 
            {"0", (uint8_t)config_defs::motor::not_used}, 
            {"AK60", (uint8_t)config_defs::motor::AK60}, 
            {"AK80", (uint8_t)config_defs::motor::AK80},
            {"AK60v1.1", (uint8_t)config_defs::motor::AK60_v1_1},
            {"AK70", (uint8_t)config_defs::motor::AK70},
        };
        
        const IniKeyCode gearing 
        { 
            {"1", (uint8_t)config_defs::gearing::gearing_1_1}, 
            {"2", (uint8_t)config_defs::gearing::gearing_2_1}, 
            {"3", (uint8_t)config_defs::gearing::gearing_3_1}, 
            {"4.5", (uint8_t)config_defs::gearing::gearing_4_5_1},
        };
        
        
        const IniKeyCode hip_controllers 
        { 
            {"0", (uint8_t)config_defs::hip_controllers::disabled}, 
            {"zeroTorque", (uint8_t)config_defs::hip_controllers::zero_torque}, 
            {"franksCollinsHip", (uint8_t)config_defs::hip_controllers::franks_collins_hip},
            {"constantTorque", (uint8_t)config_defs::hip_controllers::constant_torque},
            {"chirp", (uint8_t)config_defs::hip_controllers::chirp},
            {"step", (uint8_t)config_defs::hip_controllers::step},
            {"phmc", (uint8_t)config_defs::hip_controllers::phmc},
            {"angleBased", (uint8_t)config_defs::hip_controllers::angle_based},

        };
        
        const IniKeyCode knee_controllers 
        { 
            {"0", (uint8_t)config_defs::knee_controllers::disabled}, 
            {"zeroTorque", (uint8_t)config_defs::knee_controllers::zero_torque}, 
            {"constantTorque", (uint8_t)config_defs::knee_controllers::constant_torque},
            {"elbowMinMax", (uint8_t)config_defs::knee_controllers::elbow_min_max},
            {"chirp", (uint8_t)config_defs::knee_controllers::chirp},
            {"step", (uint8_t)config_defs::knee_controllers::step},
        };
        
        const IniKeyCode ankle_controllers 
        { 
            {"0", (uint8_t)config_defs::ankle_controllers::disabled}, 
            {"zeroTorque", (uint8_t)config_defs::ankle_controllers::zero_torque}, 
            {"PJMC", (uint8_t)config_defs::ankle_controllers::pjmc},
            {"zhangCollins", (uint8_t)config_defs::ankle_controllers::zhang_collins},
            {"constantTorque", (uint8_t)config_defs::ankle_controllers::constant_torque},
            {"TREC", (uint8_t)config_defs::ankle_controllers::trec},
			{"elbowMinMax", (uint8_t)config_defs::ankle_controllers::elbow_min_max},
			{"calibrManager", (uint8_t)config_defs::ankle_controllers::calibr_manager},
            {"chirp", (uint8_t)config_defs::ankle_controllers::chirp},
            {"step", (uint8_t)config_defs::ankle_controllers::step},
        };  

        const IniKeyCode elbow_controllers
        {
            {"0", (uint8_t)config_defs::elbow_controllers::disabled},
            {"zeroTorque", (uint8_t)config_defs::elbow_controllers::zero_torque},
            {"elbowMinMax", (uint8_t)config_defs::elbow_controllers::elbow_min_max},
            {"calibrManager", (uint8_t)config_defs::elbow_controllers::calibr_manager},
            {"chirp", (uint8_t)config_defs::elbow_controllers::chirp},
            {"step", (uint8_t)config_defs::elbow_controllers::step},
        };
        
        const IniKeyCode use_torque_sensor
        { 
            {"0", (uint8_t)config_defs::use_torque_sensor::no},
            {"yes", (uint8_t)config_defs::use_torque_sensor::yes},
        }; 

        const IniKeyCode flip_motor_dir
        {
            {"0", (uint8_t)config_defs::flip_motor_dir::neither},
            {"left", (uint8_t)config_defs::flip_motor_dir::left},
            {"right", (uint8_t)config_defs::flip_motor_dir::right},
            {"both", (uint8_t)config_defs::flip_motor_dir::both},
        };

        const IniKeyCode flip_torque_dir
        {
            {"0", (uint8_t)config_defs::flip_torque_dir::neither},
            {"left", (uint8_t)config_defs::flip_torque_dir::left},
            {"right", (uint8_t)config_defs::flip_torque_dir::right},
            {"both", (uint8_t)config_defs::flip_torque_dir::both},
        };

        const IniKeyCode flip_angle_dir
        {
            {"0", (uint8_t)config_defs::flip_angle_dir::neither},
            {"left", (uint8_t)config_defs::flip_angle_dir::left},
            {"right", (uint8_t)config_defs::flip_angle_dir::right},
            {"both", (uint8_t)config_defs::flip_angle_dir::both},
        };
    };

    /**
     * @brief Holds the raw key value strings from the ini file
     */
    struct ConfigData{
        
        std::string board_name;
        std::string board_version;
        
        std::string battery;
        
        std::string exo_name;
        std::string exo_sides;
        
        std::string exo_hip;
        std::string exo_knee;
        std::string exo_ankle;
        std::string exo_elbow;
        
        std::string hip_gearing;
        std::string knee_gearing;
        std::string ankle_gearing;
        std::string elbow_gearing;
        
        std::string exo_hip_default_controller;
        std::string exo_knee_default_controller;
        std::string exo_ankle_default_controller;
        std::string exo_elbow_default_controller;
        
        std::string hip_use_torque_sensor;
        std::string knee_use_torque_sensor;
        std::string ankle_use_torque_sensor;
        std::string elbow_use_torque_sensor;
		
		std::string hip_flip_motor_dir;
        std::string knee_flip_motor_dir;
        std::string ankle_flip_motor_dir;
        std::string elbow_flip_motor_dir;
		
		std::string hip_flip_torque_dir;
        std::string knee_flip_torque_dir;
        std::string ankle_flip_torque_dir;
        std::string elbow_flip_torque_dir;
		
		std::string hip_flip_angle_dir;
        std::string knee_flip_angle_dir;
        std::string ankle_flip_angle_dir;
        std::string elbow_flip_angle_dir;
		
		float left_hip_RoM;
		float right_hip_RoM;
		float left_knee_RoM;
		float right_knee_RoM;
		float left_ankle_RoM;
		float right_ankle_RoM;
        float left_elbow_RoM;
        float right_elbow_RoM;
		
    };
#endif


#endif