/**
 * @file FSR.h
 *
 * @brief Declares classes used to interface with a force sensitive resistor, note there is a regressed version and a non-regressed version
 * 
 * @author P. Stegall 
 * @date Jan. 2022
*/


#ifndef FSR_h
#define FSR_h

//Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)

#include "Arduino.h"
#include "Board.h"
#include "Utilities.h"
#include "ExoData.h"

/**
    * @brief Handles raw (non-regressed) FSR signal. 
    * 
    */
class FSR
{
	public:
		FSR(int pin);
		
        /**
         * @brief Does an initial time based calculation to find a rough range for the signal
         * Person should be walking
         * These values will be used for tracking the number of transitions for the calibration refinement.
         * 
         * @param if the calibration is active.
         * 
         * @return if the calibration is continuing.
         */
        bool calibrate(bool do_calibrate); 
		
        /**
         * @brief Does a refinement of the calibration based on averaging across an number of steps.
         * Person should be walking.
         * Refines the calibration, by finding the max and min over a set number of low to high transitions
         * 
         * @param if the calibration is active.
         * 
         * @return if the calibration is continuing.
         */
        bool refine_calibration(bool do_refinement);
        
        /**
         * @brief Reads the sensor and applies the calibration.  
         * If the refinement isn't done returns the regular calibration, 
         * If the regular calibration isn't done returns the raw value.
         * 
         * @return the sensor reading
         */
        float read(); //Reads the pins and updates the data object
		
        /**
         * @brief Uses a schmitt trigger to determine if the sensor is in contact with the ground (foot/shoe)
         * 
         * @return if the FSR is in contact with the ground
         */
        bool get_ground_contact();

        /**
         * @brief Get the thresholds for the schmitt trigger
         * 
         * @param lower_threshold_percent_ground_contact lower threshold for the schmitt trigger
         * @param upper_threshold_percent_ground_contact upper threshold for the schmitt trigger
         */
        void get_contact_thresholds(float &lower_threshold_percent_ground_contact, float &upper_threshold_percent_ground_contact);
		
        /**
         * @brief Set the thresholds for the schmitt trigger
         * 
         * @param lower_threshold_percent_ground_contact lower threshold
         * @param upper_threshold_percent_ground_contact uppder threshold
         */
        void set_contact_thresholds(float lower_threshold_percent_ground_contact, float upper_threshold_percent_ground_contact);
	
    private:
		/**
         * @brief Calculates if the fsr is in contact with the ground based on a schmitt trigger
         * This is called in read()
         * 
         * @return if the sensor is in contact with the ground
         */
        bool _calc_ground_contact();  

        //Stores the sensor readings
        float _raw_reading;             /**< Current raw sensor reading */
		float _calibrated_reading;      /**< Sensor reading with calibration applied */
        
        int _pin;                       /**< The pin the sensor is connected to */
        
        //Used for calibration
        const uint16_t _cal_time = 5000;    /**< This is time to do the initial calibration */
        uint16_t _start_time;               /**< Stores the time we started the calibration */
        bool _last_do_calibrate;            /**< Used to find rising edge for calibration */
        float _calibration_min;             /**< Minimum value during the time period */
        float _calibration_max;             /**< Maximum value during the time period */
        
        //Used for calibration refinement
        const uint8_t _num_steps = 7;                                       /**< This is the number of steps to do the calibration_refinement */
        const float _lower_threshold_percent_calibration_refinement = .33;  /**< Lower threshold for the schmitt trigger. This can be relatively high since we don't really care about the exact moment the ground contact happens. */
        const float _upper_threshold_percent_calibration_refinement = .66;  /**< Upper threshold for the schmitt trigger */
        bool _state;                                                        /**< Stores the signal high/low state from the schmitt trigger to find when there is a new step. */
        bool _last_do_refinement;                                           /**< Used to track the rising edge of do_refinement, so we can reset on the first run. */
        unsigned int _step_max_sum;                                         /**< Stores the running sum of maximums from each step so we can average. */
        uint16_t _step_max;                                                 /**< Keeps track of the max value for the step. */
        unsigned int _step_min_sum;                                         /**< Stores the running sum of minimums from each step so we can average. */
        uint16_t _step_min;                                                 /**< Keeps track of the min value for the step. */
        uint8_t _step_count;                                                /**< Used to track if we have done the required number of steps. */
        float _calibration_refinement_min;                                  /**< The refined min used for doing the calibration */
        float _calibration_refinement_max;                                  /**< The refined max used for doing the calibration */
        
        //Used for ground_contact()
        bool _ground_contact;                                   /**< Is the FSR in contact with the ground */
        const uint8_t _ground_state_count_threshold = 4;        /**< Used to track if the FSR has been in contact with the ground for a while. */
        float _lower_threshold_percent_ground_contact = .15;    /**< Lower threshold for the schmitt trigger. This should be relatively low as we want to detect toe off as closely as possible. */
        float _upper_threshold_percent_ground_contact = .25;    /**< Should be slightly higher than the lower threshold but by as little as you can get by with as the sensor must go above this value to register contact. */
};

/**
    * @brief Handles regressed FSR signal.
    * This is used for PJMC controller and is dependent on the type of FSR being used.
    * Current regression equation is for: Interlink 
    * 
    */
class FSR_Regressed
{
	public:
		FSR_Regressed(int pin);
		
        /**
         * @brief Does an initial time based calculation to find a rough range for the signal
         * Person should be walking
         * These values will be used for tracking the number of transitions for the calibration refinement.
         * 
         * @param if the calibration is active.
         * 
         * @return if the calibration is continuing.
         */
        bool calibrate(bool do_calibrate); 
		
        /**
         * @brief Does a refinement of the calibration based on averaging across an number of steps.
         * Person should be walking.
         * Refines the calibration, by finding the max and min over a set number of low to high transitions
         * 
         * @param if the calibration is active.
         * 
         * @return if the calibration is continuing.
         */
        bool refine_calibration(bool do_refinement);
        
        /**
         * @brief Reads the sensor and applies the calibration.  
         * If the refinement isn't done returns the regular calibration, 
         * if the regular calibration isn't done returns the raw value.
         * 
         * @return the sensor reading
         */
        float read(); //Reads the pins and updates the data object
		
        /**
         * @brief Uses a schmitt trigger to determine if the sensor is in contact with the ground (foot/shoe)
         * 
         * @return if the FSR is in contact with the ground
         */
        bool get_ground_contact();

        /**
         * @brief Get the thresholds for the schmitt trigger
         * 
         * @param lower_threshold_percent_ground_contact lower threshold for the schmitt trigger
         * @param upper_threshold_percent_ground_contact upper threshold for the schmitt trigger
         */
        void get_contact_thresholds(float &lower_threshold_percent_ground_contact, float &upper_threshold_percent_ground_contact);
		
        /**
         * @brief Set the thresholds for the schmitt trigger
         * 
         * @param lower_threshold_percent_ground_contact lower threshold
         * @param upper_threshold_percent_ground_contact uppder threshold
         */
        void set_contact_thresholds(float lower_threshold_percent_ground_contact, float upper_threshold_percent_ground_contact);
	
    private:
		/**
         * @brief Calculates if the fsr is in contact with the ground based on a schmitt trigger
         * This is called in read()
         * 
         * @return if the sensor is in contact with the ground
         */
        bool _calc_ground_contact();  

        //Stores the sensor readings
		float _raw_reading;         /**< Current raw sensor reading */
		float _calibrated_reading;  /**< Sensor reading with calibration applied */
        
        int _pin;                   /**< The pin the sensor is connected to. */
        
        //Used for calibration
        const uint16_t _cal_time = 5000;    /**< This is time to do the initial calibration */
        uint16_t _start_time;               /**< Stores the time we started the calibration */
        bool _last_do_calibrate;            /**< Used to find rising edge for calibration */
		float _calibration_min;             /**< Minimum value during the time period */
		float _calibration_max;             /**< Maximum value during the time period */
        
        //Used for calibration refinement
        const uint8_t _num_steps = 7;                                       /**< This is the number of steps to do the calibration_refinement */
        const float _lower_threshold_percent_calibration_refinement = .33;  /**< Lower threshold for the schmitt trigger. This can be relatively high since we don't really care about the exact moment the ground contact happens. */
        const float _upper_threshold_percent_calibration_refinement = .66;  /**< Upper threshold for the schmitt trigger */
        bool _state;                                                        /**< Stores the signal high/low state from the schmitt trigger to find when there is a new step. */
        bool _last_do_refinement;                                           /**< Used to track the rising edge of do_refinement, so we can reset on the first run. */
        unsigned int _step_max_sum;                                         /**< Stores the running sum of maximums from each step so we can average. */
        uint16_t _step_max;                                                 /**< Keeps track of the max value for the step. */
        unsigned int _step_min_sum;                                         /**< Stores the running sum of minimums from each step so we can average */
        uint16_t _step_min;                                                 /**< Keeps track of the min value for the step. */
        uint8_t _step_count;                                                /**< Used to track if we have done the required number of steps. */
        float _calibration_refinement_min;                                  /**< The refined min used for doing the calibration */
        float _calibration_refinement_max;                                  /**< The refined max used for doing the calibration */
        
        //Used for ground_contact()
        bool _ground_contact;                                   /**< Is the FSR in contact with the ground */
        const uint8_t _ground_state_count_threshold = 4;        /**< Used to track if the FSR has been in contact with the ground for a while. */
        float _lower_threshold_percent_ground_contact = .15;    /**< Lower threshold for the schmitt trigger. This should be relatively low as we want to detect as close to ground contact as possible. */
        float _upper_threshold_percent_ground_contact = .25;    /**< Should be slightly higher than the lower threshold but by as little as you can get by with as the sensor must go above this value to register contact. */
        
};
#endif
#endif