/*
   Code to validate motor torque.
   Two modes
      - Calibrate just uses the load cell, and you apply static loads and record the measurements
      - Validate applies motor torque and measures the load cell values.  It reports both the raw voltage for comparison to the calibration, but you can also have it calculate the measured torque using a linear fit.
      
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
  #include "src\TorqueSensor.h"
  #include <math.h>

  #define CALIBRATE 1
  #define VALIDATE 2

  #define MODE VALIDATE
  
  namespace config_info
  {
    uint8_t (config_to_send)[ini_config::number_of_keys];
  }
  
  void setup()
  {
    Serial.setTimeout(10000);
    Serial.begin(115200);
    while(!Serial)
    {
      ;
    }
    // enable the estop pullup.
    pinMode(logic_micro_pins::motor_stop_pin,INPUT_PULLUP);
  }
  
  void loop()
  {
    config_info::config_to_send[config_defs::board_name_idx] = (uint8_t)config_defs::board_name::AK_board;
    config_info::config_to_send[config_defs::board_version_idx] = (uint8_t)config_defs::board_version::zero_three;
    config_info::config_to_send[config_defs::battery_idx] = (uint8_t)config_defs::battery::dumb;
    config_info::config_to_send[config_defs::exo_name_idx] = (uint8_t)config_defs::exo_name::bilateral_hip;
    config_info::config_to_send[config_defs::exo_side_idx] = (uint8_t)config_defs::exo_side::bilateral;
    config_info::config_to_send[config_defs::hip_idx] = (uint8_t)config_defs::motor::AK60;
    config_info::config_to_send[config_defs::knee_idx] = (uint8_t)config_defs::motor::not_used;
    config_info::config_to_send[config_defs::ankle_idx] = (uint8_t)config_defs::motor::not_used;
    config_info::config_to_send[config_defs::hip_gear_idx] = (uint8_t)config_defs::gearing::gearing_1_1;
    config_info::config_to_send[config_defs::knee_gear_idx] = (uint8_t)config_defs::gearing::gearing_1_1;
    config_info::config_to_send[config_defs::ankle_gear_idx] = (uint8_t)config_defs::gearing::gearing_1_1;
    config_info::config_to_send[config_defs::exo_hip_default_controller_idx] = (uint8_t)config_defs::hip_controllers::disabled;
    config_info::config_to_send[config_defs::exo_knee_default_controller_idx] = (uint8_t)config_defs::knee_controllers::disabled;
    config_info::config_to_send[config_defs::exo_ankle_default_controller_idx] = (uint8_t)config_defs::ankle_controllers::disabled;
    //!!! Change Direction of motor here. !!!
    config_info::config_to_send[config_defs::hip_flip_dir_idx] = (uint8_t)config_defs::flip_dir::left;
    config_info::config_to_send[config_defs::knee_flip_dir_idx] = (uint8_t)config_defs::flip_dir::neither;
    config_info::config_to_send[config_defs::ankle_flip_dir_idx] = (uint8_t)config_defs::flip_dir::neither;
    
    static ExoData exo_data(config_info::config_to_send);
    
    // these should be changed to match the ID of the motors.
    static AK60 motor(config_defs::joint_id::left_hip, &exo_data, _Joint::get_motor_enable_pin(config_defs::joint_id::left_hip, &exo_data));
    static TorqueSensor load_cell((logic_micro_pins::torque_sensor_left[0]));


    #if MODE == CALIBRATE
        static bool new_measurement = 1;
        int num_readings_to_average = 5000;
        static int reading_sum = 0;
        static int reading_count = 0;  // if stuff doesn't make sense this may be rolling over and the number of readings should be reduced.

        float load = 0;

        Serial.clearReadError();        
        
        if (new_measurement)
        {
            logger::print("Enter Load kg (value will not show till you hit enter) : ");
            new_measurement = 0;
        }
    
        if (Serial.available() > 0) {
                String load_str = 0;//Serial.readStringUntil('\n');
                if (Serial.getReadError()) {
                    logger::println("error or timeout waiting");
                }
                else
                {
                    load = load_str.toFloat();
                    // will not show value till 
//                    logger::println(load);
    
                    while (reading_count++ < num_readings_to_average)
                    {
                        reading_sum += load_cell.read();
                        logger::println(load_cell.read());
                    }
    
//                    logger::print(load);
//                    logger::print(" kg gives reading of ");
//                    logger::print(reading_sum / num_readings_to_average);  // this will round but should be close enough
//                    logger::print("\n");
                    
                    // reset for new reading
                    new_measurement = 1;
                    reading_sum = 0;
                    reading_count = 0;
                }
        }

    #elif MODE == VALIDATE

        float max_torque_Nm = 8; 
        float torque_step_Nm = .5; 
        int step_time_ms = 2000;
        static float torque_cmd = 0;
        static bool sequence_is_running = 1;
        static bool is_rising = 1;

        float moment_arm_m = .07;

        float calibration_loads_kg[2] = {1.66, 15};
        int calibration_reading[2] = {705, 745};

        static int reading_sum = 0;
        static int reading_count = 0;  // if stuff doesn't make sense this may be rolling over and the number of readings should be reduced.

        // frequency to take readings
        int state_period_ms = 1;
        static int last_transition_time = millis();
        int current_time = millis();
        
        static bool first_run = true;
        if (first_run)
        {
            first_run = false;
      
            motor._motor_data->enabled = true;
            motor.on_off(motor._motor_data->enabled);
            motor.zero();

            logger::print("torque_cmd\t");
            logger::print("raw_reading\t");
            logger::print("calibrated_torque\t");
            logger::print("\n");
        }
        
        if (state_period_ms <= (current_time - last_transition_time))
        {
          
          static int torque_cmd_start_time_ms= millis();
          int timestamp = millis();

          
          if ((timestamp - torque_cmd_start_time_ms) > step_time_ms)
          {
              torque_cmd_start_time_ms = timestamp;

              if (motor._motor_data->enabled)
              {
                  float mean_reading = (float)reading_sum/reading_count;
                  float calibrated_load_kg = map(mean_reading, calibration_reading[0], calibration_reading[1], calibration_loads_kg[0], calibration_loads_kg[1]); 
                  float measured_torque = moment_arm_m * calibrated_load_kg * 9.81; 
                  
                  logger::print(torque_cmd);
                  logger::print("\t");
                  logger::print(mean_reading);
                  logger::print("\t");
                  logger::print(measured_torque);
                 
                  logger::print("\n");
              }
              
              if (is_rising)
              {
                  torque_cmd += torque_step_Nm;
                  
                  if (torque_cmd >= max_torque_Nm)
                  {
                      is_rising = false;
                  }
//                  logger::print("Superloop: is_rising : ");
//                  logger::print(torque_cmd);
//                  logger::print("\n");
              }
              else
              {
//                  logger::println("Superloop: NOT is_rising");
                  torque_cmd -= torque_step_Nm;
                  if (torque_cmd < 0)
                  {
//                      logger::println("Superloop: end sequence");
                      sequence_is_running = false;
                      torque_cmd = 0;
                  }
              }
          
              
              reading_sum = 0;
              reading_count = 0;

              motor.on_off(motor._motor_data->enabled);
              
              // The code below helps prevent bus contention in bilateral mode
              if (config_info::config_to_send[config_defs::exo_side_idx] == (uint8_t)config_defs::exo_side::bilateral) {
                  // Add slight delay if running bilateral to stagger commands
                  delayMicroseconds(motor._is_left ? 0 : 500);
              }
              
              motor.transaction(torque_cmd);
          }
          
          reading_sum += load_cell.read();
          reading_count++;
          if (!sequence_is_running)
          {
//              logger::println("disabled motor");
              motor._motor_data->enabled = false;
          }

          last_transition_time = current_time;
          
          
        }

        
    #endif
    
  }


#endif
