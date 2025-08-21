/**
 * @file Joint.h
 *
 * @brief Declares a class for controlling and reading sensors for a joint
 * 
 * @author P. Stegall 
 * @date Jan. 2022
*/

#ifndef Joint_h
#define Joint_h

//Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)

#include "Arduino.h"

#include "ExoData.h"
#include "Motor.h"
#include "Controller.h"
#include "TorqueSensor.h"
#include "ParseIni.h"
#include "board.h"
#include "Joint.h"
#include "config.h"
#include "Utilities.h"
#include "StatusDefs.h"
#include "ErrorManager.h"
#include <Wire.h>

#include "AnkleAngles.h"
#include "AnkleIMU.h"

#include <stdint.h>

/**
 * @brief Abstract class used to define the interface for the other joints.
 * All joints should have a:
 * void run_joint()
 * void read_data()
 * check_calibration()
 * void set_controller(uint8_t)
 * void set_motor(_Motor*);
 * 
 */
class _Joint
{
	static uint8_t left_torque_sensor_used_count;   /**< Used to record how many sensors are already set */
    static uint8_t right_torque_sensor_used_count;  /**< Used to record how many sensors are already set */
    
    static uint8_t left_motor_used_count;           /**< Used to record how many motors are already set for torque sensor pin assignment */
    static uint8_t right_motor_used_count;          /**< Used to record how many motors are already set for torque sensor pin assignment */
    
    public:
		/**
         * @brief Constructor 
         * 
         * @param id of the joint being used
         * @param pointer to the full ExoData instance
         */
        _Joint(config_defs::joint_id id, ExoData* exo_data);  //Constructor:  
		virtual ~_Joint(){};
        
        /**
         * @brief Updates the controller and sends the motor command
         */
        virtual void run_joint() = 0;   
		
        /**
         * @brief Reads data from motor and sensors
         */
        virtual void read_data(); 

        /**
         * @brief Checks if we need to do the calibration for the motor and sensors and runs the calibration.
         */
        virtual void check_calibration();         
		
        /**
         * @brief Changes the high level controller in Controller, and the low level controller in Motor
         * 
         * @param controller id for that joint
         */
        virtual void set_controller(uint8_t) = 0;  
		
        /**
         * @brief Sets the motor to use. Not strictly needed since everything stays internal.
         *
         * @param pointer to motor instance
         */
        void set_motor(_Motor* new_motor);
        
        /**
         * @brief Takes in the joint id and exo data, and checks if the current joint is used.
         * If it is used it pulls the next open torque sensor pin for the side, and increments the counter.
         * If the joint is not used, or we have used up all the available torque sensor pins for the side, it sets the pin to a pin that is not connected.
         *
         * @param joint id
         * @param ExoData pointer
         * 
         * @return next pin to assign
         */
        static unsigned int get_torque_sensor_pin(config_defs::joint_id, ExoData*);
        
        /**
         * @brief Takes in the joint id and exo data, and checks if the current joint is used.
         * If it is used it pulls the next open motor enable pin for the side, and increments the counter.
         * If the joint is not used, or we have used up all the available pins for the side, it sets the pin to a pin that is not connected.
         *
         * @param joint id
         * @param ExoData pointer
         * 
         * @return next pin to assign
         */
        static unsigned int get_motor_enable_pin(config_defs::joint_id, ExoData*);

        /** MOVE THESE BACK TO PROTECTED WHEN APP IS READY*/
        _Motor* _motor;                 /**< Pointer to the base _Motor class so we can use any motor type.*/
        TorqueSensor _torque_sensor;    /**< Torque sensor for the joint*/
        _Controller* _controller;       /**< Pointer to the current controller. Using pointer so we just need to change the object we are pointing to when the controller changes.*/

        
    protected:
        //Give access to the larger data object and the joint specific data 
        ExoData* _data;                 /**< Pointer to the full data instance*/
        JointData* _joint_data;         /**< Pointer to this joints data */
        ErrorManager _error_manager;    /**< Error manager for the joint */
        
        //Joint info
        config_defs::joint_id _id;      /**< Joint id */
        bool _is_left;                  /**< If the joint is on the left side so we don't have to keep calculating it */
    
};

/**
 * @brief Class for the hip joint which contains joint specific controllers.
 */
class HipJoint : public _Joint
{
    public:
        HipJoint(config_defs::joint_id id, ExoData* exo_data);
        ~HipJoint(){};
        
        /**
         * @brief Reads the sensors for the joint and sends a torque command, See _Joint
         */
        void run_joint();  
        
        /**
         * @brief Sets the controller that is to be used, See _Joint
         *
         * @param controller id.
         */
        void set_controller(uint8_t); 

    protected:
        
        //Objects for joint specific controllers
        ZeroTorque _zero_torque;                        /**< Zero torque controller */
        FranksCollinsHip _franks_collins_hip;           /**< Franks Collins Hip controller */
        ConstantTorque _constant_torque;                /**< Constant torque controller */
        Chirp _chirp;                                   /**< Chirp Controller for Device Characterization */                    
        Step _step;                                     /**< Step Controller for Device Characterization */
        ProportionalHipMoment _proportional_hip_moment; /**< Proportional Hip Moment Conroller */
        CalibrManager _calibr_manager;                  /**< Calibration Manager "Controller" */
        AngleBased _angle_based;                        /**< Angle Based controller */
        
        const int right_hip_nano = 8;                   //interger address for the hip nano relevant to the side the nano is on
        const int left_hip_nano = 9;

        union {                                         //data type to convert from i2c read data to float representing the hip data (estim angle comes in deg)
            float estim_angle;
            byte angle_bytes[4];
        }data;

        unsigned long prevtime = 0;

        void read_imu();                                /**< function for reading angle data from the imu */
        
};

/**
 * @brief Class for the knee joint which contains joint specific controllers.
 */
class KneeJoint : public _Joint
{
    public:
        KneeJoint(config_defs::joint_id id, ExoData* exo_data);
        ~KneeJoint(){};
        
        /**
         * @brief Reads the sensors for the joint and sends a torque command, See _Joint
         */
        void run_joint();
        
        /**
         * @brief Sets the controller that is to be used, See _Joint
         *
         * @param controller id.
         */
        void set_controller(uint8_t);
	
    protected:
        
        //Objects for joint specific controllers	
        ZeroTorque _zero_torque;                /**< Zero torque controller */
        ConstantTorque _constant_torque;        /**< Constant torque controller */
        Chirp _chirp;                           /**< Chirp Controller for Device Characterization */ 
        Step _step;                             /**< Step Controller for Device Characterization */
		CalibrManager _calibr_manager;          /**< Calibration Manager "Controller" */
};

/**
 * @brief Class for the ankle joint which contains joint specific controllers.
 */ 
class AnkleJoint : public _Joint
{
    public:
        AnkleJoint(config_defs::joint_id id, ExoData* exo_data);
        ~AnkleJoint(){};
        
        /**
         * @brief Reads the sensors for the joint and sends a torque command, See _Joint
         */
        void run_joint();
        
        /**
         * @brief Sets the controller that is to be used, See _Joint
         *
         * @param controller id.
         */
        void set_controller(uint8_t);
		
    protected:
        
        //Ankle IMU Information
        AnkleIMU _imu;
        const float _imu_sample_rate_us{15000.0f};
        float _previous_sample_us{0.0f};

        //Ankle Angle Infromation
        AnkleAngles _ankle_angle;

        //Objects for joint specific controllers
        ZeroTorque _zero_torque;                                /**< Zero torque controller */
        ProportionalJointMoment _proportional_joint_moment;     /**< Proportional joint moment controller */
        ZhangCollins _zhang_collins;                            /**< Zhang Collins controller */
        ConstantTorque _constant_torque;                        /**< Constant torque controller*/
        TREC _trec;                                             /**< TREC */
		CalibrManager _calibr_manager;                          /**< Calibration Manager "Controller" */
        Chirp _chirp;                                           /**< Chirp Controller for Device Characterization */
        Step _step;                                             /**< Step Controller for Device Characterization */
        SPV2 _spv2;												/**< SPV2 */
		PJMC_PLUS _pjmc_plus;									/**< The new proportional joint moment controller */
};

/**
 * @brief Class for the elbow joint which contains joint specific controllers.
 */
class ElbowJoint : public _Joint
{
public:
    ElbowJoint(config_defs::joint_id id, ExoData* exo_data);
    ~ElbowJoint() {};

    /**
     * @brief Reads the sensors for the joint and sends a torque command, See _Joint
     */
    void run_joint();

    /**
     * @brief Sets the controller that is to be used, See _Joint
     *
     * @param controller id.
     */
    void set_controller(uint8_t);

protected:

    //Objects for joint specific controllers
    ZeroTorque _zero_torque;                                /**< Zero torque controller */
    ElbowMinMax _elbow_min_max;                             /**< Elbow Flexion/Extension Controller for Lifting */
    CalibrManager _calibr_manager;                          /**< Calibration Manager "Controller" */
    Chirp _chirp;                                           /**< Chirp Controller for Device Characterization */
    Step _step;                                             /**< Step Controller for Device Characterization */

};

#endif
#endif