#include "ParseIni.h"
#include "Logger.h"
#include <stdlib.h> 

//We only need to parse the INI file if we have access to the SD card.
//The nano will get the info through SPI so doesn't need these functions.
#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41) 
    void ini_print_error_message(uint8_t e, bool eol = true)
    {
        if(Serial)
        {
            switch (e) {
            case IniFile::errorNoError:
                logger::print("no error");
                break;
            case IniFile::errorFileNotFound:
                logger::print("file not found");
                break;
            case IniFile::errorFileNotOpen:
                logger::print("file not open");
                break;
            case IniFile::errorBufferTooSmall:
                logger::print("buffer too small");
                break;
            case IniFile::errorSeekError:
                logger::print("seek error");
                break;
            case IniFile::errorSectionNotFound:
                logger::print("section not found");
                break;
            case IniFile::errorKeyNotFound:
                logger::print("key not found");
                break;
            case IniFile::errorEndOfFile:
                logger::print("end of file");
                break;
            case IniFile::errorUnknownError:
                logger::print("unknown error");
                break;
            default:
                logger::print("unknown error value");
                break;
            }
            if (eol)
            {
                logger::print("\n");
            }
        }
    }


    void ini_parser(uint8_t* config_to_send)
    {
        ini_parser("/config.ini", config_to_send);   
    }


    void ini_parser(char* filename, uint8_t* config_to_send)
    {
        ConfigData data;  //Creates object to hold the key values

        //Set pin to select the SD card
        pinMode(SD_SELECT, OUTPUT);
        digitalWrite(SD_SELECT, HIGH); //Disable SD card

        //Create buffer to hold the data read from the file
        const size_t buffer_len = ini_config::buffer_length;
        char buffer[buffer_len];

        //Setup the SPI to read the SD card
        SPI.begin();

        if (!SD.begin(SD_SELECT))
        {
                while (1)
                
                if (Serial)
                {
                    logger::print("SD.begin() failed");
                    logger::print("\n");
                }
        }

        //Check the for the ini file
        IniFile ini(filename);

        if (!ini.open()) 
        {
            if(Serial)
            {
                logger::print("Ini file ");
                logger::print(filename);
                logger::print(" does not exist");
                logger::print("\n");
            }
            
            //Cannot do anything else
            while (1);
        }

        if(Serial)
        {
            logger::print("Ini file exists");
            logger::print("\n");
        }

        //Check the file is valid. This can be used to warn if any lines are longer than the buffer.
        if (!ini.validate(buffer, buffer_len))
        {
            if(Serial)
            {
                logger::print("ini file ");
                logger::print(ini.getFilename());
                logger::print(" not valid: ");
                ini_print_error_message(ini.getError());
            }

            //Cannot do anything else
            while (1);
        }
          
        get_section_key(ini,"Board","name",buffer,buffer_len); //Read the key.
        data.board_name = buffer;                              //Store the value
        
        // logger::print(data.board_name.c_str());
        // logger::print("\t");
        // logger::println(config_map::board_name[data.board_name]);

        config_to_send[config_defs::board_name_idx] = config_map::board_name[data.board_name];  //Encode the key to an uint8_t
        
        get_section_key(ini,"Board","version",buffer,buffer_len);
        data.board_version = buffer;

        // logger::print(data.board_version.c_str());
        // logger::print("\t");
        // logger::println(config_map::board_version[data.board_version]);

        config_to_send[config_defs::board_version_idx] = config_map::board_version[data.board_version];
        
        //=========================================================
        
        get_section_key(ini,"Battery","name",buffer,buffer_len);
        data.battery = buffer;

        // logger::print(data.board_version.c_str());
        // logger::print("\t");
        // logger::println(config_map::board_version[data.board_version]);

        config_to_send[config_defs::battery_idx] = config_map::battery[data.battery];
        
        //=========================================================
        
        get_section_key(ini,"Exo","name",buffer,buffer_len);
        data.exo_name = buffer;

        // logger::print(data.exo_name.c_str());
        // logger::print("\t");
        // logger::println(config_map::exo_name[data.exo_name]);

        config_to_send[config_defs::exo_name_idx] = config_map::exo_name[data.exo_name];
        
        //=========================================================
        // Read all exoskeleton configuration keys directly from the [Exo] section.
        // [Exo] name is still used as an exo type selector for comms/packet format.
        get_section_key(ini,"Exo","sides",buffer,buffer_len); 
        data.exo_sides = buffer;
		
        // logger::print(data.exo_sides.c_str());
        // logger::print("\t");
        // logger::println(config_map::exo_side[data.exo_sides]);

        config_to_send[config_defs::exo_side_idx] = config_map::exo_side[data.exo_sides];
        
        //--------------------------------------------------------
        
        get_section_key(ini,"Exo","hip",buffer,buffer_len);
        data.exo_hip = buffer;

        // logger::print(data.exo_hip.c_str());
        // logger::print("\t");
        // logger::println(config_map::motor[data.exo_hip]);

        config_to_send[config_defs::hip_idx] = config_map::motor[data.exo_hip];
        
        get_section_key(ini,"Exo","knee",buffer,buffer_len);
        data.exo_knee = buffer;

        // logger::print(data.exo_knee.c_str());
        // logger::print("\t");
        // logger::println(config_map::motor[data.exo_knee]);

        config_to_send[config_defs::knee_idx] = config_map::motor[data.exo_knee];
        
        get_section_key(ini,"Exo","ankle",buffer,buffer_len);
        data.exo_ankle = buffer;

        // logger::print(data.exo_ankle.c_str());
        // logger::print("\t");
        // logger::println(config_map::motor[data.exo_ankle]);

        config_to_send[config_defs::ankle_idx] = config_map::motor[data.exo_ankle];

        get_section_key(ini,"Exo","elbow",buffer,buffer_len);
        data.exo_elbow = buffer;

        // logger::print(data.exo_elbow.c_str());
        // logger::print("\t");
        // logger::println(config_map::motor[data.exo_elbow]);

        config_to_send[config_defs::elbow_idx] = config_map::motor[data.exo_elbow];

        get_section_key(ini,"Exo","arm_1",buffer,buffer_len);
        data.exo_arm_1 = buffer;

        config_to_send[config_defs::arm_1_idx] = config_map::motor[data.exo_arm_1];

        get_section_key(ini,"Exo","arm_2",buffer,buffer_len);
        data.exo_arm_2 = buffer;

        config_to_send[config_defs::arm_2_idx] = config_map::motor[data.exo_arm_2];
        
        //--------------------------------------------------------
        
        get_section_key(ini,"Exo","hipGearRatio",buffer,buffer_len);
        data.hip_gearing = buffer;

        // logger::print(data.hip_gearing.c_str());
        // logger::print("\t");
        // logger::println(config_map::motor[data.hip_gearing]);

        config_to_send[config_defs::hip_gear_idx] = config_map::gearing[data.hip_gearing];
        
        get_section_key(ini,"Exo","kneeGearRatio",buffer,buffer_len);
        data.knee_gearing = buffer;

        // logger::print(data.knee_gearing.c_str());
        // logger::print("\t");
        // logger::println(config_map::motor[data.knee_gearing]);

        config_to_send[config_defs::knee_gear_idx] = config_map::gearing[data.knee_gearing];
        
        get_section_key(ini,"Exo","ankleGearRatio",buffer,buffer_len);
        data.ankle_gearing = buffer;

        // logger::print(data.exo_ankle.c_str());
        // logger::print("\t");
        // logger::println(config_map::motor[data.exo_ankle]);

        config_to_send[config_defs::ankle_gear_idx] = config_map::gearing[data.ankle_gearing];

        get_section_key(ini,"Exo","elbowGearRatio",buffer,buffer_len);
        data.elbow_gearing = buffer;

        // logger::print(data.exo_elbow.c_str());
        // logger::print("\t");
        // logger::println(config_map::motor[data.exo_elbow]);

        config_to_send[config_defs::elbow_gear_idx] = config_map::gearing[data.elbow_gearing];

        get_section_key(ini,"Exo","arm_1GearRatio",buffer,buffer_len);
        data.arm_1_gearing = buffer;
        config_to_send[config_defs::arm_1_gear_idx] = config_map::gearing[data.arm_1_gearing];

        get_section_key(ini,"Exo","arm_2GearRatio",buffer,buffer_len);
        data.arm_2_gearing = buffer;
        config_to_send[config_defs::arm_2_gear_idx] = config_map::gearing[data.arm_2_gearing];
        
        //--------------------------------------------------------
        
        get_section_key(ini,"Exo","hipDefaultController",buffer,buffer_len);
        data.exo_hip_default_controller = buffer;

        // logger::print(data.exo_hip_default_controller.c_str());
        // logger::print("\t");
        // logger::println(config_map::hip_controllers[data.exo_hip_default_controller]);

        config_to_send[config_defs::exo_hip_default_controller_idx] = config_map::hip_controllers[data.exo_hip_default_controller];
        
        get_section_key(ini,"Exo","kneeDefaultController",buffer,buffer_len);
        data.exo_knee_default_controller = buffer;

        // logger::print(data.exo_knee_default_controller.c_str());
        // logger::print("\t");
        // logger::println(config_map::knee_controllers[data.exo_knee_default_controller]);

        config_to_send[config_defs::exo_knee_default_controller_idx] = config_map::knee_controllers[data.exo_knee_default_controller];
        
        get_section_key(ini,"Exo","ankleDefaultController",buffer,buffer_len);
        data.exo_ankle_default_controller = buffer;

        // logger::print(data.exo_ankle_default_controller.c_str());
        // logger::print("\t");
        // logger::println(config_map::ankle_controllers[data.exo_ankle_default_controller]);

        config_to_send[config_defs::exo_ankle_default_controller_idx] = config_map::ankle_controllers[data.exo_ankle_default_controller];

        get_section_key(ini,"Exo","elbowDefaultController",buffer,buffer_len);
        data.exo_elbow_default_controller = buffer;

        // logger::print(data.exo_elbow_default_controller.c_str());
        // logger::print("\t");
        // logger::println(config_map::elbow_controllers[data.exo_elbow_default_controller]);

        config_to_send[config_defs::exo_elbow_default_controller_idx] = config_map::elbow_controllers[data.exo_elbow_default_controller];

        get_section_key(ini,"Exo","arm_1DefaultController",buffer,buffer_len);
        data.exo_arm_1_default_controller = buffer;
        config_to_send[config_defs::exo_arm_1_default_controller_idx] = config_map::arm_1_controllers[data.exo_arm_1_default_controller];

        get_section_key(ini,"Exo","arm_2DefaultController",buffer,buffer_len);
        data.exo_arm_2_default_controller = buffer;
        config_to_send[config_defs::exo_arm_2_default_controller_idx] = config_map::arm_2_controllers[data.exo_arm_2_default_controller];
        
        #ifdef SIMPLE_DEBUG
		Serial.print("\nLoaded exoskeleton configuration: ");
		Serial.print("\nexo_name: ");
		Serial.print(config_map::exo_name[data.exo_name]);
		Serial.print(", side(s): ");
		Serial.print(config_map::exo_side[data.exo_sides]);
		Serial.print("\nFor a list of all pre-defined exoskeleton types, see namespace config_map in ParseIni.h (one-based indexing here)");
		#endif
		
		//--------------------------------------------------------
        
        get_section_key(ini,"Exo","hipUseTorqueSensor",buffer,buffer_len);
        data.hip_use_torque_sensor = buffer;

        // logger::print(data.hip_use_torque_sensor.c_str());
        // logger::print("\t");
        // logger::println(config_map::use_torque_sensor[data.hip_use_torque_sensor]);

        config_to_send[config_defs::hip_use_torque_sensor_idx] = config_map::use_torque_sensor[data.hip_use_torque_sensor];
        
        get_section_key(ini,"Exo","kneeUseTorqueSensor",buffer,buffer_len);
        data.knee_use_torque_sensor = buffer;

        // logger::print(data.knee_use_torque_sensor.c_str());
        // logger::print("\t");
        // logger::println(config_map::use_torque_sensor[data.knee_use_torque_sensor]);

        config_to_send[config_defs::knee_use_torque_sensor_idx] = config_map::use_torque_sensor[data.knee_use_torque_sensor];
        
        get_section_key(ini,"Exo","ankleUseTorqueSensor",buffer,buffer_len);
        data.ankle_use_torque_sensor = buffer;

        // logger::print(data.ankle_use_torque_sensor.c_str());
        // logger::print("\t");
        // logger::println(config_map::use_torque_sensor[data.ankle_use_torque_sensor]);

        config_to_send[config_defs::ankle_use_torque_sensor_idx] = config_map::use_torque_sensor[data.ankle_use_torque_sensor];

        get_section_key(ini,"Exo","elbowUseTorqueSensor",buffer,buffer_len);
        data.elbow_use_torque_sensor = buffer;

        // logger::print(data.elbow_use_torque_sensor.c_str());
        // logger::print("\t");
        // logger::println(config_map::use_torque_sensor[data.elbow_use_torque_sensor]);

        config_to_send[config_defs::elbow_use_torque_sensor_idx] = config_map::use_torque_sensor[data.elbow_use_torque_sensor];

        get_section_key(ini,"Exo","arm_1UseTorqueSensor",buffer,buffer_len);
        data.arm_1_use_torque_sensor = buffer;
        config_to_send[config_defs::arm_1_use_torque_sensor_idx] = config_map::use_torque_sensor[data.arm_1_use_torque_sensor];

        get_section_key(ini,"Exo","arm_2UseTorqueSensor",buffer,buffer_len);
        data.arm_2_use_torque_sensor = buffer;
        config_to_send[config_defs::arm_2_use_torque_sensor_idx] = config_map::use_torque_sensor[data.arm_2_use_torque_sensor];

        //--------------------------------------------------------
		
        get_section_key(ini,"Exo","hipFlipMotorDir",buffer,buffer_len);
        data.hip_flip_motor_dir = buffer;
        config_to_send[config_defs::hip_flip_motor_dir_idx] = config_map::flip_motor_dir[data.hip_flip_motor_dir];

        get_section_key(ini,"Exo","kneeFlipMotorDir",buffer,buffer_len);
        data.knee_flip_motor_dir = buffer;
        config_to_send[config_defs::knee_flip_motor_dir_idx] = config_map::flip_motor_dir[data.knee_flip_motor_dir];
        
        get_section_key(ini,"Exo","ankleFlipMotorDir",buffer,buffer_len);
        data.ankle_flip_motor_dir = buffer;
        config_to_send[config_defs::ankle_flip_motor_dir_idx] = config_map::flip_motor_dir[data.ankle_flip_motor_dir];

        get_section_key(ini,"Exo","elbowFlipMotorDir",buffer,buffer_len);
        data.elbow_flip_motor_dir = buffer;
        config_to_send[config_defs::elbow_flip_motor_dir_idx] = config_map::flip_motor_dir[data.elbow_flip_motor_dir];

        get_section_key(ini,"Exo","arm_1FlipMotorDir",buffer,buffer_len);
        data.arm_1_flip_motor_dir = buffer;
        config_to_send[config_defs::arm_1_flip_motor_dir_idx] = config_map::flip_motor_dir[data.arm_1_flip_motor_dir];

        get_section_key(ini,"Exo","arm_2FlipMotorDir",buffer,buffer_len);
        data.arm_2_flip_motor_dir = buffer;
        config_to_send[config_defs::arm_2_flip_motor_dir_idx] = config_map::flip_motor_dir[data.arm_2_flip_motor_dir];

        //--------------------------------------------------------
		
        get_section_key(ini,"Exo","hipFlipTorqueDir",buffer,buffer_len);
        data.hip_flip_torque_dir = buffer;
        config_to_send[config_defs::hip_flip_torque_dir_idx] = config_map::flip_torque_dir[data.hip_flip_torque_dir];

        get_section_key(ini,"Exo","kneeFlipTorqueDir",buffer,buffer_len);
        data.knee_flip_torque_dir = buffer;
        config_to_send[config_defs::knee_flip_torque_dir_idx] = config_map::flip_torque_dir[data.knee_flip_torque_dir];
        
        get_section_key(ini,"Exo","ankleFlipTorqueDir",buffer,buffer_len);
        data.ankle_flip_torque_dir = buffer;
        config_to_send[config_defs::ankle_flip_torque_dir_idx] = config_map::flip_torque_dir[data.ankle_flip_torque_dir];

        get_section_key(ini,"Exo","elbowFlipTorqueDir",buffer,buffer_len);
        data.elbow_flip_torque_dir = buffer;
        config_to_send[config_defs::elbow_flip_torque_dir_idx] = config_map::flip_torque_dir[data.elbow_flip_torque_dir];

        get_section_key(ini,"Exo","arm_1FlipTorqueDir",buffer,buffer_len);
        data.arm_1_flip_torque_dir = buffer;
        config_to_send[config_defs::arm_1_flip_torque_dir_idx] = config_map::flip_torque_dir[data.arm_1_flip_torque_dir];

        get_section_key(ini,"Exo","arm_2FlipTorqueDir",buffer,buffer_len);
        data.arm_2_flip_torque_dir = buffer;
        config_to_send[config_defs::arm_2_flip_torque_dir_idx] = config_map::flip_torque_dir[data.arm_2_flip_torque_dir];

        //--------------------------------------------------------

        get_section_key(ini,"Exo","hipFlipAngleDir",buffer,buffer_len);
        data.hip_flip_angle_dir = buffer;
        config_to_send[config_defs::hip_flip_angle_dir_idx] = config_map::flip_angle_dir[data.hip_flip_angle_dir];

        get_section_key(ini,"Exo","kneeFlipAngleDir",buffer,buffer_len);
        data.knee_flip_angle_dir = buffer;
        config_to_send[config_defs::knee_flip_angle_dir_idx] = config_map::flip_angle_dir[data.knee_flip_angle_dir];

        get_section_key(ini,"Exo","ankleFlipAngleDir",buffer,buffer_len);
        data.ankle_flip_angle_dir = buffer;
        config_to_send[config_defs::ankle_flip_angle_dir_idx] = config_map::flip_angle_dir[data.ankle_flip_angle_dir];

        get_section_key(ini,"Exo","elbowFlipAngleDir",buffer,buffer_len);
        data.elbow_flip_angle_dir = buffer;
        config_to_send[config_defs::elbow_flip_angle_dir_idx] = config_map::flip_angle_dir[data.elbow_flip_angle_dir];

        get_section_key(ini,"Exo","arm_1FlipAngleDir",buffer,buffer_len);
        data.arm_1_flip_angle_dir = buffer;
        config_to_send[config_defs::arm_1_flip_angle_dir_idx] = config_map::flip_angle_dir[data.arm_1_flip_angle_dir];

        get_section_key(ini,"Exo","arm_2FlipAngleDir",buffer,buffer_len);
        data.arm_2_flip_angle_dir = buffer;
        config_to_send[config_defs::arm_2_flip_angle_dir_idx] = config_map::flip_angle_dir[data.arm_2_flip_angle_dir];

        //--------------------------------------------------------
		
		get_section_key(ini,"Exo","leftHipRoM",buffer,buffer_len);
        data.left_hip_RoM = atof(buffer);
        config_to_send[config_defs::left_hip_RoM_idx] =  data.left_hip_RoM;
		
		get_section_key(ini,"Exo","rightHipRoM",buffer,buffer_len);
        data.right_hip_RoM = atof(buffer);
        config_to_send[config_defs::right_hip_RoM_idx] =  data.right_hip_RoM;
		
		get_section_key(ini,"Exo","leftKneeRoM",buffer,buffer_len);
        data.left_knee_RoM = atof(buffer);
        config_to_send[config_defs::left_knee_RoM_idx] =  data.left_knee_RoM;
		
		get_section_key(ini,"Exo","rightKneeRoM",buffer,buffer_len);
        data.right_knee_RoM = atof(buffer);
        config_to_send[config_defs::right_knee_RoM_idx] =  data.right_knee_RoM;
		
		get_section_key(ini,"Exo","leftAnkleRoM",buffer,buffer_len);
        data.left_ankle_RoM = atof(buffer);
        config_to_send[config_defs::left_ankle_RoM_idx] =  data.left_ankle_RoM;
		
		get_section_key(ini,"Exo","rightAnkleRoM",buffer,buffer_len);
        data.right_ankle_RoM = atof(buffer);
        config_to_send[config_defs::right_ankle_RoM_idx] =  data.right_ankle_RoM;

        get_section_key(ini,"Exo","leftElbowRoM",buffer,buffer_len);
        data.left_elbow_RoM = atof(buffer);
        config_to_send[config_defs::left_elbow_RoM_idx] = data.left_elbow_RoM;

        get_section_key(ini,"Exo","rightElbowRoM",buffer,buffer_len);
        data.right_elbow_RoM = atof(buffer);
        config_to_send[config_defs::right_elbow_RoM_idx] = data.right_elbow_RoM;

        get_section_key(ini,"Exo","leftArm1RoM",buffer,buffer_len);
        data.left_arm_1_RoM = atof(buffer);
        config_to_send[config_defs::left_arm_1_RoM_idx] = data.left_arm_1_RoM;

        get_section_key(ini,"Exo","rightArm1RoM",buffer,buffer_len);
        data.right_arm_1_RoM = atof(buffer);
        config_to_send[config_defs::right_arm_1_RoM_idx] = data.right_arm_1_RoM;

        get_section_key(ini,"Exo","leftArm2RoM",buffer,buffer_len);
        data.left_arm_2_RoM = atof(buffer);
        config_to_send[config_defs::left_arm_2_RoM_idx] = data.left_arm_2_RoM;

        get_section_key(ini,"Exo","rightArm2RoM",buffer,buffer_len);
        data.right_arm_2_RoM = atof(buffer);
        config_to_send[config_defs::right_arm_2_RoM_idx] = data.right_arm_2_RoM;
		
		get_section_key(ini,"Exo","leftHipTorqueOffset",buffer,buffer_len);
		data.left_hip_torque_offset = atof(buffer);
		config_to_send[config_defs::left_hip_torque_offset_idx] =  data.left_hip_torque_offset;
		
		get_section_key(ini,"Exo","rightHipTorqueOffset",buffer,buffer_len);
		data.right_hip_torque_offset = atof(buffer);
		config_to_send[config_defs::right_hip_torque_offset_idx] =  data.right_hip_torque_offset;
		
		get_section_key(ini,"Exo","leftKneeTorqueOffset",buffer,buffer_len);
		data.left_knee_torque_offset = atof(buffer);
		config_to_send[config_defs::left_knee_torque_offset_idx] =  data.left_knee_torque_offset;
		
		get_section_key(ini,"Exo","rightKneeTorqueOffset",buffer,buffer_len);
		data.right_knee_torque_offset = atof(buffer);
		config_to_send[config_defs::right_knee_torque_offset_idx] =  data.right_knee_torque_offset;
		
		get_section_key(ini,"Exo","leftAnkleTorqueOffset",buffer,buffer_len);
		data.left_ankle_torque_offset = atof(buffer);
		config_to_send[config_defs::left_ankle_torque_offset_idx] =  data.left_ankle_torque_offset;
		
		get_section_key(ini,"Exo","rightAnkleTorqueOffset",buffer,buffer_len);
		data.right_ankle_torque_offset = atof(buffer);
		config_to_send[config_defs::right_ankle_torque_offset_idx] =  data.right_ankle_torque_offset;
		
		get_section_key(ini,"Exo","leftElbowTorqueOffset",buffer,buffer_len);
		data.left_elbow_torque_offset = atof(buffer);
		config_to_send[config_defs::left_elbow_torque_offset_idx] =  data.left_elbow_torque_offset;
		
		get_section_key(ini,"Exo","rightElbowTorqueOffset",buffer,buffer_len);
		data.right_elbow_torque_offset = atof(buffer);
		config_to_send[config_defs::right_elbow_torque_offset_idx] =  data.right_elbow_torque_offset;

        get_section_key(ini,"Exo","leftArm1TorqueOffset",buffer,buffer_len);
        data.left_arm_1_torque_offset = atof(buffer);
        config_to_send[config_defs::left_arm_1_torque_offset_idx] = data.left_arm_1_torque_offset;

        get_section_key(ini,"Exo","rightArm1TorqueOffset",buffer,buffer_len);
        data.right_arm_1_torque_offset = atof(buffer);
        config_to_send[config_defs::right_arm_1_torque_offset_idx] = data.right_arm_1_torque_offset;

        get_section_key(ini,"Exo","leftArm2TorqueOffset",buffer,buffer_len);
        data.left_arm_2_torque_offset = atof(buffer);
        config_to_send[config_defs::left_arm_2_torque_offset_idx] = data.left_arm_2_torque_offset;

        get_section_key(ini,"Exo","rightArm2TorqueOffset",buffer,buffer_len);
        data.right_arm_2_torque_offset = atof(buffer);
        config_to_send[config_defs::right_arm_2_torque_offset_idx] = data.right_arm_2_torque_offset;
    }

    /*
     * Retrieve the key from the ini file, and put it in the buffer.
     * Also prints the value or the error (if it can't find the key).
     * 
     * Requires Serial exists.
     */
    void get_section_key(IniFile ini, const char* section, const char* key, char* buffer, size_t buffer_len)
    {
        // Always clear the output buffer so callers never see stale values if a key is missing.
        if (buffer_len > 0) { buffer[0] = '\0'; }

        // Fetch a value from a key which is present.
        if (ini.getValue(section, key, buffer, buffer_len))
        {
            if(Serial)
            {
                // logger::print("section '");
                // logger::print(section);
                // logger::print("' has an entry '");
                // logger::print(key);
                // logger::print("' with value ");
                // logger::print(buffer);
                // logger::print("\n");
            }
        }
        //Print the error if the key can't be found.
        else 
        {
            if(Serial)
            {
                logger::print("Could not read '");
                logger::print(key);
                logger::print("' from section '");
                logger::print(section);
                logger::print("' , error was ");
                ini_print_error_message(ini.getError());
            }
        }
    }
#endif
