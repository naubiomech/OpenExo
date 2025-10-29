/**
 * @file ExoData.h
 *
 * @brief Declares a class used to store data for the Exo to access 
 * 
 * @author P. Stegall 
 * @date Jan. 2022
*/


#ifndef ExoData_h
#define ExoData_h

#include "Arduino.h"

#include "SideData.h"
#include <stdint.h>
#include "ParseIni.h"
#include "Board.h"
#include "StatusLed.h"
#include "StatusDefs.h"
#include "Config.h"
#include "Utilities.h"

#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)
	#if BATTERY_SENSOR == 260
		#include <Adafruit_INA260.h>
	#elif BATTERY_SENSOR == 219
		#include <Adafruit_INA219.h>
	#endif
#endif

/* 
 * ExoData was broken out from the Exo class to have it mirrored on a second microcontroller that handles BLE.
 * It doesn't need to be done this way if we aren't, and is pretty cumbersome.
 * Just thought you might be wondering about the approach.
 */

//Note: Status values are in StatusDefs.h

//Type used for the for each joint method, the function should take JointData as input and return void
typedef void (*for_each_joint_function_t) (JointData*, float*); 

/**
 * @brief Class to store all the data related to the exo
 */
class ExoData 
{
	public:
        ExoData(uint8_t* config_to_send); //Constructor
        
        /**
         * @brief Reconfigures the the exo data if the configuration changes after constructor called.
         * 
         * @param configuration array
         */
        void reconfigure(uint8_t* config_to_send);
        
        /**
         * @brief performs a function for each joint
         * 
         * @param pointer to the function that should be done for each used joint
         */
        template <typename F>
        void for_each_joint(F &&func)
        {
                func(&left_side.hip, NULL);
                func(&left_side.knee, NULL);
                func(&left_side.ankle, NULL);
                func(&left_side.elbow, NULL);
                func(&right_side.hip, NULL);
                func(&right_side.knee, NULL);
                func(&right_side.ankle, NULL);
                func(&right_side.elbow, NULL);
        }
        template <typename F>
        void for_each_joint(F &&func, float* args)
        {
                func(&left_side.hip, args);
                func(&left_side.knee, args);
                func(&left_side.ankle, args);
                func(&left_side.elbow, args);
                func(&right_side.hip, args);
                func(&right_side.knee, args);
                func(&right_side.ankle, args);
                func(&right_side.elbow, args);
        }

        //Returns a list of all of the joint IDs that are currently being used
        uint8_t get_used_joints(uint8_t* used_joints);

        /**
         * @brief Get the joint pointer for a joint id. 
         * 
         * @param id Joint id
         * @return JointData* Pointer to JointData class for joint with id
         */
        JointData* get_joint_with(uint8_t id);
        
        /**
         * @brief Prints all the exo data
         */
        void print();

        /**
         * @brief Set the status object
         * 
         * @param status_to_set status_defs::messages::status_t
         */
        void set_status(uint16_t status_to_set);
        
        /**
         * @brief Get the status object
         * 
         * @return uint16_t status_defs::messages::status_t
         */
        uint16_t get_status(void);

        /**
         * @brief Set the default controller parameters for the current controller. These are the first row in the controller csv file on the SD Card
         *
         */
        void set_default_parameters();
        
        /**
         * @brief Set the default controller parameters for the current controller. These are the first row in the controller csv file on the SD Card
         * 
         */
        void set_default_parameters(uint8_t id);

        /**
         * @brief Start the pretrial calibration process
         * 
         */
        void start_pretrial_cal();
		
		#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)
			#if BATTERY_SENSOR == 260
				Adafruit_INA260 ina260 = Adafruit_INA260();
			#elif BATTERY_SENSOR == 219
				Adafruit_INA219 ina219;
			#endif
		#endif
		
		/**
         * @brief Communicate with the power sensor and pull battery-related information such as voltage, current and power.
         * 
         */
		float get_batt_info(uint8_t batt_info_type);
        
        bool sync_led_state;    /**< State of the sync led */
        bool estop;             /**< State of the estop */
        float battery_value;    /**< Could be Voltage or SOC, depending on the battery type*/
		float filtered_batt_pwr = 0;/**< Filtered battery power*/
        SideData left_side;     /**< Data for the left side */
        SideData right_side;    /**< Data for the right side */

        uint32_t mark;          /**< Used for timing, currently only used by the nano */

        uint8_t* config;        /**< Pointer to the configuration array */
        uint8_t config_len;     /**< Length of the configuration array */

        int error_code;         /**< Current error code for the system */
        int error_joint_id;
        bool user_paused;       /**< If the user has paused the system */

        int hip_torque_flag = 0;    /**< Flag to determine if we want to use torque sensor for that joint */
        int knee_torque_flag = 0;   /**< Flag to determine if we want to use torque sensor for that joint */
        int ankle_torque_flag = 0;  /**< Flag to determine if we want to use torque sensor for that joint */
        int elbow_torque_flag = 0;  /**< Flag to determine if we want to use torque sensor for that joint */

        bool connected = false;
        bool first_pass = true;
        bool acknowledgedPacket = false;
        bool plotting_param_recieved = false;
        bool real_time_active = false;
        int current_sent_index = 0;
        bool controller_param_recieved =  false;
        bool initial_handshake_recieved = false;
        // This acknowledgement index keeps track of plotting parameter process
        // function inside of ble_commands
        int ackIndex = 0;
		
        private:
        uint16_t _status;           /**< Status of the system*/
};

#endif