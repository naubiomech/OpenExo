#include "Side.h"
#include "Logger.h"
//#define SIDE_DEBUG 1

//Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41) 
/*
 * Constructor for the side
 * Takes in if the side is the left one and a pointer to the exo_data
 * Uses initializer list for hip, knee, and ankle joint; and the FSRs.
 * Only stores these objects, exo_data pointer, and if it is left (for easy access)
 */
Side::Side(bool is_left, ExoData* exo_data)
: _hip((config_defs::joint_id)((uint8_t)(is_left ? config_defs::joint_id::left : config_defs::joint_id::right) | (uint8_t)config_defs::joint_id::hip), exo_data)        //We need to cast to uint8_t to do bitwise or, then we have to cast it back to joint_id
, _knee((config_defs::joint_id)((uint8_t)(is_left ? config_defs::joint_id::left : config_defs::joint_id::right) | (uint8_t)config_defs::joint_id::knee), exo_data)
, _ankle((config_defs::joint_id)((uint8_t)(is_left ? config_defs::joint_id::left : config_defs::joint_id::right) | (uint8_t)config_defs::joint_id::ankle), exo_data)
, _elbow((config_defs::joint_id)((uint8_t)(is_left ? config_defs::joint_id::left : config_defs::joint_id::right) | (uint8_t)config_defs::joint_id::elbow), exo_data)
, _heel_fsr(is_left ? logic_micro_pins::fsr_sense_left_heel_pin : logic_micro_pins::fsr_sense_right_heel_pin) //Check if it is the left and use the appropriate pin for the side.
, _toe_fsr(is_left ? logic_micro_pins::fsr_sense_left_toe_pin : logic_micro_pins::fsr_sense_right_toe_pin)
, _toe_fsr_regressed(is_left ? logic_micro_pins::fsr_sense_left_toe_pin : logic_micro_pins::fsr_sense_right_toe_pin)
{

    _data = exo_data;
    
    _is_left = is_left;
    
    //This data object is set for the specific side so we don't have to keep checking.
    _side_data = _is_left ? &(_data->left_side) : &(_data->right_side);
    
    #ifdef SIDE_DEBUG
        logger::print(_is_left ? "Left " : "Right ");
        logger::println("Side :: Constructor : _data set");
    #endif

    _prev_heel_contact_state = true;        //Initialized to true so we don't get a strike the first time we read
    _prev_toe_contact_state = true;
    _prev_toe_contact_state_toe_off = true;
    _prev_toe_contact_state_toe_on = true;
    
    for (int i = 0; i<_num_steps_avg; i++)
    {
        _step_times[i] = 0;
    }

    _ground_strike_timestamp = 0;
    _prev_ground_strike_timestamp = 0;
    //_expected_step_duration = 0;

    #ifdef SIDE_DEBUG
        logger::print(_is_left ? "Left " : "Right ");
        logger::println("Side :: Constructor : Exit");
    #endif

    _heel_fsr.get_contact_thresholds(_side_data->heel_fsr_lower_threshold, _side_data->heel_fsr_upper_threshold);
    _toe_fsr.get_contact_thresholds(_side_data->toe_fsr_lower_threshold, _side_data->toe_fsr_upper_threshold);
    _toe_fsr_regressed.get_contact_thresholds(_side_data->toe_fsr_lower_threshold, _side_data->toe_fsr_upper_threshold);

    inclination_detector = new InclinationDetector();
};

void Side::disable_motors()
{
    _hip._motor->enable(true);
    _knee._motor->enable(true);
    _ankle._motor->enable(true);
    _elbow._motor->enable(true);
};


void Side::run_side()
{
    #ifdef SIDE_DEBUG
        logger::print("\nmicros : ");
        logger::println(micros());
        logger::print(_is_left ? "Left " : "Right ");
        logger::println("Side :: run_side : checking calibration");
    #endif

    check_calibration();

    #ifdef SIDE_DEBUG
        logger::print(_is_left ? "Left " : "Right ");
        logger::println("Side :: run_side : reading data");
    #endif

    _check_thresholds();

    //Read all the data before we calculate and send the new motor commands
    read_data();

    #ifdef SIDE_DEBUG
        logger::print(_is_left ? "Left " : "Right ");
        logger::println("Side :: run_side : updating motor commands");
    #endif

    //Calculates the new motor commands and sends them.
    update_motor_cmds();

}; 

void Side::read_data()
{
    //Check the FSRs
    _side_data->heel_fsr = _heel_fsr.read();
    _side_data->toe_fsr = _toe_fsr.read();
    _side_data->toe_fsr_regressed = _toe_fsr_regressed.read();

    //Check if a ground strike is detected
    _side_data->ground_strike = _check_ground_strike();

    //If a strike is detected, update the expected duration of the step 
    if (_side_data->ground_strike)
    {
        _side_data->expected_step_duration = _update_expected_duration();
    }

    //Check if the toe on or toe off is occuring
    _side_data->toe_off = _check_toe_off();
    _side_data->toe_on = _check_toe_on();

    //If toe off is detected, updated expected stance duration
    if (_side_data->toe_off == true)
    {
        _side_data->expected_stance_duration = _update_expected_stance_duration();
    }

    //If toe on is detected, update expected swing duration
    if (_side_data->toe_on == true)
    {
        _side_data->expected_swing_duration = _update_expected_swing_duration();
    }

    //Calculate Percent Gait, Percent Stance, and Percent Swing
    _side_data->percent_gait = _calc_percent_gait();
    _side_data->percent_stance = _calc_percent_stance();
    _side_data->percent_swing = _calc_percent_swing();

    //Get the contact thesholds for the Heel and Toe FSRs
    _heel_fsr.get_contact_thresholds(_side_data->heel_fsr_lower_threshold, _side_data->heel_fsr_upper_threshold);
    _toe_fsr.get_contact_thresholds(_side_data->toe_fsr_lower_threshold, _side_data->toe_fsr_upper_threshold);
    _toe_fsr_regressed.get_contact_thresholds(_side_data->toe_fsr_lower_threshold, _side_data->toe_fsr_upper_threshold);

    //Check the inclination
    _side_data->inclination = inclination_detector->check(_side_data->toe_stance, _side_data->do_calibration_refinement_toe_fsr_regressed, _side_data->ankle.joint_position);
    
    //Check the joint sensors if the joint is used.
    if (_side_data->hip.is_used)
    {
        _hip.read_data();
    }
    if (_side_data->knee.is_used)
    {
        _knee.read_data();
    }
    if (_side_data->ankle.is_used)
    {
        _ankle.read_data();
    }
    if (_side_data->elbow.is_used)
    {
        _elbow.read_data();
    }
    
};

void Side::check_calibration()
{
    if (_side_data->is_used)
    {
        //Make sure FSR calibration is done before refinement.
        if (_side_data->do_calibration_toe_fsr)
        {
            _side_data->do_calibration_toe_fsr = _toe_fsr.calibrate(_side_data->do_calibration_toe_fsr);
            _data->set_status(status_defs::messages::fsr_calibration);
        }
        else if (_side_data->do_calibration_refinement_toe_fsr) 
        {
            _side_data->do_calibration_refinement_toe_fsr = _toe_fsr.refine_calibration(_side_data->do_calibration_refinement_toe_fsr);
            _data->set_status(status_defs::messages::fsr_refinement);
        }

        if (_side_data->do_calibration_toe_fsr_regressed)
        {
            _side_data->do_calibration_toe_fsr_regressed = _toe_fsr_regressed.calibrate(_side_data->do_calibration_toe_fsr_regressed);
            _data->set_status(status_defs::messages::fsr_calibration);
        }
        else if (_side_data->do_calibration_refinement_toe_fsr_regressed)
        {
            _side_data->do_calibration_refinement_toe_fsr_regressed = _toe_fsr_regressed.refine_calibration(_side_data->do_calibration_refinement_toe_fsr_regressed);
            _data->set_status(status_defs::messages::fsr_refinement);
        }
        
        if (_side_data->do_calibration_heel_fsr)
        {
            _side_data->do_calibration_heel_fsr = _heel_fsr.calibrate(_side_data->do_calibration_heel_fsr);
            _data->set_status(status_defs::messages::fsr_calibration);
        }
        else if (_side_data->do_calibration_refinement_heel_fsr) 
        {
            _side_data->do_calibration_refinement_heel_fsr = _heel_fsr.refine_calibration(_side_data->do_calibration_refinement_heel_fsr);
            _data->set_status(status_defs::messages::fsr_refinement);
        }
     
        //Check the joint sensors if the joint is used.
        if (_side_data->hip.is_used)
        {
            _hip.check_calibration();
        }
        if (_side_data->knee.is_used)
        {
            _knee.check_calibration();
        }
        if (_side_data->ankle.is_used)
        {
            _ankle.check_calibration();
        }
        if (_side_data->elbow.is_used)
        {
            _elbow.check_calibration();
        }
        
    }        
};

void Side::_check_thresholds()
{
    _toe_fsr.set_contact_thresholds(_side_data->toe_fsr_lower_threshold, _side_data->toe_fsr_upper_threshold);
    _heel_fsr.set_contact_thresholds(_side_data->heel_fsr_lower_threshold, _side_data->heel_fsr_upper_threshold);
    _toe_fsr_regressed.set_contact_thresholds(_side_data->toe_fsr_regressed_lower_threshold, _side_data->toe_fsr_regressed_upper_threshold);
}

bool Side::_check_ground_strike()
{
    _side_data->prev_heel_stance = _prev_heel_contact_state;  
    _side_data->prev_toe_stance = _prev_toe_contact_state;

    bool heel_contact_state = _heel_fsr.get_ground_contact();
    bool toe_contact_state = _toe_fsr.get_ground_contact();
    bool toe_regressed_contact_state = _toe_fsr_regressed.get_ground_contact();

    _side_data->heel_stance = heel_contact_state;
    _side_data->toe_stance = toe_contact_state;

    bool ground_strike = false;
    
    // logger::print("Side::_check_ground_strike : _prev_heel_contact_state - ");
    // logger::print(_prev_heel_contact_state);
    // logger::print("\n");
    // logger::print("\t_prev_toe_contact_state - ");
    // logger::print(_prev_toe_contact_state);
    // logger::print("\n");

    //Only check if in swing
    _side_data->toe_strike = toe_contact_state > _prev_toe_contact_state;

    if(!_prev_heel_contact_state & !_prev_toe_contact_state) //If we were previously in swing
    {
        //Check for rising edge on heel and toe, toe is to account for flat foot landings
        if ((heel_contact_state > _prev_heel_contact_state) | (toe_contact_state > _prev_toe_contact_state))    //If either the heel or toe FSR is on the ground and it previously wasn't on the ground
        {
            ground_strike = true;
            _prev_ground_strike_timestamp = _ground_strike_timestamp;
            _ground_strike_timestamp = millis();
        }
    }

    _prev_heel_contact_state = heel_contact_state;
    _prev_toe_contact_state = toe_contact_state;
    
    return ground_strike;
};

bool Side::_check_toe_on()
{
    bool toe_on = false;

    if (_side_data->toe_stance > _prev_toe_contact_state_toe_on)
    {
        toe_on = true;

        _prev_toe_strike_timestamp = _toe_strike_timestamp;
        _toe_strike_timestamp = millis();
    }

    _prev_toe_contact_state_toe_on = _side_data->toe_stance;

    return toe_on;
};

bool Side::_check_toe_off()
{
    bool toe_off = false;

    if (_side_data->toe_stance < _prev_toe_contact_state_toe_off)
    {
        toe_off = true;

        _prev_toe_off_timestamp = _toe_off_timestamp;
        _toe_off_timestamp = millis();
    }

    _prev_toe_contact_state_toe_off = _side_data->toe_stance;

    return toe_off;
};

float Side::_calc_percent_gait()
{
    int timestamp = millis();
    int percent_gait = -1;
    
    //Only calulate if the expected step duration has been established.
    if (_side_data->expected_step_duration > 0)
    {
        percent_gait = 100 * ((float)timestamp - _ground_strike_timestamp) / _side_data->expected_step_duration;
        percent_gait = min(percent_gait, 100); //Set saturation.
        
        // logger::print("Side::_calc_percent_gait : percent_gait_x10 = ");
        // logger::print(percent_gait_x10);
        // logger::print("\n");
    }
    return percent_gait;
};

float Side::_calc_percent_stance()
{
    int timestamp = millis();
    int percent_stance = -1;
    
    //Only calulate if the expected stance duration has been established.
    if (_side_data->expected_stance_duration > 0)
    {
        percent_stance = 100 * ((float)timestamp - _toe_strike_timestamp) / _side_data->expected_stance_duration;
        percent_stance = min(percent_stance, 100); //Set saturation.
    }

    if (_side_data->toe_stance == 0)
    {
        percent_stance = 0;
    }
    return percent_stance;
};

float Side::_calc_percent_swing()
{
    int timestamp = millis();
    int percent_swing = -1;
    
    //Only calulate if the expected swing duration has been established.
    if (_side_data->expected_stance_duration > 0)
    {
        percent_swing = 100 * ((float)timestamp - _toe_off_timestamp) / _side_data->expected_swing_duration;
        percent_swing = min(percent_swing, 100); //Set saturation.
        
        // logger::print("Side::_calc_percent_gait : percent_gait_x10 = ");
        // logger::print(percent_gait_x10);
        // logger::print("\n");
    }
    return percent_swing;
};

float Side::_update_expected_duration()
{
    unsigned int step_time = _ground_strike_timestamp - _prev_ground_strike_timestamp;
    float expected_step_duration = _side_data->expected_step_duration;
		
    if (0 == _prev_ground_strike_timestamp) //If the prev time isn't set just return.
    {
        return expected_step_duration;
    }

    uint8_t num_uninitialized = 0;
    
    //Check that everything is set.
    for (int i = 0; i < _num_steps_avg; i++)
    {
        num_uninitialized += (_step_times[i] == 0);
    }
    
    //Get the max and min values of the array for determining the window for expected values.
    unsigned int* max_val = std::max_element(_step_times, _step_times + _num_steps_avg);
    unsigned int* min_val = std::min_element(_step_times, _step_times + _num_steps_avg);
    
    if  (num_uninitialized > 0)  //If all the values haven't been replaced
    {
        //Shift all the values and insert the new one
        for (int i = (_num_steps_avg - 1); i>0; i--)
        {
            _step_times[i] = _step_times[i-1];
        }
        _step_times[0] = step_time;
        
        // logger::print("Side::_update_expected_duration : _step_times not fully initialized- [\t");
        // for (int i = 0; i < _num_steps_avg; i++)
        // {
            // logger::print(_step_times[i]);
            // logger::print("\t");
        // }
        // logger::print("\t]\n");    
    }

    //Consider it a good step if the ground strike falls within a window around the expected duration. Then shift the step times and put in the new value.
    else if ((step_time <= (_side_data->expected_duration_window_upper_coeff * *max_val)) & (step_time >= (_side_data->expected_duration_window_lower_coeff * *min_val))) // and (armed_time > ARMED_DURATION_PERCENT * self.expected_duration)): # a better check can be used.  If the person hasn't stopped or the step is good update the vector.  
    {
        int sum_step_times = step_time;
        for (int i = (_num_steps_avg - 1); i>0; i--)
        {
            sum_step_times += _step_times[i-1];
            _step_times[i] = _step_times[i-1];
        }
        _step_times[0] = step_time;
        
        expected_step_duration = sum_step_times / _num_steps_avg;  //Average to the nearest ms
        
        // logger::print("Side::_update_expected_duration : _expected_step_duration - ");
        // logger::print(_expected_step_duration);
        // logger::print("\n");
    }
    return expected_step_duration;
};

float Side::_update_expected_stance_duration()
{
    unsigned int stance_time = _toe_off_timestamp - _toe_strike_timestamp;
    float expected_stance_duration = _side_data->expected_stance_duration;

    if (0 == _prev_toe_strike_timestamp) //If the prev time isn't set just return.
    {
        return expected_stance_duration;
    }
    
    uint8_t num_uninitialized = 0;
    
    //Check that everything is set.
    for (int i = 0; i < _num_steps_avg; i++)
    {
        num_uninitialized += (_stance_times[i] == 0);
    }

    //Get the max and min values of the array for determining the window for expected values.
    unsigned int* max_val = std::max_element(_stance_times, _stance_times + _num_steps_avg);
    unsigned int* min_val = std::min_element(_stance_times, _stance_times + _num_steps_avg);

    if (num_uninitialized > 0)  //If all the values haven't been replaced
    {
        //Shift all the values and insert the new one
        for (int i = (_num_steps_avg - 1); i>0; i--)
        {
            _stance_times[i] = _stance_times[i - 1];
        }
        _stance_times[0] = stance_time;

        // logger::print("Side::_update_expected_duration : _step_times not fully initialized- [\t");
        // for (int i = 0; i < _num_steps_avg; i++)
        // {
            // logger::print(_step_times[i]);
            // logger::print("\t");
        // }
        // logger::print("\t]\n");
    }

    //Consider it a good step if the ground strike falls within a window around the expected duration. Then shift the step times and put in the new value.
    else if ((stance_time <= (_side_data->expected_duration_window_upper_coeff * *max_val)) & (stance_time >= (_side_data->expected_duration_window_lower_coeff * *min_val))) // and (armed_time > ARMED_DURATION_PERCENT * self.expected_duration)): # a better check can be used.  If the person hasn't stopped or the step is good update the vector.  
    {
        int sum_stance_times = stance_time;
        for (int i = (_num_steps_avg - 1); i>0; i--)
        {
            sum_stance_times += _stance_times[i - 1];
            _stance_times[i] = _stance_times[i - 1];
        }
        _stance_times[0] = stance_time;

        expected_stance_duration = sum_stance_times / _num_steps_avg;  //Average to the nearest ms
        
        // logger::print("Side::_update_expected_duration : _expected_step_duration - ");
        // logger::print(_expected_step_duration);
        // logger::print("\n");
    }
    return expected_stance_duration;
};


float Side::_update_expected_swing_duration()
{
    unsigned int swing_time = _toe_strike_timestamp - _toe_off_timestamp;
    float expected_swing_duration = _side_data->expected_swing_duration;

    if (0 == _prev_toe_off_timestamp) //If the prev time isn't set just return.
    {
        return expected_swing_duration;
    }

    uint8_t num_uninitialized = 0;
    
    //Check that everything is set.
    for (int i = 0; i < _num_steps_avg; i++)
    {
        num_uninitialized += (_swing_times[i] == 0);
    }

    //Get the max and min values of the array for determining the window for expected values.
    unsigned int* max_val = std::max_element(_swing_times, _swing_times + _num_steps_avg);
    unsigned int* min_val = std::min_element(_swing_times, _swing_times + _num_steps_avg);

    if (num_uninitialized > 0)  //If all the values haven't been replaced
    {
        //Shift all the values and insert the new one
        for (int i = (_num_steps_avg-1); i>0; i--)
        {
            _swing_times[i] = _swing_times[i - 1];
        }
        _swing_times[0] = swing_time;

        // logger::print("Side::_update_expected_duration : _step_times not fully initialized- [\t");
        // for (int i = 0; i < _num_steps_avg; i++)
        // {
            // logger::print(_step_times[i]);
            // logger::print("\t");
        // }
        // logger::print("\t]\n");
    }
    
    //Consider it a good step if the ground strike falls within a window around the expected duration. Then shift the step times and put in the new value.
    else if ((swing_time <= (_side_data->expected_duration_window_upper_coeff * *max_val)) & (swing_time >= (_side_data->expected_duration_window_lower_coeff * *min_val))) // and (armed_time > ARMED_DURATION_PERCENT * self.expected_duration)): # a better check can be used.  If the person hasn't stopped or the step is good update the vector.  
    {
        int sum_swing_times = swing_time;
        for (int i = (_num_steps_avg - 1); i>0; i--)
        {
            sum_swing_times += _swing_times[i - 1];
            _swing_times[i] = _swing_times[i - 1];
        }
        _swing_times[0] = swing_time;

        expected_swing_duration = sum_swing_times / _num_steps_avg;  //Average to the nearest ms
        
        // logger::print("Side::_update_expected_duration : _expected_step_duration - ");
        // logger::print(_expected_step_duration);
        // logger::print("\n");
    }
    return expected_swing_duration;
};

void Side::clear_step_time_estimate()
{
    for (int i = 0; i<_num_steps_avg; i++)
    {
        _step_times[i] = 0;
    }
};

void Side::update_motor_cmds()
{
    if (_side_data->hip.is_used)
    {
        _hip.run_joint();
    }
    if (_side_data->knee.is_used)
    {
        _knee.run_joint();
    }
    if (_side_data->ankle.is_used)
    {
        _ankle.run_joint();
    }
    if (_side_data->elbow.is_used)
    {
        _elbow.run_joint();
    }
};

#endif