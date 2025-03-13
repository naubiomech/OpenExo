/**
 * @file MotorData.h
 *
 * @brief Declares a class used to store data for motors to access 
 * 
 * @author P. Stegall 
 * @date Jan. 2022
*/

#ifndef MotorData_h
#define MotorData_h

#include "Arduino.h"

#include "ParseIni.h"
#include "Board.h"

#include <stdint.h>

//Forward declaration
class ExoData;

class MotorData 
{
	public:
        MotorData(config_defs::joint_id id, uint8_t* config_to_send);
        
        /**
         * @brief reconfigures the the motor data if the configuration changes after constructor called.
         * 
         * @param configuration array
         */
        void reconfigure(uint8_t* config_to_send);
        
        config_defs::joint_id id;   /**< Motor id, should be the same as the joint id. */ 
        uint8_t motor_type;         /**< Type of motor being used. */
        float last_command;         /**< Last command sent to the motor. */
        float p;                    /**< Read position. */ 
        float v;                    /**< Read velocity. */
        float i;                    /**< Read current. */
        float kt;                   /**< Motor torque constant. */
        float p_des = 0;            /**< Desired position, not currently used but available for position control */
        float v_des = 0;            /**< Desired velocity, not currently used but available for velocity control */
        float kp = 0;               /**< Proportional gain */
        float kd = 0;               /**< Derivative gain */
        float t_ff = 0;             /**< Torque command */
        
        bool do_zero;               /**< Flag to zero the position of the motor */
        bool enabled;               /**< Motor enable state*/
        bool is_on;                 /**< Motor power state */
        bool is_left;               /**< Motor side information 1 if on the left, 0 otherwise */
        bool flip_direction;        /**< Should the motor direction be flipped, if true torque commands and position/velocity information will be inverted */
        float gearing;              /**< Motor gearing used to convert motor position, velocity, and torque between the motor and joint frames. */

        //Timeout state
        int timeout_count;          /**< Number of timeouts in a row */
        int timeout_count_max = 40; /**< Number of timeouts in a row before the motor is disabled */
		
		//Real-time Maxon Motor Reset Feedback
		int maxon_plotting_scalar = 1;/**< maxon motor debugging*/
		bool maxon_disabled = false;/**< maxon motor debugging*/
		uint16_t maxon_pwm_delivered = 2048;/**< maxon motor debugging*/

};


#endif