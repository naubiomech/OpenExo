#include "ParamsFromSD.h"
#include "Logger.h"
//#define SD_PARAM_DEBUG 1

#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)

    void print_param_error_message(uint8_t error_type)
    {
        //logger::print(utils::get_is_left(error_type)? "Left " : "Right ");
        switch (error_type & ((uint8_t)config_defs::joint_id::hip | (uint8_t)config_defs::joint_id::knee | (uint8_t)config_defs::joint_id::ankle | (uint8_t)config_defs::joint_id::elbow))
        {
            case (uint8_t)config_defs::joint_id::hip:
                //logger::print("Hip ");    
                break;
            case (uint8_t)config_defs::joint_id::knee:
                //logger::print("Knee ");
                break;
            case (uint8_t)config_defs::joint_id::ankle:
                //logger::print("Ankle ");
                break;
            case (uint8_t)config_defs::joint_id::elbow:
                //logger::print("Elbow ");
                break;
        }
        if (utils::get_bit(error_type, param_error::SD_not_found_idx))
        {
            //logger::print("SD Not Found, ");
        }            
        if (utils::get_bit(error_type, param_error::SD_not_found_idx))
        {
            //logger::print("File Not Found, ");
        } 
        //logger::println("File Not Found, ");
    }
    
    uint8_t set_controller_params(uint8_t joint_id, uint8_t controller_id, uint8_t set_num, ExoData* exo_data)
    {   
        //SD inherits from stream which has a lot more useful methods that we will use.
        File param_file;
        std::string filename;
        uint8_t header_size;        //Number of lines to skip before the parameters
        uint8_t param_num_in_file;  //Number of parameters to pull in
        uint8_t line_to_read;       //Line to read the parameters from
        uint8_t error_type = 0;     //Error message holder

       
        switch(utils::get_joint_type(joint_id))
        {
            case (uint8_t)config_defs::joint_id::hip:
            {
                #ifdef SD_PARAM_DEBUG
                    logger::println("\n\nset_controller_params : Hip");
                #endif

                //Connect to SD card
                SPI.begin();

                #ifdef SD_PARAM_DEBUG
                    logger::println("set_controller_params : SPI Begin");
                #endif

                if (!SD.begin(SD_SELECT))
                {
                    error_type = utils::update_bit((uint8_t)config_defs::joint_id::hip, 1, param_error::SD_not_found_idx);
                    
                    #ifdef SD_PARAM_DEBUG
                        logger::println("set_controller_params : SD Not Found");
                    #endif
                    
                    return error_type;
                }
                else 
                {
                    //Get filename
                    filename = controller_parameter_filenames::hip[controller_id];

                    #ifdef SD_PARAM_DEBUG
                        logger::print("set_controller_params : filename = ");
                        logger::println(filename.c_str());
                    #endif

                    //Open File
                    param_file = SD.open(filename.c_str(), FILE_READ);

                    #ifdef SD_PARAM_DEBUG
                        logger::print("set_controller_params : ");
                        logger::print(filename.c_str());
                        logger::println(" opened");
                    #endif

                    //Check file exists
                    if (param_file)
                    {   
                        while(param_file.available())
                        {
                            //First value should be header size
                            header_size = param_file.parseInt();

                            #ifdef SD_PARAM_DEBUG
                                logger::print("set_controller_params : header size ");
                                logger::println(header_size);
                            #endif

                            //Skip to the line we need
                            line_to_read = header_size + set_num;
                            for (int line_being_read = 0; line_being_read < line_to_read; line_being_read++)
                            { 
                                //First value in second line should be parameter number
                                if (line_being_read == 1)
                                {
                                    param_num_in_file = param_file.parseInt();

                                    #ifdef SD_PARAM_DEBUG
                                        logger::print("set_controller_params : Number of parameters in file = ");
                                        logger::println(param_num_in_file);
                                    #endif    
                                }

                                //Keep going through the file till the next new line. This is so it will restart if timeout happens.          
                                while(!param_file.findUntil('\n','\n'))
                                {
                                    ;
                                }

                                #ifdef SD_PARAM_DEBUG
                                    logger::print("set_controller_params : read line ");
                                    logger::println(line_being_read);
                                #endif
                            }
                            
                            //Store the line start value so we can go back here
                            unsigned long line_start = param_file.position();

                            #ifdef SD_PARAM_DEBUG
                                logger::print("set_controller_params : parameter set start ");
                                logger::println(line_start);
                            #endif
                            
                            //Find the end of the line
                            param_file.readStringUntil('\n');
                            unsigned long line_end = param_file.position();
                            
                            #ifdef SD_PARAM_DEBUG
                                logger::print("set_controller_params : parameter set end ");
                                logger::println(line_end);
                            #endif
                            
                            //Reset to the start of the line
                            param_file.seek(line_start);

                            #ifdef SD_PARAM_DEBUG
                                logger::println("set_controller_params : reset to line start");
                            #endif

                            //Set the parameters.
                            uint8_t param_num = 0;
                            float read_val = 0;
                            if(utils::get_is_left(joint_id))
                            {
                                #ifdef SD_PARAM_DEBUG
                                    logger::println("set_controller_params : is Left ");
                                #endif
                                
                                //Read till the end of the line or all the parameters are full
                                while (param_num < controller_defs::max_parameters)
                                {
                                    if (param_num_in_file > param_num)
                                    {
                                        read_val = param_file.parseFloat();
                                        
                                        #ifdef SD_PARAM_DEBUG
                                            logger::print("+Value in file :\t");
                                            logger::println(read_val);
                                        #endif  
                                    }
                                    else
                                    {
                                        read_val = 0;

                                        #ifdef SD_PARAM_DEBUG
                                            logger::print("-File Line Ended :\t");
                                            logger::println(read_val);
                                        #endif
                                    }
                                    
                                    exo_data->left_side.hip.controller.parameters[param_num] = read_val;
                                    
                                    param_num++;
                                }
                            }
                            else
                            {
                                #ifdef SD_PARAM_DEBUG
                                    logger::println("set_controller_params : is Right ");
                                #endif
                                
                                //Read till the end of the line or all the parameters are full
                                while (param_num < controller_defs::max_parameters)
                                {
                                    if (param_num_in_file > param_num)
                                    {
                                        read_val = param_file.parseFloat();

                                        #ifdef SD_PARAM_DEBUG
                                            logger::print("+Value in file :\t");
                                            logger::println(read_val);
                                        #endif 
                                    }
                                    else
                                    {
                                        read_val = 0;

                                        #ifdef SD_PARAM_DEBUG
                                            logger::print("-File Line Ended :\t");
                                            logger::println(read_val);
                                        #endif
                                    }
                                    
                                    exo_data->right_side.hip.controller.parameters[param_num] = read_val;
                                    
                                    param_num++;
                                }
                            }
                            //We don't need to read the rest of the file
                            break;
                        }
                        
                    }
                    else
                    { 
                        error_type = utils::update_bit((uint8_t)config_defs::joint_id::hip, 1, param_error::file_not_found_idx);

                        #ifdef SD_PARAM_DEBUG
                            logger::println("set_controller_params : File not found");
                        #endif
                    }
                    param_file.close();

                    #ifdef SD_PARAM_DEBUG
                        logger::println("set_controller_params : File Closed");
                    #endif
                }
                break;
            }
            case (uint8_t)config_defs::joint_id::knee:
            {
                #ifdef SD_PARAM_DEBUG
                    logger::println("\n\nset_controller_params : Knee");
                #endif

                //Connect to SD card
                SPI.begin();

                #ifdef SD_PARAM_DEBUG
                    logger::println("set_controller_params : SPI Begin");
                #endif

                if (!SD.begin(SD_SELECT))
                {
                    error_type = utils::update_bit((uint8_t)config_defs::joint_id::knee, 1, param_error::SD_not_found_idx);

                    #ifdef SD_PARAM_DEBUG
                        logger::println("set_controller_params : SD Not Found");
                    #endif

                    return error_type;
                }
                else 
                {
                    //Get filename
                    filename = controller_parameter_filenames::knee[controller_id];

                    #ifdef SD_PARAM_DEBUG
                        logger::print("set_controller_params : filename = ");
                        logger::println(filename.c_str());
                    #endif

                    //Open File
                    param_file = SD.open(filename.c_str(), FILE_READ);

                    #ifdef SD_PARAM_DEBUG
                        logger::print("set_controller_params : ");
                        logger::print(filename.c_str());
                        logger::println(" opened");
                    #endif

                    //Check file exists
                    if (param_file)
                    {   
                        while(param_file.available())
                        {
                            //First value should be header size
                            header_size = param_file.parseInt();

                            #ifdef SD_PARAM_DEBUG
                                logger::print("set_controller_params : header size ");
                                logger::println(header_size);
                            #endif

                            //Skip to the line we need
                            line_to_read = header_size + set_num;
                            for (int line_being_read = 0; line_being_read < line_to_read; line_being_read++)
                            { 
                                //First value in second line should be parameter number
                                if (line_being_read == 1)
                                {
                                    param_num_in_file = param_file.parseInt();

                                    #ifdef SD_PARAM_DEBUG
                                        logger::print("set_controller_params : Number of parameters in file = ");
                                        logger::println(param_num_in_file);
                                    #endif    
                                } 

                                //Keep going through the file till the next new line.  This is so it will restart if timeout happens.          
                                while(!param_file.findUntil('\n','\n'))
                                {
                                    ;
                                }

                                #ifdef SD_PARAM_DEBUG
                                    logger::print("set_controller_params : read line ");
                                    logger::println(line_being_read);
                                #endif
                            }
                            
                            //Store the line start value so we can go back here
                            unsigned long line_start = param_file.position();

                            #ifdef SD_PARAM_DEBUG
                                logger::print("set_controller_params : parameter set start ");
                                logger::println(line_start);
                            #endif
                            
                            //Find the end of the line
                            param_file.readStringUntil('\n');
                            unsigned long line_end = param_file.position();

                            #ifdef SD_PARAM_DEBUG
                                logger::print("set_controller_params : parameter set end ");
                                logger::println(line_end);
                            #endif
                            
                            //Reset to the start of the line
                            param_file.seek(line_start);

                            #ifdef SD_PARAM_DEBUG
                                logger::println("set_controller_params : reset to line start");
                            #endif

                            //Set the parameters.
                            uint8_t param_num = 0;
                            float read_val = 0;
                            if(utils::get_is_left(joint_id))
                            {
                                #ifdef SD_PARAM_DEBUG
                                    logger::println("set_controller_params : is Left ");
                                #endif
                                
                                //Read till the end of the line or all the parameters are full
                                while (param_num < controller_defs::max_parameters)
                                {
                                    if (param_num_in_file > param_num)
                                    {
                                        read_val = param_file.parseFloat();
                                        #ifdef SD_PARAM_DEBUG
                                            logger::print("+Value in file :\t");
                                            logger::println(read_val);
                                        #endif   
                                    }
                                    else
                                    {
                                        read_val = 0;

                                        #ifdef SD_PARAM_DEBUG
                                            logger::print("-File Line Ended :\t");
                                            logger::println(read_val);
                                        #endif
                                    }
                                    
                                    exo_data->left_side.knee.controller.parameters[param_num] = read_val;
                                    
                                    param_num++;
                                }
                            }
                            else
                            {
                                #ifdef SD_PARAM_DEBUG
                                    logger::println("set_controller_params : is Right ");
                                #endif
                                
                                //Read till the end of the line or all the parameters are full
                                while (param_num < controller_defs::max_parameters)
                                {
                                    if (param_num_in_file > param_num)
                                    {
                                        read_val = param_file.parseFloat();

                                        #ifdef SD_PARAM_DEBUG
                                            logger::print("+Value in file :\t");
                                            logger::println(read_val);
                                        #endif
                                        
                                    }
                                    else
                                    {
                                        read_val = 0;

                                        #ifdef SD_PARAM_DEBUG
                                            logger::print("-File Line Ended :\t");
                                            logger::println(read_val);
                                        #endif
                                    }
                                    
                                    exo_data->right_side.knee.controller.parameters[param_num] = read_val;
                                    
                                    param_num++;
                                }
                            }
                            //We don't need to read the rest of the file
                            break;
                        }
                        
                    }
                    else
                    { 
                        error_type = utils::update_bit((uint8_t)config_defs::joint_id::knee, 1, param_error::file_not_found_idx);

                        #ifdef SD_PARAM_DEBUG
                            logger::println("set_controller_params : File not found");
                        #endif
                    }
                    param_file.close();

                    #ifdef SD_PARAM_DEBUG
                        logger::println("set_controller_params : File Closed");
                    #endif
                }
                break;
            }
            case (uint8_t)config_defs::joint_id::ankle:
            {
                #ifdef SD_PARAM_DEBUG
                    logger::println("\n\nset_controller_params : Ankle");
                #endif

                //Connect to SD card
                SPI.begin();

                #ifdef SD_PARAM_DEBUG
                    logger::println("set_controller_params : SPI Begin");
                #endif

                if (!SD.begin(SD_SELECT))
                {
                    error_type = utils::update_bit((uint8_t)config_defs::joint_id::ankle, 1, param_error::SD_not_found_idx);

                    #ifdef SD_PARAM_DEBUG
                        logger::println("set_controller_params : SD Not Found");
                    #endif

                    return error_type;
                }
                else 
                {
                    //Get filename
                    filename = controller_parameter_filenames::ankle[controller_id];

                    #ifdef SD_PARAM_DEBUG
                        logger::print("set_controller_params : filename = ");
                        logger::println(filename.c_str());
                    #endif

                    //Open File
                    param_file = SD.open(filename.c_str(), FILE_READ);

                    #ifdef SD_PARAM_DEBUG
                        logger::print("set_controller_params : ");
                        logger::print(filename.c_str());
                        logger::println(" opened");
                    #endif

                    //Check file exists
                    if (param_file)
                    {   
                
                        //Set the parameters.
                        uint8_t param_num = 0;
                        float read_val = 0;
                        while(param_file.available())
                        {
                            //First value should be header size
                            header_size = param_file.parseInt();

                            #ifdef SD_PARAM_DEBUG
                                logger::print("set_controller_params : header size ");
                                logger::println(header_size);
                            #endif

                            //Skip to the line we need
                            line_to_read = header_size + set_num;
                            for (int line_being_read = 0; line_being_read < line_to_read; line_being_read++)
                            { 
                                //First value in second line should be parameter number
                                if (line_being_read == 1)
                                {
                                    param_num_in_file = param_file.parseInt();

                                    #ifdef SD_PARAM_DEBUG
                                        logger::print("set_controller_params : Number of parameters in file = ");
                                        logger::println(param_num_in_file);
                                    #endif    
                                } 

                                //Keep going through the file till the next new line.  This is so it will restart if timeout happens.          
                                while(!param_file.findUntil('\n','\n'))
                                {
                                    ;
                                }

                                #ifdef SD_PARAM_DEBUG
                                    logger::print("set_controller_params : read line ");
                                    logger::println(line_being_read);
                                #endif
                            }
                            
                            //Store the line start value so we can go back here
                            unsigned long line_start = param_file.position();

                            #ifdef SD_PARAM_DEBUG
                                logger::print("set_controller_params : parameter set start ");
                                logger::println(line_start);
                            #endif
                            
                            //Find the end of the line
                            param_file.readStringUntil('\n');
                            unsigned long line_end = param_file.position();

                            #ifdef SD_PARAM_DEBUG
                                logger::print("set_controller_params : parameter set end ");
                                logger::println(line_end);
                            #endif
                            
                            //Reset to the start of the line
                            param_file.seek(line_start);

                            #ifdef SD_PARAM_DEBUG
                                logger::println("set_controller_params : reset to line start");
                            #endif
                            
                            if(utils::get_is_left(joint_id))
                            {
                                #ifdef SD_PARAM_DEBUG
                                    logger::println("set_controller_params : is Left ");
                                #endif
                                
                                //Read till the end of the line or all the parameters are full
                                while (param_num < controller_defs::max_parameters)
                                {
                                    if (param_num_in_file > param_num)
                                    {
                                        read_val = param_file.parseFloat();

                                        #ifdef SD_PARAM_DEBUG
                                            logger::print("+Value in file :\t");
                                            logger::println(read_val);
                                        #endif
                                        
                                    }
                                    else
                                    {
                                        read_val = 0;

                                        #ifdef SD_PARAM_DEBUG
                                            logger::print("-File Line Ended :\t");
                                            logger::println(read_val);
                                        #endif
                                    }
                                    
                                    exo_data->left_side.ankle.controller.parameters[param_num] = read_val;
                                    
                                    param_num++;
                                }
                            }
                            else
                            {
                                #ifdef SD_PARAM_DEBUG
                                    logger::println("set_controller_params : is Right ");
                                #endif
                                
                                //Read till the end of the line or all the parameters are full
                                while (param_num < controller_defs::max_parameters)
                                {
                                    if (param_num_in_file > param_num)
                                    {
                                        read_val = param_file.parseFloat();

                                        #ifdef SD_PARAM_DEBUG
                                            logger::print("+Value in file :\t");
                                            logger::println(read_val);
                                        #endif
                                        
                                    }
                                    else
                                    {
                                        read_val = 0;

                                        #ifdef SD_PARAM_DEBUG
                                            logger::print("-File Line Ended :\t");
                                            logger::println(read_val);
                                        #endif
                                    }
                                    
                                    exo_data->right_side.ankle.controller.parameters[param_num] = read_val;
                                    
                                    param_num++;
                                }
                            }
                            //We don't need to read the rest of the file
                            break;
                        }
                        
                    }
                    else
                    { 
                        error_type = utils::update_bit((uint8_t)config_defs::joint_id::ankle, 1, param_error::file_not_found_idx);

                        #ifdef SD_PARAM_DEBUG
                            logger::println("set_controller_params : File not found");
                        #endif
                    }
                    param_file.close();

                    #ifdef SD_PARAM_DEBUG
                        logger::println("set_controller_params : File Closed");
                    #endif
                }
                break;
            }
            case (uint8_t)config_defs::joint_id::elbow:
            {
                #ifdef SD_PARAM_DEBUG
                    logger::println("\n\nset_controller_params : Elbow");
                #endif

                //Connect to SD card
                SPI.begin();

                #ifdef SD_PARAM_DEBUG
                    logger::println("set_controller_params : SPI Begin");
                #endif

                if (!SD.begin(SD_SELECT))
                {
                    error_type = utils::update_bit((uint8_t)config_defs::joint_id::elbow, 1, param_error::SD_not_found_idx);

                    #ifdef SD_PARAM_DEBUG
                        logger::println("set_controller_params : SD Not Found");
                    #endif

                    return error_type;
                }
                else
                {
                    //Get filename
                    filename = controller_parameter_filenames::elbow[controller_id];

                    #ifdef SD_PARAM_DEBUG
                        logger::print("set_controller_params : filename = ");
                        logger::println(filename.c_str());
                    #endif

                    //Open File
                    param_file = SD.open(filename.c_str(), FILE_READ);

                    #ifdef SD_PARAM_DEBUG
                        logger::print("set_controller_params : ");
                        logger::print(filename.c_str());
                        logger::println(" opened");
                    #endif

                    //Check file exists
                    if (param_file)
                    {

                        //Set the parameters.
                        uint8_t param_num = 0;
                        float read_val = 0;
                        while (param_file.available())
                        {
                            //First value should be header size
                            header_size = param_file.parseInt();

                            #ifdef SD_PARAM_DEBUG
                                logger::print("set_controller_params : header size ");
                                logger::println(header_size);
                            #endif

                            //Skip to the line we need
                            line_to_read = header_size + set_num;
                            for (int line_being_read = 0; line_being_read < line_to_read; line_being_read++)
                            {
                                //First value in second line should be parameter number
                                if (line_being_read == 1)
                                {
                                    param_num_in_file = param_file.parseInt();

                                    #ifdef SD_PARAM_DEBUG
                                        logger::print("set_controller_params : Number of parameters in file = ");
                                        logger::println(param_num_in_file);
                                    #endif    
                                }

                                //Keep going through the file till the next new line.  This is so it will restart if timeout happens.          
                                while (!param_file.findUntil('\n', '\n'))
                                {
                                    ;
                                }

                                #ifdef SD_PARAM_DEBUG
                                    logger::print("set_controller_params : read line ");
                                    logger::println(line_being_read);
                                #endif
                            }

                            //Store the line start value so we can go back here
                            unsigned long line_start = param_file.position();

                            #ifdef SD_PARAM_DEBUG
                                logger::print("set_controller_params : parameter set start ");
                                logger::println(line_start);
                            #endif

                            //Find the end of the line
                            param_file.readStringUntil('\n');
                            unsigned long line_end = param_file.position();

                            #ifdef SD_PARAM_DEBUG
                                logger::print("set_controller_params : parameter set end ");
                                logger::println(line_end);
                            #endif

                            //Reset to the start of the line
                            param_file.seek(line_start);

                            #ifdef SD_PARAM_DEBUG
                                logger::println("set_controller_params : reset to line start");
                            #endif

                            if (utils::get_is_left(joint_id))
                            {
                                #ifdef SD_PARAM_DEBUG
                                    logger::println("set_controller_params : is Left ");
                               #endif

                                //Read till the end of the line or all the parameters are full
                                while (param_num < controller_defs::max_parameters)
                                {
                                    if (param_num_in_file > param_num)
                                    {
                                        read_val = param_file.parseFloat();

                                        #ifdef SD_PARAM_DEBUG
                                            logger::print("+Value in file :\t");
                                            logger::println(read_val);
                                        #endif

                                    }
                                    else
                                    {
                                        read_val = 0;

                                        #ifdef SD_PARAM_DEBUG
                                            logger::print("-File Line Ended :\t");
                                            logger::println(read_val);
                                        #endif
                                    }

                                    exo_data->left_side.elbow.controller.parameters[param_num] = read_val;

                                    param_num++;
                                }
                            }
                            else
                            {
                                #ifdef SD_PARAM_DEBUG
                                    logger::println("set_controller_params : is Right ");
                                #endif

                                //Read till the end of the line or all the parameters are full
                                while (param_num < controller_defs::max_parameters)
                                {
                                    if (param_num_in_file > param_num)
                                    {
                                        read_val = param_file.parseFloat();

                                        #ifdef SD_PARAM_DEBUG
                                            logger::print("+Value in file :\t");
                                            logger::println(read_val);
                                        #endif
                                    }
                                    else
                                    {
                                        read_val = 0;

                                        #ifdef SD_PARAM_DEBUG
                                            logger::print("-File Line Ended :\t");
                                            logger::println(read_val);
                                        #endif
                                    }

                                    exo_data->right_side.elbow.controller.parameters[param_num] = read_val;

                                    param_num++;
                                }
                            }
                            //We don't need to read the rest of the file
                            break;
                        }

                    }
                    else
                    {
                        error_type = utils::update_bit((uint8_t)config_defs::joint_id::elbow, 1, param_error::file_not_found_idx);

                        #ifdef SD_PARAM_DEBUG
                            logger::println("set_controller_params : File not found");
                        #endif
                    }
                    param_file.close();

                    #ifdef SD_PARAM_DEBUG
                        logger::println("set_controller_params : File Closed");
                    #endif
                }
                break;
            }
            
        }

        #ifdef SD_PARAM_DEBUG
            logger::println("set_controller_params : Never Entered Switch case");
        #endif

        return error_type;
    }

    /*get_controller_names(config_to_send)
    {
        if(config_to_send::config_defs::JointType == hip)
            {
                controller_list_msg = strcat(controller_list_msg,
                "disabled,
                zeor_torque,
                frank_collins_hip,
                constant_torque,
                chirp,
                step,
                phmc,"
                );
            }
            if(config_to_send::config_defs::JointType == knee)
            {
                controller_list_msg = strcat(controller_list_msg,
                "disabled,
                zero_torque,
                constant_torque,
                chirp,
                step,"
                );
            }
            if(config_to_send::config_defs::JointType == ankle)
            {
                controller_list_msg = strcat(controller_list_msg,
                "disabled,
                zero_torque,
                pjmc,
                zhang_collins,
                constant_torque,
                trec,
                chirp,
                step,
                spv2,"
                );
            }
            if(config_to_send::config_defs::JointType == elbow)
            {
                controller_list_msg = strcat(controller_list_msg,
                "disabled,
                zero_torque,
                elbow_min_max,
                chirp,
                step,"
                );
            }
    }
    */

#endif