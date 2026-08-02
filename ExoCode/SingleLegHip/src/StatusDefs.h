/**
 * @file StatusDefs.h
 *
 * @brief Stores the different status messages for the system
 * 
 * @author P. Stegall 
 * @date Dec. 2021
*/


#ifndef StatusDefs_h
#define StatusDefs_h

#include "Arduino.h"
#include "Board.h"

/**
 * @brief stores the status messages
 */
namespace status_defs
{
    /**
     * @brief stores the status messages
     */
    namespace messages 
    {
        
        //0b error ,trial_on, trial_off
        const uint16_t off =  0;                /**< Off status id */
        const uint16_t trial_off = 1;           /**< Trial off status id */
        const uint16_t trial_on = 2;            /**< Trial on status id */
        const uint16_t test = 3;                /**< Test status id, generally won't be used.*/
        const uint16_t torque_calibration = 4;  /**< Torque calibration status id */
        const uint16_t fsr_calibration = 5;     /**< Fsr calibration status id */
        const uint16_t fsr_refinement =6;       /**< Fsr refinement status id */
        const uint16_t motor_start_up =7;       /**< Motor startup status id */

        //Specific error messages will use the 4 highest bits, giving 31 error messages
        const uint16_t error_bit = 4;               /**< Bit to indicate an error, gives 12 bits of error messages*/ 
        const uint16_t error = 1<<(error_bit-1);    /**< General error message status id */
        
        const uint16_t error_left_heel_fsr =  1<<error_bit | error;     /**< Left heel fsr error id */
        const uint16_t error_left_toe =  2<<error_bit | error;          /**< Left toe fsr error id */
        const uint16_t error_right_heel_fsr =  3<<error_bit | error;    /**< Right heel fsr error id */
        const uint16_t error_right_toe_fsr =  4<<error_bit | error;     /**< Right toe fsr error id */
        
        const uint16_t error_left_hip_torque_sensor =  5<<error_bit | error;        /**< Left hip torque sensor error id */
        const uint16_t error_left_knee_torque_sensor =  6<<error_bit | error;       /**< Left knee torque sensor  error id */
        const uint16_t error_left_ankle_torque_sensor =  7<<error_bit | error;      /**< Left ankle torque sensor  error id */
        const uint16_t error_left_elbow_torque_sensor = 8<< error_bit | error;      /**< Left elbow torque sensor  error id */
        const uint16_t error_right_hip_torque_sensor =  9<<error_bit | error;       /**< Right hip torque sensor  error id */
        const uint16_t error_right_knee_torque_sensor =  10<<error_bit | error;     /**< Right knee torque sensor  error id */
        const uint16_t error_right_ankle_torque_sensor =  11<<error_bit | error;    /**< Right ankle torque sensor  error id */
        const uint16_t error_right_elbow_torque_sensor = 12<< error_bit | error;    /**< Right elbow torque sensor  error id */
        
        const uint16_t error_left_hip_motor =  13<<error_bit | error;               /**< Left hip motor error id */
        const uint16_t error_left_knee_motor =  14<<error_bit | error;              /**< Left knee motor error id */
        const uint16_t error_left_ankle_motor =  15<<error_bit | error;             /**< Left ankle motor error id */
        const uint16_t error_left_elbow_motor = 16<< error_bit | error;             /**< Left elbow motor error id */
        const uint16_t error_right_hip_motor =  17<<error_bit | error;              /**< Right hip motor error id */
        const uint16_t error_right_knee_motor =  18<<error_bit | error;             /**< Right knee motor error id */
        const uint16_t error_right_ankle_motor =  19<<error_bit | error;            /**< Right ankle motor error id */
        const uint16_t error_right_elbow_motor = 20<< error_bit | error;            /**< Right elbow motor error id */
        
        const uint16_t error_left_hip_controller =  21<<error_bit | error;          /**< Left hip controller error id */
        const uint16_t error_left_knee_controller =  22<<error_bit | error;         /**< Left knee controller error id */
        const uint16_t error_left_ankle_controller =  23<<error_bit | error;        /**< Left ankle controller error id */
        const uint16_t error_left_elbow_controller = 24<< error_bit | error;        /**< Left elbow controller error id */
        const uint16_t error_right_hip_controller =  25<<error_bit | error;         /**< Right hip controller error id */
        const uint16_t error_right_knee_controller =  26<<error_bit | error;        /**< Right knee controller error id */
        const uint16_t error_right_ankle_controller =  27<<error_bit | error;       /**< Right ankle controller error id */
        const uint16_t error_right_elbow_controller = 28<< error_bit | error;       /**< Right elbow controller error id */
        
        const uint16_t error_to_be_used_1 =  29<<error_bit | error;     /**< Placeholder error id */
        const uint16_t error_to_be_used_2 =  30<<error_bit | error;     /**< Placeholder error id */
        const uint16_t error_to_be_used_3 =  31<<error_bit | error;     /**< Placeholder error id */
        const uint16_t error_to_be_used_4 =  32<<error_bit | error;    /**< Placeholder error id */
        const uint16_t error_to_be_used_5 =  33<<error_bit | error;    /**< Placeholder error id */
        const uint16_t error_to_be_used_6 =  34<<error_bit | error;    /**< Placeholder error id */
        const uint16_t error_to_be_used_7 =  35<<error_bit | error;    /**< Placeholder error id */
        
        const uint16_t warning_bit = error_bit + 6 + 1;                     /**< Bit to indicate warning, error bit plus 2^6-1 error messages, bit 11 for warning and 2^5-1 warning messages.*/
        const uint16_t warning = 1<<(warning_bit-1);                        /**< General warning id */
        const uint16_t warning_exo_run_time =  1<<warning_bit | warning;    /**< Running time overflow warning id */
        
        const uint16_t warning_to_be_used_1 =  2<<warning_bit | warning;    /**< Placeholder warning id */
        const uint16_t warning_to_be_used_2 =  3<<warning_bit | warning;    /**< Placeholder warning id */
        const uint16_t warning_to_be_used_3 =  4<<warning_bit | warning;    /**< Placeholder warning id */
        const uint16_t warning_to_be_used_4 =  5<<warning_bit | warning;    /**< Placeholder warning id */
        const uint16_t warning_to_be_used_5 =  6<<warning_bit | warning;    /**< Placeholder warning id */
        const uint16_t warning_to_be_used_6 =  7<<warning_bit | warning;    /**< Placeholder warning id */
        const uint16_t warning_to_be_used_7 =  8<<warning_bit | warning;    /**< Placeholder warning id */
        const uint16_t warning_to_be_used_8 =  9<<warning_bit | warning;    /**< Placeholder warning id */
        const uint16_t warning_to_be_used_9 =  10<<warning_bit | warning;   /**< Placeholder warning id */
        const uint16_t warning_to_be_used_10 =  11<<warning_bit | warning;  /**< Placeholder warning id */
        const uint16_t warning_to_be_used_11 =  12<<warning_bit | warning;  /**< Placeholder warning id */
        const uint16_t warning_to_be_used_12 =  13<<warning_bit | warning;  /**< Placeholder warning id */
        const uint16_t warning_to_be_used_13 =  14<<warning_bit | warning;  /**< Placeholder warning id */
        const uint16_t warning_to_be_used_14 =  15<<warning_bit | warning;  /**< Placeholder warning id */
    }
}

/**
 * @brief prints the status message
 *
 * @param message id
 */
void print_status_message(uint16_t message);

#endif
