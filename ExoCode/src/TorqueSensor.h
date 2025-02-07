/**
 * @file TorqueSensor.h
 * 
 * @brief Declares the class that is used to interface with a torque sensor.
 * 
 * 
 * @author P. Stegall 
 * @date Jan. 2022
*/


#ifndef TorqueSensor_h
#define TorqueSensor_h

//Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)

#include "Board.h"
#include "Arduino.h"

/**
 * @brief Class used to interface with a torque sensor
 */
class TorqueSensor
{
	public:
		TorqueSensor(unsigned int pin);
        
        /**
         * @brief Does the calibration, while do_calibration is true, and returns 0 when the calibration has finished.
         * This gets the average of the readings during quite standing.
         * 
         * @param should the calibration be done
         *
         * @return 0 if the calibration is finished.
         */
        bool calibrate(bool do_calibration); 
		
        /**
         * @brief Reads the pin and returns the calibrated value.
         * 
         * @return the calibrated torque reading
         */
        float read();
		
		/**
         * @brief Reads the pin and returns the offset value.
         * 
         * @return torque calibration offset
         */
		float readOffset();
		
		/**
         * @brief Reads the pin and returns the calibrated value.
         * 
         * @return the calibrated torque reading (offset pulled from the SD card)
         */
		float read_microSD(float _calibration_microSD);
	    
        int _raw_reading;                   /**< Raw pin reading */
		
	private:
		int _pin;                           /**< Pin to read for the sensor */
        bool _is_used;                      /**< Flag indicating if the sensor is used */
        
        float _calibration;                 /**< Stores the value used for calibration. This is a zero torque offset*/
		float _calibrated_reading;          /**< Torque value with offset applied */
        float _calibrated_reading_microSD;	/**< Torque value with offset (pulled from the SD Card) applied */
		
        const uint16_t _cal_time = 1000;    /**< The time to do the initial calibration in ms*/  
        uint16_t _start_time;               /**< Time the calibration starts. */   
        bool _last_do_calibrate;            /**< This when the calibration ends. */
        float _zero_sum;                    /**< Sum of values over the calibration period used for averaging. */  
        uint32_t _num_calibration_samples;  /**< Number of samples collected during calibration, denominator for averaging. */   
        
};
#endif
#endif