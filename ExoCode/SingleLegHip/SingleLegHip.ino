/*
 * Single-leg hip exoskeleton firmware.
 *
 * This is a standalone sketch for a ONE-SIDED (single-leg) hip exoskeleton.
 * It uses the shared src/ folder from ExoCode (copy it here if you build this
 * sketch, same as the systemCheck sketches).
 *
 * It builds on the OpenExo architecture (P. Stegall, Jan. 2022) with one key
 * addition: the gait phase is detected with an EVENT-DRIVEN state machine
 * (GaitPhaseDetector) that watches the FSRs on BOTH feet, instead of
 * extrapolating the phase forward from an expected step duration.
 *
 * Hardware: Teensy 4.1, one hip motor (CAN), four FSRs (heel+toe on both
 * feet), one hip torque sensor.
 *
 * Set the side in the config below (left = 2, right = 3).
 */

//Teensy Operation
#if defined(ARDUINO_TEENSY36) | defined(ARDUINO_TEENSY41)

//UNCOMMENT TO UTILIZE
//#define MAKE_PLOTS              //Flag to serial plot
#define MAIN_DEBUG              //Flag to print Arduino debugging statements

//Standard Libraries
#include <stdint.h>

//Common Libraries
#include "src/Board.h"
#include "src/ExoData.h"
#include "src/Exo.h"
#include "src/Utilities.h"
#include "src/StatusDefs.h"
#include "src/Config.h"
#include "src/GaitPhaseDetector.h"

//Array used to store config information
namespace config_info
{
    //Single-leg hip exo config. Only the hip motor on one side is used, but
    //BOTH feet have FSRs for the event-driven gait phase detector.
    //Index values must match config_defs in ParseIni.h.
    uint8_t config_to_send[ini_config::number_of_keys] = {
            1,   //[0]  Board name
            6,   //[1]  Board version (Single_Leg_Hip_Board)
            2,   //[2]  Battery
            10,  //[3]  Exo name (left_hip)  -- change to 11 for right_hip
            2,   //[4]  Exo side (left)      -- change to 3 for right
            2,   //[5]  Hip motor (AK60)
            1,   //[6]  Knee (not used)
            1,   //[7]  Ankle (not used)
            1,   //[8]  Elbow (not used)
            1,   //[9]  Arm 1 (not used)
            1,   //[10] Arm 2 (not used)
            1,   //[11] Hip gear
            1,   //[12] Knee gear
            1,   //[13] Ankle gear
            1,   //[14] Elbow gear
            1,   //[15] Arm 1 gear
            1,   //[16] Arm 2 gear
            3,   //[17] Hip default controller (franksCollinsHip)
            1,   //[18] Knee default controller
            1,   //[19] Ankle default controller
            1,   //[20] Elbow default controller
            1,   //[21] Arm 1 default controller
            1,   //[22] Arm 2 default controller
            1,   //[23] Hip use torque sensor (1=yes, 0=no)
            1,   //[24] Knee use torque sensor
            1,   //[25] Ankle use torque sensor
            1,   //[26] Elbow use torque sensor
            1,   //[27] Arm 1 use torque sensor
            1,   //[28] Arm 2 use torque sensor
            1,   //[29] Hip flip motor dir (1=normal)
            1,   //[30] Knee flip motor dir
            1,   //[31] Ankle flip motor dir
            1,   //[32] Elbow flip motor dir
            1,   //[33] Arm 1 flip motor dir
            1,   //[34] Arm 2 flip motor dir
            1,   //[35] Hip flip torque dir
            1,   //[36] Knee flip torque dir
            1,   //[37] Ankle flip torque dir
            1,   //[38] Elbow flip torque dir
            1,   //[39] Arm 1 flip torque dir
            1,   //[40] Arm 2 flip torque dir
            1,   //[41] Hip flip angle dir
            1,   //[42] Knee flip angle dir
            1,   //[43] Ankle flip angle dir
            1,   //[44] Elbow flip angle dir
            1,   //[45] Arm 1 flip angle dir
            1,   //[46] Arm 2 flip angle dir
          };
}

void setup()
{
    analogReadResolution(12);
    
    Serial.begin(115200);
    delay(500);

    #ifdef MAIN_DEBUG
        Serial.print("\nSingle-leg hip exo boot. Programmed PCB version: ");
        Serial.print(BOARD_VERSION);
        Serial.print("\nGait phase is detected from BOTH feet's FSRs (event-driven).");
    #endif

    //Print to confirm config came through correctly
    #ifdef MAIN_DEBUG
        for (int i = 0; i < ini_config::number_of_keys; i++)
        {
          Serial.print("[" + String(i) + "] : " + String((int)config_info::config_to_send[i]) + "\n");
        }
        Serial.print("\n");
    #endif
}

void loop()
{
    static bool first_run = true;
    
    //Create the data object
    static ExoData exo_data(config_info::config_to_send);     

    //Create the exo object
    static Exo exo(&exo_data);                                

    if (first_run)
    {
        first_run = false;
        
        #ifdef MAIN_DEBUG
            Serial.print("Superloop :: exo_data.left_side.hip.is_used = ");
            Serial.print(exo_data.left_side.hip.is_used);
            Serial.print("\n");
            Serial.print("Superloop :: exo_data.right_side.hip.is_used = ");
            Serial.print(exo_data.right_side.hip.is_used);
            Serial.print("\n");
        #endif

        //Turn on / enable the used hip motor, zero the gains so there is no
        //funny business during startup, and set the default controller.
        if (exo_data.left_side.hip.is_used)
        {
            exo_data.left_side.hip.motor.is_on = true;
            exo_data.left_side.hip.motor.enabled = true;
            exo_data.left_side.hip.motor.kp = 0;
            exo_data.left_side.hip.motor.kd = 0;
            
            exo_data.left_side.hip.calibrate_torque_sensor = true;
            exo_data.left_side.hip.controller.controller = (uint8_t)config_defs::hip_controllers::zero_torque;
            exo.left_side.set_controller((int)config_defs::joint_id::hip, (uint8_t)config_defs::hip_controllers::zero_torque);
        }
        if (exo_data.right_side.hip.is_used)
        {
            exo_data.right_side.hip.motor.is_on = true;
            exo_data.right_side.hip.motor.enabled = true;
            exo_data.right_side.hip.motor.kp = 0;
            exo_data.right_side.hip.motor.kd = 0;
            
            exo_data.right_side.hip.calibrate_torque_sensor = true;
            exo_data.right_side.hip.controller.controller = (uint8_t)config_defs::hip_controllers::zero_torque;
            exo.right_side.set_controller((int)config_defs::joint_id::hip, (uint8_t)config_defs::hip_controllers::zero_torque);
        }
        
        //Give the motors time to wake up
        #ifdef MAIN_DEBUG
          Serial.print("Superloop :: Motor Charging Delay - Please be patient");
        #endif 

        exo_data.set_status(status_defs::messages::motor_start_up); 

        unsigned int motor_start_delay_ms = 10;                     //Delay duration
        unsigned int motor_start_time = millis();                   

        while (millis() - motor_start_time < motor_start_delay_ms)
        {
            exo.status_led.update(exo_data.get_status());
        }
        
        #ifdef MAIN_DEBUG
          Serial.println();
        #endif
    }

    //Run the exo calculations
    bool ran = exo.run();

    //Once the torque sensor calibration completes, switch to Franks-Collins.
    //This is a simple demo hook; in a full build the app would switch the
    //controller after the user calibrates FSRs and walks through refinement.
    if (!exo_data.left_side.hip.calibrate_torque_sensor)
    {
        if (exo_data.left_side.hip.is_used)
        {
            exo_data.left_side.hip.controller.controller = config_info::config_to_send[config_defs::exo_hip_default_controller_idx];
            exo.left_side.set_controller((int)config_defs::joint_id::hip, exo_data.left_side.hip.controller.controller);
        }
        if (exo_data.right_side.hip.is_used)
        {
            exo_data.right_side.hip.controller.controller = config_info::config_to_send[config_defs::exo_hip_default_controller_idx];
            exo.right_side.set_controller((int)config_defs::joint_id::hip, exo_data.right_side.hip.controller.controller);
        }
    }
}

#else //Code that operates when the microcontroller is not recognized

#include "Utilities.h"

void setup()
{
  Serial.begin(115200);
  utils::spin_on_error_with("Unknown Microcontroller");
}

void loop()
{

}

#endif
