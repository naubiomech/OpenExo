/**
 * @file Side.h
 *
 * @brief Declares a class used to operate a side 
 * 
 * @author P. Stegall 
 * @date Jan. 2022
*/


#ifndef Side_h
#define Side_h

//Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)

#include "Arduino.h"

#include "joint.h"
#include "controller.h"
#include "FSR.h"
#include "ParseIni.h"
#include "Board.h"
#include "Utilities.h"
#include "StatusDefs.h"
#include "ThIMU.h"
#include "InclinationDetector.h"

#include <stdint.h>
#include <algorithm>

/**
 * @brief Class to operate a side.
 * 
 */
class Side
{
    public:
        Side(bool is_left, ExoData* exo_data); //Constructor: 
        
        /**
         * @brief Read FSR, calc percent gait, read joint data, send joint commands
         */
        void run_side(); 
		
        /**
         * @brief Checks if calibration flags are set, and runs calibration if they are.
         */
        void check_calibration();  
		
        /**
         * @brief Reads motor data from each motor used on that side and stores the values
         * Reads the FSR, detects ground strike, and calculates percent gait.
         * Sets the values to the corresponding place in data class.
         */
        void read_data(); 
		
        /**
         * @brief Sends new control command to the motors used on that side, based on the defined controllers
         */
        void update_motor_cmds();   
		
        /**
         * @brief Changes the controller for an individual joint
         */
        void set_controller(int joint, int controller);  
		
        /**
         * @brief Clears the step time estimate for when it gets off by more than can be adjusted for.
         */
        void clear_step_time_estimate();

        /**
         * @brief Disables all motors on a side
         * 
         */
        void disable_motors();
        
	private:
        /**
         * @brief Applies the FSR thresholds set in the data class to the FSRs
         * 
         */
        void _check_thresholds();
		
        /**
         * @brief Calculates the percentage of gait cycle based on the ground contact reading
         * and an estimate of the step time based on the average time of the last few steps.
         * Returns the percent gait which saturates at 100%
         * 
         * @return percent gait from heel strike
         */
        float _calc_percent_gait();

        /**
         * @brief Calculates the percentage of stance phase based on the toe contact reading
         * and an estimate of the stance time based on the average time of the last few stance phases.
         * Returns the percent stance which saturates at 100%
         *
         * @return percent stance from toe strike
         */
        float _calc_percent_stance();

        /**
         * @brief Calculates the percentage of swing phase based on the FSR reading
         * and an estimate of the swing time based on the average time of the last few swing phases.
         * Returns the percent swing which saturates at 100%
         *
         * @return percent swing
         */
        float _calc_percent_swing();
        
        /**
         * @brief Calculates the expected duration of a step by averaging the time the last N steps took.
         * Should only be called when a ground strike has occurred.
         *
         * @return expected duration in ms 
         */
        float _update_expected_duration();
        
        /**
         * @brief Calculates the expected duration of stance by averaging the time the last N stance phases.
         * 
         * @return expected stance duration in ms
         */
        float _update_expected_stance_duration();

        /**
         * @brief Calculates the expected duration of the swing phase by averaging the time the last N swing phases.
         *
         * @return expected stance duration in ms
         */
        float _update_expected_swing_duration();

        /**
         * @brief Checks for state changes in the FSRs to find the point when ground contact is made
         * Simple check for a rising edge of either FSR during swing and returns 1 if they have.
         * The returned value should just be high for a single cycle.
         *
         * @return 1 if the foot has gone from non-contact to ground contact. 0 Otherwise
         */
        bool _check_ground_strike();

        /**
         * @brief MUST BE CALLED AFTER _check_ground_strike()! Checks for state changes in the FSRs to find 
         * the point when ground contact is gained and lost. Simple check for a rising/falling edge of either FSR during stance 
         * and returns 1 if they have. The returned value should just be high for a single cycle. 
         *
         * @return 1 if the foot has gone from ground contact to non-contact or vice versa. 0 Otherwise
         */
        bool _check_toe_on();
        bool _check_toe_off();
		
        //Data that can be accessed
        ExoData* _data;         /**< Pointer to the overall exo data */
        SideData* _side_data;   /**< Pointer to the specific side we are using.*/
        
        //Joint objects for the side. The order these are listed are important as it will determine the order their constructors are called in the initializer list.
        HipJoint _hip;      /**< Instance of a hip joint */
        KneeJoint _knee;    /**< Instance of a knee joint */
        AnkleJoint _ankle;  /**< Instance of an ankle joint */
        ElbowJoint _elbow;  /**< Instance of an elbow joint */
        
        //FSR objects for the side. 
        FSR _heel_fsr;                      /**< Heel force sensitive resistor, raw value */
		FSR _toe_fsr;                       /**< Toe force sensitive resistor, raw value */    
        FSR_Regressed _toe_fsr_regressed;   /**< Toe force sensitive resistor, regression applied to make value proportional to ankle joint moment */

        //Inclination object for the side
        InclinationDetector* inclination_detector;
        
        bool _is_left;                              /**< Stores which side it is on */
        
        bool _prev_heel_contact_state;              /**< Prev heel contact state used for ground strike detection */
        bool _prev_toe_contact_state;               /**< Prev toe contact state used for ground strike detection */

        bool _prev_toe_contact_state_toe_off;       /**< Prev toe off state used for toe off detection */
        bool _prev_toe_contact_state_toe_on;        /**< Prev toe off state used for toe off detection */
        
        static const uint8_t _num_steps_avg = 3;    /**< Number of prior steps used to estimate the expected duration, used for percent gait calculation */
        unsigned int _step_times[_num_steps_avg];   /**< Stores the duration of the last N steps, used for percent gait calculation */ 

        unsigned int _stance_times[_num_steps_avg]; /**< Stores the duration of the last N stance phases, used for percent stance calculation */ 
        unsigned int _swing_times[_num_steps_avg];  /**< Stores the duration of the last N swing phases, used for percent swing calculation */ 
        
        unsigned int _ground_strike_timestamp;      /**< Records the time of the ground strike to determine if the next strike is within the expected window. */ 
        unsigned int _prev_ground_strike_timestamp; /**< Stores the last value to determine the difference in strike times. */ 
        unsigned int _expected_step_duration;       /**< The expected step duration to calculate the percent gait.*/
        
        unsigned int _toe_strike_timestamp;         /**< Records the time of the toe's ground strike to determine if the next strike is within the expected window. */ 
        unsigned int _prev_toe_strike_timestamp;    /**< Stores the last value to determine the difference in strike times. */
        unsigned int _expected_stance_duration;     /**< The expected stance duration to calculate the percent stance.*/
        unsigned int _toe_off_timestamp;            /**< Records the time of the toe off to determine if the next is within the expected window. */ 
        unsigned int _prev_toe_off_timestamp;       /**< Stores the last value to determine the difference in times. */
        unsigned int _expected_swing_duration;      /**< The expected swing duration to calculate the percent swing.*/
        
};
#endif
#endif