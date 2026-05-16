/**
 * @file Board.h
 * @author Chancelor Cuddeback
 * @brief Defines the pins and peripherals for the board being used.
 * @date 2023-07-18
 * 
 */

#ifndef BOARD_SETTINGS_HEADER
    #define BOARD_SETTINGS_HEADER
    
    #include "Config.h"     //Include config so we know what board is being used.

    #if BOARD_VERSION == AK_Board_V0_1
        #include "Arduino.h"
        namespace logic_micro_pins  //Teensy
        {
			//Pin to use when we need a value but don't actually want to use it.
            const unsigned int not_connected_pin = 42;  //Selected 42 as it is a pad on the back so I figure it won't hurt anything if something goes wrong.
			
            #if defined(ARDUINO_TEENSY36)
                //Serial Pins, NC
                const unsigned int rx1_pin = 0;
                const unsigned int tx1_pin = 1;
                
                //CAN Pins
                const unsigned int can_rx_pin = 4;
                const unsigned int can_tx_pin = 3;
                
                //FSR Pins
                const unsigned int fsr_sense_left_heel_pin = A14;
                const unsigned int fsr_sense_left_toe_pin = A15;
                const unsigned int fsr_sense_right_heel_pin= A7;
                const unsigned int fsr_sense_right_toe_pin = A6;
                
                //Torque Sensor Pins
                const unsigned int num_available_joints = 2;
                const unsigned int torque_sensor_left[] = {A17, A16};
                //const unsigned int torque_sensor_left1 = A16;
                const unsigned int torque_sensor_right[] = {A9, A8};
                //const unsigned int torque_sensor_right1 = A8;
                
                
                //Sync LED Pins
                const unsigned int sync_led_pin = 29;
                const unsigned int sync_default_pin = 25;
				
				//For Maxon PCB only
				const unsigned int maxon_err_right_pin = not_connected_pin;
				const unsigned int maxon_err_left_pin = not_connected_pin;
				const unsigned int maxon_ctrl_left_pin = not_connected_pin;
				const unsigned int maxon_ctrl_right_pin = not_connected_pin;
				const unsigned int maxon_current_left_pin = not_connected_pin;
				const unsigned int maxon_current_right_pin = not_connected_pin;
				const unsigned int maxon_pwm_neutral_val = 2048;
				const unsigned int maxon_pwm_u_bound = 3690;
				const unsigned int maxon_pwm_l_bound = 655;
				
				//Servo motor pins
				const unsigned int servo_pins[] = {};
				
				//Battery voltage sensing pin (for 0.5.1 Mark 3 only)
				const unsigned int volt_sense = not_connected_pin;
            #endif

            const unsigned int sync_led_on_state = LOW;
            const unsigned int sync_led_off_state = HIGH;
            
            //Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
            #if defined(ARDUINO_TEENSY36)
                //Status LED Pins
                const unsigned int status_led_r_pin= 28;
                const unsigned int status_led_g_pin = 27;
                const unsigned int status_led_b_pin = 26;
            #endif

            //If you have connected to pins with PWM set to true.
            const bool status_has_pwm = false;

            //For high to be on use 255 for the on state and 0 for the off, for low as on flip it.
            const uint8_t status_led_on_state = 0;//255;
            const uint8_t status_led_off_state = 4095;//0;
            
            //Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
            #if defined(ARDUINO_TEENSY36)
                //SPI Follower Pins
                const unsigned int miso_pin = 12;
                const unsigned int mosi_pin= 11;
                const unsigned int sck_pin = 13;
                const unsigned int cs_pin = 10;
                const unsigned int spi_mode = 16;
                
                //Pin to Stop the Motors
                const unsigned int motor_stop_pin = 6;
            #endif
            
            const unsigned int enable_left_pin[] = {not_connected_pin, not_connected_pin};
            const unsigned int enable_right_pin[] = {not_connected_pin, not_connected_pin};
            
            const unsigned int motor_enable_on_state = HIGH;
            const unsigned int motor_enable_off_state = LOW;
        };
        
        #if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)
            namespace coms_micro_pins  //Nano
            {
                const unsigned int blue = 24;
                const unsigned int green = 23;
                const unsigned int red = 22;
                const unsigned int led_active_low = 1;
            };
        #endif    
    #elif BOARD_VERSION == AK_Board_V0_3 
       
        #include "Arduino.h"
        namespace logic_micro_pins  //Teensy
        {
			//Pin to use when we need a value but don't actually want to use it.
            const unsigned int not_connected_pin = 51;  //Selected 51 as it is a pad on the back so I figure it won't hurt anything if something goes wrong.
             #if defined(ARDUINO_TEENSY41)
                //Serial Pins, NC
                const unsigned int rx1_pin = 0;
                const unsigned int tx1_pin = 1;
                
                //CAN Pins
                const unsigned int can_rx_pin = 23;
                const unsigned int can_tx_pin = 22;
                
                //FSR Pins
                const unsigned int fsr_sense_left_heel_pin = A14;
                const unsigned int fsr_sense_left_toe_pin = A15;
                const unsigned int fsr_sense_right_heel_pin= A5;
                const unsigned int fsr_sense_right_toe_pin = A4;
                
                //Torque Sensor Pins
                const unsigned int num_available_joints = 2;
                const unsigned int torque_sensor_left[] = {A12, A13};
                //const unsigned int torque_sensor_left1 = A16;
                const unsigned int torque_sensor_right[] = {A7, A6};
                //const unsigned int torque_sensor_right1 = A8;
                
                
                //Sync LED Pins
                const unsigned int sync_led_pin = 15;
                const unsigned int sync_default_pin = 5;
				
				//For Maxon PCB only
				const unsigned int maxon_err_right_pin = not_connected_pin;
				const unsigned int maxon_err_left_pin = not_connected_pin;
				const unsigned int maxon_ctrl_left_pin = not_connected_pin;
				const unsigned int maxon_ctrl_right_pin = not_connected_pin;
				const unsigned int maxon_current_left_pin = not_connected_pin;
				const unsigned int maxon_current_right_pin = not_connected_pin;
				const unsigned int maxon_pwm_neutral_val = 2048;
				const unsigned int maxon_pwm_u_bound = 3690;
				const unsigned int maxon_pwm_l_bound = 655;
				
				//Servo motor pins
				const unsigned int servo_pins[] = {};
				
				//Battery voltage sensing pin (for 0.5.1 Mark 3 only)
				const unsigned int volt_sense = not_connected_pin;
            #endif

            //Arduino compiles all files not just the ones that are used so this is not under teensy to prevent errors
            const unsigned int sync_led_on_state = LOW;
            const unsigned int sync_led_off_state = HIGH;

             #if defined(ARDUINO_TEENSY41)
                //Status LED Pins
                const unsigned int status_led_r_pin = 14;
                const unsigned int status_led_g_pin = 25;
                const unsigned int status_led_b_pin = 24;
            #endif

            //If you have connected to pins with PWM set to true.
            const bool status_has_pwm = true;

            //For high to be on use 255 for the on state and 0 for the off, for low as on flip it.
            const uint8_t status_led_on_state = 0;//255;
            const uint8_t status_led_off_state = 4095;//0;  
                
            #if defined(ARDUINO_TEENSY41)    
                //SPI Follower Pins
                const unsigned int miso_pin = 12;
                const unsigned int mosi_pin= 11;
                const unsigned int sck_pin = 13;
                const unsigned int cs_pin = 10;
                const unsigned int irq_pin = 34;
                const unsigned int rst_pin = 4;
                const unsigned int spi_mode = 8; //This is 8 or 16 bit, not the actual SPI mode, I know it is confusing but that is how they chose to make the library.
                
                
                //Pin to Stop the Motors
                const unsigned int motor_stop_pin = 9;       
                
                //Motor enable Pins
                const unsigned int enable_left_pin[] = {28, 29};
                const unsigned int enable_right_pin[] = {8, 7};
                
                const unsigned int speed_check_pin = 33;

                const unsigned int left_ankle_angle_pin = A16;
                const unsigned int right_ankle_angle_pin = A17;
            #endif
            
            const unsigned int motor_enable_on_state = HIGH;
            const unsigned int motor_enable_off_state = LOW;
        };
        
        #if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)
        namespace coms_micro_pins  //Nano
        {
            const unsigned int blue = 24;
            const unsigned int green = 23;
            const unsigned int red = 22;
            const unsigned int led_active_low = 1;
            
            //SPI Conroller Pins
            const unsigned int miso_pin = 11;
            const unsigned int mosi_pin= 12;
            const unsigned int sck_pin = 13;
            const unsigned int cs_pin = 10;
            const unsigned int spi_mode = 16;  
        };
        #endif
    #elif BOARD_VERSION == AK_Board_V0_4 
       
        #include "Arduino.h"
        namespace logic_micro_pins  //Teensy
        {
			//Pin to use when we need a value but don't actually want to use it.
            const unsigned int not_connected_pin = 51;  //Selected 51 as it is a pad on the back so I figure it won't hurt anything if something goes wrong.
			
             #if defined(ARDUINO_TEENSY41)
                //Serial Pins, NC
                const unsigned int rx1_pin = 0;
                const unsigned int tx1_pin = 1;
                
                //CAN Pins
                const unsigned int can_rx_pin = 23;
                const unsigned int can_tx_pin = 22;
                
                //FSR Pins
                const unsigned int fsr_sense_left_heel_pin = A14;
                const unsigned int fsr_sense_left_toe_pin = A15;
                const unsigned int fsr_sense_right_heel_pin= A3;
                const unsigned int fsr_sense_right_toe_pin = A2;
                
                //Torque Sensor Pins
                const unsigned int num_available_joints = 2;
                const unsigned int torque_sensor_left[] = {A17, A16};
                //const unsigned int torque_sensor_left1 = A16;
                const unsigned int torque_sensor_right[] = {A7, A6};
                //const unsigned int torque_sensor_right1 = A8;
                
                
                //Sync LED Pins
                const unsigned int sync_led_pin = 15;
                const unsigned int sync_default_pin = 5;
				
				//For Maxon PCB only
				const unsigned int maxon_err_right_pin = not_connected_pin;
				const unsigned int maxon_err_left_pin = not_connected_pin;
				const unsigned int maxon_ctrl_left_pin = not_connected_pin;
				const unsigned int maxon_ctrl_right_pin = not_connected_pin;
				const unsigned int maxon_current_left_pin = not_connected_pin;
				const unsigned int maxon_current_right_pin = not_connected_pin;
				const unsigned int maxon_pwm_neutral_val = 2048;
				const unsigned int maxon_pwm_u_bound = 3690;
				const unsigned int maxon_pwm_l_bound = 655;
				
				//Servo motor pins
				const unsigned int servo_pins[] = {};
				
				//Battery voltage sensing pin (for 0.5.1 Mark 3 only)
				const unsigned int volt_sense = not_connected_pin;
            #endif

            //Arduino compiles all files not just the ones that are used so this is not under teensy to prevent errors
            const unsigned int sync_led_on_state = LOW;
            const unsigned int sync_led_off_state = HIGH;

             #if defined(ARDUINO_TEENSY41)
                //Status LED Pins
                const unsigned int status_led_r_pin= 14;
                const unsigned int status_led_g_pin = 25;
                const unsigned int status_led_b_pin = 24;
            #endif

            //If you have connected to pins with PWM set to true.
            const bool status_has_pwm = true; 

            //For high to be on use 255 for the on state and 0 for the off, for low as on flip it.
            const uint8_t status_led_on_state = 0;//255;
            const uint8_t status_led_off_state = 4095;//0;  
                
            #if defined(ARDUINO_TEENSY41)    
                //SPI Follower Pins
                const unsigned int miso_pin = 12;
                const unsigned int mosi_pin= 11;
                const unsigned int sck_pin = 13;
                const unsigned int cs_pin = 10;
                const unsigned int irq_pin = 34;
                const unsigned int rst_pin = 4;
                const unsigned int spi_mode = 8; //This is 8 or 16 bit, not the actual SPI mode, I know it is confusing but that is how they chose to make the library.
                
                
                //Pin to Stop the Motors
                const unsigned int motor_stop_pin = 9;

                
                //Motor enable Pins
                const unsigned int enable_left_pin[] = {28, 29};
                const unsigned int enable_right_pin[] = {8, 7};
                
                const unsigned int speed_check_pin = 33;

                const unsigned int left_ankle_angle_pin;
                const unsigned int right_ankle_angle_pin;
            #endif
            
            const unsigned int motor_enable_on_state = HIGH;
            const unsigned int motor_enable_off_state = LOW;
        };
        
        #if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)
        namespace coms_micro_pins  //Nano
        {
            const unsigned int blue = 24;
            const unsigned int green = 23;
            const unsigned int red = 22;
            const unsigned int led_active_low = 1;
            
            //SPI Conroller Pins
            const unsigned int miso_pin = 11;
            const unsigned int mosi_pin= 12;
            const unsigned int sck_pin = 13;
            const unsigned int cs_pin = 10;
            const unsigned int spi_mode = 16;
        };
        #endif
    #elif BOARD_VERSION == AK_Board_V0_5_1 
       
        #include "Arduino.h"
        namespace logic_micro_pins  //Teensy
        {
			//Pin to use when we need a value but don't actually want to use it.
            const unsigned int not_connected_pin = 51;  //Selected 51 as it is a pad on the back so I figure it won't hurt anything if something goes wrong.
			
             #if defined(ARDUINO_TEENSY41)
                //Serial Pins, NC
                const unsigned int rx1_pin = 0;
                const unsigned int tx1_pin = 1;
                
                //CAN Pins
                const unsigned int can_rx_pin = 23;
                const unsigned int can_tx_pin = 22;
                
                //FSR Pins
                const unsigned int fsr_sense_left_heel_pin = A14;
                const unsigned int fsr_sense_left_toe_pin = A15;
                const unsigned int fsr_sense_right_heel_pin= A3;
                const unsigned int fsr_sense_right_toe_pin = A2;
                
                //Torque Sensor Pins
                const unsigned int num_available_joints = 2;
                //const unsigned int torque_sensor_left[] = {A16, A17};
				const unsigned int torque_sensor_left[] = {A16};
                //const unsigned int torque_sensor_left1 = A16;
                //const unsigned int torque_sensor_right[] = {A6, A7};
				const unsigned int torque_sensor_right[] = {A6};
                //const unsigned int torque_sensor_right1 = A8;
                
                
                //Sync LED Pins
                const unsigned int sync_led_pin = 15;
                const unsigned int sync_default_pin = 5;
				
				//For Maxon PCB only
				const unsigned int maxon_err_right_pin = not_connected_pin;
				const unsigned int maxon_err_left_pin = not_connected_pin;
				const unsigned int maxon_ctrl_left_pin = not_connected_pin;
				const unsigned int maxon_ctrl_right_pin = not_connected_pin;
				const unsigned int maxon_current_left_pin = not_connected_pin;
				const unsigned int maxon_current_right_pin = not_connected_pin;
				const unsigned int maxon_pwm_neutral_val = 2048;
				const unsigned int maxon_pwm_u_bound = 3690;
				const unsigned int maxon_pwm_l_bound = 655;
				
				//Servo motor pins
				const unsigned int servo_pins[] = {};
				
				//Battery voltage sensing pin (for 0.5.1 Mark 3 only)
				const unsigned int volt_sense = not_connected_pin;//Set it to 27 to use the onboard voltage sensing. Refer to Config.h and modify RESISTOR_1 and RESISTOR_2 if needed.
            #endif

            //Arduino compiles all files not just the ones that are used so this is not under teensy to prevent errors
            const unsigned int sync_led_on_state = LOW;
            const unsigned int sync_led_off_state = HIGH;

             #if defined(ARDUINO_TEENSY41)
                //Status LED Pins
                const unsigned int status_led_r_pin= 14;
                const unsigned int status_led_g_pin = 25;
                const unsigned int status_led_b_pin = 24;
            #endif

            //If you have connected to pins with PWM set to true.
            const bool status_has_pwm = true;  

            //For high to be on use 255 for the on state and 0 for the off, for low as on flip it.
            const uint8_t status_led_on_state = 0;//255;
            const uint8_t status_led_off_state = 4095;//0;  
                
            #if defined(ARDUINO_TEENSY41)    
                //SPI Follower Pins
                const unsigned int miso_pin = not_connected_pin;
                const unsigned int mosi_pin = 11;
                const unsigned int sck_pin = not_connected_pin;
                const unsigned int cs_pin = 10;
                const unsigned int irq_pin = 34;
                const unsigned int rst_pin = 4;
                const unsigned int spi_mode = 8; //This is 8 or 16 bit, not the actual SPI mode, I know it is confusing but that is how they chose to make the library.
                
                //Pin to Stop the Motors
                const unsigned int motor_stop_pin = 9;
                
                //Motor enable Pins
                const unsigned int enable_left_pin[] = {28, 29};
                const unsigned int enable_right_pin[] = {8, 7};
                
                const unsigned int speed_check_pin = 33;
				
				const unsigned int left_ankle_angle_pin = A13;
                const unsigned int right_ankle_angle_pin = A12;
            #endif
            
            const unsigned int motor_enable_on_state = HIGH;
            const unsigned int motor_enable_off_state = LOW;
        };
        
        #if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)
        namespace coms_micro_pins  //Nano
        {
            const unsigned int blue = 24;
            const unsigned int green = 23;
            const unsigned int red = 22;
            const unsigned int led_active_low = 1;
            
            //SPI Conroller Pins
            const unsigned int miso_pin = 11;
            const unsigned int mosi_pin= 12;
            const unsigned int sck_pin = 13;
            const unsigned int cs_pin = 10;
            const unsigned int spi_mode = 16;
        };
        #endif
	#elif BOARD_VERSION == OpenExo_Board_V0_6_Maxon //Note: Also Compatible with OpenExo Board 0.7 (No need to have a seperate v0.7)
   
		#include "Arduino.h"
		namespace logic_micro_pins  //teensy
		{
			//Pin to use when we need a value but don't actually want to use it.
            const unsigned int not_connected_pin = 51;  //Selected 51 as it is a pad on the back so I figure it won't hurt anything if something goes wrong.
			
			 #if defined(ARDUINO_TEENSY41)
				//Maxon motor Pins
				const unsigned int maxon_err_right_pin = 37;
				const unsigned int maxon_err_left_pin = 36;
				const unsigned int maxon_ctrl_left_pin = A8;
				const unsigned int maxon_ctrl_right_pin = A9;
				const unsigned int maxon_current_left_pin = A0;
				const unsigned int maxon_current_right_pin = A1;
				const unsigned int maxon_pwm_neutral_val = 2048;
				const unsigned int maxon_pwm_u_bound = 3690;
				const unsigned int maxon_pwm_l_bound = 655;
				
				//Servo motor pins
				const unsigned int servo_pins[] = {26,27};
				
				//Battery voltage sensing pin (for 0.5.1 Mark 3 only)
				const unsigned int volt_sense = not_connected_pin;
			 
				//Serial Pins, NC
				const unsigned int rx1_pin = 35;
				const unsigned int tx1_pin = 34;
				
				//CAN Pins
				const unsigned int can_rx_pin = not_connected_pin;
				const unsigned int can_tx_pin = not_connected_pin;
				
				//FSR Pins
				const unsigned int fsr_sense_left_heel_pin = A14;
				const unsigned int fsr_sense_left_toe_pin = A15;
				const unsigned int fsr_sense_right_heel_pin= A3;
				const unsigned int fsr_sense_right_toe_pin = A2;
				
				//Torque Sensor Pins (This will need to be updated/fixed)
				const unsigned int num_available_joints = 2;
				//const unsigned int torque_sensor_left[] = {A16, A17};
				const unsigned int torque_sensor_left[] = {A16};
				//const unsigned int torque_sensor_left1 = A16;
				//const unsigned int torque_sensor_right[] = {A6, A7};
				const unsigned int torque_sensor_right[] = {A6};
				//const unsigned int torque_sensor_right1 = A8;
				
				//Sync LED Pins
				const unsigned int sync_led_pin = A17;
				const unsigned int sync_default_pin = not_connected_pin;

                const unsigned int SmBusSDA = A4;
                const unsigned int SmBusSCL = A5;
			#endif
			
            //Arduino compiles all files not just the ones that are used so this is not under teensy to prevent errors
			const unsigned int sync_led_on_state = LOW;
			const unsigned int sync_led_off_state = HIGH;

			 #if defined(ARDUINO_TEENSY41)
				//Status LED Pins
				const unsigned int status_led_r_pin = A7;
				const unsigned int status_led_g_pin = A10;
				const unsigned int status_led_b_pin = A11;
			#endif

			//If you have connected to pins with PWM set to true.
			const bool status_has_pwm = true;

			//For high to be on use 255 for the on state and 0 for the off, for low as on flip it.
			const uint8_t status_led_on_state = 0;
			const uint8_t status_led_off_state = 4095; 
				
			#if defined(ARDUINO_TEENSY41)    
				//SPI Follower Pins
				const unsigned int miso_pin = not_connected_pin;
				const unsigned int mosi_pin = not_connected_pin;
				const unsigned int sck_pin = not_connected_pin;
				const unsigned int cs_pin = not_connected_pin;
				const unsigned int irq_pin = not_connected_pin;
				const unsigned int rst_pin = not_connected_pin;
				const unsigned int spi_mode = 8; // This is 8 or 16 bit, not the actual SPI mode, I know it is confusing but that is how they chose to make the library.
				
				//Pin to Stop the Motors
				const unsigned int motor_stop_pin = not_connected_pin;
				
				//Motor enable Pins
				const unsigned int enable_left_pin[] = {33};
				const unsigned int enable_right_pin[] = {33};
				
				const unsigned int speed_check_pin = not_connected_pin;
				
				const unsigned int left_ankle_angle_pin = A13;
				const unsigned int right_ankle_angle_pin = A12;
				
				// I2C 
				// SDA 18
				// SCL 19
				
				//SPI
				// 
			#endif
			
			const unsigned int motor_enable_on_state = HIGH;
			const unsigned int motor_enable_off_state = LOW;
		};
		
		#if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)
		namespace coms_micro_pins  //Nano
		{
			const unsigned int blue = 24;
			const unsigned int green = 23;
			const unsigned int red = 22;
			const unsigned int led_active_low = 1;
			
			//SPI Conroller Pins
			const unsigned int miso_pin = 11;
			const unsigned int mosi_pin= 12;
			const unsigned int sck_pin = 13;
			const unsigned int cs_pin = 10;
			const unsigned int spi_mode = 16;
		};
		#endif
	#endif
#endif    

