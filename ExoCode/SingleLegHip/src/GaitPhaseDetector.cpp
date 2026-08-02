/*
 * Event-driven gait phase detection. See GaitPhaseDetector.h
 *
 * @author Your Name
 */

#include "GaitPhaseDetector.h"
#include "Logger.h"

//Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)

GaitPhaseDetector::GaitPhaseDetector(SideData* exo_side, ExoData* exo_data)
: _exo_heel(gait_phase_config::CONTACT_DEBOUNCE_MS)
, _exo_toe(gait_phase_config::CONTACT_DEBOUNCE_MS)
, _contra_heel(gait_phase_config::CONTACT_DEBOUNCE_MS)
, _contra_toe(gait_phase_config::CONTACT_DEBOUNCE_MS)
{
    _exo_side = exo_side;
    
    //Point at the contralateral side of the same exo_data.
    _contra_side = exo_side->is_left ? &(exo_data->right_side) : &(exo_data->left_side);
    
    _exo_side->exo_leg = true;
};

bool GaitPhaseDetector::DebouncedEdge::update(bool raw)
{
    unsigned long now = millis();
    
    //First sample: just establish the state, never emit an edge. This prevents
    //a spurious swing event at boot when the foot happens to be on the ground.
    if (!_initialized)
    {
        _initialized = true;
        _state = raw;
        _change_start_ms = 0;
        _rising = false;
        _falling = false;
        return _state;
    }
    
    if (raw != _state)
    {
        //The raw signal disagrees with the debounced state. Start/continue the
        //debounce window from the FIRST differing sample.
        if (_change_start_ms == 0)
        {
            _change_start_ms = now;
        }
        
        //Hold time satisfied: accept the change.
        if ((now - _change_start_ms) >= _debounce_ms)
        {
            _rising = raw && !_state;
            _falling = !raw && _state;
            _state = raw;
            _change_start_ms = 0;
        }
    }
    else
    {
        //Back to the debounced state, clear any pending change and edge flags.
        _change_start_ms = 0;
        _rising = false;
        _falling = false;
    }
    
    return _state;
};

bool GaitPhaseDetector::DebouncedEdge::rising_edge()
{
    bool val = _rising;
    _rising = false;
    return val;
};

bool GaitPhaseDetector::DebouncedEdge::falling_edge()
{
    bool val = _falling;
    _falling = false;
    return val;
};

void GaitPhaseDetector::_on_swing_start()
{
    //Reject swings that are too short (stomps, hops) or that happen too soon
    //after the last swing end (noise from double support).
    unsigned long now = millis();
    
    unsigned long time_since_last_end = (now - _last_swing_end_ms);
    if (time_since_last_end < gait_phase_config::MIN_SWING_MS)
    {
        return;
    }
    
    if (_in_swing)
    {
        return;
    }
    
    _in_swing = true;
    _last_swing_start_ms = _swing_start_ms;
    _swing_start_ms = now;
    
    _exo_side->swing_started = true;
    _exo_side->swing_start_ms = now;
    _exo_side->in_swing = true;
};

void GaitPhaseDetector::_on_swing_end()
{
    if (!_in_swing)
    {
        return;
    }
    
    unsigned long now = millis();
    unsigned long swing_ms = now - _swing_start_ms;
    
    //Reject unrealistically short swings.
    if (swing_ms < gait_phase_config::MIN_SWING_MS)
    {
        return;
    }
    
    _in_swing = false;
    _last_swing_end_ms = now;
    
    //Update the measured durations, which are only used to normalize the
    //percent progress within the NEXT swing. The phase itself is event-driven.
    _measured_swing_ms = swing_ms;
    _measured_stance_ms = (_last_swing_start_ms > 0) ? (_swing_start_ms - _last_swing_start_ms) : _measured_stance_ms;
    
    _exo_side->swing_ended = true;
    _exo_side->in_swing = false;
    _exo_side->measured_swing_duration = _measured_swing_ms;
    _exo_side->measured_stance_duration = _measured_stance_ms;
};

void GaitPhaseDetector::update()
{
    //Debounce all four FSR contact states.
    bool exo_heel_contact = _exo_heel.update(_exo_side->heel_stance);
    bool exo_toe_contact  = _exo_toe.update(_exo_side->toe_stance);
    bool contra_heel_contact = _contra_heel.update(_contra_side->heel_stance);
    bool contra_toe_contact  = _contra_toe.update(_contra_side->toe_stance);
    (void)exo_heel_contact;
    (void)exo_toe_contact;
    (void)contra_heel_contact;
    (void)contra_toe_contact;
    
    //Swing start: the supporting (contralateral) foot just heel-struck during
    //double support. This means the exo leg is about to push off and swing.
    //Cross-check with the exo leg's own toe falling edge as confirmation.
    bool contra_heel_rise = _contra_heel.rising_edge();
    bool exo_toe_fall = _exo_toe.falling_edge();
    if (contra_heel_rise || exo_toe_fall)
    {
        _on_swing_start();
    }
    
    //Swing end: the supporting foot just toe'd off during double support. This
    //means the exo leg is about to heel strike. Cross-check with the exo leg's
    //own heel rising edge.
    bool contra_toe_fall = _contra_toe.falling_edge();
    bool exo_heel_rise = _exo_heel.rising_edge();
    if (contra_toe_fall || exo_heel_rise)
    {
        _on_swing_end();
    }
    
    //Compute the percent progress through the current swing using the measured
    //duration of the PREVIOUS swing. This is event-anchored: the phase is
    //correct even if the user suddenly speeds up or slows down.
    if (_in_swing)
    {
        unsigned long now = millis();
        unsigned long since_start = now - _swing_start_ms;
        
        if (_measured_swing_ms > 0)
        {
            _exo_side->ev_percent_swing = min((float)since_start / _measured_swing_ms * 100.0f, 100.0f);
        }
        else
        {
            _exo_side->ev_percent_swing = 0;
        }
        
        _exo_side->ev_percent_stance = 0;
    }
    else
    {
        //In stance. Progress is based on the measured stance duration.
        if (_last_swing_end_ms > 0)
        {
            unsigned long since_end = millis() - _last_swing_end_ms;
            if (_measured_stance_ms > 0)
            {
                _exo_side->ev_percent_stance = min((float)since_end / _measured_stance_ms * 100.0f, 100.0f);
            }
            else
            {
                _exo_side->ev_percent_stance = 0;
            }
        }
        _exo_side->ev_percent_swing = 0;
    }
};

#endif
