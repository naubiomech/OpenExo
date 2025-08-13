#include "Joint.h"
#include "Time_Helper.h"
#include "Logger.h"
#include "ErrorReporter.h"
#include "error_codes.h"

//#define JOINT_DEBUG       //Uncomment if you want to print debug statements to serial monitor.

//Arduino compiles everything in the src folder even if not included so it causes and error for the nano if this is not included.
#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41) 

//Initialize the used joint counters that will be used to select the TorqueSensor pin. If you don't do it it won't work.
uint8_t _Joint::left_torque_sensor_used_count = 0;
uint8_t _Joint::right_torque_sensor_used_count = 0;

uint8_t _Joint::left_motor_used_count = 0;
uint8_t _Joint::right_motor_used_count = 0;

/*
 * Constructor for the joint
 * Takes the joint id and a pointer to the exo_data
 * Uses initializer list for motor, controller, and torque sensor.
 * Only stores these objects, the id, exo_data pointer, and if it is left (for easy access)
 */
_Joint::_Joint(config_defs::joint_id id, ExoData* exo_data)
: _torque_sensor(_Joint::get_torque_sensor_pin(id, exo_data)) // <-- Initializer list
{
    // logger::print("_Joint::right_torque_sensor_used_count : ");
    // logger::println(_Joint::right_torque_sensor_used_count);

    #ifdef JOINT_DEBUG
        logger::println("_Joint :: Constructor : entered"); 
    #endif

    _id = id;
    _is_left = utils::get_is_left(_id); 
        
    _data = exo_data;

    #ifdef JOINT_DEBUG
        logger::print(_is_left ? "Left " : "Right ");
        switch (utils::get_joint_type(_id))
        {
            case (uint8_t)config_defs::joint_id::hip:
                logger::print("Hip");
                break;
            case (uint8_t)config_defs::joint_id::knee:
                logger::print("Knee");
                break;
            case (uint8_t)config_defs::joint_id::ankle:
                logger::print("Ankle");
                break;
            case (uint8_t)config_defs::joint_id::elbow:
                logger::print("Elbow");
                break;
            default:
                break;
        }
        logger::println(" :: Constructor : _data set");
    #endif

    // logger::print(uint8_t(id));
    // logger::print("\t");
    // logger::print(_is_used);
    // logger::print("\t");
    // logger::print(uint8_t(_torque_sensor._pin));
    // logger::print("\t");
    
};  

void _Joint::read_data()  
{
    //Read the torque sensor, and change sign based on side.
    _joint_data->torque_reading = (_joint_data->flip_direction ? -1.0 : 1.0) * _torque_sensor.read();
    
    _joint_data->position = _joint_data->motor.p / _joint_data->motor.gearing;
    _joint_data->velocity = _joint_data->motor.v / _joint_data->motor.gearing;
	
	//Read the true torque sensor offset
	_joint_data->torque_offset_reading = _torque_sensor.readOffset();
	
	//Return calculated torque reading based on the offset pulled from the SD Card
	_joint_data->torque_reading_microSD = (_joint_data->flip_direction ? -1.0 : 1.0) * _torque_sensor.read_microSD(_joint_data->torque_offset / 100);
	
	//To bypass the torque calibration process at the beginning of each trial, modify the torque offset placeholder "255" on the SD card
	//Example: If the true offset is 1.19, use 119; if the true offset is 0.95, use 95.
	if (_joint_data->torque_offset != 255)
    {
		_joint_data->torque_reading = _joint_data->torque_reading_microSD;
	}
};

void _Joint::check_calibration()  
{
    // logger::print("id: ");
    // logger::print(uint8_t(_id));
    // logger::print("\t");
    
    //Check if we are doing the calibration on the torque sensor
    _joint_data->calibrate_torque_sensor = _torque_sensor.calibrate(_joint_data->calibrate_torque_sensor);
    
    if(_joint_data->calibrate_torque_sensor)
    {
        _data->set_status(status_defs::messages::torque_calibration);
    }

    //logger::print("_Joint::check_calibration\n"); 
    
    if (_joint_data->motor.do_zero)
    {
        _motor->zero();
    }
};

unsigned int _Joint::get_torque_sensor_pin(config_defs::joint_id id, ExoData* exo_data)
{
    //logger::print("utils::get_joint_type(id) : ");
    //logger::println(utils::get_joint_type(id));
    //logger::print("utils::get_is_left(id) : ");
    //logger::println(utils::get_is_left(id));
    
    //First check which joint we are looking at. Then go through and if it is the left or right and if it is used. If it is set return the appropriate pin and increment the counter.
    switch (utils::get_joint_type(id))
    {
        case (uint8_t)config_defs::joint_id::hip:
        {
            if (utils::get_is_left(id) && exo_data->left_side.hip.is_used && exo_data->hip_torque_flag == 1)  //Check if the left side is used and we want to use the torque sensor
            {
                if (_Joint::left_torque_sensor_used_count < logic_micro_pins::num_available_joints) //If we still have available pins send the next one and increment the counter. If we don't send the not connected pin.
                {
                    return logic_micro_pins::torque_sensor_left[_Joint::left_torque_sensor_used_count++];
                }
                else
                {
                    return logic_micro_pins::not_connected_pin;
                }
            }
            else if (!(utils::get_is_left(id)) && exo_data->right_side.hip.is_used && exo_data->hip_torque_flag == 1)  //Check if the right side is used and we want to use the torque sensor
            {
                if (_Joint::right_torque_sensor_used_count < logic_micro_pins::num_available_joints) //If we still have available pins send the next one and increment the counter. If we don't send the not connected pin.
                {
                    return logic_micro_pins::torque_sensor_right[_Joint::right_torque_sensor_used_count++];
                }
                else
                {
                    return logic_micro_pins::not_connected_pin;
                }
            }
            else  //The joint isn't used.
            {
                return logic_micro_pins::not_connected_pin;
            }
            break;
        }
        case (uint8_t)config_defs::joint_id::knee:
        {
            if (utils::get_is_left(id) && exo_data->left_side.knee.is_used && exo_data->knee_torque_flag == 1)  //Check if the left side is used and we want to use the torque sensor 
            {
                if (_Joint::left_torque_sensor_used_count < logic_micro_pins::num_available_joints) //If we still have available pins send the next one and increment the counter. If we don't send the not connected pin.
                {
                    return logic_micro_pins::torque_sensor_left[_Joint::left_torque_sensor_used_count++];
                }
                else
                {
                    return logic_micro_pins::not_connected_pin;
                }
            }
            else if (!(utils::get_is_left(id)) && exo_data->right_side.knee.is_used && exo_data->knee_torque_flag == 1)  //Check if the right side is used and we want to use the torque sensor
            {
                if (_Joint::right_torque_sensor_used_count < logic_micro_pins::num_available_joints) //If we still have available pins send the next one and increment the counter. If we don't send the not connected pin.
                {
                    return logic_micro_pins::torque_sensor_right[_Joint::right_torque_sensor_used_count++];
                }
                else
                {
                    return logic_micro_pins::not_connected_pin;
                }
            }
            else  //The joint isn't used.
            {
                return logic_micro_pins::not_connected_pin;
            }
            break;
        }
        case (uint8_t)config_defs::joint_id::ankle:
        {
            if (utils::get_is_left(id) && exo_data->left_side.ankle.is_used && exo_data->ankle_torque_flag == 1)   //Check if the left side is used and we want to use the torque sensor
            {
                if (_Joint::left_torque_sensor_used_count < logic_micro_pins::num_available_joints)         //If we still have available pins send the next one and increment the counter. If we don't send the not connected pin.
                {
                    return logic_micro_pins::torque_sensor_left[_Joint::left_torque_sensor_used_count++];
                }
                else
                {
                    return logic_micro_pins::not_connected_pin;
                }
            }
            else if (!(utils::get_is_left(id)) && exo_data->right_side.ankle.is_used && exo_data->ankle_torque_flag == 1)  //Check if the right side is used and we want to use the torque sensor
            {
                if (_Joint::right_torque_sensor_used_count < logic_micro_pins::num_available_joints)        //If we still have available pins send the next one and increment the counter. If we don't send the not connected pin.
                {
                    // logger::print("_Joint::right_torque_sensor_used_count : ");
                    // logger::println(_Joint::right_torque_sensor_used_count);
                    return logic_micro_pins::torque_sensor_right[_Joint::right_torque_sensor_used_count++];
                }
                else
                {
                    return logic_micro_pins::not_connected_pin;
                }
            }
            else  //The joint isn't used.
            {
                return logic_micro_pins::not_connected_pin;
            }
            break;
        }
        case (uint8_t)config_defs::joint_id::elbow:
        {
            if (utils::get_is_left(id) && exo_data->left_side.elbow.is_used && exo_data->elbow_torque_flag == 1)   //Check if the left side is used and we want to use the torque sensor
            {
                if (_Joint::left_torque_sensor_used_count < logic_micro_pins::num_available_joints)         //If we still have available pins send the next one and increment the counter. If we don't send the not connected pin.
                {
                    return logic_micro_pins::torque_sensor_left[_Joint::left_torque_sensor_used_count++];
                }
                else
                {
                    return logic_micro_pins::not_connected_pin;
                }
            }
            else if (!(utils::get_is_left(id)) && exo_data->right_side.elbow.is_used && exo_data->elbow_torque_flag == 1)  //Check if the right side is used and we want to use the torque sensor
            {
                if (_Joint::right_torque_sensor_used_count < logic_micro_pins::num_available_joints)        //If we still have available pins send the next one and increment the counter. If we don't send the not connected pin.
                {
                    return logic_micro_pins::torque_sensor_right[_Joint::right_torque_sensor_used_count++];
                }
                else
                {
                    return logic_micro_pins::not_connected_pin;
                }
            }
            else  //The joint isn't used. 
            {
                return logic_micro_pins::not_connected_pin;
            }
            break;
        }
        default :
        {
            return logic_micro_pins::not_connected_pin;
        }

    }
};

unsigned int _Joint::get_motor_enable_pin(config_defs::joint_id id, ExoData* exo_data)
{
    //First check which joint we are looking at. Then go through and if it is the left or right and if it is used. If it is set return the appropriate pin and increment the counter.
    switch (utils::get_joint_type(id))
    {
        case (uint8_t)config_defs::joint_id::hip:
        {
            if (utils::get_is_left(id) & exo_data->left_side.hip.is_used)                        //Check if the left side is used
            {
                if (_Joint::left_motor_used_count < logic_micro_pins::num_available_joints)     //If we still have available pins send the next one and increment the counter. If we don't send the not connected pin.
                {
                    return logic_micro_pins::enable_left_pin[_Joint::left_motor_used_count++];
                }
                else
                {
                    return logic_micro_pins::not_connected_pin;
                }
            }
            else if (!(utils::get_is_left(id)) && exo_data->right_side.hip.is_used)              //Check if the right side is used
            {
                if (_Joint::right_motor_used_count < logic_micro_pins::num_available_joints)    //If we still have available pins send the next one and increment the counter. If we don't send the not connected pin.
                {
                    return logic_micro_pins::enable_right_pin[_Joint::right_motor_used_count++];
                }
                else
                {
                    return logic_micro_pins::not_connected_pin;
                }
            }
            else  //The joint isn't used. I didn't optimize for the minimal number of logical checks because this should just be used at startup.
            {
                return logic_micro_pins::not_connected_pin;
            }
            break;
        }
        case (uint8_t)config_defs::joint_id::knee:
        {
            if (utils::get_is_left(id) & exo_data->left_side.knee.is_used)                       //Check if the left side is used
            {
                if (_Joint::left_motor_used_count < logic_micro_pins::num_available_joints)     //If we still have available pins send the next one and increment the counter.  If we don't send the not connected pin.
                {
                    return logic_micro_pins::enable_left_pin[_Joint::left_motor_used_count++];
                }
                else
                {
                    return logic_micro_pins::not_connected_pin;
                }
            }
            else if (!(utils::get_is_left(id)) && exo_data->right_side.knee.is_used)             //Check if the right side is used
            {
                if (_Joint::right_motor_used_count < logic_micro_pins::num_available_joints)    //If we still have available pins send the next one and increment the counter. If we don't send the not connected pin.
                {
                    return logic_micro_pins::enable_right_pin[_Joint::right_motor_used_count++];
                }
                else
                {
                    return logic_micro_pins::not_connected_pin;
                }
            }
            else  //The joint isn't used. I didn't optimize for the minimal number of logical checks because this should just be used at startup.
            {
                return logic_micro_pins::not_connected_pin;
            }
            break;
        }
        case (uint8_t)config_defs::joint_id::ankle:
        {
            if (utils::get_is_left(id) & exo_data->left_side.ankle.is_used)                      //Check if the left side is used
            {
                if (_Joint::left_motor_used_count < logic_micro_pins::num_available_joints)     //If we still have available pins send the next one and increment the counter. If we don't send the not connected pin.
                {
                    return logic_micro_pins::enable_left_pin[_Joint::left_motor_used_count++];
                }
                else
                {
                    return logic_micro_pins::not_connected_pin;
                }
            }
            else if (!(utils::get_is_left(id)) && exo_data->right_side.ankle.is_used)            //Check if the right side is used
            {
                if (_Joint::right_motor_used_count < logic_micro_pins::num_available_joints)    //If we still have available pins send the next one and increment the counter. If we don't send the not connected pin.
                {
                    return logic_micro_pins::enable_right_pin[_Joint::right_motor_used_count++];
                }
                else
                {
                    return logic_micro_pins::not_connected_pin;
                }
            }
            else  //The joint isn't used. I didn't optimize for the minimal number of logical checks because this should just be used at startup.
            {
                return logic_micro_pins::not_connected_pin;
            }
            break;
        }
        case (uint8_t)config_defs::joint_id::elbow:
        {
            if (utils::get_is_left(id) & exo_data->left_side.elbow.is_used)                      //Check if the left side is used
            {
                if (_Joint::left_motor_used_count < logic_micro_pins::num_available_joints)     //If we still have available pins send the next one and increment the counter. If we don't send the not connected pin.
                {
                    return logic_micro_pins::enable_left_pin[_Joint::left_motor_used_count++];
                }
                else
                {
                    return logic_micro_pins::not_connected_pin;
                }
            }
            else if (!(utils::get_is_left(id)) && exo_data->right_side.elbow.is_used)            //Check if the right side is used
            {
                if (_Joint::right_motor_used_count < logic_micro_pins::num_available_joints)    //If we still have available pins send the next one and increment the counter. If we don't send the not connected pin.
                {
                    return logic_micro_pins::enable_right_pin[_Joint::right_motor_used_count++];
                }
                else
                {
                    return logic_micro_pins::not_connected_pin;
                }
            }
            else  //The joint isn't used. I didn't optimize for the minimal number of logical checks because this should just be used at startup.
            {
                return logic_micro_pins::not_connected_pin;
            }
            break;
        }
        default :
        {
            return logic_micro_pins::not_connected_pin;
        }

    }
};

void _Joint::set_motor(_Motor* new_motor)
{
    _motor = new_motor;
};


//*********************************************
HipJoint::HipJoint(config_defs::joint_id id, ExoData* exo_data)
: _Joint(id, exo_data)  // <-- Initializer list
, _zero_torque(id, exo_data)
, _franks_collins_hip(id, exo_data)
, _constant_torque(id, exo_data)
, _chirp(id, exo_data)
, _step(id, exo_data)
, _proportional_hip_moment(id, exo_data)
, _calibr_manager(id, exo_data)
{
    #ifdef JOINT_DEBUG
        logger::print(_is_left ? "Left " : "Right ");
        logger::println("Hip : Hip Constructor");
    #endif

    //logger::print("HipJoint::HipJoint\n");
    
    //Set _joint_data to point to the data specific to this joint.
    if (_is_left)
    {
        _joint_data = &(exo_data->left_side.hip);
    }
    else
    {
        _joint_data = &(exo_data->right_side.hip);
    }

    #ifdef JOINT_DEBUG
        logger::print(_is_left ? "Left " : "Right ");
        logger::println("Hip : _joint_data set");
    #endif

    // logger::print(uint8_t(id));
    // logger::print("\t");
    // logger::print(_joint_data->is_used);
    // logger::print("\t");
    
    //Don't need to check side as we assume symmetry and create both side data objects. Setup motor from here as it will be easier to check which motor is used
    if(_joint_data->is_used)
    {
        #ifdef JOINT_DEBUG
            logger::print(_is_left ? "Left " : "Right ");
            logger::print("Hip : setting motor to ");
        #endif

        switch (exo_data->left_side.hip.motor.motor_type)
        {
            //Using new so the object of the specific motor type persists.
            case (uint8_t)config_defs::motor::AK60 :
                #ifdef JOINT_DEBUG
                    logger::println("AK60");
                #endif
                HipJoint::set_motor(new AK60(id, exo_data, _Joint::get_motor_enable_pin(id, exo_data)));
                break;
            case (uint8_t)config_defs::motor::AK80 :
                #ifdef JOINT_DEBUG
                    logger::println("AK80");
                #endif
                HipJoint::set_motor(new AK80(id, exo_data, _Joint::get_motor_enable_pin(id, exo_data)));
                break;
            case (uint8_t)config_defs::motor::AK60_v1_1 :
                #ifdef JOINT_DEBUG
                    logger::println("AK60 v1.1");
                #endif
                HipJoint::set_motor(new AK60_v1_1(id, exo_data, _Joint::get_motor_enable_pin(id, exo_data)));
                break;
            case (uint8_t)config_defs::motor::AK70:
                #ifdef JOINT_DEBUG
                    logger::println("AK70");
                #endif
                HipJoint::set_motor(new AK70(id, exo_data, _Joint::get_motor_enable_pin(id, exo_data)));
                break;
			case (uint8_t)config_defs::motor::MaxonMotor:
                #ifdef JOINT_DEBUG
                    logger::println("MaxonMotor");
                #endif
                HipJoint::set_motor(new MaxonMotor(id, exo_data, _Joint::get_motor_enable_pin(id, exo_data)));
                break;
            default :
                #ifdef JOINT_DEBUG
                    logger::println("NULL");
                #endif
                HipJoint::set_motor(new NullMotor(id, exo_data, _Joint::get_motor_enable_pin(id, exo_data)));
                break;
        }

        delay(5);

        #ifdef JOINT_DEBUG
            logger::print(_is_left ? "Left " : "Right ");
            logger::println("Hip : Setting Controller");
        #endif

        set_controller(exo_data->left_side.hip.controller.controller);

        #ifdef JOINT_DEBUG
            logger::print(_is_left ? "Left " : "Right ");
            logger::println("Hip : _controller set");
        #endif
    }
};

void HipJoint::run_joint()
{
    #ifdef JOINT_DEBUG
        logger::print("HipJoint::run_joint::Start");
    #endif

    //Make sure the correct controller is running.
    set_controller(_joint_data->controller.controller);
    
    //Calculate the motor command
    _joint_data->controller.setpoint = _controller->calc_motor_cmd();

    //Check for joint errors
    const uint16_t exo_status = _data->get_status();
    const bool correct_status = (exo_status == status_defs::messages::trial_on) || 
            (exo_status == status_defs::messages::fsr_calibration) || 
            (exo_status == status_defs::messages::fsr_refinement);
    const bool error = correct_status ? _error_manager.run(_joint_data) : false;
    if (error) 
    {
        //Send all errors to the other microcontroller
        for (int i=0; i < _error_manager.errorQueueSize(); i++)
        {
            _motor->set_error();
            ErrorReporter::get_instance()->report(_error_manager.popError(),_id);
        }
    }

    //Enable or disable the motor.
    _motor->on_off(); 
    _motor->enable();               //Do not enable the motors until the GUI tells you to, defaults to not enabled to prevent running until desired. 

    //Send the new command to the motor.
    _motor->transaction(_joint_data->controller.setpoint / _joint_data->motor.gearing);

    #ifdef JOINT_DEBUG
        logger::print("HipJoint::run_joint::Motor Command:: ");
        logger::print(_controller->calc_motor_cmd());
        logger::print("\n");
    #endif

};  

/*
 * Changes the high level controller in Controller
 * Each joint has their own version since they have joint specific controllers.
 */
void HipJoint::set_controller(uint8_t controller_id)   
{
    #ifdef JOINT_DEBUG
        logger::print(_is_left ? "Left " : "Right ");
        logger::println("Hip : set_controller : Entered");
        logger::print("Hip : set_controller : Controller ID : ");
        logger::println(controller_id);
    #endif

    switch (controller_id)
    {
        case (uint8_t)config_defs::hip_controllers::disabled :
            _joint_data->motor.enabled = false;
            _controller = &_zero_torque;
            break;
        case (uint8_t)config_defs::hip_controllers::zero_torque :
            _controller = &_zero_torque;
            break;
        case (uint8_t)config_defs::hip_controllers::franks_collins_hip :
            _controller = &_franks_collins_hip;
            break;
        case (uint8_t)config_defs::hip_controllers::constant_torque:
            _controller = &_constant_torque;
            break;
        case (uint8_t)config_defs::hip_controllers::chirp:
            _controller = &_chirp;
            break;
        case (uint8_t)config_defs::hip_controllers::step:
            _controller = &_step;
            break;
        case (uint8_t)config_defs::hip_controllers::phmc:
            _controller = &_proportional_hip_moment;
            break;
		case (uint8_t)config_defs::hip_controllers::calibr_manager:
            _controller = &_calibr_manager;
            break;
        default :
            logger::print("Unkown Controller!\n", LogLevel::Error);
            _controller = &_zero_torque;
            break;
    }
    #ifdef JOINT_DEBUG
        logger::println("Hip : set_controller : End");
    #endif
};

//================================================================

KneeJoint::KneeJoint(config_defs::joint_id id, ExoData* exo_data)
: _Joint(id, exo_data) // <-- Initializer list
, _zero_torque(id, exo_data)
, _constant_torque(id, exo_data)
, _chirp(id, exo_data)
, _step(id, exo_data)
, _calibr_manager(id, exo_data)
{
    #ifdef JOINT_DEBUG
        logger::print(_is_left ? "Left " : "Right ");
        logger::println("Knee : Knee Constructor");
    #endif

    // logger::print("KneeJoint::KneeJoint\n");
    
    //Set _joint_data to point to the data specific to this joint.
    if (_is_left)
    {
        _joint_data = &(exo_data->left_side.knee);
    }
    else
    {
        _joint_data = &(exo_data->right_side.knee);
    }
    
    #ifdef JOINT_DEBUG
        logger::print(_is_left ? "Left " : "Right ");
        logger::println("Knee : _joint_data set");
    #endif

    // logger::print(uint8_t(id));
    // logger::print("\t");
    // logger::print(_joint_data->is_used);
    // logger::print("\t");
    
    //Don't need to check side as we assume symmetry and create both side data objects. Setup motor from here as it will be easier to check which motor is used
   if(_joint_data->is_used)
   {
        #ifdef JOINT_DEBUG
            logger::print(_is_left ? "Left " : "Right ");
            logger::print("Knee : setting motor to ");
        #endif

        switch (_data->left_side.knee.motor.motor_type)
        {
            //Using new so the object of the specific motor type persists.
            case (uint8_t)config_defs::motor::AK60 :
                #ifdef JOINT_DEBUG
                    logger::println("AK60");
                #endif
                KneeJoint::set_motor(new AK60(id, exo_data, _Joint::get_motor_enable_pin(id, exo_data)));
                break;
            case (uint8_t)config_defs::motor::AK80 :
                #ifdef JOINT_DEBUG
                    logger::println("AK80");
                #endif
                KneeJoint::set_motor(new AK80(id, exo_data, _Joint::get_motor_enable_pin(id, exo_data)));
                break;
            case (uint8_t)config_defs::motor::AK60_v1_1 :
                #ifdef JOINT_DEBUG
                    logger::println("AK60 v1.1");
                #endif
                KneeJoint::set_motor(new AK60_v1_1(id, exo_data, _Joint::get_motor_enable_pin(id, exo_data)));
                break;
            case (uint8_t)config_defs::motor::AK70:
                #ifdef JOINT_DEBUG
                    logger::println("AK70");
                #endif
                KneeJoint::set_motor(new AK70(id, exo_data, _Joint::get_motor_enable_pin(id, exo_data)));
                break;
			case (uint8_t)config_defs::motor::MaxonMotor:
                #ifdef JOINT_DEBUG
                    logger::println("MaxonMotor");
                #endif
                KneeJoint::set_motor(new MaxonMotor(id, exo_data, _Joint::get_motor_enable_pin(id, exo_data)));
                break;
            default :
                #ifdef JOINT_DEBUG
                    logger::println("NULL");
                #endif
                KneeJoint::set_motor(new NullMotor(id, exo_data, _Joint::get_motor_enable_pin(id, exo_data)));
                break;
        }

        delay(5);

        #ifdef JOINT_DEBUG
            logger::print(_is_left ? "Left " : "Right ");
            logger::println("Knee : Setting Controller");
        #endif

        set_controller(exo_data->left_side.knee.controller.controller);

        #ifdef JOINT_DEBUG
            logger::print(_is_left ? "Left " : "Right ");
            logger::println("Knee : _controller set");
        #endif
   }
};

/*
 * Reads the data and sends motor commands
 */
void KneeJoint::run_joint()
{
    #ifdef JOINT_DEBUG
        logger::print("KneeJoint::run_joint::Start");
    #endif

    //Make sure the correct controller is running.
    set_controller(_joint_data->controller.controller);
    
    //Calculate the motor command
    _joint_data->controller.setpoint = _controller->calc_motor_cmd();

    //Check for joint errors
    const uint16_t exo_status = _data->get_status();
    const bool correct_status = (exo_status == status_defs::messages::trial_on) || 
            (exo_status == status_defs::messages::fsr_calibration) || 
            (exo_status == status_defs::messages::fsr_refinement);
    const bool error = correct_status ? _error_manager.run(_joint_data) : false;
    if (error)
    {
        _motor->set_error();
        // end all errors to the other microcontroller
        for (int i=0; i < _error_manager.errorQueueSize(); i++)
        {
            ErrorReporter::get_instance()->report(_error_manager.popError(),_id);
        }
    }

    //Enable or disable the motor.
    _motor->on_off(); 
    _motor->enable();

    //Send the new command to the motor.
    _motor->transaction(_joint_data->controller.setpoint / _joint_data->motor.gearing);

    #ifdef JOINT_DEBUG
        logger::print("KneeJoint::run_joint::Motor Command:: ");
        logger::print(_controller->calc_motor_cmd());
        logger::print("\n");
    #endif
};

/*
 * Changes the high level controller in Controller
 * Each joint has their own version since they have joint specific controllers.
 */
void KneeJoint::set_controller(uint8_t controller_id)  //Changes the high level controller in Controller, and the low level controller in Motor
{
        #ifdef JOINT_DEBUG
        logger::print(_is_left ? "Left " : "Right ");
        logger::println("Knee : set_controller : Entered");
        logger::print("Knee : set_controller : Controller ID : ");
        logger::println(controller_id);
    #endif
    switch (controller_id)
    {
        case (uint8_t)config_defs::knee_controllers::disabled :
            _joint_data->motor.enabled = false;
            _controller = &_zero_torque;
            break;
        case (uint8_t)config_defs::knee_controllers::zero_torque :
            _controller = &_zero_torque;
            break;
        case (uint8_t)config_defs::knee_controllers::constant_torque:
            _controller = &_constant_torque;
            break;
        case (uint8_t)config_defs::knee_controllers::chirp:
            _controller = &_chirp;
            break;
        case (uint8_t)config_defs::knee_controllers::step:
            _controller = &_step;
            break;
		case (uint8_t)config_defs::knee_controllers::calibr_manager:
            _controller = &_calibr_manager;
            break;
        default :
            logger::print("Unkown Controller!\n", LogLevel::Error);
            _controller = &_zero_torque;
            break;
    } 
};

//=================================================================
AnkleJoint::AnkleJoint(config_defs::joint_id id, ExoData* exo_data)
: _Joint(id, exo_data) // <-- Initializer list
, _imu(_is_left)
, _zero_torque(id, exo_data)
, _proportional_joint_moment(id, exo_data)
, _zhang_collins(id, exo_data)
, _constant_torque(id, exo_data)
, _trec(id, exo_data)
, _calibr_manager(id, exo_data)
, _chirp(id, exo_data)
, _step(id, exo_data)
, _spv2(id, exo_data)
, _pjmc_plus(id, exo_data)
{
    #ifdef JOINT_DEBUG
        logger::print(_is_left ? "Left " : "Right ");
        logger::println("Ankle : Ankle Constructor");
    #endif


    //Set _joint_data to point to the data specific to this joint.
    if (_is_left)
    {
        _joint_data = &(exo_data->left_side.ankle);
    }
    else
    {
        _joint_data = &(exo_data->right_side.ankle);
    }
    
    #ifdef JOINT_DEBUG
        logger::print(_is_left ? "Left " : "Right ");
        logger::println("Ankle : _joint_data set");
    #endif

    // logger::print(uint8_t(id));
    // logger::print("\t");
    // logger::print(_joint_data->is_used);
    // logger::print("\t");
            
    //Don't need to check side as we assume symmetry and create both side data objects. Setup motor from here as it will be easier to check which motor is used
    if(_joint_data->is_used)
    {
        #ifdef JOINT_DEBUG
            logger::print(_is_left ? "Left " : "Right ");
            logger::print("Ankle : setting motor to ");
        #endif
        switch (_data->left_side.ankle.motor.motor_type)
        {
            //Using new so the object of the specific motor type persists.
            case (uint8_t)config_defs::motor::AK60 :
                #ifdef JOINT_DEBUG
                    logger::println("AK60");
                #endif
                AnkleJoint::set_motor(new AK60(id, exo_data, _Joint::get_motor_enable_pin(id, exo_data)));
                break;
            case (uint8_t)config_defs::motor::AK80 :
                #ifdef JOINT_DEBUG
                    logger::println("AK80");
                #endif
                AnkleJoint::set_motor(new AK80(id, exo_data, _Joint::get_motor_enable_pin(id, exo_data)));
                break;
            case (uint8_t)config_defs::motor::AK60_v1_1 :
                #ifdef JOINT_DEBUG
                    logger::println("AK60 v1.1");
                #endif
                AnkleJoint::set_motor(new AK60_v1_1(id, exo_data, _Joint::get_motor_enable_pin(id, exo_data)));
                break;
            case (uint8_t)config_defs::motor::AK70:
                #ifdef JOINT_DEBUG
                    logger::println("AK70");
                #endif
                AnkleJoint::set_motor(new AK70(id, exo_data, _Joint::get_motor_enable_pin(id, exo_data)));
                break;
			case (uint8_t)config_defs::motor::MaxonMotor:
                #ifdef JOINT_DEBUG
                    logger::println("MaxonMotor");
                #endif
                AnkleJoint::set_motor(new MaxonMotor(id, exo_data, _Joint::get_motor_enable_pin(id, exo_data)));
                break;
            default :
                #ifdef JOINT_DEBUG
                    logger::println("NULL");
                #endif
                AnkleJoint::set_motor(new NullMotor(id, exo_data, _Joint::get_motor_enable_pin(id, exo_data)));
                break;
        }

        delay(5);

        #ifdef JOINT_DEBUG
            logger::println("_is_left section");
            logger::print(_is_left ? "Left " : "Right ");
            logger::println("Ankle : Setting Controller");
        #endif

        set_controller(exo_data->left_side.ankle.controller.controller);

        #ifdef JOINT_DEBUG
            logger::print(_is_left ? "Left " : "Right ");
            logger::println("Ankle : _controller set");
        #endif
    }  

};

/*
 * Reads the data and sends motor commands
 */
void AnkleJoint::run_joint()
{
    Serial.print("\nAnkleJoint::run_joint()");
	#ifdef JOINT_DEBUG
        logger::print("AnkleJoint::run_joint::Start");
    #endif

    //Angle Sensor data
    _joint_data->prev_joint_position = _joint_data->joint_position;
    const float raw_angle = _joint_data->joint_RoM * _ankle_angle.get(_is_left, false);
    const float new_angle = _joint_data->do_flip_angle ? (_joint_data->joint_RoM - raw_angle):(raw_angle);
    _joint_data->joint_position = utils::ewma(new_angle, _joint_data->joint_position, _joint_data->joint_position_alpha);
    _joint_data->joint_velocity = utils::ewma((_joint_data->joint_position - _joint_data->prev_joint_position) / (1.0f / LOOP_FREQ_HZ), _joint_data->joint_velocity, _joint_data->joint_velocity_alpha);

    // Serial.print(_is_left ? "Left " : "Right ");
    // Serial.print("Ankle Angle: ");
    // Serial.print(_joint_data->joint_position);
    // Serial.print("\n");

    //Make sure the correct controller is running.
    set_controller(_joint_data->controller.controller);

    //Calculate the motor command
    _joint_data->controller.setpoint = _controller->calc_motor_cmd();

    //Check for joint errors
    static float start = micros();

    //Check if the exo is in the correct state to run the error manager (i.e. not in a trial
    const uint16_t exo_status = _data->get_status();
    const bool correct_status = (exo_status == status_defs::messages::trial_on) || 
            (exo_status == status_defs::messages::fsr_calibration) || 
            (exo_status == status_defs::messages::fsr_refinement);
    const bool error = correct_status ? _error_manager.run(_joint_data) : false;

    if (error) 
    {
        _motor->set_error();
        _motor->on_off();
        _motor->enable();
        
        //Send all errors to the other microcontroller
        for (int i=0; i < _error_manager.errorQueueSize(); i++)
        {
            ErrorReporter::get_instance()->report(_error_manager.popError(),_id);
        }
    }

    //Enable or disable the motor.
    _motor->on_off();
    _motor->enable();

    //Send the new command to the motor.
    _motor->transaction(_joint_data->controller.setpoint / _joint_data->motor.gearing);

    #ifdef JOINT_DEBUG
        logger::print("Ankle::run_joint::Motor Command:: ");
        logger::print(_controller->calc_motor_cmd());
        logger::print("\n");
    #endif
};

/*
 * Changes the high level controller in Controller
 * Each joint has their own version since they have joint specific controllers.
 */
void AnkleJoint::set_controller(uint8_t controller_id)  //Changes the high level controller in Controller, and the low level controller in Motor
{
    #ifdef JOINT_DEBUG
        logger::print(_is_left ? "Left " : "Right ");
        logger::println("Ankle : set_controller : Entered");
        logger::print("Ankle : set_controller : Controller ID : ");
        logger::println(controller_id);
    #endif
    switch (controller_id)
    {
        case (uint8_t)config_defs::ankle_controllers::disabled :
            _joint_data->motor.enabled = false;
            _controller = &_zero_torque;
            break;
        case (uint8_t)config_defs::ankle_controllers::zero_torque :
            _controller = &_zero_torque;
            break;
        case (uint8_t)config_defs::ankle_controllers::pjmc :
            _controller = &_proportional_joint_moment;
            break;
        case (uint8_t)config_defs::ankle_controllers::zhang_collins :
            _controller = &_zhang_collins;
            break;
        case (uint8_t)config_defs::ankle_controllers::constant_torque:
            _controller = &_constant_torque;
            break;
        case (uint8_t)config_defs::ankle_controllers::trec:
            _controller = &_trec;
            break;
		case (uint8_t)config_defs::ankle_controllers::calibr_manager:
            _controller = &_calibr_manager;
            break;
        case (uint8_t)config_defs::ankle_controllers::chirp:
            _controller = &_chirp;
            break;
        case (uint8_t)config_defs::ankle_controllers::step:
            _controller = &_step;
            break;
		case (uint8_t)config_defs::ankle_controllers::spv2:
            _controller = &_spv2;
            break;
		case (uint8_t)config_defs::ankle_controllers::pjmc_plus:
            _controller = &_pjmc_plus;
            break;
        default :
            logger::print("Unkown Controller!\n", LogLevel::Error);
            _controller = &_zero_torque;
            break;
    }
};

//=================================================================
ElbowJoint::ElbowJoint(config_defs::joint_id id, ExoData* exo_data)
    : _Joint(id, exo_data) // <-- Initializer list
    , _zero_torque(id, exo_data)
    , _elbow_min_max(id, exo_data)
    , _calibr_manager(id, exo_data)
    , _chirp(id, exo_data)
    , _step(id, exo_data)
{
    #ifdef JOINT_DEBUG
        logger::print(_is_left ? "Left " : "Right ");
        logger::println("Elbow : Elbow Constructor");
    #endif

    //Set _joint_data to point to the data specific to this joint.
    if (_is_left)
    {
        _joint_data = &(exo_data->left_side.elbow);
    }
    else
    {
        _joint_data = &(exo_data->right_side.elbow);
    }

    #ifdef JOINT_DEBUG
        logger::print(_is_left ? "Left " : "Right ");
        logger::println("Elbow : _joint_data set");
    #endif

    // logger::print(uint8_t(id));
    // logger::print("\t");
    // logger::print(_joint_data->is_used);
    // logger::print("\t");

    //Don't need to check side as we assume symmetry and create both side data objects. Setup motor from here as it will be easier to check which motor is used
    if (_joint_data->is_used)
    {
        #ifdef JOINT_DEBUG
                logger::print(_is_left ? "Left " : "Right ");
                logger::print("Elbow : setting motor to ");
        #endif

        switch (_data->left_side.elbow.motor.motor_type)
        {
            //Using new so the object of the specific motor type persists.
            case (uint8_t)config_defs::motor::AK60:
                #ifdef JOINT_DEBUG
                            logger::println("AK60");
                #endif
                ElbowJoint::set_motor(new AK60(id, exo_data, _Joint::get_motor_enable_pin(id, exo_data)));
            break;
            case (uint8_t)config_defs::motor::AK80:
                #ifdef JOINT_DEBUG
                            logger::println("AK80");
                #endif
                ElbowJoint::set_motor(new AK80(id, exo_data, _Joint::get_motor_enable_pin(id, exo_data)));
            break;
            case (uint8_t)config_defs::motor::AK60_v1_1:
                #ifdef JOINT_DEBUG
                            logger::println("AK60 v1.1");
                #endif
                ElbowJoint::set_motor(new AK60_v1_1(id, exo_data, _Joint::get_motor_enable_pin(id, exo_data)));
            break;
            case (uint8_t)config_defs::motor::AK70:
                #ifdef JOINT_DEBUG
                            logger::println("AK70");
                #endif
                ElbowJoint::set_motor(new AK70(id, exo_data, _Joint::get_motor_enable_pin(id, exo_data)));
            break;
			case (uint8_t)config_defs::motor::MaxonMotor:
                #ifdef JOINT_DEBUG
                    logger::println("MaxonMotor");
                #endif
                ElbowJoint::set_motor(new MaxonMotor(id, exo_data, _Joint::get_motor_enable_pin(id, exo_data)));
                break;
            default:
                #ifdef JOINT_DEBUG
                            logger::println("NULL");
                #endif
                ElbowJoint::set_motor(new NullMotor(id, exo_data, _Joint::get_motor_enable_pin(id, exo_data)));
            break;
        }

        delay(5);

        #ifdef JOINT_DEBUG
                logger::println("_is_left section");
                logger::print(_is_left ? "Left " : "Right ");
                logger::println("Elbow : Setting Controller");
        #endif

        set_controller(exo_data->left_side.elbow.controller.controller);

        #ifdef JOINT_DEBUG
                logger::print(_is_left ? "Left " : "Right ");
                logger::println("Elbow : _controller set");
        #endif
    }

};

/*
 * Reads the data and sends motor commands
 */
void ElbowJoint ::run_joint()
{
    #ifdef JOINT_DEBUG
        logger::print("ElbowJoint::run_joint::Start");
    #endif

    //Make sure the correct controller is running.
    set_controller(_joint_data->controller.controller);

    //Calculate the motor command
    _joint_data->controller.setpoint = _controller->calc_motor_cmd();

    //Check for joint errors
    static float start = micros();

    //Check if the exo is in the correct state to run the error manager (i.e. not in a trial
    const uint16_t exo_status = _data->get_status();
    const bool correct_status = (exo_status == status_defs::messages::trial_on) ||
        (exo_status == status_defs::messages::fsr_calibration) ||
        (exo_status == status_defs::messages::fsr_refinement);
    const bool error = correct_status ? _error_manager.run(_joint_data) : false;

    if (error)
    {
        _motor->set_error();
        _motor->on_off();
        _motor->enable();

        //Send all errors to the other microcontroller
        for (int i = 0; i < _error_manager.errorQueueSize(); i++)
        {
            ErrorReporter::get_instance()->report(_error_manager.popError(),_id);
        }
    }

    //Enable or disable the motor.
    _motor->on_off();
    _motor->enable();

    //Send the new command to the motor.
    _motor->transaction(_joint_data->controller.setpoint / _joint_data->motor.gearing);

    #ifdef JOINT_DEBUG
        logger::print("Elbow::run_joint::Motor Command:: ");
        logger::print(_controller->calc_motor_cmd());
        logger::print("\n");
    #endif
};

/*
 * Changes the high level controller in Controller
 * Each joint has their own version since they have joint specific controllers.
 */
void ElbowJoint::set_controller(uint8_t controller_id)  //Changes the high level controller in Controller, and the low level controller in Motor
{
    #ifdef JOINT_DEBUG
        logger::print(_is_left ? "Left " : "Right ");
        logger::println("Elbow : set_controller : Entered");
        logger::print("Elbow : set_controller : Controller ID : ");
        logger::println(controller_id);
    #endif

    switch (controller_id)
    {
        case (uint8_t)config_defs::elbow_controllers::disabled:
            _joint_data->motor.enabled = false;
            _controller = &_zero_torque;
            break;
        case (uint8_t)config_defs::elbow_controllers::zero_torque:
            _controller = &_zero_torque;
            break;
        case (uint8_t)config_defs::elbow_controllers::elbow_min_max:
            _controller = &_elbow_min_max;
            break;
        case (uint8_t)config_defs::elbow_controllers::calibr_manager:
            _controller = &_calibr_manager;
            break;
        case (uint8_t)config_defs::elbow_controllers::chirp:
            _controller = &_chirp;
            break;
        case (uint8_t)config_defs::elbow_controllers::step:
            _controller = &_step;
            break;
        default:
            logger::print("Unkown Controller!\n", LogLevel::Error);
            _controller = &_zero_torque;
            break;
    }
};
#endif
