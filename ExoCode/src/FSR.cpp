
#include "FSR.h"

#include "Board.h"
#include "Logger.h"
//#define FSR_DEBUG 1   //Uncomment if you want to print debug statements

//Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41) 
/*
 * Constructor for the force sensitive resistor
 * Takes in the pin to use and sets it as an analog input
 * Calibration and readings are initialized to 0 
 */
FSR::FSR(int pin)
{
    _pin = pin;
    
    _raw_reading = 0;
    _calibrated_reading = 0;
    
    _last_do_calibrate = false; 
    _start_time = 0;
    _calibration_min = 0;
    _calibration_max = 0;
    
    _state = false;
    _last_do_refinement = false;
    _step_count = 0;
    _calibration_refinement_min = 0;
    _calibration_refinement_max = 0;
    
    #ifdef FSR_DEBUG
        logger::println("FSR:: Constructor : Exit");
    #endif
}


bool FSR::calibrate(bool do_calibrate)
{
    //Check for rising edge of do_calibrate and start the timer
    if (do_calibrate > _last_do_calibrate)
    {
        // logger::print("FSR::calibrate : Starting Cal for pin - ");
        // logger::println(_pin);
        _start_time = millis();

        /* Set the Max & Min Values */
        /* If You Want Raw FSR Signal, Uncomment this and Comment out Regression Section Below */
        //_calibration_max = analogRead(_pin);
        //_calibration_min = _calibration_max;

        /* Regression Equation FSR to Make Proportional to Ankle Moment*/
        double p[4] = { 0.0787, -0.8471, 20.599, -22.670 };
        float Vo = 10 * 3.3 * analogRead(_pin) / 4095;                      //Note: 10* is to scale the output
        Vo = (Vo) / (87.43 * pow((Vo), (-0.6721)) - 7.883);                 //Apply interlink conversion
        Vo = p[0] * Vo * Vo * Vo + p[1] * Vo * Vo + p[2] * Vo + p[3];       //Apply amplification polynomial
        Vo = (Vo < 0.2) ? (0) : (Vo);                                       //If the value is less than 0.2, set it to zero
        _calibration_max = Vo;
        _calibration_min = _calibration_max;
    }
    
    //Check if we are within the time window and need to do the calibration
    uint16_t delta = millis()-_start_time;
    // logger::print("FSR::calibrate : delta - ");
    // logger::println(delta);
    
    if((_cal_time >= (delta)) & do_calibrate)
    {
        // logger::print("FSR::calibrate : Continuing Cal for pin - ");
        // logger::println(_pin);

        /* If You Want Raw FSR Signal, Uncomment this and Comment out Regression Section Below */
        //uint16_t current_reading = analogRead(_pin);

        /* Regression Equation FSR to Make Proportional to Ankle Moment*/
        double p[4] = { 0.0787, -0.8471, 20.599, -22.670 };
        float Vo = 10 * 3.3 * analogRead(_pin) / 4095;                      //ZL Added in the 10* to scale the output
        Vo = (Vo) / (87.43 * pow((Vo), (-0.6721)) - 7.883);                 //Apply interlink conversion
        Vo = p[0] * Vo * Vo * Vo + p[1] * Vo * Vo + p[2] * Vo + p[3];       //Apply amplification polynomial
        Vo = (Vo < 0.2) ? (0) : (Vo);                                       //If the value is less than 0.2, set it to zero
        float current_reading= Vo;

        //Track the min and max.
        _calibration_max = max(_calibration_max, current_reading);
        _calibration_min = min(_calibration_min, current_reading);
    } 

    //The time window ran out so we are done.
    else if (do_calibrate)
    {
        // logger::print("FSR::calibrate : FSR Cal Done for pin - ");
        // logger::println(_pin);
        // logger::print("FSR::calibrate : _calibration_max - ");
        // logger::print(_calibration_max);
        // logger::print("\n");
        do_calibrate = false;
    }
        
    //Store the reading for next time.
    _last_do_calibrate = do_calibrate;
    
    return do_calibrate;
};

bool FSR::refine_calibration(bool do_refinement)
{
    if (do_refinement)
    {
        //Check for rising edge of do_calibrate
        if (do_refinement > _last_do_refinement)
        {
            _step_count = 0;
            
            //Set the step max min to the middle value so the initial value is likely not used.
            _step_max = (_calibration_max+_calibration_min)/2;
            _step_min = (_calibration_max+_calibration_min)/2;

            //Reset the sum that will be used for averaging
            _step_max_sum = 0;
            _step_min_sum = 0;
        }
        
        //Check if we are done with the calibration
        if (_step_count < _num_steps)
        {
            /* If You Want Raw FSR Signal, Uncomment this and Comment out Regression Section Below */
            //uint16_t current_reading = analogRead(_pin);

            /* Regression Equation FSR to Make Proportional to Ankle Moment*/
            double p[4] = { 0.0787, -0.8471, 20.599, -22.670 };
            float Vo = 10 * 3.3 * analogRead(_pin) / 4095;                      //ZL Added in the 10* to scale the output
            Vo = (Vo) / (87.43 * pow((Vo), (-0.6721)) - 7.883);                 //Apply interlink conversion
            Vo = p[0] * Vo * Vo * Vo + p[1] * Vo * Vo + p[2] * Vo + p[3];       //Apply amplification polynomial
            Vo = (Vo < 0.2) ? (0) : (Vo);                                       //If the value is less than 0.2, set it to zero
            float current_reading = Vo;

            //For each step find max and min for every step, keep a running record of the max and min for the step.
            _step_max = max(_step_max, current_reading);
            _step_min = min(_step_min, current_reading);
            
            //Store the current state so we can check for change
            bool last_state = _state;
            _state = utils::schmitt_trigger(current_reading, last_state, _lower_threshold_percent_calibration_refinement * (_calibration_max-_calibration_min) + _calibration_min, _upper_threshold_percent_calibration_refinement * (_calibration_max-_calibration_min) + _calibration_min); 
            
            //There is a new low -> high transition (next step), add the step max and min to their respective sums.
            if (_state > last_state) 
            {
                _step_max_sum = _step_max_sum + _step_max;
                _step_min_sum = _step_min_sum + _step_min;
                
                //Reset the step max/min tracker for the next step
                _step_max = (_calibration_max+_calibration_min)/2;
                _step_min = (_calibration_max+_calibration_min)/2;
                
                _step_count++;
                
                // logger::print("FSR::refine_calibration : New Step - ");
            }

        }
        else //We are still at do_refinement but the _step_count is at the _num_steps
        {
            //Set the calibration as the average of the max values; average max and min, offset by min and normalize by (max-min), (val-avg_min)/(avg_max-avg_min)
            _calibration_refinement_max = static_cast<decltype(_calibration_refinement_max)>(_step_max_sum)/_num_steps;     //Casting to the type of _calibration_refinement_max before division 
            _calibration_refinement_min = static_cast<decltype(_calibration_refinement_min)>(_step_min_sum)/_num_steps;     //Casting to the type of _calibration_refinement_max before division 
 
            //Refinement is done
            do_refinement = false;
        } 
    }

    //Store the value so we can check for a rising edge next time.
    _last_do_refinement = do_refinement;
    
    return do_refinement;
};


float FSR::read()
{
    /* If You Want Raw FSR Signal, Uncomment this and Comment out Regression Section Below */
    //_raw_reading = analogRead(_pin);

    /* Regression Equation FSR to Make Proportional to Ankle Moment*/
    double p[4] = { 0.0787, -0.8471, 20.599, -22.670 };
    float Vo = 10 * 3.3 * analogRead(_pin) / 4095;                      //ZL Added in the 10* to scale the output
    Vo = (Vo) / (87.43 * pow((Vo), (-0.6721)) - 7.883);                 //Apply interlink conversion
    Vo = p[0] * Vo * Vo * Vo + p[1] * Vo * Vo + p[2] * Vo + p[3];       //Apply amplification polynomial
    Vo = (Vo < 0.2) ? (0) : (Vo);                                       //If the value is less than 0.2, set it to zero
    float _raw_reading = Vo;


    //Return the value using the calibrated refinement if it is done.
    if (_calibration_refinement_max > 0)
    {
        _calibrated_reading = ((float)_raw_reading - _calibration_refinement_min)/(_calibration_refinement_max-_calibration_refinement_min);
    }

    //If we haven't refined yet just use the regular calibration.
    else if (_calibration_max > 0)
    {
        _calibrated_reading = ((float)_raw_reading - _calibration_min)/(_calibration_max-_calibration_min);
    }

    //If no calibrations are done just return the raw reading.
    else
    {
        _calibrated_reading = _raw_reading;
    }
    
    //Based on the readings update the ground contact state.
    _calc_ground_contact();
    
    return  _calibrated_reading;

};

bool FSR::_calc_ground_contact()
{
    //Only do this if the refinement is done.
    bool current_state_estimate = false;

    if (_calibration_refinement_max > 0)
    {
        current_state_estimate = utils::schmitt_trigger(_calibrated_reading, _ground_contact, _lower_threshold_percent_ground_contact, _upper_threshold_percent_ground_contact);
    }

    _ground_contact = current_state_estimate;

    return _ground_contact;
};


bool FSR::get_ground_contact()
{
    // logger::print("FSR::refine_calibration : FSR pin - ");
    // logger::print(_pin);
    // logger::print("\t _ground_contact -");
    // logger::println(_ground_contact);
    return _ground_contact;
};

void FSR::get_contact_thresholds(float &lower_threshold_percent_ground_contact, float &upper_threshold_percent_ground_contact)
{
    lower_threshold_percent_ground_contact = _lower_threshold_percent_ground_contact;
    upper_threshold_percent_ground_contact = _upper_threshold_percent_ground_contact;
};

void FSR::set_contact_thresholds(float lower_threshold_percent_ground_contact, float upper_threshold_percent_ground_contact)
{
    _lower_threshold_percent_ground_contact = lower_threshold_percent_ground_contact;
    _upper_threshold_percent_ground_contact = upper_threshold_percent_ground_contact;
};
#endif