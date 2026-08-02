#ifndef PILOGGER_H
#define PILOGGER_H

#include "ExoData.h"

class PiLogger 
{
    public: 
    PiLogger(ExoData* data) : _data(data) {}

    void sendUpdate()
    {
        _start();

        _print_tab("LTFsr", _data->left_side.toe_fsr);
        _print_tab("LTSta", _data->left_side.toe_stance);
        _print_tab("LJTor", _data->left_side.ankle.torque_reading);
        _print_tab("LJPos", _data->left_side.ankle.joint_position);
        _print_tab("LJVel", _data->left_side.ankle.joint_velocity);
        _print_tab("LMPos", _data->left_side.ankle.motor.p);
        _print_tab("LMVel", _data->left_side.ankle.motor.v);
        _print_tab("LMCur", _data->left_side.ankle.motor.i);
        _print_tab("LMCom", _data->left_side.ankle.motor.last_command);

        _print_tab("LTFsr", _data->right_side.toe_fsr);
        _print_tab("LTSta", _data->right_side.toe_stance);
        _print_tab("LJTor", _data->right_side.ankle.torque_reading);
        _print_tab("LJPos", _data->right_side.ankle.joint_position);
        _print_tab("LJVel", _data->right_side.ankle.joint_velocity);
        _print_tab("LMPos", _data->right_side.ankle.motor.p);
        _print_tab("LMVel", _data->right_side.ankle.motor.v);
        _print_tab("LMCur", _data->right_side.ankle.motor.i);
        _print_tab("LMCom", _data->right_side.ankle.motor.last_command);

        _print_tab("Error", _data->error_code);
        _print_tab("ErJID", _data->error_joint_id);
        uint16_t exo_status = _data->get_status();
        bool active_trial = (exo_status == status_defs::messages::trial_on) || 
        (exo_status == status_defs::messages::fsr_calibration) ||
        (exo_status == status_defs::messages::fsr_refinement) ||
        (exo_status == status_defs::messages::error);
        _print_tab("ActiveTrial", active_trial);

        _end();
    }

    private:
    ExoData* _data;

    String _format(String name, float value)
    {
        return name+":"+String(value);
    }

    void _print_tab(String name, float value)
    {
        Serial.print(_format(name, value)+"\t");
    }

    void _start()
    {
        Serial.println("piLoggerStart");
    }
    
    void _end()
    {
        Serial.println("piLoggerEnd");
    }
    
};

#endif