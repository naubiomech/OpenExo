/**
 * @file ControllerData.h
 *
 * @brief Declares a class used to store data for controllers to access 
 * 
 * @author P. Stegall 
 * @date Jan. 2022
*/

#ifndef Controllerdata_h
#define ControllerData_h
#include <stdint.h>

#include "Arduino.h"

#include "Board.h"
#include "ParseIni.h"
#include <stdint.h>

//Forward declaration
class ExoData;

namespace controller_defs                   /**< Stores the parameter indexes for different controllers */
{
    namespace zero_torque
    {
        const uint8_t use_pid_idx = 0;              //Flag to use PID control
        const uint8_t p_gain_idx = 1;               //Value of P Gain for PID control
        const uint8_t i_gain_idx = 2;               //Value of I Gain for PID control
        const uint8_t d_gain_idx = 3;               //Value of D Gain for PID control 
        const uint8_t num_parameter = 4;
    }
    
    namespace proportional_joint_moment
    {
        const uint8_t stance_max_idx = 0;                   //Parameter for peak exo torque during stance 
        const uint8_t swing_max_idx = 1;                    //Parameter for peak exo torque during swing
        const uint8_t is_assitance_idx = 2;                 //When this is 1(assistive) the system will apply the torque in the plantar flexion direction, when 0(resistive) will be in the dorsiflexion direction.
        const uint8_t use_pid_idx = 3;                      //Flag to use PID control
        const uint8_t p_gain_idx = 4;                       //Value of P Gain for PID control
        const uint8_t i_gain_idx = 5;                       //Value of I Gain for PID control
        const uint8_t d_gain_idx = 6;                       //Value of D Gain for PID control 
        const uint8_t torque_alpha_idx = 7;
        const uint8_t num_parameter = 8;
    }

    namespace zhang_collins
    {

        const uint8_t torque_idx = 0;                           //Magnitude of Peak Torque in Nm
        const uint8_t peak_time_idx = 1;                        //Time when peak torque occurs (% of gait cycle)
        const uint8_t rise_time_idx = 2;                        //Time from zero torque until peak torque (Expressed as a % of gait cycle)
        const uint8_t fall_time_idx = 3;                        //Time from peak torque until zero torque (Expressed as a % of gait cycle)
        const uint8_t direction_idx = 4;                        //Flag to flip torque from PF to DF
        const uint8_t use_pid_idx = 5;                          //Flag to use PID control
        const uint8_t p_gain_idx = 6;                           //Value of P Gain for PID control
        const uint8_t i_gain_idx = 7;                           //Value of I Gain for PID control
        const uint8_t d_gain_idx = 8;                           //Value of D Gain for PID control 
        const uint8_t num_parameter = 9;
    }

    namespace franks_collins_hip
    {
        const uint8_t mass_idx = 0;                             //Mass of the User in kg
        const uint8_t trough_normalized_torque_Nm_kg_idx = 1;   //Extension Torque in Nm/kg
        const uint8_t peak_normalized_torque_Nm_kg_idx = 2;     //Flexion Torque in Nm/kg
        const uint8_t start_percent_gait_idx = 3;               //Percent of Gait Cycle where the curve starts (does not start at 0 so that there is no discontinuity)
        const uint8_t trough_onset_percent_gait_idx = 4;        //Percent of Gait Cycle where curve for extension torque starts
        const uint8_t trough_percent_gait_idx = 5;              //Percent of Gait Cycle where peak extension torque occurs
        const uint8_t mid_time_idx = 6;                         //Time from when curve starts until midpoint of zero torque period between extension and flexion torques
        const uint8_t mid_duration_idx = 7;                     //Time of zero torque period between extension and flexion torques
        const uint8_t peak_percent_gait_idx = 8;                //Percent of Gait Cycle where peak flexion torque occurs
        const uint8_t peak_offset_percent_gait_idx = 9;         //Percent of Gait Cycle where torque stops being applied
        const uint8_t use_pid_idx = 10;                         //Flag to determine whether or not PID used
        const uint8_t p_gain_idx = 11;                          //Value of P Gain for PID control
        const uint8_t i_gain_idx = 12;                          //Value of I Gain for PID control
        const uint8_t d_gain_idx = 13;                          //Value of D Gain for PID control 
        const uint8_t num_parameter = 14;                   
    }

    namespace constant_torque
    {
        const uint8_t amplitude_idx = 0;                //Magnitude of the applied torque, in Nm
        const uint8_t direction_idx = 1;                //Flag to flip the direction of the applied torque 
        const uint8_t alpha_idx = 2;                    //Filtering term for exponentially wieghted moving average (EWMA) filter, used on torque sensor to cut down on noise.
        const uint8_t use_pid_idx = 3;                  //Flag to determine whether or not PID used
        const uint8_t p_gain_idx = 4;                   //Value of P Gain for PID control
        const uint8_t i_gain_idx = 5;                   //Value of I Gain for PID control
        const uint8_t d_gain_idx = 6;                   //Value of D Gain for PID control 
        const uint8_t num_parameter = 7;
    }

    namespace elbow_min_max
    {
        const uint8_t FLEXamplitude_idx = 0;            // Flexion Torque setpoint in Nm
        const uint8_t DigitFSR_threshold_idx = 1;       // Grip Upper Threshhold
        const uint8_t PalmFSR_threshold_idx = 2;        // Palm Upper Threshold
        const uint8_t DigitFSR_LOWthreshold_idx = 3;    // Grip lower Threshhold
        const uint8_t PalmFSR_LOWthreshold_idx = 4;     // Palm lower Threshold
        const uint8_t CaliRequest_idx = 5;              // Calibration Request - 1 = factory recalibrate
        const uint8_t TrqProfile_idx = 6;               // Toggles between torque profiles, 1 = spring torque, 0 = constant torque
        const uint8_t P_gain_idx = 7;                   // Proportion gain for closed loop torque control
        const uint8_t I_gain_idx = 8;                   // Integral gain
        const uint8_t D_gain_idx = 9;                   // Differntial gain
        const uint8_t TorqueLimit_idx = 10;             // Setpoint Limiter - max pos/neg amplitude - default 16
        const uint8_t SpringPkTorque_idx = 11;          // Sets the maximum spring torque (Nm)
        const uint8_t EXTamplitude_idx = 12;            // Extension Torque Setpoint in Nm
        const uint8_t FiltStrength_idx = 13;            // Setpoint Filter Strength
        const uint8_t num_parameter = 14;               // Number of unique commands      
    }

    namespace trec 
    {
        const uint8_t plantar_scaling = 0;
        const uint8_t dorsi_scaling = 1;
        const uint8_t timing_threshold = 2;
        const uint8_t spring_stiffness = 3;
        const uint8_t neutral_angle = 4;
        const uint8_t damping = 5;
        const uint8_t propulsive_gain = 6;
        const uint8_t kp = 7;
        const uint8_t kd = 8;
		const uint8_t turn_on_peak_limiter = 9;
        const uint8_t num_parameter = 10;
    }
	
	namespace calibr_manager
	{
		const uint8_t calibr_cmd = 0;
		const uint8_t num_parameter = 1;
	}

    namespace chirp                                                     //Parameters for Sine Wave Used in Chirp Testing
    {
        const uint8_t amplitude_idx = 0;                        //Amplitude, in Nm, of the torque sine wave
        const uint8_t start_frequency_idx = 1;                  //Starting frequency for the chirp  
        const uint8_t end_frequency_idx = 2;                    //Ending frequency for the chirp
        const uint8_t duration_idx = 3;                         //The duration that you want the chirp to be applied
        const uint8_t yshift_idx = 4;                           //Shifts the center of the chirp if you want it to be something other than zero
        const uint8_t pid_flag_idx = 5;                         //Flag to determine whether or not PID used
        const uint8_t p_gain_idx = 6;                           //Value of P Gain for PID control
        const uint8_t i_gain_idx = 7;                           //Value of I Gain for PID control
        const uint8_t d_gain_idx = 8;                           //Value of D Gain for PID control
        const uint8_t num_parameter = 9;
    }

    namespace step                                              //Parameters for step torque used in max torque capacity testing
    {
        const uint8_t amplitude_idx = 0;                        //Magnitude of the applied torque in Nm             
        const uint8_t duration_idx = 1;                         //Duration of the applied torque
        const uint8_t repetitions_idx = 2;                      //Number of times the torque is applied
        const uint8_t spacing_idx = 3;                          //Time between each application of torque
        const uint8_t pid_flag_idx = 4;                         //Flag to determine whether or not PID used
        const uint8_t p_gain_idx = 5;                           //Value of P Gain for PID control
        const uint8_t i_gain_idx = 6;                           //Value of I Gain for PID control
        const uint8_t d_gain_idx = 7;                           //Value of D Gain for PID control
        const uint8_t alpha_idx = 8;                            //Filtering term for exponentially wieghted moving average (EWMA) filter, used on torque sensor to cut down on noise.
        const uint8_t num_parameter = 9;
    }

    namespace proportional_hip_moment
    {
        const uint8_t extension_setpoint_idx = 0;               //Parameter for extension setpoint 
        const uint8_t flexion_setpoint_idx = 1;                 //Parameter for flexion setpoin
        const uint8_t num_parameter = 2;
    }

	namespace spv2 
    {
        const uint8_t plantar_scaling = 0;
        const uint8_t dorsi_scaling = 1;
        const uint8_t timing_threshold = 2;                     //Toe FSR threshold (unit: %)
        const uint8_t spring_stiffness_adj_factor = 3;
        const uint8_t neutral_angle = 4;
		const uint8_t min_angle = 5;
        const uint8_t max_angle = 6;
        const uint8_t kp = 7;
		const uint8_t turn_on_peak_limiter = 9;                 
        const uint8_t kd = 8;
		const uint8_t do_update_stiffness = 10;
		const uint8_t ki = 11;
		const uint8_t do_use_servo = 12;
		const uint8_t fsr_servo_threshold = 13;
		const uint8_t servo_origin = 14;
		const uint8_t servo_terminal = 15;
		const uint8_t motor_current_calc_win = 16;
        const uint8_t num_parameter = 17;
    }

    const uint8_t max_parameters = spv2::num_parameter;         //This should be the largest of all the num_parameters
}

/**
 * @brief class to store information related to controllers.
 * 
 */
class ControllerData {
	public:
        ControllerData(config_defs::joint_id id, uint8_t* config_to_send);
        
        /**
         * @brief Reconfigures the the controller data if the configuration changes after constructor called.
         * 
         * @param configuration array
         */
        void reconfigure(uint8_t* config_to_send);

        /**
         * @brief Get the parameter length for the current controller
         * 
         * @return uint8_t parameter length 
         */
        uint8_t get_parameter_length();
        
        
        uint8_t controller;                                 /**< Id of the current controller */
        config_defs::JointType joint;                       /**< Id of the current joint */

        float setpoint;                                     /**< Controller setpoint, basically the motor command. */
        float ff_setpoint;                                  /**< Feed forwared setpoint, only updated in closed loop controllers */
        float parameters[controller_defs::max_parameters];  /**< Parameter list for the controller see the controller_defs namespace for the specific controller. */
        uint8_t parameter_set;                              /**< Temporary value used to store the parameter set while we are pulling from the sd card. */

        float filtered_torque_reading;                      /**< Filtered torque reading, used for filtering torque signal */
        float filtered_cmd;                                 /**< Filtered command, used for filtering motor commands */
        float filtered_setpoint;                            /**< Filtered setpoint for the controller */
        
        //Variables for Auto Kf in the PID Controller
        float kf = 1;                                       /**< Gain for the controller */
        float prev_max_measured = 0;                        /**< Previous max measured value */
        float prev_max_setpoint = 0;                        /**< Previous max setpoint value */
        float max_measured = 0;                             /**< Max measured value */
        float max_setpoint = 0;                             /**< Max setpoint value */

        /* Controller Specific Variables That You Want To Plot. If you do not want to plot, than put variables in Controller.h under the controller of interest. */

        //Variables for TREC Controller (MOVE TO Controller.h)
        float reference_angle = 0;                              /**< Reference angle for the spring term */
        float reference_angle_offset = 0;                       /**< Offset for the reference angle */
        bool reference_angle_updated = false;                   /**< Flag to indicate if the reference angle was updated this step */
        float filtered_squelched_supportive_term = 0;           /**< Low pass on final spring output */
        float neutral_angle = 0.0f;                             /**< Neutral angle for the spring term */
        bool prev_calibrate_trq_sensor = false;                 /**< Previous value of the calibrate torque sensor flag */
        const float cal_neutral_angle_alpha = 0.01f;            /**< Alpha for the low pass on the neutral angle calibration */
        float level_entrance_angle = 0.0f;                      /**< Level entrance angle for the spring term */
        bool prev_calibrate_level_entrance = false;             /**< Previous value of the calibrate level entrance flag */
        const float cal_level_entrance_angle_alpha = 0.01f;     /**< Alpha for the low pass on the level entrance calibration */
		float stateless_pjmc_term = 0;
		float toeFsrThreshold = 0.2f;
		bool wait4HiHeelFSR = false;
		uint16_t iPidHiTorque = 0;
		bool pausePid = false;
		float currentTime = 0.0000f;
		float previousTime = 0.0000f;
		float itrTime = 0.0000f;
		uint8_t numBelow500 = 0;
		int maxTorqueCache = 0;
		float previousMaxCmdCache = 15;
		float previousMinCmdCache = -15;
		float currentMaxCmdCache = 0;
		float currentMinCmdCache = 0;
		uint16_t cmdCacheItr = 0;
		bool doIncrUpperLmt = false;
		bool doIncrLowerLmt = false;
		float setpoint2use = 0;
		float maxPjmcSpringDamper = 0;
		bool wasStance = false;
		float prevMaxPjmcSpringDamper = 0;
		float cmd_2nd = 0;
		float cmd_1st = 0;	

        //Variables for the ElbowMinMax Controller
        float FlexSense;
        float ExtenseSense;
		
		//Variables for the Calibration Manger "Controller"
		bool calibrComplete = false;
		uint16_t iCalibr = 0;
		int PIDMLTPLR = 0;
		bool calibrStart = false;
		float calibrSum = 0;
		
		//Variables for the Zhang-Collins Controller
		float previous_cmd = 0;
		
        //Variables for SPV2
		int plotting_scalar = 1;                //Maxon servo interrupter
		unsigned long servo_departure_time;
		bool servo_did_go_down = true;
		bool servo_get_ready = false;
		float setpoint2use_spv2 = 0;			//Peak prescribed torque regulator
		bool wasStance_spv2 = false;
		float oldMaxPrescription = 0;
		float currentMaxPrescription = 0;
		unsigned long SPV2_motor_current;
		unsigned long SPV2_motor_current_count = 0;
		bool SPV2_motor_current_ready = false;
		uint16_t SPV2_oldCurrent = 0;
		uint16_t SPV2_newCurrent = 10000;
		uint8_t SPV2_currentAngle = 90;
		bool SPV2_do_count_steps = true;
		uint16_t SPV2_step_count = 0;
		//bool SPV2_servo1_counter_1stStage = false;
		//unsigned long SPV2_servo1_stopWatch;
		//bool SPV2_stiffness_adjustment_ready = false;
		bool SPV2_do_calc_new_stiffness = false;
		bool SPV2_iniAngle_imported = false;
		int8_t do_adv_optimizer = 0;
		uint8_t x1 = 90;
		uint8_t x2 = 90;
		bool SPV2_gs_is_ini_itr = true;
		uint16_t x1_current = 0;
		uint16_t x2_current = 0;
		uint8_t x_l = 90;
		uint8_t x_u = 90;
		
		//SPV2 Simulated Annealing Optimizer
		float curr = 90;
		float curr_eval = 0;
		float candidate = 90;
		float candidate_eval = 0;
		uint16_t i_SA = 0;
		float best = 90;
		float best_eval = 10000;
		
		//SPV2 Cost Function
		float SPV2_error_sum = 0;
		unsigned long SPV2_error_count = 0;
		float SPV2_RMSE = 0;
		float SPV2_CF_output = 0;
		

        //Variables for the PHMC Controller
        float fs;
        float state;
};      

#endif