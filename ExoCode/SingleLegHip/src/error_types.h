/**
 * @file error_types.h
 * @author Chancelor Cuddeback
 * @brief Defines the abstract class for the error types, and implements the error types.
 * 
 */
#ifndef ERROR_TYPES_H
#define ERROR_TYPES_H
#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)

#include "JointData.h"
#include "error_codes.h"
#include "Utilities.h"
#include "Logger.h"

//Create abstract class for error types
class ErrorType
{
    public:
        ErrorType() {};
        virtual bool check(JointData* _data) = 0;
        virtual void handle(JointData* _data) = 0;
};

class TestError : public ErrorType
{
    public:
        TestError() : ErrorType() {};

        bool check(JointData* _data)
        {
            //return millis() > 45000;
            return false;
        }
        void handle(JointData* _data)
        {
            _data->motor.enabled = false;
            logger::println("Test Error", LogLevel::Error);
        }
};

class PoorStateVarianceError : public ErrorType
{
    public:
        PoorStateVarianceError() : ErrorType() {};

        bool check(JointData* _data)
        {

            return false;
        }
        void handle(JointData* _data)
        {

        }
};

class PoorTransmissionEfficiencyError : public ErrorType
{
    public:
        PoorTransmissionEfficiencyError() : ErrorType() {};

        bool check(JointData* _data)
        {
            //Calcualate motor torque
            const float motor_torque = _data->motor.i * _data->motor.kt;

            //Low pass motor torque
            _data->smoothed_motor_torque = utils::ewma(motor_torque, _data->smoothed_motor_torque, _data->motor_torque_smoothing);
        
            //If average motor torque is not close to 0, calculate the transmission efficiency
            const float torque_error = utils::is_close_to(_data->smoothed_motor_torque, 0, _data->close_to_zero_tolerance) ? (0) : abs((float(_data->smoothed_motor_torque) - float(_data->torque_reading)) / float(_data->smoothed_motor_torque));
            const uint8_t _id = static_cast<uint8_t>(_data->id);
            _data->torque_error = utils::ewma(torque_error, _data->torque_error, _data->torque_error_smoothing);

            return false;  //abs(torque_error) > 100 * (1 - _data->transmission_efficiency_threshold); //This should be the normal method for handling this error but we encountered issues so currently set to do nothing
        }
        void handle(JointData* _data)
        {
            //_data->motor.enabled = false;
            logger::println("Transmission Efficiency Error", LogLevel::Error);
        }
};

class TorqueOutOfBoundsError : public ErrorType
{
    public:
        TorqueOutOfBoundsError() : ErrorType() {};

        bool check(JointData* _data)
        {
            return abs(_data->torque_reading) > _data->torque_output_threshold;
        }
        void handle(JointData* _data)
        {
            //_data->motor.enabled = false;
            logger::println("Torque Out of Bounds Error", LogLevel::Error);
        }
};

class TorqueVarianceError : public ErrorType
{
    public:
        TorqueVarianceError() : ErrorType() {};

        bool check(JointData* _data)
        {
            //Append new torque reading to the window
            _data->torque_data_window.push(_data->torque_reading);

            //If the window is larger than the max size, pop the oldest value
            if (_data->torque_data_window.size() > _data->torque_data_window_max_size)
            {
                _data->torque_data_window.pop();
            
                //Calculate the standard deviation of the window
                std::pair<float, float> pop_vals = utils::online_std_dev(_data->torque_data_window);

                //Generate symmetric bounds around the mean
                std::pair<float, float> bounds = std::make_pair(pop_vals.first - _data->torque_std_dev_multiple*pop_vals.second, pop_vals.first + _data->torque_std_dev_multiple*pop_vals.second);
            
                //Increment the failure count if the current torque reading is outside the bounds
                _data->torque_failure_count += (int)utils::is_outside_range(_data->torque_reading, bounds.first, bounds.second);
            }

            //If the failure count is greater than the max, return true
            return _data->torque_failure_count >= _data->torque_failure_count_max;
        }
        void handle(JointData* _data)
        {
            //_data->motor.enabled = false;
            logger::println("Torque Variance Error", LogLevel::Error);
        }
};

class ForceVarianceError : public ErrorType
{
    public:
        ForceVarianceError() : ErrorType() {};

        bool check(JointData* _data)
        {
            return false;
        }
        void handle(JointData* _data)
        {

        }
};

class TrackingError : public ErrorType
{
    public:
        TrackingError() : ErrorType() {};

        bool check(JointData* _data)
        {
            return false;
        }
        void handle(JointData* _data)
        {

        }
};

class MotorTimeoutError : public ErrorType
{
    public:
        MotorTimeoutError() : ErrorType() {};

        bool check(JointData* _data)
        {
            const bool timeout_error = _data->motor.timeout_count >= _data->motor.timeout_count_max;
            if (timeout_error) 
            {
                _data->motor.timeout_count = 0;
            }
            return timeout_error;
        }
        void handle(JointData* _data)
        {
            //_data->motor.enabled = false;
            logger::println("Motor Timeout Error", LogLevel::Error);
        }
};


#endif
#endif // ERROR_TYPES_H