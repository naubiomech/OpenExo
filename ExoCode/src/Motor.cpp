/*
 * P. Stegall Jan. 2022
*/

#include "Arduino.h" 

#include "Motor.h"
#include "CAN.h"
#include "ErrorManager.h"
#include "error_codes.h"
#include "Logger.h"
#include "ErrorReporter.h"
#include "error_codes.h"
//#define MOTOR_DEBUG           //Uncomment if you want to print debug statments to the serial monitor

//Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41) 


_Motor::_Motor(config_defs::joint_id id, ExoData* exo_data, int enable_pin)
{
    _id = id;
    _is_left = ((uint8_t)this->_id & (uint8_t)config_defs::joint_id::left) == (uint8_t)config_defs::joint_id::left;
    _data = exo_data;
    _enable_pin = enable_pin;
    _prev_motor_enabled = false; 
    _prev_on_state = false;
    
    #ifdef MOTOR_DEBUG
        logger::print("_Motor::_Motor : _enable_pin = ");
        logger::print(_enable_pin);
        logger::print("\n");
    #endif
    
    pinMode(_enable_pin, OUTPUT);
    
    //Set _motor_data to point to the data specific to this motor.
    switch (utils::get_joint_type(_id))
    {
        case (uint8_t)config_defs::joint_id::hip:
            if (_is_left)
            {
                _motor_data = &(exo_data->left_side.hip.motor);
            }
            else
            {
                _motor_data = &(exo_data->right_side.hip.motor);
            }
            break;
            
        case (uint8_t)config_defs::joint_id::knee:
            if (_is_left)
            {
                _motor_data = &(exo_data->left_side.knee.motor);
            }
            else
            {
                _motor_data = &(exo_data->right_side.knee.motor);
            }
            break;
        
        case (uint8_t)config_defs::joint_id::ankle:
            if (_is_left)
            {
                _motor_data = &(exo_data->left_side.ankle.motor);
            }
            else
            {
                _motor_data = &(exo_data->right_side.ankle.motor);
            }
            break;
        case (uint8_t)config_defs::joint_id::elbow:
            if (_is_left)
            {
                _motor_data = &(exo_data->left_side.elbow.motor);
            }
            else
            {
                _motor_data = &(exo_data->right_side.elbow.motor);
            }
            break;
    }

    #ifdef MOTOR_DEBUG
        logger::println("_Motor::_Motor : Leaving Constructor");
    #endif

};

bool _Motor::get_is_left() 
{
    return _is_left;
};

config_defs::joint_id _Motor::get_id()
{
    return _id;
};

/*
 * Constructor for the CAN Motor.  
 * We are using multilevel inheritance, so we have a general motor type, which is inherited by the CAN (e.g. TMotor) or other type (e.g. Maxon) since models within these types will share communication protocols, which is then inherited by the specific motor model (e.g. AK60), which may have specific torque constants etc.
 */
_CANMotor::_CANMotor(config_defs::joint_id id, ExoData* exo_data, int enable_pin) //Constructor: type is the motor type
: _Motor(id, exo_data, enable_pin)
{
    _KP_MIN = 0.0f;
    _KP_MAX = 500.0f;
    _KD_MIN = 0.0f;
    _KD_MAX = 5.0f;
    _P_MAX = 12.5f;

    JointData* j_data = exo_data->get_joint_with(static_cast<uint8_t>(id));
    j_data->motor.kt = this->get_Kt();

    _enable_response = false;

    #ifdef MOTOR_DEBUG
        logger::println("_CANMotor::_CANMotor : Leaving Constructor");
    #endif
};

void _CANMotor::transaction(float torque)
{
    //Send data and read response 
    send_data(torque);
    read_data();
    check_response();
};

void _CANMotor::read_data()
{
    //Read data from motor
    bool searching = true;
    uint32_t start = micros();

    //Only send and receive data if enabled
    if (_motor_data->enabled)
    {
        CAN* can = can->getInstance();
        do
        {
            int direction_modifier = _motor_data->flip_direction ? -1 : 1;

            CAN_message_t msg = can->read();
            if (msg.buf[0] == uint32_t(_motor_data->id))
            {
                //Unpack data
                uint32_t p_int = (msg.buf[1] << 8) | msg.buf[2];
                uint32_t v_int = (msg.buf[3]) << 4 | (msg.buf[4] >> 4);
                uint32_t i_int = ((msg.buf[4] & 0xF) << 8) | msg.buf[5];

                //Set data in ExoData object
                _motor_data->p = direction_modifier * _uint_to_float(p_int, -_P_MAX, _P_MAX, 16);
                _motor_data->v = direction_modifier * _uint_to_float(v_int, -_V_MAX, _V_MAX, 12);
                _motor_data->i = direction_modifier * _uint_to_float(i_int, -_I_MAX, _I_MAX, 12);

                #ifdef MOTOR_DEBUG
                    logger::print("_CANMotor::read_data():Got data-");
                    logger::print("ID:" + String(uint32_t(_motor_data->id)) + ",");
                    logger::print("P:"+String(_motor_data->p) + ",V:" + String(_motor_data->v) + ",I:" + String(_motor_data->i));
                    logger::print("\n");
                #endif

                //Reset timout_count because we got a valid message
                _motor_data->timeout_count = 0;
                return;
            }

            searching = ((micros() - start) < _timeout);

        }
        while(searching);

        //If we get here, we timed out
        _handle_read_failure();
    }
    return;
};

void _CANMotor::send_data(float torque)
{
    #ifdef MOTOR_DEBUG
        logger::print("Sending data: ");
        logger::print(uint32_t(_motor_data->id));
        logger::print("\n");
    #endif

    int direction_modifier = _motor_data->flip_direction ? -1 : 1;

    _motor_data->t_ff = torque;
    const float current = torque / get_Kt();

    //Read data from ExoData object, constraint it, and package it
    float p_sat = constrain(direction_modifier * _motor_data->p_des, -_P_MAX, _P_MAX);
    float v_sat = constrain(direction_modifier * _motor_data->v_des, -_V_MAX, _V_MAX);
    float kp_sat = constrain(_motor_data->kp, _KP_MIN, _KP_MAX);
    float kd_sat = constrain(_motor_data->kd, _KD_MIN, _KD_MAX);
    float i_sat = constrain(direction_modifier * current, -_I_MAX, _I_MAX);
    _motor_data->last_command = i_sat;
    uint32_t p_int = _float_to_uint(p_sat, -_P_MAX, _P_MAX, 16);
    uint32_t v_int = _float_to_uint(v_sat, -_V_MAX, _V_MAX, 12);
    uint32_t kp_int = _float_to_uint(kp_sat, _KP_MIN, _KP_MAX, 12);
    uint32_t kd_int = _float_to_uint(kd_sat, _KD_MIN, _KD_MAX, 12);
    uint32_t i_int = _float_to_uint(i_sat, -_I_MAX, _I_MAX, 12);
    CAN_message_t msg;
    msg.id = uint32_t(_motor_data->id);
    msg.buf[0] = p_int >> 8;
    msg.buf[1] = p_int & 0xFF;
    msg.buf[2] = v_int >> 4;
    msg.buf[3] = ((v_int & 0xF) << 4) | (kp_int >> 8);
    msg.buf[4] = kp_int & 0xFF;
    msg.buf[5] = kd_int >> 4;
    msg.buf[6] = ((kd_int & 0xF) << 4) | (i_int >> 8);
    msg.buf[7] = i_int & 0xFF;

    //logger::print("_CANMotor::send_data::t_sat:: ");
    //logger::print(t_sat);
    //logger::print("\n");
    //logger::print("T:"+String(t_sat)+",");
    
    //Only send messages if enabled
    if (_motor_data->enabled)
    {
        //Set data in motor
        CAN* can = can->getInstance();
        can->send(msg);
    }
    return;
};

void _CANMotor::check_response()
{
    //Only run if the motor is supposed to be enabled
    uint16_t exo_status = _data->get_status();
    bool active_trial = (exo_status == status_defs::messages::trial_on) ||
        (exo_status == status_defs::messages::fsr_calibration) ||
        (exo_status == status_defs::messages::fsr_refinement);

    if (_data->user_paused || !active_trial || _data->estop || _error)
    {
        return;
    }

    //Measured current variance should be non-zero
    _measured_current.push(_motor_data->i);

    if (_measured_current.size() > _current_queue_size)
    {
        _measured_current.pop();
        auto pop_vals = utils::online_std_dev(_measured_current);

        if (pop_vals.second < _variance_threshold)
        {
            _motor_data->enabled = true;
            enable(true);
        }

    }
};

void _CANMotor::on_off()
{
    if (_data->estop || _error)
    {
        _motor_data->is_on = false;

        // logger::print("_CANMotor::on_off(bool is_on) : E-stop pulled - ");
        // logger::print(uint32_t(_motor_data->id));
        // logger::print("\n");
    }

    if (_prev_on_state != _motor_data->is_on) //If was here to save time, can be removed if making problems, or add overide
    {
        if (_motor_data->is_on)
        {
            digitalWrite(_enable_pin, logic_micro_pins::motor_enable_on_state);

            // logger::print("_CANMotor::on_off(bool is_on) : Power on- ");
            // logger::print(uint32_t(_motor_data->id));
            // logger::print("\n");
        }
        else 
        {
            digitalWrite(_enable_pin, logic_micro_pins::motor_enable_off_state);

            // logger::print("_CANMotor::on_off(bool is_on) : Power off- ");
            // logger::print(uint32_t(_motor_data->id));
            // logger::print("\n");
        }
    }
    _prev_on_state = _motor_data->is_on;

    #ifdef HEADLESS
        delay(2000);    //Two second delay between motor's turning on and enabeling, we've run into some issues with enabling while in headless mode if this delay is not present. 
    #endif

};

bool _CANMotor::enable()
{
    return enable(false);
};

bool _CANMotor::enable(bool overide)
{
    #ifdef MOTOR_DEBUG
        //  logger::print(_prev_motor_enabled);
        //  logger::print("\t");
        //  logger::print(_motor_data->enabled);
        //  logger::print("\t");
        //  logger::print(_motor_data->is_on);
        //  logger::print("\n");
    #endif
    
    //Only change the state and send messages if the enabled state has changed.
    if ((_prev_motor_enabled != _motor_data->enabled) || overide || !_enable_response)
    {
        CAN_message_t msg;
        msg.id = uint32_t(_motor_data->id);
        msg.buf[0] = 0xFF;
        msg.buf[1] = 0xFF;
        msg.buf[2] = 0xFF;
        msg.buf[3] = 0xFF;
        msg.buf[4] = 0xFF;
        msg.buf[5] = 0xFF;
        msg.buf[6] = 0xFF;

        if (_motor_data->enabled && !_error && !_data->estop)
        {
            msg.buf[7] = 0xFC;
        }
        else 
        {
            _enable_response = false;
            msg.buf[7] = 0xFD;
        }

        CAN* can = can->getInstance();
        can->send(msg);
        delayMicroseconds(500);
        read_data();

        if (_motor_data->timeout_count == 0)
        {
            _enable_response = true;
        }
    }

    _prev_motor_enabled = _motor_data->enabled;
    return _enable_response;
};

void _CANMotor::zero()
{
    CAN_message_t msg;
    msg.id = uint32_t(_motor_data->id);
    msg.buf[0] = 0xFF;
    msg.buf[1] = 0xFF;
    msg.buf[2] = 0xFF;
    msg.buf[3] = 0xFF;
    msg.buf[4] = 0xFF;
    msg.buf[5] = 0xFF;
    msg.buf[6] = 0xFF;
    msg.buf[7] = 0xFE;
    CAN* can = can->getInstance();
    can->send(msg);
    
    read_data();
};

float _CANMotor::get_Kt()
{
    return _Kt;
};

void _CANMotor::set_error()
{
    _error = true;
};

void _CANMotor::set_Kt(float Kt)
{
    _Kt = Kt;
};

void _CANMotor::_handle_read_failure()
{
    logger::println("CAN Motor - Handle Read Failure", LogLevel::Error);
    _motor_data->timeout_count++;
};

float _CANMotor::_float_to_uint(float x, float x_min, float x_max, int bits)
{
    float span = x_max - x_min;
    float offset = x_min;
    unsigned int pgg = 0;
    if (bits == 12) {
      pgg = (unsigned int) ((x-offset)*4095.0/span); 
    }
    if (bits == 16) {
      pgg = (unsigned int) ((x-offset)*65535.0/span);
    }
    return pgg;
};

float _CANMotor::_uint_to_float(unsigned int x_int, float x_min, float x_max, int bits)
{
    float span = x_max - x_min;
    float offset = x_min;
    float pgg = 0;
    if (bits == 12) {
      pgg = ((float)x_int)*span/4095.0 + offset;
    }
    if (bits == 16) {
      pgg = ((float)x_int)*span/65535.0 + offset;
    }
    return pgg;
};

//**************************************
/*
 * Constructor for the motor
 * Takes the joint id and a pointer to the exo_data
 * Only stores the id, exo_data pointer, and if it is left (for easy access)
 */
AK60::AK60(config_defs::joint_id id, ExoData* exo_data, int enable_pin): //Constructor: type is the motor type
_CANMotor(id, exo_data, enable_pin)
{
    _I_MAX = 22.0f;
    _V_MAX = 41.87f;
    
    float kt = 0.068 * 6;
    set_Kt(kt);
    exo_data->get_joint_with(static_cast<uint8_t>(id))->motor.kt = kt;

    #ifdef MOTOR_DEBUG
        logger::println("AK60::AK60 : Leaving Constructor");
    #endif
};

/*
 * Constructor for the motor
 * Takes the joint id and a pointer to the exo_data
 * Only stores the id, exo_data pointer, and if it is left (for easy access)
 */
AK60_v1_1::AK60_v1_1(config_defs::joint_id id, ExoData* exo_data, int enable_pin): //Constructor: type is the motor type
_CANMotor(id, exo_data, enable_pin)
{
    _I_MAX = 13.5f;
    _V_MAX = 23.04f;

    float kt = 0.1725 * 6; //We set KT to 0.1725 * 6 whcih differs from the manufacturer's stated KT, that's because they are wrong (This has been validated mulitple ways). We only have validated for this version as we use open loop at the hip with these, other motors are used with closed loop and thus are corrected in real-time. We recommend validating these KTs if using for open loop. 
    set_Kt(kt);
    exo_data->get_joint_with(static_cast<uint8_t>(id))->motor.kt = kt;

    #ifdef MOTOR_DEBUG
        logger::println("AK60_v1_1::AK60_v1_1 : Leaving Constructor");
    #endif
};

/*
 * Constructor for the motor
 * Takes the joint id and a pointer to the exo_data
 * Only stores the id, exo_data pointer, and if it is left (for easy access)
 */
AK80::AK80(config_defs::joint_id id, ExoData* exo_data, int enable_pin): //Constructor: type is the motor type
_CANMotor(id, exo_data, enable_pin)
{
    _I_MAX = 24.0f;
    _V_MAX = 25.65f;

    float kt = 0.091 * 9;
    set_Kt(kt);
    exo_data->get_joint_with(static_cast<uint8_t>(id))->motor.kt = kt;

    #ifdef MOTOR_DEBUG
        logger::println("AK80::AK80 : Leaving Constructor");
    #endif
};

/*
 * Constructor for the motor
 * Takes the joint id and a pointer to the exo_data
 * Only stores the id, exo_data pointer, and if it is left (for easy access)
 */
AK70::AK70(config_defs::joint_id id, ExoData* exo_data, int enable_pin): //Constructor: type is the motor type
_CANMotor(id, exo_data, enable_pin)
{
    _I_MAX = 23.2f;
    _V_MAX = 15.5f;
    
    float kt = 0.13 * 10;
    set_Kt(kt);
    exo_data->get_joint_with(static_cast<uint8_t>(id))->motor.kt = kt;

    #ifdef MOTOR_DEBUG
        logger::println("AK70::AK70 : Leaving Constructor");
    #endif
};


/*
 * Constructor for the PWM (Maxon) Motor.  
 * We are using multilevel inheritance, so we have a general motor type, which is inherited by the PWM (e.g. Maxon) or other type (e.g. Maxon) since models within these types will share communication protocols, which is then inherited by the specific motor model, which may have specific torque constants etc.
 */
MaxonMotor::MaxonMotor(config_defs::joint_id id, ExoData* exo_data, int enable_pin) //Constructor: type is the motor type
: _Motor(id, exo_data, enable_pin)
{
    JointData* j_data = exo_data->get_joint_with(static_cast<uint8_t>(id));
	
    #ifdef MOTOR_DEBUG
        logger::println("MaxonMotor::MaxonMotor: Leaving Constructor");
    #endif
};

void MaxonMotor::transaction(float torque)
{
    //Send data
    send_data(torque);

    //Only enable the motor when it is an active trial 
    master_switch();

	if (_motor_data->enabled)
	{
		maxon_manager(true); //Monitors for and corrects motor resetting error if the system is operational.
	}
	else
	{
		maxon_manager(false);   //Reset the motor error detection function, in case user pauses device in middle of error event
	}

	// Serial.print("\nRight leg MaxonMotor::transaction(float torque)  |  torque = ");
	// Serial.print(torque);
};

bool MaxonMotor::enable()
{
    return true;    //This function is currently bypassed for this motor at the moment.
};

bool MaxonMotor::enable(bool overide)
{	
	//Only change the state and send messages if the enabled state (used as a master switch for this motor) has changed.
    if ((_prev_motor_enabled != _motor_data->enabled) || overide)
    {
		if (_motor_data->enabled)   //_motor_data->enabled is controlled by the GUI
		{
            //Enable motor
			digitalWrite(_enable_pin,HIGH);         //Relocate in the future
		}

		_enable_response = true;
	}

	if (!overide)                   //When enable(false), send the disable motor command, set the analogWrite resolution, and send 50% PWM command
    {
		_enable_response = false;
		
        //Disable motor, the message after this shouldn't matter as the power is cut, and the send() doesn't send a message if not enabled.
		digitalWrite(_enable_pin,LOW);
		analogWrite(_ctrl_right_pin,_pwm_neutral_val);
		analogWrite(_ctrl_left_pin,_pwm_neutral_val);
    }
	
	if (!_motor_data->enabled)   //_motor_data->enabled is controlled by the GUI
		{
            //Disable motor
			digitalWrite(_enable_pin,LOW);         //Relocate in the future
		}

	_prev_motor_enabled = _motor_data->enabled;

    return _enable_response;
	
    #ifdef MOTOR_DEBUG
        logger::print(_prev_motor_enabled);
        logger::print("\t");
        logger::print(_motor_data->enabled);
        logger::print("\t");
        logger::print(_motor_data->is_on);
        logger::print("\n");
    #endif
};

void MaxonMotor::send_data(float torque) //Always send motor command regardless of the motor "enable" status
{
    #ifdef MOTOR_DEBUG
        logger::print("Sending data: ");
        logger::print(uint32_t(_motor_data->id));
        logger::print("\n");
    #endif
	
	int direction_modifier = _motor_data->flip_direction ? -1 : 1; 

	_motor_data->t_ff = torque;
    _motor_data->last_command = torque;
	
	uint16_t exo_status = _data->get_status();
    bool active_trial = (exo_status == status_defs::messages::trial_on) ||
        (exo_status == status_defs::messages::fsr_calibration) ||
        (exo_status == status_defs::messages::fsr_refinement);
   
	if (_data->user_paused || !active_trial || _data->estop)        //Ignores the exo error handler for the moment
    {
        analogWrite(_ctrl_left_pin,_pwm_neutral_val);   //Set 50% PWM (0 current)
		analogWrite(_ctrl_right_pin,_pwm_neutral_val);	//Set 50% PWM (0 current)
    }
    else
    {
		//Constrain the motor pwm command
		uint16_t post_fuse_torque = max(_pwm_l_bound,_pwm_neutral_val+(direction_modifier*torque));    //Set the lowest allowed PWM command
		post_fuse_torque = min(_pwm_u_bound,post_fuse_torque);                              //Set the highest allowed PWM command
		analogWrite((_motor_data->is_left? _ctrl_left_pin : _ctrl_right_pin),post_fuse_torque);	//Send the motor command to the motor driver
	}
};

void MaxonMotor::master_switch()
{
   //Only run if the motor is supposed to be enabled
    uint16_t exo_status = _data->get_status();
    bool active_trial = (exo_status == status_defs::messages::trial_on) || 
        (exo_status == status_defs::messages::fsr_calibration) ||
        (exo_status == status_defs::messages::fsr_refinement);

	if (_data->user_paused || !active_trial || _data->estop)
    {
		pinMode(_err_left_pin, INPUT_PULLUP);
		pinMode(_err_right_pin, INPUT_PULLUP);
		pinMode(_current_left_pin,INPUT);
		pinMode(_current_right_pin,INPUT);
		analogWriteResolution(12);
		analogWriteFrequency(_ctrl_left_pin, 5000);
		analogWriteFrequency(_ctrl_right_pin, 5000);
		
		//_motor_data->enabled = false;
        enable(false);
    }
	else
    {
		//_motor_data->enabled = true;
        enable(true);
	}
};

//Our implementation of the Maxon motor including the ec motor and the Escon 50_8 Motor Controller would occasionally cause 50_8 to enter error mode, with "Over current" being one of the errors.
//To address this issue, we have developed a solution contained in maxon_manager() below. 
void MaxonMotor::maxon_manager(bool manager_active)
{
    //Initialize variables when switch is set to false, run the error detection and rest code when switch is set to true. 
    if (!manager_active)
    {
		//Reset Maxon motor reset utilities
        do_scan4maxon_err_left = true;       
        maxon_counter_active_left = false;
		do_scan4maxon_err_right = true;       
        maxon_counter_active_right = false;
    }
    else
    {
		unsigned long maxon_reset_current_t = millis();
        
		//Scan for left motor error
		if ((do_scan4maxon_err_left) && (!digitalRead(_err_left_pin)))
		{
			do_scan4maxon_err_left = false;          
			maxon_counter_active_left = true;
			zen_millis_left = maxon_reset_current_t;
		}

		//Left motor reset
		if (maxon_counter_active_left) 
		{
			//Two iterations after maxon_counter_actie = true, de-enable motor
			if (maxon_reset_current_t - zen_millis_left >= 2)
			{
				enable(false);
			}

			//Ten iterations after maxon_counter_actie = true, re-enable motor
			if (maxon_reset_current_t - zen_millis_left >= 10)
			{
				enable(true);
			}
			
			//Thirty iterations after maxon_counter_actie = true, start scanning for error again
			if (maxon_reset_current_t - zen_millis_left >= 30)
			{
				do_scan4maxon_err_left = true;
				maxon_counter_active_left = false;                                   
				_motor_data->maxon_plotting_scalar = -1 * _motor_data->maxon_plotting_scalar;
			}
		}

		//Scan for right motor error
		if ((do_scan4maxon_err_right) && (!digitalRead(_err_right_pin)))
		{
			do_scan4maxon_err_right = false;          
			maxon_counter_active_right = true;
			zen_millis_right = maxon_reset_current_t;
		}
		
		//Right motor reset
		if (maxon_counter_active_right) 
		{
			//Two iterations after maxon_counter_actie = true, de-enable motor
			if (maxon_reset_current_t - zen_millis_right >= 2)
			{
				enable(false);
			}

			//Ten iterations after maxon_counter_actie = true, re-enable motor
			if (maxon_reset_current_t - zen_millis_right >= 10)
			{
				enable(true);
			}
			
			//Thirty iterations after maxon_counter_actie = true, start scanning for error again
			if (maxon_reset_current_t - zen_millis_right >= 30)
			{
				do_scan4maxon_err_right = true;
				maxon_counter_active_right = false;                                   
				_motor_data->maxon_plotting_scalar = -1 * _motor_data->maxon_plotting_scalar;
			}
		}
    }
};

#endif