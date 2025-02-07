/**
 * @file SideData.h
 *
 * @brief Declares a class used to store data for side to access 
 * 
 * @author P. Stegall 
 * @date Jan. 2022
*/


#ifndef SideData_h
#define SideData_h

#include "Arduino.h"

#include "JointData.h"
#include "ParseIni.h"
#include "Board.h"
#include "InclinationDetector.h"

#include <stdint.h>

//Forward declaration
class ExoData;

/**
 * @brief class to store information related to the side.
 * 
 */
class SideData {
	   
    public:
        SideData(bool is_left, uint8_t* config_to_send);
        
        /**
         * @brief Reconfigures the side data if the configuration changes after constructor called.
         * 
         * @param configuration array
         */
        void reconfigure(uint8_t* config_to_send);
        
        JointData hip;      /**< Data for the hip joint */
        JointData knee;     /**< Data for the knee joint */
        JointData ankle;    /**< Data for the ankle joint */
        JointData elbow;    /**< Data for the elbow joint */
        
        float percent_gait;             /**< Estimate of the percent gait based on heel strike */
        float expected_step_duration;   /**< Estimate of how long the next step will take based on the most recent step times */

        float percent_stance;           /**< Estimate of the percent stance based on heel strike and toe off */
        float expected_stance_duration; /**< Estimate of how long the next stance will take based on the most recent stance times */

        float percent_swing;            /**< Estimate of the percent swing based on toe off and heel strike */
        float expected_swing_duration;  /**< Estimate of how long the next swing will take based on the most recent swing times */
        
        float heel_fsr;                 /**< Calibrated FSR reading for the heel */
        float heel_fsr_upper_threshold; /**< Upper threshold for the heel */
        float heel_fsr_lower_threshold; /**< Lower threshold for the heel */
        float toe_fsr;                  /**< Calibrated FSR reading for the toe */
        float toe_fsr_upper_threshold;  /**< Upper threshold for the toe */
        float toe_fsr_lower_threshold;  /**< Lower threshold for the toe */
        
        bool ground_strike;             /**< Trigger when we go from swing to one FSR making contact. */
        bool toe_strike;                /**< Trigger when we detect toe strike after the last detcted toe off */
        bool toe_off;                   /**< Trigger when we go from toe FSR making contact to swing. */
        bool toe_on;                    /**< Trigger when we go from toe FSR not making contact to making contact */
        bool heel_stance;               /**< High when the heel FSR is in ground contact */
        bool toe_stance;                /**< High when the toe FSR is in ground contact */
        bool prev_heel_stance;          /**< High when the heel FSR was in ground contact on the previous iteration */
        bool prev_toe_stance;           /**< High when the toe FSR was in ground contact on the previous iteration */
        
        bool is_left;                               /**< 1 if the side is on the left, 0 otherwise */
        bool is_used;                               /**< 1 if the side is used, 0 otherwise */
        bool do_calibration_toe_fsr;                /**< Flag for if the toe calibration should be done */
        bool do_calibration_refinement_toe_fsr;     /**< Flag for if the toe calibration refinement should be done */
        bool do_calibration_heel_fsr;               /**< Flag for if the heel calibration should be done */
        bool do_calibration_refinement_heel_fsr;    /**< Flag for if the heel calibration refinement should be done */

        float ankle_angle_at_ground_strike;         /**< Estimated angle of the ankle when at ground strike */
        float expected_duration_window_upper_coeff; /**< Factor to multiply by the expected duration to get the upper limit of the window to determine if a ground strike is considered a new step. */
        float expected_duration_window_lower_coeff; /**< Factor to multiply by the expected duration to get the lower limit of the window to determine if a ground strike is considered a new step. */

        Inclination inclination;        /**< Data for inclination */
};

#endif