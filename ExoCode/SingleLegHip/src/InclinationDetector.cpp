/**
 * @file InclinationDetector.cpp
 * @author Chancelor Cuddeback
 * @brief 
 * @date 2023-07-18
 * 
 */

#include "InclinationDetector.h"
#include <Arduino.h> // millis()
#include "Logger.h"
#include "LogLevels.h"
#include "Utilities.h"

#define INCLINATION_DETECTOR_DEBUG 0

/* Default detection parameters */
static constexpr float k_decline_angle_threshold{0.2f};
static constexpr float k_incline_angle_theshold{0.075f}; //Note: Previously was 0.1


InclinationDetector::InclinationDetector()
{
    _incline_angle_theshold = k_incline_angle_theshold;
    _decline_angle_threshold = k_decline_angle_threshold;

    _incline_features_sum = 0;
    _decline_features_sum = 0;
    _calibration_step_count = _steps_until_calibrated;
    _calibrated_stance_entry_angle = 0.5;
    _calibrated_stance_exit_angle = 0.5;
    _previous_should_calibrate = false;
    _returned_this_stance = true;
    _prev_estimate = Inclination::Level;
    _prev_stance = true;
    _prev_stance_time = 1;
    _current_stance_start = millis();
}

void InclinationDetector::set_incline_angle(float threshold)
{
    _incline_angle_theshold = threshold;
}

void InclinationDetector::set_decline_angle(float threshold)
{
    _decline_angle_threshold = threshold;
}

InclinationDetector::Edge InclinationDetector::_check_for_edge(const bool is_stance)
{
    #if INCLINATION_DETECTOR_DEBUG
        //logger::println("Checking for edge with (is_stance): "+ String(is_stance), LogLevel::Debug);
    #endif

    Edge edge = Edge::None;
    if (is_stance > _prev_stance)
    {
        edge = Edge::Rising;
    } else if (is_stance < _prev_stance)
    {
        edge = Edge::Falling;
    }

    _prev_stance = is_stance;
    return edge;
}
float InclinationDetector::_update_stance_phase(const InclinationDetector::Edge edge, const bool is_stance)
{
    #if INCLINATION_DETECTOR_DEBUG
        logger::println("Updating stance phase with (edge, is_stance): " + String(static_cast<int>(edge)) + ", " + String(is_stance), LogLevel::Debug);
    #endif

    if (!is_stance && edge != Edge::Falling)
    {
        return 0;
    }

    const float now = static_cast<float>(micros())/1000;
    if (edge == Edge::Rising)
    {
        #if INCLINATION_DETECTOR_DEBUG
            logger::println("Updating _current_stance_start with (millis): " + String(now), LogLevel::Debug);
        #endif
        _current_stance_start = now;
        
        return 0;
    } else if (edge == Edge::Falling)
    {
        const float stance_time = now - _current_stance_start;

        #if INCLINATION_DETECTOR_DEBUG
            logger::println("Updating _prev_stance_time with (stance_time): " + String(stance_time), LogLevel::Debug);
        #endif

        _prev_stance_time = stance_time;
        return 100;
    } else if (edge == Edge::None)
    {   
        const float stance_phase = (now-_current_stance_start) / _prev_stance_time;
        #if INCLINATION_DETECTOR_DEBUG
            logger::println("Updating stance phase with (millis, current start, prev time, phase): " + String(now) + ", "+ String(_current_stance_start) + ", " + String(_prev_stance_time) + ", " + String(stance_phase), LogLevel::Debug);
        #endif

        return stance_phase;
    }
}
void InclinationDetector::_calculate_gait_features()
{
    const float incline_size = _steps_until_calibrated;
    const float decline_size = _steps_until_calibrated - 1;

    _calibrated_stance_entry_angle = _incline_features_sum / incline_size;
    _calibrated_stance_exit_angle = _decline_features_sum / decline_size;
    //Serial.println("Generated averages(I,D): " + String(_calibrated_stance_entry_angle) + ", " + String(_calibrated_stance_exit_angle));
}
void InclinationDetector::_update_gait_features(const InclinationDetector::Edge edge, const float norm_angle)
{
    if (edge == Edge::Rising) {
        _incline_features_sum += norm_angle;
    } else if (edge == Edge::Falling) {
        _decline_features_sum += norm_angle;
    }
}
bool InclinationDetector::_should_calibrate(const bool fsr_calibrating, const InclinationDetector::Edge edge)
{
    const bool fsr_calibrated = (fsr_calibrating < _previous_fsr_calibrating);
    _previous_fsr_calibrating = fsr_calibrating;
    if (fsr_calibrated) {
        _incline_features_sum = 0;
        _decline_features_sum = 0;
        _calibration_step_count = 0;
    }
    
    if (edge == Edge::Falling) {
        _calibration_step_count++;
    }

    return _calibration_step_count < _steps_until_calibrated;
}
bool InclinationDetector::_incline_check(const float norm_angle)
{
    const bool is_incline = norm_angle < (_calibrated_stance_entry_angle - _incline_angle_theshold);

    #if INCLINATION_DETECTOR_DEBUG
        //logger::println("Incline check with (angle, is_incline): " + String(norm_angle) + ", " + String(is_incline), LogLevel::Debug);
    #endif

    return is_incline;
}
bool InclinationDetector::_decline_check(const float norm_angle)
{
    const bool is_decline = norm_angle < (_calibrated_stance_exit_angle - _decline_angle_threshold);

    #if INCLINATION_DETECTOR_DEBUG
        //logger::println("Decline check with (angle, is_decline): " + String(norm_angle) + ", " + String(is_decline), LogLevel::Debug);
    #endif

    return is_decline;
}

Inclination InclinationDetector::check(const bool is_stance, const bool fsr_calibrating, const float norm_angle)
{
    #if INCLINATION_DETECTOR_DEBUG
        logger::println("Checking for inclination with (is_stance, fsr_calibrating, norm_angle): " + String(is_stance) + ", " + String(fsr_calibrating) + ", " + String(norm_angle), LogLevel::Debug);
    #endif

    const Edge edge = _check_for_edge(is_stance);
    //const float stance_phase_percent = _update_stance_phase(edge, is_stance);
    //_update_angles_at(stance_phase_percent, norm_angle, edge);

    //Check for calibration
    const bool should_calibrate = _should_calibrate(fsr_calibrating, edge);
    if (should_calibrate) {
        _update_gait_features(edge, norm_angle);
    }

    //Check for end of calibration
    const bool end_of_calibration = (should_calibrate < _previous_should_calibrate);
    _previous_should_calibrate = should_calibrate;

    if (end_of_calibration) {
        _calculate_gait_features();
    }

    if (edge == Edge::Rising) {
        #if INCLINATION_DETECTOR_DEBUG
            logger::println("Rising Edge with (is_stance, norm_angle): " + String(is_stance) + ", " + String(norm_angle), LogLevel::Debug);
        #endif

        //Reset ability to return an estimate
        _returned_this_stance = false;

        const bool is_incline = _incline_check(norm_angle);
        if (is_incline) {
            _returned_this_stance = true;
            //Serial.println("Got Incline with: " + String(norm_angle) + " < " + String(_calibrated_stance_entry_angle - _incline_angle_theshold));
            #if INCLINATION_DETECTOR_DEBUG
                logger::println("Returning Incline", LogLevel::Debug);
            #endif

            _prev_estimate = Inclination::Incline;
            return Inclination::Incline;
        }
    }

    if (edge == Edge::Falling && !_returned_this_stance) {
        #if INCLINATION_DETECTOR_DEBUG
            logger::println("Falling Edge with (is_stance, norm_angle): " + String(is_stance) + ", " + String(norm_angle), LogLevel::Debug);
        #endif

        const bool is_decline = _decline_check(norm_angle);
        if (is_decline) {
            //Serial.println("Got Decline with: " + String(norm_angle) + " < " + String(_calibrated_stance_exit_angle - _decline_angle_threshold));
            #if INCLINATION_DETECTOR_DEBUG
                logger::println("Returning Decline", LogLevel::Debug);
            #endif

            _prev_estimate = Inclination::Decline;
            return Inclination::Decline;
        } else {
            //Must be level ground
            //Serial.println("Got Level with: " + String(norm_angle));
            #if INCLINATION_DETECTOR_DEBUG
                logger::println("Returning Level", LogLevel::Debug);
            #endif

            _prev_estimate = Inclination::Level;
            return Inclination::Level;
        }
    }

    return _prev_estimate;
}