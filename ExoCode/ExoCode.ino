   /*
   Code used to run the exo from the teensy.  This communicates with the nano over UART.
   
   P. Stegall Jan 2022
*/  

//Teensy Operation
#if defined(ARDUINO_TEENSY36) | defined(ARDUINO_TEENSY41)

//UNCOMMENT TO UTILIZE
// #define INCLUDE_FLEXCAN_DEBUG   //Flag to print CAN debugging messages for the motors
#define MAKE_PLOTS              //Flag to serial plot
#define MAIN_DEBUG              //Flag to print Arduino debugging statements
// #define HEADLESS                //Flag to be used when there is no app access

//Standard Libraries
#include <stdint.h>
#include <IntervalTimer.h>

//Common Libraries
#include "src/Board.h"
#include "src/ExoData.h"
#include "src/Exo.h"                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
#include "src/Utilities.h"
#include "src/StatusDefs.h"

//Specific Libraries
#include "src/ParseIni.h"
#include "src/ParamsFromSD.h"

//Board to board coms
#include "src/UARTHandler.h"
#include "src/uart_commands.h"
#include "src/UART_msg_t.h"

//Logging
#include "src/Logger.h"
#include "src/PiLogger.h"

//Array used to store config information
namespace config_info
{
    uint8_t (config_to_send)[ini_config::number_of_keys];
}

void setup()
{
    analogReadResolution(12);
    
    Serial.begin(115200);
    delay(100);

    //Get the config information from the SD card (calls function in ParseIni).
    ini_parser(config_info::config_to_send);              
    
    //Print to confirm config came through correctly (Should not contain zeros).
    #ifdef MAIN_DEBUG
        for (int i = 0; i < ini_config::number_of_keys; i++)
        {
          logger::print("[" + String(i) + "] : " + String((int)config_info::config_to_send[i]) + "\n");
        }
        logger::print("\n");
    #endif
    
    //Labels for the signals if plotting.
    #ifdef MAKE_PLOTS
          logger::print("Left_hip_trq_cmd, ");
          logger::print("Left_hip_current, ");
          logger::print("Right_hip_trq_cmd, ");
          logger::print("Right_hip_current, ");
          logger::print("Left_ankle_trq_cmd, ");
          logger::print("Left_ankle_current, ");
          logger::print("Right_ankle_trq_cmd, ");
          logger::print("Right_ankle_current, ");
          logger::print("Left_ankle_torque_measure, ");
          logger::print("\n");
      #endif
}

void loop()
{       
    static bool first_run = true;
    
    //Create the data object
    static ExoData exo_data(config_info::config_to_send);     

    //Print to make sure object was created
    #ifdef MAIN_DEBUG
        if (first_run)
        {
            logger::print("Superloop :: exo_data created"); 
        }
    #endif

    //Create the exo object
    static Exo exo(&exo_data);                                

    //Print to make sure object was created
    #ifdef MAIN_DEBUG
        if (first_run)
        {
            logger::print("Superloop :: exo created");
        }
    #endif

    //Creates instance of UART Handler
    static UARTHandler* uart_handler = UARTHandler::get_instance();
    
    if (first_run)
    {
        first_run = false;

        //Waits for the message telling it to get the config information 
        UART_command_utils::wait_for_get_config(uart_handler, &exo_data, UART_times::CONFIG_TIMEOUT);

        //Print detailing which joint and side is used
        #ifdef MAIN_DEBUG
            logger::print("Superloop :: Start First Run Conditional\n");
            logger::print("Superloop :: exo_data.left_side.hip.is_used = ");
            logger::print(exo_data.left_side.hip.is_used);
            logger::print("\n");
            logger::print("Superloop :: exo_data.right_side.hip.is_used = ");
            logger::print(exo_data.right_side.hip.is_used);
            logger::print("\n");
            logger::print("Superloop :: exo_data.left_side.knee.is_used = ");
            logger::print(exo_data.left_side.knee.is_used);
            logger::print("\n");
            logger::print("Superloop :: exo_data.right_side.knee.is_used = ");
            logger::print(exo_data.right_side.knee.is_used);
            logger::print("\n");
            logger::print("Superloop :: exo_data.left_side.ankle.is_used = ");
            logger::print(exo_data.left_side.ankle.is_used);
            logger::print("\n");
            logger::print("Superloop :: exo_data.right_side.ankle.is_used = ");
            logger::print(exo_data.right_side.ankle.is_used);
            logger::print("\n");
            logger::print("Superloop :: exo_data.left_side.elbow.is_used = ");
            logger::print(exo_data.left_side.elbow.is_used);
            logger::print("\n");
            logger::print("Superloop :: exo_data.right_side.elbow.is_used = ");
            logger::print(exo_data.right_side.elbow.is_used);
            logger::print("\n");
            logger::print("\n");
        #endif
        
        //Only call functions related to used motors
        if (exo_data.left_side.hip.is_used)
        {
            //Turn motor on
            exo_data.left_side.hip.motor.is_on = true;
            
            //Make sure motor gains are set to 0 so there is no funny business
            exo_data.left_side.hip.motor.kp = 0;
            exo_data.left_side.hip.motor.kd = 0;

            //Handles desired operations if in headless mode
            #ifdef HEADLESS

                //Set the controller parameters to thier default
                set_controller_params((uint8_t) exo_data.left_side.hip.id, config_info::config_to_send[config_defs::exo_hip_default_controller_idx], 0, &exo_data); //This function is found in ParamsFromSD
                
                #ifdef MAIN_DEBUG
                  logger::print("Superloop :: Left Hip Parameters Set");
                #endif
                
                //Waits until calibration is done to set actual controller
                exo_data.left_side.hip.controller.controller = (uint8_t)config_defs::hip_controllers::zero_torque;  //Start in zero torque
                exo.left_side._hip.set_controller(exo_data.left_side.hip.controller.controller);                    //Then sets to desired controller                  
                
            #endif
        }
        
        if (exo_data.right_side.hip.is_used)
        {
            //Turn motor on 
            exo_data.right_side.hip.motor.is_on = true;

            //Make sure motor gains are set to 0 so there is no funny business
            exo_data.right_side.hip.motor.kp = 0;
            exo_data.right_side.hip.motor.kd = 0;

            //Handles desired operations if in headless mode
            #ifdef HEADLESS
=
                //Set the controller parameters to thier default
                set_controller_params((uint8_t) exo_data.right_side.hip.id, config_info::config_to_send[config_defs::exo_hip_default_controller_idx], 0, &exo_data);
                
                #ifdef MAIN_DEBUG
                  logger::print("Superloop :: Right Hip Parameters Set");
                #endif
                
                //Waits until calibration is done to set actual controller
                exo_data.right_side.hip.controller.controller = (uint8_t)config_defs::hip_controllers::zero_torque;   //Start in zero torque
                exo.right_side._hip.set_controller(exo_data.right_side.hip.controller.controller);                    //Then sets to desired controller
            
            #endif
        }

        if (exo_data.left_side.knee.is_used)
        {
            //Turn motor on
            exo_data.left_side.knee.motor.is_on = true;
            
            //Make sure motor gains are set to 0 so there is no funny business
            exo_data.left_side.knee.motor.kp = 0;
            exo_data.left_side.knee.motor.kd = 0;

            //Handles desired operations if in headless mode
            #ifdef HEADLESS

                //Set the controller parameters to thier default
                set_controller_params((uint8_t) exo_data.left_side.knee.id, config_info::config_to_send[config_defs::exo_knee_default_controller_idx], 0, &exo_data); //This function is found in ParamsFromSD
                
                #ifdef MAIN_DEBUG
                  logger::print("Superloop :: Left Knee Parameters Set");
                #endif
                
                //Waits until calibration is done to set actual controller
                exo_data.left_side.knee.controller.controller = (uint8_t)config_defs::knee_controllers::zero_torque; //Start in zero torque
                exo.left_side._knee.set_controller(exo_data.left_side.knee.controller.controller);                    //Then sets to desired controller                  
                
            #endif
        }
        
        if (exo_data.right_side.knee.is_used)
        {
            //Turn motor on 
            exo_data.right_side.knee.motor.is_on = true;

            //Make sure motor gains are set to 0 so there is no funny business
            exo_data.right_side.knee.motor.kp = 0;
            exo_data.right_side.knee.motor.kd = 0;

            //Handles desired operations if in headless mode
            #ifdef HEADLESS
=
                //Set the controller parameters to thier default
                set_controller_params((uint8_t) exo_data.right_side.knee.id, config_info::config_to_send[config_defs::exo_knee_default_controller_idx], 0, &exo_data);
                
                #ifdef MAIN_DEBUG
                  logger::print("Superloop :: Right Knee Parameters Set");
                #endif
                
                //Waits until calibration is done to set actual controller
                exo_data.right_side.knee.controller.controller = (uint8_t)config_defs::knee_controllers::zero_torque;   //Start in zero torque
                exo.right_side._knee.set_controller(exo_data.right_side.knee.controller.controller);                    //Then sets to desired controller
            
            #endif
        }

        if (exo_data.left_side.ankle.is_used)
        {
            #ifdef MAIN_DEBUG
              logger::print("Superloop :: Left Ankle Used");
            #endif

            //Turn motor on
            exo_data.left_side.ankle.motor.is_on = true;

            //Make sure motor gains are set to 0 so there is no funny business
            exo_data.left_side.ankle.motor.kp = 0;
            exo_data.left_side.ankle.motor.kd = 0;

            //Handles desired operations if in headless mode
            #ifdef HEADLESS

                //Set the controller parameters to thier default
                set_controller_params((uint8_t) exo_data.left_side.ankle.id, config_info::config_to_send[config_defs::exo_ankle_default_controller_idx], 0, &exo_data);
                
                #ifdef MAIN_DEBUG
                  logger::print("Superloop :: Left Ankle Parameters Set");
                #endif
                
                //Waits until calibration is done to set actual controller
                exo_data.left_side.ankle.controller.controller = (uint8_t)config_defs::ankle_controllers::zero_torque;   //Start in zero torque
                exo.left_side._ankle.set_controller(exo_data.left_side.ankle.controller.controller);                      //Then sets to desired controller
                
            #endif
        }
        
        if (exo_data.right_side.ankle.is_used)
        {
            //Turn motor on
            exo_data.right_side.ankle.motor.is_on = true;

            //Make sure motor gains are set to 0 so there is no funny business
            exo_data.right_side.ankle.motor.kp = 0;
            exo_data.right_side.ankle.motor.kd = 0;

            //Handles desired operations if in headless mode
            #ifdef HEADLESS

                //Set the controller parameters to thier default
                set_controller_params((uint8_t) exo_data.right_side.ankle.id, config_info::config_to_send[config_defs::exo_ankle_default_controller_idx], 0, &exo_data);
                
                #ifdef MAIN_DEBUG
                  logger::print("Superloop :: Right Ankle Parameters Set");
                #endif
                
                //Waits until calibration is done to set actual controller
                exo_data.right_side.ankle.controller.controller = (uint8_t)config_defs::ankle_controllers::zero_torque;   //Start in zero torque
                exo.right_side._ankle.set_controller(exo_data.right_side.ankle.controller.controller);                    //Then sets to desired controller
                
            #endif
        }

        if (exo_data.left_side.elbow.is_used)
        {
            #ifdef MAIN_DEBUG
              logger::print("Superloop :: Left Elbow Used");
            #endif

            //Turn motor on
            exo_data.left_side.elbow.motor.is_on = true;

            //Make sure motor gains are set to 0 so there is no funny business
            exo_data.left_side.elbow.motor.kp = 0;
            exo_data.left_side.elbow.motor.kd = 0;

            //Handles desired operations if in headless mode
            #ifdef HEADLESS

                //Set the controller parameters to thier default
                set_controller_params((uint8_t) exo_data.left_side.elbow.id, config_info::config_to_send[config_defs::exo_elbow_default_controller_idx], 0, &exo_data);
                
                #ifdef MAIN_DEBUG
                  logger::print("Superloop :: Left Elbow Parameters Set");
                #endif
                
                //Waits until calibration is done to set actual controller
                exo_data.left_side.elbow.controller.controller = (uint8_t)config_defs::elbow_controllers::zero_torque;    //Start in zero torque
                exo.left_side._elbow.set_controller(exo_data.left_side.elbow.controller.controller);                      //Then sets to desired controller
                
            #endif
        }
        
        if (exo_data.right_side.elbow.is_used)
        {
            //Turn motor on
            exo_data.right_side.elbow.motor.is_on = true;

            //Make sure motor gains are set to 0 so there is no funny business
            exo_data.right_side.elbow.motor.kp = 0;
            exo_data.right_side.elbow.motor.kd = 0;

            //Handles desired operations if in headless mode
            #ifdef HEADLESS

                //Set the controller parameters to thier default
                set_controller_params((uint8_t) exo_data.right_side.elbow.id, config_info::config_to_send[config_defs::exo_elbow_default_controller_idx], 0, &exo_data);
                
                #ifdef MAIN_DEBUG
                  logger::print("Superloop :: Right Elbow Parameters Set");
                #endif
                
                //Waits until calibration is done to set actual controller
                exo_data.right_side.elbow.controller.controller = (uint8_t)config_defs::elbow_controllers::zero_torque;   //Start in zero torque
                exo.right_side._elbow.set_controller(exo_data.right_side.elbow.controller.controller);                    //Then sets to desired controller
                
            #endif
        }
        
        //Give the motors time to wake up
        #ifdef MAIN_DEBUG
          logger::print("Superloop :: Motor Charging Delay - Please be patient");
        #endif 

        //Set the status to Motor Startup
        exo_data.set_status(status_defs::messages::motor_start_up); 

        //Define the Parameters involved with motor startup delay
        unsigned int motor_start_delay_ms = 10;                     //Delay duration, previously set to 60000, if you are having issues with startup try using this time instead 
        unsigned int motor_start_time = millis();                   
        unsigned int dot_print_ms = 1000;                           
        unsigned int last_dot_time = millis();                     

        //Loop that gives motors time to wake up
        while (millis() - motor_start_time < motor_start_delay_ms)
        {
            //Updates LED status to let you know it is in its delay
            exo.status_led.update(exo_data.get_status());

            #ifdef MAIN_DEBUG
              if(millis() - last_dot_time > dot_print_ms)
              {
                last_dot_time = millis();
                logger::print(".");
              } 
            #endif
        }
        
        #ifdef MAIN_DEBUG
          logger::println();  //Just gives some spacing to Serial Monitor while de-bugging
        #endif

        //Configure the system if you can't set it with the app
        #ifdef HEADLESS
            bool enable_overide = true;
            
            //Calibrates torque sensor and enables motor for each used joint
            if(exo_data.left_side.hip.is_used)
            { 
              exo_data.left_side.hip.calibrate_torque_sensor = true;
              exo_data.left_side.hip.motor.enabled = true;
            }
           
            if(exo_data.right_side.hip.is_used)
            {
              exo_data.right_side.hip.calibrate_torque_sensor = true;
              exo_data.right_side.hip.motor.enabled = true; 
            }

            if(exo_data.left_side.knee.is_used)
            { 
              exo_data.left_side.knee.calibrate_torque_sensor = true;
              exo_data.left_side.knee.motor.enabled = true;
            }
           
            if(exo_data.right_side.knee.is_used)
            {
              exo_data.right_side.knee.calibrate_torque_sensor = true;
              exo_data.right_side.knee.motor.enabled = true; 
            }
            
            if(exo_data.left_side.ankle.is_used)
            {
                exo_data.left_side.ankle.calibrate_torque_sensor = true; 
                exo_data.left_side.ankle.motor.enabled = true;
            }
           
            if(exo_data.right_side.ankle.is_used)
            {
                exo_data.right_side.ankle.calibrate_torque_sensor = true;  
                exo_data.right_side.ankle.motor.enabled = true;
            }

            if(exo_data.left_side.elbow.is_used)
            {
                exo_data.left_side.elbow.calibrate_torque_sensor = true; 
                exo_data.left_side.elbow.motor.enabled = true;
            }
           
            if(exo_data.right_side.elbow.is_used)
            {
                exo_data.right_side.elbow.calibrate_torque_sensor = true;  
                exo_data.right_side.elbow.motor.enabled = true;
            }
        #endif

        //Print to tell you if motors are enabled, the parameters are set, and if the functions for the first run are complete
        #ifdef MAIN_DEBUG
            #ifdef HEADLESS
                logger::print("Superloop :: Motors Enabled");
                logger::print("Superloop :: Parameters Set");
            #endif
            logger::print("Superloop :: End First Run Conditional");
        #endif
    }

    //Run the calibrations we need to do if not using the app
    #ifdef HEADLESS
        
        static bool static_calibration_done = false;
        unsigned int pause_after_static_calibration_ms = 10000;
        static unsigned int time_dynamic_calibration_finished; 
        static bool pause_between_calibration_done = false;   
        static bool dynamic_calibration_done = false;
        
        //Data Plotting 
        static float old_time = micros();
        float new_time = micros();
        if(new_time - old_time > 10000 && dynamic_calibration_done)
        {
            //Uncomment which plots you would want in Serial Monitor, can always change what is plotting too
            #ifdef MAKE_PLOTS
                //logger::print(exo_data.left_side.hip.motor.t_ff);
                //logger::print(", ");
                //logger::print(exo_data.left_side.hip.motor.i);
                //logger::print(", ");
                //logger::print(exo_data.right_side.hip.motor.t_ff);
                //logger::print(", ");
                //logger::print(exo_data.right_side.hip.motor.i);
                //logger::print(", ");
                //logger::print(exo_data.left_side.ankle.motor.t_ff);
                //logger::print(", ");
                //logger::print(exo_data.right_side.hip.motor.i);
                //logger::print(", ");
                //logger::print(exo_data.right_side.ankle.motor.t_ff);
                //logger::print(", ");
                //logger::print(exo_data.right_side.ankle.motor.i);
                //logger::print(", ");
                //logger::print(exo_data.right_side.hip.torque_reading);
                //logger::print("\n");
            #endif

            old_time = new_time;
            
        }
        
        //Calibrate the Torque Sensors
        if ((!static_calibration_done) && (!exo_data.left_side.ankle.calibrate_torque_sensor && !exo_data.right_side.ankle.calibrate_torque_sensor))
        {
            #ifdef MAIN_DEBUG
              logger::print("Superloop : Static Calibration Done");
            #endif
            
            static_calibration_done  = true;
            time_dynamic_calibration_finished = millis();
            exo_data.set_status(status_defs::messages::test);
        }
    
        //Pause between static (torque sensor, standing still) and dynamic (FSRs, during walking) calibration so we have time to start walking
        if (!pause_between_calibration_done && (static_calibration_done && ((time_dynamic_calibration_finished +  pause_after_static_calibration_ms) < millis() ))) 
        {
            #ifdef MAIN_DEBUG
              logger::print("Superloop : Pause Between Calibration Finished");
            #endif
            
            if(exo_data.left_side.is_used)
            {
                exo_data.left_side.do_calibration_toe_fsr = true;              
                exo_data.left_side.do_calibration_refinement_toe_fsr = true;   
                exo_data.left_side.do_calibration_heel_fsr = true;             
                exo_data.left_side.do_calibration_refinement_heel_fsr = true;  
            }
           
            if(exo_data.right_side.is_used)
            {
                exo_data.right_side.do_calibration_toe_fsr = true;
                exo_data.right_side.do_calibration_refinement_toe_fsr = true;
                exo_data.right_side.do_calibration_heel_fsr = true;
                exo_data.right_side.do_calibration_refinement_heel_fsr = true;
            }
            
            pause_between_calibration_done = true;
        }
            
        //When we are done with the dynamic calibrations, set the controllers
        if ((!dynamic_calibration_done) && (pause_between_calibration_done) && (!exo_data.left_side.do_calibration_toe_fsr && !exo_data.left_side.do_calibration_refinement_toe_fsr && !exo_data.left_side.do_calibration_heel_fsr && !exo_data.left_side.do_calibration_refinement_heel_fsr))
        {
            #ifdef MAIN_DEBUG
                logger::print("Superloop : Dynamic Calibration Done");
            #endif
            
            if (exo_data.left_side.hip.is_used)
            {
                //Set the default controller
                exo_data.left_side.hip.controller.controller = config_info::config_to_send[config_defs::exo_hip_default_controller_idx];
                exo.left_side._hip.set_controller(exo_data.left_side.hip.controller.controller);
                
                #ifdef MAIN_DEBUG
                    logger::print("Superloop : Left Hip Controller Set");
                #endif
            }
            
            if (exo_data.right_side.hip.is_used)
            {
                //Set the default controller
                exo_data.right_side.hip.controller.controller = config_info::config_to_send[config_defs::exo_hip_default_controller_idx];
                exo.right_side._hip.set_controller(exo_data.right_side.hip.controller.controller); 
                
                #ifdef MAIN_DEBUG
                    logger::print("Superloop : Right Hip Controller Set");
                #endif
            }

            if (exo_data.left_side.knee.is_used)
            {
                //Set the default controller
                exo_data.left_side.knee.controller.controller = config_info::config_to_send[config_defs::exo_knee_default_controller_idx];
                exo.left_side._knee.set_controller(exo_data.left_side.knee.controller.controller);
                
                #ifdef MAIN_DEBUG
                    logger::print("Superloop : Left Knee Controller Set");
                #endif
            }
            
            if (exo_data.right_side.knee.is_used)
            {
                //Set the default controller
                exo_data.right_side.knee.controller.controller = config_info::config_to_send[config_defs::exo_knee_default_controller_idx];
                exo.right_side._knee.set_controller(exo_data.right_side.knee.controller.controller); 
                
                #ifdef MAIN_DEBUG
                    logger::print("Superloop : Right Knee Controller Set");
                #endif
            }
            
            if (exo_data.left_side.ankle.is_used)
            {
                //Set the default controller
                exo_data.left_side.ankle.controller.controller = config_info::config_to_send[config_defs::exo_ankle_default_controller_idx];
                exo.left_side._ankle.set_controller(exo_data.left_side.ankle.controller.controller);
                
                #ifdef MAIN_DEBUG
                    logger::print("Superloop : Left Ankle Controller Set");
                #endif
            }
      
            if (exo_data.right_side.ankle.is_used)
            {
                //Set the default controller
                exo_data.right_side.ankle.controller.controller = config_info::config_to_send[config_defs::exo_ankle_default_controller_idx];
                exo.right_side._ankle.set_controller(exo_data.right_side.ankle.controller.controller);
                
                #ifdef MAIN_DEBUG
                    logger::print("Superloop : Right Ankle Controller Set");
                #endif
            }

            if (exo_data.left_side.elbow.is_used)
            {
                //Set the default controller
                exo_data.left_side.elbow.controller.controller = config_info::config_to_send[config_defs::exo_elbow_default_controller_idx];
                exo.left_side._elbow.set_controller(exo_data.left_side.elbow.controller.controller);
                
                #ifdef MAIN_DEBUG
                    logger::print("Superloop : Left Elbow Controller Set");
                #endif
            }
      
            if (exo_data.right_side.elbow.is_used)
            {
                //Set the default controller
                exo_data.right_side.elbow.controller.controller = config_info::config_to_send[config_defs::exo_elbow_default_controller_idx];
                exo.right_side._elbow.set_controller(exo_data.right_side.elbow.controller.controller);
                
                #ifdef MAIN_DEBUG
                    logger::print("Superloop : Right Elbow Controller Set");
                #endif
            }
            
            dynamic_calibration_done = true;
          
        }
    #endif                                                                                        

    //Run the exo calculations (go to exo.h/exo.cpp to follow the cascade of functions this runs)
    bool ran = exo.run();     
    
    //Print some dots so we know it is doing something if we are trying to debug
    #ifdef MAIN_DEBUG
        unsigned int dot_print_ms = 5000;
        static unsigned int last_dot_time = millis();
        if(millis() - last_dot_time > dot_print_ms)
        {
          last_dot_time = millis();
          logger::print(".");
        }
    #endif 
}

//Nano Operation
#elif defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)  //Board name is ARDUINO_[build.board] property in the board.txt file found at C:\Users\[USERNAME]\AppData\Local\Arduino15\packages\arduino\hardware\mbed_nano\2.6.1  They just already prepended it with ARDUINO so you have to do it twice.

#include <stdint.h>
#include "src/ParseIni.h"
#include "src/ExoData.h"
#include "src/ComsMCU.h"
#include "src/Config.h"
#include "src/Utilities.h"

//Board to board coms
#include "src/UARTHandler.h"
#include "src/uart_commands.h"
#include "src/UART_msg_t.h"
#include "src/ComsLed.h"
#include "src/RealTimeI2C.h"

#include "src/WaistBarometer.h"
#include "src/InclineDetector.h"

#define MAIN_DEBUG 1

//Create an array to store config
namespace config_info
{
     uint8_t config_to_send[ini_config::number_of_keys] = {
            1,  //Board name
            3,  //Board version
            2,  //Battery
            22,  //Exo name
            3,  //Exo side
            5,  //Hip
            5,  //Knee
            5,  //Ankle
            4,  //Hip gear
            4,  //Knee gear
            4,  //Ankle gear
            6,  //Hip default controller
            6,  //Knee default controller
            10, //Ankle default controller
            6,  //Elbow default controller
            2,  //Hip use torque sensor
            2,  //Knee use torque sensor
            2,  //Ankle use torque sensor
            2,  //Elbow use torque sensor
            4,  //Hip flip motor dir
            4,  //Knee flip motor dir
            4,  //Ankle flip motor dir
            4,  //Elbow flip motor dir
            4,  //Hip flip torque dir
            4,  //Knee flip torque dir
            4,  //Ankle flip torque dir
            4,  //Elbow flip torque dir
            4,  //Hip flip angle dir
            4,  //Knee flip angle dir
            4,  //Ankle flip angle dir
            4,  //Elbow flip angle dir
          };
}

void setup()
{
    Serial.begin(115200);

    delay(100);
    randomSeed(analogRead(A0));   // floating pin gives nicely unpredictable seed

    #if MAIN_DEBUG
      while (!Serial);
        logger::print("Setup->Getting config");
    #endif
    
    //Get the SD card config from the teensy, this has a timeout
    UARTHandler* handler = UARTHandler::get_instance();
    const bool timed_out = UART_command_utils::get_config(handler, config_info::config_to_send, (float)UART_times::CONFIG_TIMEOUT);

    //Creates new instance of LED on communication board (Nano)
    ComsLed* led = ComsLed::get_instance();

    //If there is a time out, set the LED to Yellow, otherwise turn the LED green
    if (timed_out)
    {
        #if MAIN_DEBUG
        logger::print("Setup->Timed Out Getting Config", LogLevel::Warn);
        #endif

        //Yellow
        led->set_color(255, 255, 0);
    }
    else
    {
        //Green
        led->set_color(0, 255, 0);
    }

    #if REAL_TIME_I2C
      logger::print("Init I2C");  
      real_time_i2c::init();
      logger::print("Setup->End Setup");
    #endif
}

void loop()
{
    #if MAIN_DEBUG
        
        static bool first_run = true;
        
        if (first_run)
        {
          logger::println("Start Loop");
        }
        
    #endif

    //Constructs a new ExoData object with configuration
    static ExoData* exo_data = new ExoData(config_info::config_to_send);
    
    #if MAIN_DEBUG
        if (first_run)
        {
          logger::println("Construced exo_data");
        }
    #endif

    //Constructs a new ComsMCU object with the exo data and the configuration information
    static ComsMCU* mcu = new ComsMCU(exo_data, config_info::config_to_send);
    
    #if MAIN_DEBUG
        if (first_run)
        {
          logger::println("Construced mcu");
        }
    #endif

    //Performs key communication protocols
    mcu->handle_ble();
    mcu->local_sample();
    mcu->update_UART();
    mcu->update_gui();
    mcu->handle_errors();

    #if MAIN_DEBUG
        static float then = millis();
        float now = millis();
        if ((now - then) > 1000)
        {
            then = now;
            logger::println("...");
        }
        first_run = false;
    #endif
    
}

#else //Code that operates when the microcontroller is not recognized

#include "src/Utilities.h"

void setup()
{
  Serial.begin(115200);
  utils::spin_on_error_with("Unknown Microcontroller");
}

void loop()
{

}

#endif
