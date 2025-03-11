/**
 * @file JointData.h
 *
 * @brief Declares a class used to store data for joint to access 
 * 
 * @author P. Stegall 
 * @date Jan. 2022
*/


#ifndef JointData_h
#define JointData_h

#include "Arduino.h"

#include "MotorData.h"
#include "ControllerData.h"
#include "ParseIni.h"
#include "Board.h"

#include <stdint.h>
#include <queue>

//Forward declaration
class ExoData;

/**
 * @brief Class to store information related to joint.
 * 
 */
class JointData {
	public:
        JointData(config_defs::joint_id id, uint8_t* config_to_send, float joint_RoM, bool flip_ankle_angle, float torque_offset);
        
        /**
         * @brief Reconfigures the the joint data if the configuration changes after constructor called.
         * 
         * @param configuration array
         */
        void reconfigure(uint8_t* config_to_send);
        
        config_defs::joint_id id;       /**< Id of the joint */
        MotorData motor;                /**< Data for the motor attached to the joint */
        ControllerData controller;      /**< Data for the controller running the joint */
        float torque_reading;           /**< Calibrated reading from the torque sensor */ 
        bool is_left;                   /**< If the leg is left */
        bool flip_direction;            /**< If true, invert the sign of the torque readings for the current joint (for example, left ankle) */
        bool is_used;                   /**< Stores if the joint is used, joint is skipped if it is not used */
        bool calibrate_torque_sensor;   /**< Flag for if we should calibrate the torque sensor. */ 
        bool calibrate_angle_sensor;    /**< Flag for if we should calibrate the angle sensor. */
        
        float position;                 /**< The position of the joint, this should be motor position with compensated for gearing. */
        float velocity;                 /**< The velocity of the joint, this should be motor velocity with compensated for gearing. */

        float joint_position;                       /**< The position of the joint, after any transmission */
        float joint_global_angle;                   /**< The angle of the joint relative to the ground */
        float prev_joint_position;                  /**< The previous position of the joint, after any transmission */
        float joint_velocity;                       /**< The velocity of the joint, after any transmission */
        const float joint_position_alpha = 0.05f;
        const float joint_velocity_alpha = 0.05f;
		const float joint_RoM;                      /**< Joint Range of Motion */
		bool do_flip_angle;                         /**< If true invert the angle */

		float torque_reading_microSD;				/**< Torque reading based on the stored offset on the SD card */
		const float torque_offset;					/**< Torque offset pulled from the SD card for the current torque sensor */
		float torque_offset_reading;				/**< True torque offset for the current torque sensor; note that the true offset may change */

        //Torque tracking check
        float torque_output_alpha = 0.2;    /**< Low pass to describe the lag of the low level controller relative to setpoint. */
        float post_transmission_torque = 0; /**< Torque after torque_output_alpha. */

        //Torque absolute check
        float torque_output_threshold = 60; /**< Maximum value of the the filtered torque. */

        //Torque variance check
        const int torque_failure_count_max = 2;         /**< Amount of samples outside of error bounds. */
        const int torque_std_dev_multiple = 10;         /**< Number of standard deviations from the mean to be considered an error. */
        const float torque_data_window_max_size = 100;  /**< Number of samples to use for the standard deviation calculation. */
        std::queue<float> torque_data_window;           /**< Queue to store torque sensor values. */
        int torque_failure_count = 0;                   /**< Number of successive samples outside of error bounds. */

        //Transmission efficiency check
        const float transmission_efficiency_threshold = 0.001;
        const float motor_torque_smoothing = 0.1;
        const float torque_error_smoothing = 1;
        float smoothed_motor_torque = 0;
        const float close_to_zero_tolerance = 0.1;
        float torque_error = 0;

};


#endif