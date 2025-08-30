/**
 * @file ble_commands.h
 * @author Chance Cuddeback
 * @brief This file declares the BLE commands, and the functions that should be called when they are received. 
 * @date 2022-08-22
 * 
 */
 #ifndef BLE_COMMANDS_H
 #define BLE_COMMANDS_H
 
 #include "Arduino.h"
 #include "ExoData.h"        // ✅ Move this UP - needs full definition
 #include "ParseIni.h"       //For config_defs
 #include "StatusDefs.h"     //For ExoDataStatus
 #include "BleMessage.h"
 #include "ParamsFromSD.h"
 
 #include "UARTHandler.h"
 #include "uart_commands.h"  // ✅ This comes AFTER ExoData.h
 #include "UART_msg_t.h"
 #include "Logger.h"
 
 
 /**
  * @brief Type to associate a command with an ammount of data
  * 
  */
 typedef struct
 {
     char command;
     int length; 
 } ble_command_t;
 
 /**
  * @brief Creates a variable for each command value
  * 
  */
 namespace ble_names
 {
     //Recieved Commands (GUI->Firmware)
     static const char start             = 'E';
     static const char stop              = 'G';
     static const char cal_trq           = 'H';
     static const char cal_fsr           = 'L';
     static const char new_trq           = 'F';
     static const char new_fsr           = 'R';
     static const char assist            = 'c';
     static const char resist            = 'S';
     static const char motors_on         = 'x';
     static const char motors_off        = 'w';
     static const char mark              = 'N';
     static const char update_param      = 'f';
     static const char send_param_recieved = 'i';
 
     //Sending Commands (Firmware->GUI)
     static const char send_real_time_data = '?';
     static const char send_batt           = '~';
     static const char send_cal_done       = 'n';
     static const char send_error_count    = 'w';
     static const char send_trq_cal        = 'H';
     static const char send_step_count     = 's';
     static const char cal_fsr_finished    = 'n';
 
 };
 
 /**
  * @brief Associates the command and ammount of data that it expects to be sent/received
  * 
  */
 namespace ble
 {
     static const ble_command_t commands[] = 
     {
         //Recieved Commands
         {ble_names::start,              0},
         {ble_names::stop,               0},
         {ble_names::cal_trq,            0},
         {ble_names::cal_fsr,            0},
         {ble_names::assist,             0},
         {ble_names::resist,             0},
         {ble_names::motors_on,          0},
         {ble_names::motors_off,         0},
         {ble_names::mark,               0},
         {ble_names::new_fsr,            2},
         {ble_names::new_trq,            4},
         {ble_names::update_param,       4},
         {ble_names::send_param_recieved, 0},
         
         //Sending Commands
         {ble_names::send_batt,              1},
         {ble_names::send_real_time_data,    9},
         {ble_names::send_error_count,       1},
         {ble_names::send_cal_done,          0},
         {ble_names::send_trq_cal,           2},
         {ble_names::send_step_count,        2},
         {ble_names::cal_fsr_finished,       0},

     };
 };
 
 /**
  * @brief Helper function(s) to be used with the command array
  * 
  */
 namespace ble_command_helpers
 {
     /**
      * @brief Get the ammount of data a command is expecting
      * 
      * @param command to get the length
      * @return int amount of data for a command, -1 if command not found
      */
     inline static int get_length_for_command(char command)
     {
         int length = -1;
 
         //Get the amount of characters to wait for
         for(unsigned int i=0; i < sizeof(ble::commands)/sizeof(ble::commands[0]); i++)
         {
             if(command == ble::commands[i].command)
             {
                 length = ble::commands[i].length;
                 break;
             }
         }
         return length;
     }
 
 }
 
 /**
  * @brief Variables used by the Handlers to track state
  * 
  */
 namespace ble_handler_vars
 {
     //Should be used sparingly, we chose to do this so that ExoData wasn't needlessly populated with variables
     static const uint8_t k_max_joints = 6;
     static uint8_t prev_controllers[k_max_joints] = {0, 0, 0, 0, 0, 0};
 
 }
 
 /**
  * @brief Holds the functions that should be called when a command is received. All command handlers should have 
  * static linkage, return void, and accept a pointer to ExoData.
  * ie "inline static void my_handler(ExoData* data, BleMessage* msg)"
  * 
  */
 namespace ble_handlers
 {
     inline static void start(ExoData* data, BleMessage* msg)
     {
         //Start the trial (ie Enable motors and begin streaming data). If the joint is used; enable the motor, and set the controller to zero torque
         data->for_each_joint(
             
             // This is a lamda or anonymous function, see https://www.learncpp.com/cpp-tutorial/introduction-to-lambdas-anonymous-functions/
             [](JointData* j_data, float* args)
             {
                 if (j_data->is_used)
                 {
                     j_data->motor.enabled = 1;
                 }
                 return;
             }
         );
 
         //Set the data status to running
         data->set_status(status_defs::messages::trial_on);
 
         //Send status update
         UARTHandler* uart_handler = UARTHandler::get_instance();
         UART_msg_t tx_msg;
         tx_msg.command = UART_command_names::update_status;
         tx_msg.joint_id = 0;
         tx_msg.data[(uint8_t)UART_command_enums::status::STATUS] = data->get_status();
         tx_msg.len = (uint8_t)UART_command_enums::status::LENGTH;
         uart_handler->UART_msg(tx_msg);
 
         delayMicroseconds(10);
 
         //Send motor enable update
         tx_msg.command = UART_command_names::update_motor_enable_disable;
         tx_msg.joint_id = 0;
         tx_msg.data[(uint8_t)UART_command_enums::motor_enable_disable::ENABLE_DISABLE] = 1;
         tx_msg.len = (uint8_t)UART_command_enums::motor_enable_disable::LENGTH;
         uart_handler->UART_msg(tx_msg);
 
         delayMicroseconds(10);
 
         //Send FSR Calibration and Refinement
         tx_msg.command = UART_command_names::update_cal_fsr;
         tx_msg.len = 0;
         uart_handler->UART_msg(tx_msg);
     }
     inline static void stop(ExoData* data, BleMessage* msg)
     {
         //Stop the trial (inverse of start) & send trial summary data (step information)
         data->for_each_joint(
             
             //This is a lamda or anonymous function, see https://www.learncpp.com/cpp-tutorial/introduction-to-lambdas-anonymous-functions/
             [](JointData* j_data, float* args)
             {
                 if (j_data->is_used)
                 {
                     j_data->motor.enabled = 0;
                 }
                 return;
             }
         );
 
         //Set the data status to off
         data->set_status(status_defs::messages::trial_off);
 
         //Send status update
         UARTHandler* uart_handler = UARTHandler::get_instance();
         UART_msg_t tx_msg;
         tx_msg.command = UART_command_names::update_status;
         tx_msg.joint_id = 0;
         tx_msg.data[(uint8_t)UART_command_enums::status::STATUS] = data->get_status();
         tx_msg.len = (uint8_t)UART_command_enums::status::LENGTH;
         uart_handler->UART_msg(tx_msg);
 
         delayMicroseconds(100);
 
         //Send motor enable update
         tx_msg.command = UART_command_names::update_motor_enable_disable;
         tx_msg.joint_id = 0;
         tx_msg.data[(uint8_t)UART_command_enums::motor_enable_disable::ENABLE_DISABLE] = 0;
         tx_msg.len = (uint8_t)UART_command_enums::motor_enable_disable::LENGTH;
         uart_handler->UART_msg(tx_msg);
 
         data->mark = 10;
     }
     inline static void cal_trq(ExoData* data, BleMessage* msg)
     {   
         //Raise cal_trq flag for all joints being used, (Out of context: Should send calibration info upon cal completion)
         data->for_each_joint([](JointData* j_data, float* args) {j_data->calibrate_torque_sensor = j_data->is_used;});
 
         //Send cal_trq
         UARTHandler* uart_handler = UARTHandler::get_instance();
         UART_msg_t tx_msg;
         tx_msg.command = UART_command_names::update_cal_trq_sensor;
         tx_msg.joint_id = 0;
         tx_msg.data[(uint8_t)UART_command_enums::cal_trq_sensor::CAL_TRQ_SENSOR] = 1;
         tx_msg.len = (uint8_t)UART_command_enums::cal_trq_sensor::LENGTH;
         uart_handler->UART_msg(tx_msg);
     }
     inline static void cal_fsr(ExoData* data, BleMessage* msg)
     {
         UARTHandler* uart_handler = UARTHandler::get_instance();
         UART_msg_t tx_msg;
         tx_msg.command = UART_command_names::update_cal_fsr;
         tx_msg.joint_id = 0;
         tx_msg.data[(uint8_t)UART_command_enums::cal_fsr::CAL_FSR] = 1;
         tx_msg.len = (uint8_t)UART_command_enums::cal_fsr::LENGTH;
         uart_handler->UART_msg(tx_msg);
         tx_msg.command = UART_command_names::update_refine_fsr;
         tx_msg.len = 0;
         uart_handler->UART_msg(tx_msg);
     }
     inline static void assist(ExoData* data, BleMessage* msg)
     {
         //Right now we are approaching defining assistance and resistance directly in the controllers via a controller specific parameter, future work may populate these functions instead
     }
     inline static void resist(ExoData* data, BleMessage* msg)
     {
         //Right now we are approaching defining assistance and resistance directly in the controllers via a controller specific parameter, future work may populate these functions instead
     }
     inline static void motors_on(ExoData* data, BleMessage* msg)
     {        
         data->for_each_joint(
             
             //This is a lamda or anonymous function, see https://www.learncpp.com/cpp-tutorial/introduction-to-lambdas-anonymous-functions/
             [](JointData* j_data, float* args)
             {
                 if (j_data->is_used)
                 {
                     j_data->motor.enabled = 1;
                 }
                 return;
             }
         );
 
         UARTHandler* uart_handler = UARTHandler::get_instance();
         UART_msg_t tx_msg;
         tx_msg.command = UART_command_names::update_motor_enable_disable;
         tx_msg.joint_id = 0;
         tx_msg.data[(uint8_t)UART_command_enums::motor_enable_disable::ENABLE_DISABLE] = 1;
         tx_msg.len = (uint8_t)UART_command_enums::motor_enable_disable::LENGTH;
         uart_handler->UART_msg(tx_msg);
         
     }
     inline static void motors_off(ExoData* data, BleMessage* msg)
     {   
         data->for_each_joint(
             
             //This is a lamda or anonymous function, see https://www.learncpp.com/cpp-tutorial/introduction-to-lambdas-anonymous-functions/
             [](JointData* j_data, float* args)
             {
                 if (j_data->is_used)
                 {
                     j_data->motor.enabled = 0;
                 }
                 return;
             }
         );
 
         UARTHandler* uart_handler = UARTHandler::get_instance();
         UART_msg_t tx_msg;
         tx_msg.command = UART_command_names::update_motor_enable_disable;
         tx_msg.joint_id = 0;
         tx_msg.data[(uint8_t)UART_command_enums::motor_enable_disable::ENABLE_DISABLE] = 0;
         tx_msg.len = (uint8_t)UART_command_enums::motor_enable_disable::LENGTH;
         uart_handler->UART_msg(tx_msg);
 
     }
     inline static void mark(ExoData* data, BleMessage* msg)
     {
         //Increment mark variable (Done by sending different data on one of the real time signals, we should raise a flag or inc a var in exo_data)
         data->mark++;
     }
     inline static void new_trq(ExoData* data, BleMessage* msg)
     {
         //(LSP, LDSP, RSP, RDSP) Unpack message data
         config_defs::joint_id joint_id = (config_defs::joint_id)msg->data[0];
         uint8_t controller_id = (uint8_t)msg->data[1];
         uint8_t set_num = (uint8_t)msg->data[2];
         
         //Update Exo_Data controller for each joint
         ControllerData* cont_data = NULL;
 
         //Map the joint IDs because the GUI limits the maximum number for the message
         joint_id = (joint_id==(config_defs::joint_id)1)?(data->left_side.hip.id):(joint_id);
         joint_id = (joint_id==(config_defs::joint_id)2)?(data->left_side.knee.id):(joint_id);
         joint_id = (joint_id==(config_defs::joint_id)3)?(data->left_side.ankle.id):(joint_id);
         joint_id = (joint_id==(config_defs::joint_id)4)?(data->left_side.elbow.id):(joint_id);
         joint_id = (joint_id==(config_defs::joint_id)5)?(data->right_side.hip.id):(joint_id);
         joint_id = (joint_id==(config_defs::joint_id)6)?(data->right_side.knee.id):(joint_id);
         joint_id = (joint_id==(config_defs::joint_id)7)?(data->right_side.ankle.id):(joint_id);
         joint_id = (joint_id==(config_defs::joint_id)8)?(data->right_side.elbow.id):(joint_id);
 
         if (joint_id == data->left_side.ankle.id)
         {
             cont_data = &data->left_side.ankle.controller;
         } 
         else if (joint_id == data->left_side.knee.id) 
         {
             cont_data = &data->left_side.knee.controller;
         } 
         else if (joint_id == data->left_side.hip.id)
         {
             cont_data = &data->left_side.hip.controller;
         } 
         else if (joint_id == data->left_side.elbow.id)
         {
             cont_data = &data->left_side.elbow.controller;
         }
         else if (joint_id == data->right_side.ankle.id) 
         {
             cont_data = &data->right_side.ankle.controller;
         } 
         else if (joint_id == data->right_side.knee.id) 
         {
             cont_data = &data->right_side.knee.controller;
         } 
         else if (joint_id == data->right_side.hip.id) 
         {
             cont_data = &data->right_side.hip.controller;
         }
         else if (joint_id == data->right_side.elbow.id)
         {
             cont_data = &data->right_side.elbow.controller;
         }
         if (cont_data == NULL) {
             logger::println("cont_data is NULL!", LogLevel::Warn);
         }
         if (cont_data != NULL) {
             cont_data->controller = controller_id;
             cont_data->parameter_set = set_num;
         }
 
         //Set_controller_params((uint8_t)joint_id, controller_id, set_num, data);
         UARTHandler* uart_handler = UARTHandler::get_instance();
         UART_msg_t tx_msg;
         tx_msg.command = UART_command_names::update_controller_params;
         tx_msg.joint_id = (uint8_t) joint_id;
         tx_msg.data[(uint8_t)UART_command_enums::controller_params::CONTROLLER_ID] = controller_id;
         tx_msg.data[(uint8_t)UART_command_enums::controller_params::PARAM_LENGTH] = 1;
         tx_msg.data[(uint8_t)UART_command_enums::controller_params::PARAM_START] = set_num;
         tx_msg.len = 3;
         uart_handler->UART_msg(tx_msg);
 
         UART_msg_t_utils::print_msg(tx_msg);
     }
     inline static void new_fsr(ExoData* data, BleMessage* msg)
     {
         //Change contact thresholds & Send UART message to update FSR thresholds
         UARTHandler* uart_handler = UARTHandler::get_instance();
         UART_msg_t tx_msg;
         tx_msg.command = UART_command_names::update_FSR_thesholds;
         tx_msg.joint_id = 0;
         tx_msg.len = (uint8_t)UART_command_enums::FSR_thresholds::LENGTH;
         tx_msg.data[(uint8_t)UART_command_enums::FSR_thresholds::LEFT_THRESHOLD] = msg->data[0];
         tx_msg.data[(uint8_t)UART_command_enums::FSR_thresholds::RIGHT_THRESHOLD] = msg->data[1];
         uart_handler->UART_msg(tx_msg);
     }
 
     inline static void update_param(ExoData* data, BleMessage* msg)
     {
         //Send UART message to update parameter
         logger::println("ble_handlers::update_param() - Got update param message");
         logger::print("ble_handlers::update_param() - Joint ID: "); logger::println((uint8_t)msg->data[0]);
         logger::print("ble_handlers::update_param() - Controller ID: "); logger::println((uint8_t)msg->data[1]);
         logger::print("ble_handlers::update_param() - Param Index: "); logger::println((uint8_t)msg->data[2]);
         logger::print("ble_handlers::update_param() - Param Value: "); logger::println((uint8_t)msg->data[3]);
         logger::print("New message\n");
 
         UARTHandler* uart_handler = UARTHandler::get_instance();
         UART_msg_t tx_msg;
         tx_msg.command = UART_command_names::update_controller_param;
         tx_msg.joint_id = (uint8_t) msg->data[0];
         tx_msg.data[(uint8_t)UART_command_enums::controller_param::CONTROLLER_ID] = (uint8_t) msg->data[1];
         tx_msg.data[(uint8_t)UART_command_enums::controller_param::PARAM_INDEX] = (uint8_t) msg->data[2];
         tx_msg.data[(uint8_t)UART_command_enums::controller_param::PARAM_VALUE] = (uint8_t) msg->data[3];
         tx_msg.len = 3;
         uart_handler->UART_msg(tx_msg);
     }
     inline static void send_param_recieved(ExoData* data)
     {
        data->parameter_names_received = true;
     }
 }
 
 #endif