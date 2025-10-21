/**
 * @file Config.h
 * @author Chancelor Cuddeback
 * @brief Configuration variables for the codebase.
 * @date 2023-07-18
 * 
 */

#ifndef Config_h
#define Config_h 

#include "Arduino.h"
#include "LogLevels.h"
	// #define SIMPLE_DEBUG //Uncomment to enter SIMPLE_DEBUG mode. The exoskeleton’s operating status will be printed to the Serial Monitor.
    #define FIRMWARE_VERSION 0_1_0

    #define AK_Board_V0_1 1
    #define AK_Board_V0_3 2
    #define AK_Board_V0_4 3
    #define AK_Board_V0_5_1 4
	#define OpenExo_Board_V0_6_Maxon 5

    #define BOARD_VERSION OpenExo_Board_V0_6_Maxon
	
	#define BATTERY_SENSOR 0 //Set it to 0 to disable, 219 to use INA219, 260 to use INA260, 3 to use the OpenExo Board 0.5.1 Mark 3's onboard voltage divider
	#define CRITICAL_BATT_VAL 18 //In volts. Battery voltage below this will trigger the low battery warning in the GUI.
	#define RESISTOR_1 47000 //Set it to the measured resistance of R1 on the OpenExo Board 0.5.1 Mark 3
	#define RESISTOR_2 4700 //Set it to the measured resistance of R2 on the OpenExo Board 0.5.1 Mark 3
    #define REAL_TIME_I2C 1
    #define LOOP_FREQ_HZ 500
    #define LOOP_TIME_TOLERANCE 0.1 
    
    #define USE_SPEED_CHECK 0
	#define USE_ANGLE_SENSORS 1

    //MACRO magic to convert a define to a string
    #define VAL(str) #str
    #define TOSTRING(str) VAL(str)

    namespace logging
    {
        const LogLevel level = LogLevel::Release; //Release or Debug (Note: Enter Debug to have Logger print to serial monitor)
        const int baud_rate = 115200;
    }
    
    namespace sync_time
    {
        const unsigned int NUM_START_STOP_BLINKS = 1;                                   //The number of times to have the LED on during the start stop sequence
        const unsigned int SYNC_HALF_PERIOD_US = 125000;                                //Half blink period in micro seconds
        const unsigned int SYNC_START_STOP_HALF_PERIOD_US = 4 * SYNC_HALF_PERIOD_US;    //Half blink period for the begining and end of the sequence. This is usually longer so it is easy to identify.
    }

    namespace fsr_config
    {
        const float FSR_UPPER_THRESHOLD = 0.25;
        const float FSR_LOWER_THRESHOLD = 0.15;
        const float SCHMITT_DELTA = (FSR_UPPER_THRESHOLD - FSR_LOWER_THRESHOLD)/2;
    }
	
	namespace angle_sensor
	{
		const float ANGLE_UPPER_THRESHOLD = 0.9;
		const float ANGLE_LOWER_THRESHOLD = 0.1;
		const float ROM_LEFT = 103.2f;
		const float ROM_RIGHT = 91.4f; //In degrees
	}

    namespace analog
    {
        const float RESOLUTION = 12;    //The resolution of the analog to digital converter
        const float COUNTS = 4096;      //The number of counts the ADC can have
    }
    
    namespace torque_calibration
    {
        const float AI_CNT_TO_V = 3.3 / 4096;   //Conversion from count to voltage
        const float TRQ_V_TO_NM = 53.70;        //Conversion from voltage to Nm (Negative do to mismatch in torque sensor and motor torque directions) S12:(Left) = 39.8, S05 (Right) = 44.6; (These will be sensor specific).
    }

    namespace BLE_times
    {
        const float _status_msg_delay = 2000000;    //Microseconds
        const float _real_time_msg_delay = 14285;   //Microseconds
        const float _update_delay = 1000;           //Microseconds
        const float _poll_timeout = 4;              //Milliseconds
    }
    
    //Update this namespace for future exo updates to display correct information on app
    namespace exo_info
    {
        const String FirmwareVersion = String(TOSTRING(FIRMWARE_VERSION));  //String to add to firmware char
        const String PCBVersion = String(TOSTRING(BOARD_VERSION));          //String to add to pcb char
        const String DeviceName = String("NULL");                            //String to add to device char, if you would like the system to set it use "NULL"
    }

    namespace UART_times
    {
        const float UPDATE_PERIOD = 1000;       //Microseconds, time between updating data over uart
        const float COMS_MCU_TIMEOUT = 5000;    //Microseconds
        const float CONT_MCU_TIMEOUT = 1000;    //Microseconds
        const float CONFIG_TIMEOUT = 8000;      //Milliseconds
    }

#endif