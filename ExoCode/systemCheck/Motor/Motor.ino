/*
   Code to test the Status LED
   P. Stegall April 2022
*/
#if defined(ARDUINO_TEENSY36) | defined(ARDUINO_TEENSY41)
  // general headers for using the system
  #include "src\Board.h"
  #include "src\Utilities.h"
  #include "src\ParseIni.h"
  
  // header for the component we are checking.
  #include "src\Motor.h"
  #include "src\ExoData.h"
  #include "src\Joint.h"
  #include <math.h>
  
  namespace config_info
  {
    uint8_t (config_to_send)[ini_config::number_of_keys];
  }
  
  void setup()
  {
    Serial.begin(115200);
    while(!Serial)
    {
      ;
    }
    // enable the estop pullup.
    pinMode(logic_micro_pins::motor_stop_pin,INPUT_PULLUP);

    logger::print("Left_hip_angle, ");
    logger::print("Right_hip_angle, ");
    logger::print("Left_hip_setpoint, ");
    logger::print("Right_hip_setpoint, ");
    logger::print("\n");

    
//    #if BOARD_VERSION == AK_Board_V0_1
//      logger::println("Board : AK_Board_V0_1");
//    #elif BOARD_VERSION == AK_Board_V0_3
//      logger::println("Board : AK_Board_V0_3");
//    #endif
//  
//    #if defined(ARDUINO_TEENSY36)
//      logger::println("Teensy 3.6");
//    #elif defined(ARDUINO_TEENSY41)
//      logger::println("Teensy 4.1");
//    #endif
//  
//    
//    
//    logger::print(logic_micro_pins::status_led_r_pin);
//    logger::print("\t");
//    logger::print(logic_micro_pins::status_led_g_pin);
//    logger::print("\t");
//    logger::print(logic_micro_pins::status_led_b_pin);
//    logger::print("\n");
    
  }
  
  void loop()
  {
    config_info::config_to_send[config_defs::board_name_idx] = (uint8_t)config_defs::board_name::AK_board;
    config_info::config_to_send[config_defs::board_version_idx] = (uint8_t)config_defs::board_version::zero_three;
    config_info::config_to_send[config_defs::battery_idx] = (uint8_t)config_defs::battery::dumb;
    config_info::config_to_send[config_defs::exo_name_idx] = (uint8_t)config_defs::exo_name::bilateral_hip;
    config_info::config_to_send[config_defs::exo_side_idx] = (uint8_t)config_defs::exo_side::bilateral;
    config_info::config_to_send[config_defs::hip_idx] = (uint8_t)config_defs::motor::AK60v1_1;
    config_info::config_to_send[config_defs::knee_idx] = (uint8_t)config_defs::motor::not_used;
    config_info::config_to_send[config_defs::ankle_idx] = (uint8_t)config_defs::motor::AK60v1_1;
    config_info::config_to_send[config_defs::hip_gear_idx] = (uint8_t)config_defs::gearing::gearing_1_1;
    config_info::config_to_send[config_defs::knee_gear_idx] = (uint8_t)config_defs::gearing::gearing_1_1;
    config_info::config_to_send[config_defs::ankle_gear_idx] = (uint8_t)config_defs::gearing::gearing_1_1;
    config_info::config_to_send[config_defs::exo_hip_default_controller_idx] = (uint8_t)config_defs::hip_controllers::disabled;
    config_info::config_to_send[config_defs::exo_knee_default_controller_idx] = (uint8_t)config_defs::knee_controllers::disabled;
    config_info::config_to_send[config_defs::exo_ankle_default_controller_idx] = (uint8_t)config_defs::ankle_controllers::disabled;
    config_info::config_to_send[config_defs::hip_flip_dir_idx] = (uint8_t)config_defs::flip_dir::right;
    config_info::config_to_send[config_defs::knee_flip_dir_idx] = (uint8_t)config_defs::flip_dir::neither;
    config_info::config_to_send[config_defs::ankle_flip_dir_idx] = (uint8_t)config_defs::flip_dir::neither;
    
    static ExoData exo_data(config_info::config_to_send);
    
    
    
    // these should be changed to match the ID of the motors.
    
        static AK60v1_1 left_hip_motor(config_defs::joint_id::left_hip, &exo_data, _Joint::get_motor_enable_pin(config_defs::joint_id::left_hip, &exo_data));
        static AK60v1_1 right_hip_motor(config_defs::joint_id::right_hip, &exo_data, _Joint::get_motor_enable_pin(config_defs::joint_id::right_hip, &exo_data));
        static AK60v1_1 left_ankle_motor(config_defs::joint_id::left_ankle, &exo_data, _Joint::get_motor_enable_pin(config_defs::joint_id::left_ankle, &exo_data));
        static AK60v1_1 right_ankle_motor(config_defs::joint_id::right_ankle, &exo_data, _Joint::get_motor_enable_pin(config_defs::joint_id::right_ankle, &exo_data));
    
    
    

    int time_to_stay_on_ms = 60000;

          
    int state_period_ms = 1;
    float left_magnitude = 1;
    float right_magnitude = 1;//1;
    static int last_transition_time = millis();
    int current_time = millis();
    
    
    static int pattern_start_timestamp = millis();
    int pattern_period_ms = 2000;
    
    static bool first_run = true;
    if (first_run)
    {
        first_run = false;

        if (exo_data.left_leg.hip.is_used)
        {
            left_hip_motor._motor_data->enabled = true;
            left_hip_motor.on_off(left_hip_motor._motor_data->enabled);
            left_hip_motor.zero();
        }
        if (exo_data.right_leg.hip.is_used)
        {
            right_hip_motor._motor_data->enabled = true;
            right_hip_motor.on_off(right_hip_motor._motor_data->enabled);
            right_hip_motor.zero();
        }
        if (exo_data.left_leg.ankle.is_used)
        {
            left_ankle_motor._motor_data->enabled = true;
            left_ankle_motor.on_off(left_ankle_motor._motor_data->enabled);
            left_ankle_motor.zero();
        }
        if (exo_data.right_leg.ankle.is_used)
        {
            right_ankle_motor._motor_data->enabled = true;
            right_ankle_motor.on_off(right_ankle_motor._motor_data->enabled);
            right_ankle_motor.zero();
        }
        

    }
    
    static int motor_enable_time = millis();
    
    if (state_period_ms <= (current_time - last_transition_time))
    {
      int timestamp = millis();
//      logger::print("Superloop : time since enable = ");
//      logger::print(timestamp - motor_enable_time);
//      logger::print("\n");

      if (time_to_stay_on_ms < (timestamp - motor_enable_time))
      {
          if (exo_data.left_leg.hip.is_used)
          {
              left_hip_motor._motor_data->enabled = false;
              left_hip_motor.on_off(left_hip_motor._motor_data->enabled);
          }
          if (exo_data.right_leg.hip.is_used)
          {
              right_hip_motor._motor_data->enabled = false;       
              right_hip_motor.on_off(right_hip_motor._motor_data->enabled);
          }
          if (exo_data.left_leg.ankle.is_used)
          {
              left_ankle_motor._motor_data->enabled = false;
              left_ankle_motor.on_off(left_ankle_motor._motor_data->enabled);
          }
          if (exo_data.right_leg.ankle.is_used)
          {
              right_ankle_motor._motor_data->enabled = false;
              right_ankle_motor.on_off(right_ankle_motor._motor_data->enabled);
          }
      }
    
      // This isn't the actual angle this is an angle used to create a sinusodal torque
      float angle_deg = 360.0 * (timestamp - pattern_start_timestamp) / pattern_period_ms;
      float left_torque_command = left_magnitude * sin (angle_deg * PI / 180);
      float right_torque_command = right_magnitude * sin (angle_deg * PI / 180);
      
      
      if (exo_data.left_leg.hip.is_used)
      {
          left_hip_motor.transaction(left_torque_command);
      }
      if (exo_data.right_leg.hip.is_used)
      {
          right_hip_motor.transaction(right_torque_command); 
      }
      if (exo_data.left_leg.ankle.is_used)
      {
          left_ankle_motor.transaction(left_torque_command);
      }
      if (exo_data.right_leg.ankle.is_used)
      {
          right_ankle_motor.transaction(right_torque_command);
      }
      
      last_transition_time = current_time;
//      if (left_motor._motor_data->enabled)
//      {
//          logger::print(left_motor._motor_data->p);
//          logger::print("\t");
//          logger::print(right_motor._motor_data->p);
//          logger::print("\t");
//          logger::print(left_torque_command);
//          logger::print("\t");
//          logger::print(right_torque_command);
//         
//          logger::print("\n");
//      }
      
    }
    
  }


#endif
