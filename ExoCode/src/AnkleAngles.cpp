#include "AnkleAngles.h"
#include "Utilities.h"
#include "Logger.h"

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)

// AnkleAngles* AnkleAngles::_instance = nullptr;

// AnkleAngles* AnkleAngles::GetInstance()
// {
//     if (_instance == nullptr) {
//         _instance = new AnkleAngles();
//         _instance->init();
//     }
//     return _instance;
// }


bool AnkleAngles::init(bool is_left)
{
    _left = is_left;
    pinMode(_left_pin, INPUT);
    pinMode(_right_pin, INPUT);
    _is_initialized = true;
}

float AnkleAngles::get(bool is_left, bool return_normalized)
{
    if (!_is_initialized) {
		init(is_left);
        return 0;
    }
    
    //Convert ADC counts to ratio
    const int adc_counts = _left ? analogRead(_left_pin):analogRead(_right_pin);
	const float ratio = adc_counts / 4095.0f;
    if (!return_normalized) {
        return ratio;
    }

    const float calibrated_ratio = _update_population_statistics(ratio);
    return calibrated_ratio;
}

float AnkleAngles::_update_population_statistics(const float new_value)
{
    _mean = utils::ewma(new_value, _mean, _ema_alpha);

    //Slowly bring the max min back to the mean
    _max_average -= _max_min_delta;
    _min_average += _max_min_delta;

    //Check for new peaks
    if (new_value > _max_average) {
        _max_average = new_value;
    } else if (new_value < _min_average) {
        _min_average = new_value;
    }

    const float range = _max_average - _min_average;

    static float return_value = 0;
    if (utils::is_outside_range(range, -0.01, 0.01)) {
        return_value = new_value / range;
    }
    
    return min(return_value, 1.4);
}


#endif