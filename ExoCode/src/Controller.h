/**
 * @file Controller.h
 *
 * @brief Declares for the different controllers the exo can use. 
 * Controllers should inherit from _Controller class to make sure the interface is the same.
 * 
 * @author P. Stegall 
 * @date Jan. 2022
*/

#ifndef Controller_h
#define Controller_h

//Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)

#include <Arduino.h>

#include "ExoData.h"
#include "Board.h"
#include "ParseIni.h"
#include <stdint.h>
#include "Utilities.h"
#include "config.h"
#include "Time_Helper.h"
#include <algorithm>
#include <utility>

/**
 * @brief This class defines the interface for controllers.  
 * All controllers must have a:
 * float calc_motor_cmd() that returns a torque cmd in Nm.  
 * 
 */
class _Controller
{
	public:
        /**
         * @brief Constructor 
         * 
         * @param id of the joint being used
         * @param pointer to the full ExoData instance
         */
        _Controller(config_defs::joint_id id, ExoData* exo_data);
		
        /**
         * @brief Virtual destructor is needed to make sure the correct destructor is called when the derived class is deleted.
         */
        virtual ~_Controller(){};
        
        /**
         * @brief Virtual function so that each controller must create a function that will calculate the motor command
         * 
         * @return Torque in Nm.
         */
		virtual float calc_motor_cmd() = 0; 
        
        /**
         * @brief Resets the integral sum for the controller
         */
        void reset_integral(); 
        
    protected:
        
        ExoData* _data;                     /**< Pointer to the full data instance*/
        ControllerData* _controller_data;   /**< Pointer to the data associated with this controller */
        SideData* _side_data;                 /**< Pointer for the side data the controller is associated with */
        JointData* _joint_data;             /**< Pointer to the joint data the controller is associated with */
         
        config_defs::joint_id _id;          /**< Id of the joint this controller is attached to. */
        
        Time_Helper* _t_helper;             /**< Instance of the time helper to track when things happen used to check if we have a set time for the PID */
        float _t_helper_context;            /**< Store the context for the timer helper */
        float _t_helper_delta_t;            /**< Time time since the last event */

        //Values for the PID controller
        float _pid_error_sum = 0;           /**< Summed error term for calucating intergral term */
        float _prev_input;                  /**< Prev error term for calculating derivative */
        float _prev_de_dt;                  /**< Prev error derivative used if the timestep is not good*/
        float _prev_pid_time;               /**< Prev time the PID was called */
        
		long pos;//servo parameters
		unsigned long servoWatch;
		int pos1;
		int pos2;
		bool servo_active = false;
		
        /**
         * @brief calculates the current PID contribution to the motor command. 
         * 
         * @param controller command 
         * @param measured controlled value
         * @param proportional gain
         * @param integral gain
         * @param derivative gain
         */
        float _pid(float cmd, float measurement, float p_gain, float i_gain, float d_gain);
		
		/**
         * @brief actuate the servo motor (might need to move this out of the controller class) 
         * 
         * @param signal pin 
         * @param not used
         * @param start angle
         * @param end angle
         */
		int _servo_runner(uint8_t servo_pin, uint8_t speed_level, long angle_initial, long angle_final);
        
        //Values for the Compact Form Model Free Adaptive Controller
        std::pair<float, float> measurements;
        std::pair<float, float> outputs;
        std::pair<float, float> phi;            /**< Psuedo partial derivative */
        float rho;                              /**< Penalty factor (0,1) */
        float lamda;                            /**< Weighting factor limits delta u */
        float etta;                             /**< Step size constant (0, 1] */
        float mu;                               /**< Weighting factor that limits the variance of u */
        float upsilon;                          /**< A sufficiently small integer ~10^-5 */
        float phi_1;                            /**< Initial/reset condition for estimation of psuedo partial derivitave */
        
        float _cf_mfac(float reference, float current_measurement);
};

/**
 * @brief Terrain Responsive Exoskeleton Controller (TREC) 
 * This controller is for the ankle joint
 *
 * For full details see: "Mixed Terrain Ankle Assistance and Modularity in Wearable Robotics" by Chancelor Frank Cuddeback (https://biomech.nau.edu/)
 *
 * See ControllerData.h for details on the parameters used.
 */
class TREC : public _Controller
{
    public:
        TREC(config_defs::joint_id id, ExoData* exo_data);
        ~TREC(){};

        float calc_motor_cmd();
    
    private:
        void _update_reference_angles(SideData* side_data, ControllerData* controller_data, float percent_grf, float percent_grf_heel);
        void _capture_neutral_angle(SideData* side_data, ControllerData* controller_data);
		void _grf_threshold_dynamic_tuner(SideData* side_data, ControllerData* controller_data, float threshold, float percent_grf_heel);
		void _plantar_setpoint_adjuster(SideData* side_data, ControllerData* controller_data, float pjmcSpringDamper);
};

/**
 * @brief Proportional Joint Moment Controller
 * This controller is for the ankle joint 
 * Applies a plantar torque based on the normalized magnitude of the toe FSR
 *
 * See ControllerData.h for details on the parameters used.
 */
class ProportionalJointMoment : public _Controller
{
    public:
        ProportionalJointMoment(config_defs::joint_id id, ExoData* exo_data);
        ~ProportionalJointMoment(){};
        
        float calc_motor_cmd();
    private:
        std::pair<float, float> _stance_thresholds_left, _stance_thresholds_right;
        
        float _inclination_scaling{1.0f};
};


/**
 * @brief Zero Torque Controller
 * This controller is for the any joint
 * Simply applies zero torque
 * 
 * See ControllerData.h for details on the parameters used.
 */
class ZeroTorque : public _Controller
{
    public:
        ZeroTorque(config_defs::joint_id id, ExoData* exo_data);
        ~ZeroTorque(){};
        
        float calc_motor_cmd();
};

/**
 * @brief Zhang Collins Controller
 * This controller is for the ankle joint 
 * Applies ramp between t0 and t1 to (t1, ts).
 * From t1 to t2 applies a spline going up to (t2,mass*normalized_peak_torque).
 * From t2 to t3 falls to (t3, ts)
 * From t3 to 100% applies zero torque
 * 
 * 2022-02 : This controller was based on:
 * Zhang, J., Fiers, P., Witte, K. A., Jackson, R. W., Poggensee, K. L., Atkeson, C. G., & Collins, S. H. 
 * (2017). Human-in-the-loop optimization of exoskeleton assistance during walking. Science, 356(6344), 1280-1284.
 * and written by P. Stegall
 * 
 * See ControllerData.h for details on the parameters used.
 */
class ZhangCollins: public _Controller
{
    public:
        ZhangCollins(config_defs::joint_id id, ExoData* exo_data);
        ~ZhangCollins(){};
        
        float calc_motor_cmd();

        float _spline_generation(float node1, float node2, float node3, float torque_magnitude, float percent_gait);

        float torque_cmd;
		float cmd;
};

/**
 * @brief Franks Collins Controller
 * This controller is for the Hip Joint
 *
 * Is 0 between t0_trough and t1_trough to (t1, 0).
 * From t1_trough to t2_trough applies a spline going down to (t2_trough,mass*normalized_trough_torque).
 * From t2_trough to t3_trough rises to (t3_trough, 0)
 * From t3_trough to t1_peak applies zero torque
 * From t1_peak to t2_peak applies a spline going up to (t2_peak,mass*normalized_peak_torque).
 * From t2_peak to t3_peak falls to (t3_peak, 0)
 *
 * 2022-04 : This controller was based on:
 * Franks, P. W., Bryan, G. M., Martin, R. M., Reyes, R., Lakmazaheri, A. C., & Collins, S. H.
 * (2021). Comparing optimized exoskeleton assistance of the hip, knee, and ankle in single and multi-joint configurations. Wearable Technologies, 2.
 * and written by P. Stegall
 *
 * See ControllerData.h for details on the parameters used.
 */
class FranksCollinsHip: public _Controller
{
    public:
        FranksCollinsHip(config_defs::joint_id id, ExoData* exo_data);
        ~FranksCollinsHip(){};
       
        float calc_motor_cmd();

        float _spline_generation(float node1, float node2, float node3, float torque_magnitude, float shifted_percent_gait);

        float last_percent_gait;
        float last_start_time;
       
};

class ConstantTorque : public _Controller
{
public:
    ConstantTorque(config_defs::joint_id id, ExoData* exo_data);
    ~ConstantTorque() {};

    float calc_motor_cmd();

    float previous_command;         /* Stores Previous Loop's Torque Command */
    float previous_torque_reading;  /* Stores Previous Loop's Measured Torque */
    int flag;                       /* Flag that Determines Filter Status */
    float difference;               /* Stores Difference in Command when Changed */

};

class ElbowMinMax : public _Controller
{
public:
    ElbowMinMax(config_defs::joint_id id, ExoData* exo_data);
    ~ElbowMinMax() {};

    float alpha0;
    float alpha1;
    float alpha2;
    float alpha3;

    float cmd;

    float Smoothed_Sig_Flex;
    float Smoothed_Sig_Ext;
    float Smoothed_Flex_Max;
    float Smoothed_Flex_Min;
    float Smoothed_Ext_Max;
    float Smoothed_Ext_Min;

    float starttime;

    float check;

    float Angle_Max;
    float Angle_Min;
    float Angle;

    bool flexState;
    bool extState;
    bool nullState;

    float previous_setpoint;

    float fsr_toe_previous_elbow;
    float fsr_heel_previous_elbow;

    float SpringEffect;

    float calc_motor_cmd();
    
};

class CalibrManager : public _Controller
{
public:
    CalibrManager(config_defs::joint_id id, ExoData* exo_data);
    ~CalibrManager() {};

    float calc_motor_cmd();
};

class Chirp : public _Controller
{
public:
    Chirp(config_defs::joint_id id, ExoData* exo_data);
    ~Chirp() {};

    float start_flag;               /* Flag that triggers recording of the initial start time of the controller upon usage. */
    float start_time;               /* Variable that stores the start time of the controller. */
    float current_time;             /* Variable that stores the current time of the controller. */
    float previous_amplitude;       /* Variable that stores the previous amplitude, used as a switch to restart the controller if needed. (Set amplitude to 0 and then set to desired amplitude). */

    float calc_motor_cmd();         /* Function that calculates the motor command. */

};

class Step : public _Controller
{
public:
    Step(config_defs::joint_id id, ExoData* exo_data);
    ~Step() {};

    int n;                          /* Keeps track of how many steps have been performed. */
    int start_flag;                 /* Flag that triggers the recording of the time that the step is first applied. */
    float start_time;               /* Time that the step was first applied. */
    float cmd_ff;                   /* Motor command. */
    float previous_time;            /* Stores time from previous iteration. */
    float end_time;                 /* Records time that step ended. */

    float previous_command;
    float previous_torque_reading;
    int flag;
    float difference;
    float turn;
    float flag_time;
    float change_time;

    float calc_motor_cmd();         /* Function that calculates the motor command. */

};

#endif
#endif