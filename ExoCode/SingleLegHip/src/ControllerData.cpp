/*
 * 
 * P. Stegall Jan. 2022
*/

#include "ControllerData.h"

namespace
{
    struct ParameterBoundConfig
    {
        bool enabled;
        float min_value;
        float max_value;
        bool integer_only;
    };

    inline ParameterBoundConfig param_bound(
        bool enabled,
        float min_value,
        float max_value,
        bool integer_only)
    {
        ParameterBoundConfig bound = {enabled, min_value, max_value, integer_only};
        return bound;
    }

    inline bool read_parameter_bound(
        const ParameterBoundConfig* bounds,
        uint8_t bound_count,
        uint8_t parameter_index,
        float* min_out,
        float* max_out,
        bool* integer_only_out)
    {
        if (bounds == NULL || parameter_index >= bound_count)
        {
            return false;
        }

        const ParameterBoundConfig& bound = bounds[parameter_index];
        if (!bound.enabled)
        {
            return false;
        }

        if (min_out != NULL)
        {
            *min_out = bound.min_value;
        }
        if (max_out != NULL)
        {
            *max_out = bound.max_value;
        }
        if (integer_only_out != NULL)
        {
            *integer_only_out = bound.integer_only;
        }
        return true;
    }

    #define PARAM_BOUND_COUNT(bounds) ((uint8_t)(sizeof(bounds) / sizeof(bounds[0])))

    // Per-controller bounds are edited here. Each row is: enabled, min, max, integer_only.
    // Disabled rows preserve suggested defaults but are not enforced until enabled is set true.
    const ParameterBoundConfig zero_torque_bounds[controller_defs::zero_torque::num_parameter] =
    {
        param_bound(false, 0.0f, 1.0f, true),       // 0 use_pid
        param_bound(false, 0.0f, 10000.0f, false), // 1 p_gain
        param_bound(false, 0.0f, 10000.0f, false), // 2 i_gain
        param_bound(false, 0.0f, 10000.0f, false), // 3 d_gain
    };

    const ParameterBoundConfig pjmc_bounds[controller_defs::proportional_joint_moment::num_parameter] =
    {
        param_bound(true, 0.0f, 100.0f, false),    // 0 stance_max
        param_bound(true, 0.0f, 100.0f, false),    // 1 swing_max
        param_bound(true, 0.0f, 1.0f, true),       // 2 is_assistance
        param_bound(true, 0.0f, 1.0f, true),       // 3 use_pid
        param_bound(true, 0.0f, 10000.0f, false),  // 4 p_gain
        param_bound(true, 0.0f, 10000.0f, false),  // 5 i_gain
        param_bound(true, 0.0f, 10000.0f, false),  // 6 d_gain
        param_bound(true, 0.0f, 1.0f, false),      // 7 torque_alpha
        param_bound(true, 0.0f, 1.0f, true),       // 8 GS_Flag
        param_bound(true, 0.0f, 10000.0f, false),  // 9 kp_zero
        param_bound(true, 0.0f, 10000.0f, false),  // 10 ki_zero
        param_bound(true, 0.0f, 10000.0f, false),  // 11 kd_zero
    };

    const ParameterBoundConfig zhang_collins_bounds[controller_defs::zhang_collins::num_parameter] =
    {
        param_bound(false, -100.0f, 100.0f, false), // 0 torque
        param_bound(false, 0.0f, 100.0f, false),    // 1 peak_time
        param_bound(false, 0.0f, 100.0f, false),    // 2 rise_time
        param_bound(false, 0.0f, 100.0f, false),    // 3 fall_time
        param_bound(false, 0.0f, 1.0f, true),       // 4 direction
        param_bound(false, 0.0f, 1.0f, true),       // 5 sim_gait
        param_bound(false, 0.0f, 1.0f, true),       // 6 use_pid
        param_bound(false, 0.0f, 10000.0f, false),  // 7 p_gain
        param_bound(false, 0.0f, 10000.0f, false),  // 8 i_gain
        param_bound(false, 0.0f, 10000.0f, false),  // 9 d_gain
    };

    const ParameterBoundConfig spline_bounds[controller_defs::spline::num_parameter] =
    {
        param_bound(false, 0.0f, 100.0f, false),    // 0 node1_x
        param_bound(false, -100.0f, 100.0f, false), // 1 node1_y
        param_bound(false, 0.0f, 100.0f, false),    // 2 node2_x
        param_bound(false, -100.0f, 100.0f, false), // 3 node2_y
        param_bound(false, 0.0f, 100.0f, false),    // 4 node3_x
        param_bound(false, -100.0f, 100.0f, false), // 5 node3_y
        param_bound(false, 0.0f, 100.0f, false),    // 6 node4_x
        param_bound(false, -100.0f, 100.0f, false), // 7 node4_y
        param_bound(false, 0.0f, 100.0f, false),    // 8 node5_x
        param_bound(false, -100.0f, 100.0f, false), // 9 node5_y
        param_bound(false, 0.0f, 1.0f, true),       // 10 sim_gait
        param_bound(false, 0.0f, 1.0f, true),       // 11 use_percent_gait
        param_bound(false, 0.0f, 1.0f, true),       // 12 use_pid
        param_bound(false, 0.0f, 10000.0f, false),  // 13 p_gain
        param_bound(false, 0.0f, 10000.0f, false),  // 14 i_gain
        param_bound(false, 0.0f, 10000.0f, false),  // 15 d_gain
    };

    const ParameterBoundConfig franks_collins_hip_bounds[controller_defs::franks_collins_hip::num_parameter] =
    {
        param_bound(false, 0.0f, 300.0f, false),    // 0 mass
        param_bound(false, -5.0f, 5.0f, false),     // 1 trough_normalized_torque
        param_bound(false, -5.0f, 5.0f, false),     // 2 peak_normalized_torque
        param_bound(false, 0.0f, 100.0f, false),    // 3 start_percent_gait
        param_bound(false, 0.0f, 100.0f, false),    // 4 trough_onset_percent_gait
        param_bound(false, 0.0f, 100.0f, false),    // 5 trough_percent_gait
        param_bound(false, 0.0f, 100.0f, false),    // 6 mid_time
        param_bound(false, 0.0f, 100.0f, false),    // 7 mid_duration
        param_bound(false, 0.0f, 100.0f, false),    // 8 peak_percent_gait
        param_bound(false, 0.0f, 100.0f, false),    // 9 peak_offset_percent_gait
        param_bound(false, 0.0f, 1.0f, true),       // 10 sim_gait
        param_bound(false, 0.0f, 1.0f, true),       // 11 use_pid
        param_bound(false, 0.0f, 10000.0f, false),  // 12 p_gain
        param_bound(false, 0.0f, 10000.0f, false),  // 13 i_gain
        param_bound(false, 0.0f, 10000.0f, false),  // 14 d_gain
    };

    const ParameterBoundConfig constant_torque_bounds[controller_defs::constant_torque::num_parameter] =
    {
        param_bound(false, -100.0f, 100.0f, false), // 0 amplitude
        param_bound(false, 0.0f, 1.0f, true),       // 1 direction
        param_bound(false, 0.0f, 100.0f, false),    // 2 alpha
        param_bound(false, 0.0f, 1.0f, true),       // 3 use_pid
        param_bound(false, 0.0f, 10000.0f, false),  // 4 p_gain
        param_bound(false, 0.0f, 10000.0f, false),  // 5 i_gain
        param_bound(false, 0.0f, 10000.0f, false),  // 6 d_gain
    };

    const ParameterBoundConfig elbow_min_max_bounds[controller_defs::elbow_min_max::num_parameter] =
    {
        param_bound(false, 0.0f, 100.0f, false),    // 0 FLEXamplitude
        param_bound(false, 0.0f, 1024.0f, false),   // 1 DigitFSR_threshold
        param_bound(false, 0.0f, 1024.0f, false),   // 2 PalmFSR_threshold
        param_bound(false, 0.0f, 1024.0f, false),   // 3 DigitFSR_LOWthreshold
        param_bound(false, 0.0f, 1024.0f, false),   // 4 PalmFSR_LOWthreshold
        param_bound(false, 0.0f, 1.0f, true),       // 5 CaliRequest
        param_bound(false, 0.0f, 1.0f, true),       // 6 TrqProfile
        param_bound(false, 0.0f, 10000.0f, false),  // 7 P_gain
        param_bound(false, 0.0f, 10000.0f, false),  // 8 I_gain
        param_bound(false, 0.0f, 10000.0f, false),  // 9 D_gain
        param_bound(false, 0.0f, 100.0f, false),    // 10 TorqueLimit
        param_bound(false, 0.0f, 100.0f, false),    // 11 SpringPkTorque
        param_bound(false, 0.0f, 100.0f, false),    // 12 EXTamplitude
        param_bound(false, 0.0f, 100.0f, false),    // 13 FiltStrength
    };

    const ParameterBoundConfig trec_bounds[controller_defs::trec::num_parameter] =
    {
        param_bound(false, 0.0f, 100.0f, false),    // 0 plantar_scaling
        param_bound(false, 0.0f, 100.0f, false),    // 1 dorsi_scaling
        param_bound(false, 0.0f, 100.0f, false),    // 2 timing_threshold
        param_bound(false, 0.0f, 10000.0f, false),  // 3 spring_stiffness
        param_bound(false, -180.0f, 180.0f, false), // 4 neutral_angle
        param_bound(false, 0.0f, 10000.0f, false),  // 5 damping
        param_bound(false, 0.0f, 10000.0f, false),  // 6 propulsive_gain
        param_bound(false, 0.0f, 10000.0f, false),  // 7 kp
        param_bound(false, 0.0f, 10000.0f, false),  // 8 kd
        param_bound(false, 0.0f, 1.0f, true),       // 9 turn_on_peak_limiter
    };

    const ParameterBoundConfig chirp_bounds[controller_defs::chirp::num_parameter] =
    {
        param_bound(false, -100.0f, 100.0f, false), // 0 amplitude
        param_bound(false, 0.0f, 100.0f, false),    // 1 start_frequency
        param_bound(false, 0.0f, 100.0f, false),    // 2 end_frequency
        param_bound(false, 0.0f, 10000.0f, false),  // 3 duration
        param_bound(false, -100.0f, 100.0f, false), // 4 yshift
        param_bound(false, 0.0f, 1.0f, true),       // 5 pid_flag
        param_bound(false, 0.0f, 10000.0f, false),  // 6 p_gain
        param_bound(false, 0.0f, 10000.0f, false),  // 7 i_gain
        param_bound(false, 0.0f, 10000.0f, false),  // 8 d_gain
    };

    const ParameterBoundConfig step_bounds[controller_defs::step::num_parameter] =
    {
        param_bound(false, -100.0f, 100.0f, false), // 0 amplitude
        param_bound(false, 0.0f, 10000.0f, false),  // 1 duration
        param_bound(false, 0.0f, 10000.0f, true),   // 2 repetitions
        param_bound(false, 0.0f, 10000.0f, false),  // 3 spacing
        param_bound(false, 0.0f, 1.0f, true),       // 4 pid_flag
        param_bound(false, 0.0f, 10000.0f, false),  // 5 p_gain
        param_bound(false, 0.0f, 10000.0f, false),  // 6 i_gain
        param_bound(false, 0.0f, 10000.0f, false),  // 7 d_gain
        param_bound(false, 0.0f, 100.0f, false),    // 8 alpha
    };

    const ParameterBoundConfig phmc_bounds[controller_defs::proportional_hip_moment::num_parameter] =
    {
        param_bound(false, -100.0f, 100.0f, false), // 0 extension_setpoint
        param_bound(false, -100.0f, 100.0f, false), // 1 flexion_setpoint
    };

    const ParameterBoundConfig calibr_manager_bounds[controller_defs::calibr_manager::num_parameter] =
    {
        param_bound(false, 0.0f, 1.0f, true),       // 0 calibr_cmd
    };

    const ParameterBoundConfig spv2_bounds[controller_defs::spv2::num_parameter] =
    {
        param_bound(false, 0.0f, 100.0f, false),    // 0 plantar_scaling
        param_bound(false, 0.0f, 100.0f, false),    // 1 dorsi_scaling
        param_bound(false, 0.0f, 100.0f, false),    // 2 timing_threshold
        param_bound(false, 0.0f, 10000.0f, false),  // 3 spring_stiffness_adj_factor
        param_bound(false, 0.0f, 180.0f, false),    // 4 neutral_angle
        param_bound(false, 0.0f, 180.0f, false),    // 5 min_angle
        param_bound(false, 0.0f, 180.0f, false),    // 6 max_angle
        param_bound(false, 0.0f, 10000.0f, false),  // 7 kp
        param_bound(false, 0.0f, 10000.0f, false),  // 8 kd
        param_bound(false, 0.0f, 1.0f, true),       // 9 turn_on_peak_limiter
        param_bound(false, 0.0f, 1.0f, true),       // 10 do_update_stiffness
        param_bound(false, 0.0f, 10000.0f, false),  // 11 ki
        param_bound(false, 0.0f, 1.0f, true),       // 12 do_use_servo
        param_bound(false, 0.0f, 100.0f, false),    // 13 fsr_servo_threshold
        param_bound(false, 0.0f, 180.0f, false),    // 14 servo_origin
        param_bound(false, 0.0f, 180.0f, false),    // 15 servo_terminal
        param_bound(false, 1.0f, 10000.0f, true),   // 16 motor_current_calc_win
        param_bound(false, 0.0f, 10000.0f, false),  // 17 spring_stiffness
        param_bound(false, 0.0f, 10000.0f, false),  // 18 damping
        param_bound(false, 0.0f, 1.0f, true),       // 19 soft_or_stiff
        param_bound(false, 0.0f, 180.0f, false),    // 20 servo_angle_soft
        param_bound(false, 0.0f, 180.0f, false),    // 21 servo_angle_stiff
    };

    const ParameterBoundConfig pjmc_plus_bounds[controller_defs::pjmc_plus::num_parameter] =
    {
        param_bound(true, 0.0f, 100.0f, false),     // 0 plantar_scaling
        param_bound(true, 0.0f, 100.0f, false),     // 1 dorsi_scaling
        param_bound(true, 0.0f, 100.0f, false),     // 2 timing_threshold
        param_bound(true, 0.0f, 10000.0f, false),   // 3 spring_stiffness
        param_bound(true, -180.0f, 180.0f, false),  // 4 neutral_angle
        param_bound(true, 0.0f, 10000.0f, false),   // 5 damping
        param_bound(true, 0.0f, 10000.0f, false),   // 6 propulsive_gain
        param_bound(true, 0.0f, 10000.0f, false),   // 7 kp
        param_bound(true, 0.0f, 10000.0f, false),   // 8 kd
        param_bound(true, 0.0f, 1.0f, true),        // 9 turn_on_peak_limiter
        param_bound(true, 0.0f, 1.0f, true),        // 10 step_response_mode
        param_bound(true, 0.0f, 10000.0f, false),   // 11 ki
        param_bound(true, 0.0f, 1.0f, true),        // 12 do_use_servo
        param_bound(true, 0.0f, 100.0f, false),     // 13 fsr_servo_threshold
        param_bound(true, 0.0f, 180.0f, false),     // 14 servo_origin
        param_bound(true, 0.0f, 180.0f, false),     // 15 servo_terminal
        param_bound(true, 0.0f, 10000.0f, true),    // 16 maxon_outOfOffice_itr
    };

    bool bounds_for_zero_torque(uint8_t parameter_index, float* min_out, float* max_out, bool* integer_only_out)
    {
        return read_parameter_bound(zero_torque_bounds, PARAM_BOUND_COUNT(zero_torque_bounds), parameter_index, min_out, max_out, integer_only_out);
    }

    bool bounds_for_pjmc(uint8_t parameter_index, float* min_out, float* max_out, bool* integer_only_out)
    {
        return read_parameter_bound(pjmc_bounds, PARAM_BOUND_COUNT(pjmc_bounds), parameter_index, min_out, max_out, integer_only_out);
    }

    bool bounds_for_zhang_collins(uint8_t parameter_index, float* min_out, float* max_out, bool* integer_only_out)
    {
        return read_parameter_bound(zhang_collins_bounds, PARAM_BOUND_COUNT(zhang_collins_bounds), parameter_index, min_out, max_out, integer_only_out);
    }

    bool bounds_for_spline(uint8_t parameter_index, float* min_out, float* max_out, bool* integer_only_out)
    {
        return read_parameter_bound(spline_bounds, PARAM_BOUND_COUNT(spline_bounds), parameter_index, min_out, max_out, integer_only_out);
    }

    bool bounds_for_franks_collins_hip(uint8_t parameter_index, float* min_out, float* max_out, bool* integer_only_out)
    {
        return read_parameter_bound(franks_collins_hip_bounds, PARAM_BOUND_COUNT(franks_collins_hip_bounds), parameter_index, min_out, max_out, integer_only_out);
    }

    bool bounds_for_constant_torque(uint8_t parameter_index, float* min_out, float* max_out, bool* integer_only_out)
    {
        return read_parameter_bound(constant_torque_bounds, PARAM_BOUND_COUNT(constant_torque_bounds), parameter_index, min_out, max_out, integer_only_out);
    }

    bool bounds_for_elbow_min_max(uint8_t parameter_index, float* min_out, float* max_out, bool* integer_only_out)
    {
        return read_parameter_bound(elbow_min_max_bounds, PARAM_BOUND_COUNT(elbow_min_max_bounds), parameter_index, min_out, max_out, integer_only_out);
    }

    bool bounds_for_trec(uint8_t parameter_index, float* min_out, float* max_out, bool* integer_only_out)
    {
        return read_parameter_bound(trec_bounds, PARAM_BOUND_COUNT(trec_bounds), parameter_index, min_out, max_out, integer_only_out);
    }

    bool bounds_for_chirp(uint8_t parameter_index, float* min_out, float* max_out, bool* integer_only_out)
    {
        return read_parameter_bound(chirp_bounds, PARAM_BOUND_COUNT(chirp_bounds), parameter_index, min_out, max_out, integer_only_out);
    }

    bool bounds_for_step(uint8_t parameter_index, float* min_out, float* max_out, bool* integer_only_out)
    {
        return read_parameter_bound(step_bounds, PARAM_BOUND_COUNT(step_bounds), parameter_index, min_out, max_out, integer_only_out);
    }

    bool bounds_for_phmc(uint8_t parameter_index, float* min_out, float* max_out, bool* integer_only_out)
    {
        return read_parameter_bound(phmc_bounds, PARAM_BOUND_COUNT(phmc_bounds), parameter_index, min_out, max_out, integer_only_out);
    }

    bool bounds_for_calibr_manager(uint8_t parameter_index, float* min_out, float* max_out, bool* integer_only_out)
    {
        return read_parameter_bound(calibr_manager_bounds, PARAM_BOUND_COUNT(calibr_manager_bounds), parameter_index, min_out, max_out, integer_only_out);
    }

    bool bounds_for_spv2(uint8_t parameter_index, float* min_out, float* max_out, bool* integer_only_out)
    {
        return read_parameter_bound(spv2_bounds, PARAM_BOUND_COUNT(spv2_bounds), parameter_index, min_out, max_out, integer_only_out);
    }

    bool bounds_for_pjmc_plus(uint8_t parameter_index, float* min_out, float* max_out, bool* integer_only_out)
    {
        return read_parameter_bound(pjmc_plus_bounds, PARAM_BOUND_COUNT(pjmc_plus_bounds), parameter_index, min_out, max_out, integer_only_out);
    }
}

/*
 * Constructor for the controller data.
 * Takes the joint id and the array from the INI parser.
 * Stores the id, sets the controller to the default controller for the appropriate joint, and records the joint type to check we are using appropriate controllers. 
 */
ControllerData::ControllerData(config_defs::joint_id id, uint8_t* config_to_send)
{
    
    switch ((uint8_t)id & (~(uint8_t)config_defs::joint_id::left & ~(uint8_t)config_defs::joint_id::right))  //Use the id with the side masked out.
    {
        case (uint8_t)config_defs::joint_id::hip:
        {
            controller = config_to_send[config_defs::exo_hip_default_controller_idx];
            joint = config_defs::JointType::hip;
            break;
        }
        case (uint8_t)config_defs::joint_id::knee:
        {
            controller = config_to_send[config_defs::exo_knee_default_controller_idx];
            joint = config_defs::JointType::knee;
            break;
        }
        case (uint8_t)config_defs::joint_id::ankle:
        {
            controller = config_to_send[config_defs::exo_ankle_default_controller_idx];
            joint = config_defs::JointType::ankle;
            break;
        }
        case (uint8_t)config_defs::joint_id::elbow:
        {
            controller = config_to_send[config_defs::exo_elbow_default_controller_idx];
            joint = config_defs::JointType::elbow;
            break;
        }
        case (uint8_t)config_defs::joint_id::arm_1:
        {
            controller = config_to_send[config_defs::exo_arm_1_default_controller_idx];
            joint = config_defs::JointType::arm_1;
            break;
        }
        case (uint8_t)config_defs::joint_id::arm_2:
        {
            controller = config_to_send[config_defs::exo_arm_2_default_controller_idx];
            joint = config_defs::JointType::arm_2;
            break;
        }
    }
    
    setpoint = 0;
    parameter_set = 0;

    for (int i=0; i < controller_defs::max_parameters; i++)
    {    
        parameters[i] = 0;
    }

    filtered_cmd = 0;
    filtered_torque_reading = 0;
};

void ControllerData::reconfigure(uint8_t* config_to_send) 
{
    //Just reset controller
    switch ((uint8_t)joint)
    {
        case (uint8_t)config_defs::JointType::hip:
        {
            controller = config_to_send[config_defs::exo_hip_default_controller_idx];
            break;
        }
        case (uint8_t)config_defs::JointType::knee:
        {
            controller = config_to_send[config_defs::exo_knee_default_controller_idx];
            break;
        }
        case (uint8_t)config_defs::JointType::ankle:
        {
            controller = config_to_send[config_defs::exo_ankle_default_controller_idx];
            break;
        }
        case (uint8_t)config_defs::JointType::elbow:
        {
            controller = config_to_send[config_defs::exo_elbow_default_controller_idx];
            break;
        }
        case (uint8_t)config_defs::JointType::arm_1:
        {
            controller = config_to_send[config_defs::exo_arm_1_default_controller_idx];
            break;
        }
        case (uint8_t)config_defs::JointType::arm_2:
        {
            controller = config_to_send[config_defs::exo_arm_2_default_controller_idx];
            break;
        }
    }
    
    setpoint = 0;

    for (int i=0; i < controller_defs::max_parameters; i++)
    {    
        parameters[i] = 0;
    }
};


uint8_t ControllerData::get_parameter_length()
{
    return get_parameter_length_for(joint, controller);
}

uint8_t ControllerData::get_parameter_length_for(config_defs::JointType joint, uint8_t controller_id)
{
    switch ((uint8_t)joint)
    {
        case (uint8_t)config_defs::JointType::hip:
        {
            switch (controller_id)
            {
                case (uint8_t)config_defs::hip_controllers::disabled:
                case (uint8_t)config_defs::hip_controllers::zero_torque:
                    return controller_defs::zero_torque::num_parameter;
                case (uint8_t)config_defs::hip_controllers::franks_collins_hip:
                    return controller_defs::franks_collins_hip::num_parameter;
                case (uint8_t)config_defs::hip_controllers::constant_torque:
                    return controller_defs::constant_torque::num_parameter;
                case (uint8_t)config_defs::hip_controllers::chirp:
                    return controller_defs::chirp::num_parameter;
                case (uint8_t)config_defs::hip_controllers::step:
                    return controller_defs::step::num_parameter;
                case (uint8_t)config_defs::hip_controllers::phmc:
                    return controller_defs::proportional_hip_moment::num_parameter;
                case (uint8_t)config_defs::hip_controllers::calibr_manager:
                    return controller_defs::calibr_manager::num_parameter;
                case (uint8_t)config_defs::hip_controllers::spline:
                    return controller_defs::spline::num_parameter;
                default:
                    return 0;
            }
        }
        case (uint8_t)config_defs::JointType::knee:
        {
            switch (controller_id)
            {
                case (uint8_t)config_defs::knee_controllers::disabled:
                case (uint8_t)config_defs::knee_controllers::zero_torque:
                    return controller_defs::zero_torque::num_parameter;
                case (uint8_t)config_defs::knee_controllers::constant_torque:
                    return controller_defs::constant_torque::num_parameter;
                case (uint8_t)config_defs::knee_controllers::chirp:
                    return controller_defs::chirp::num_parameter;
                case (uint8_t)config_defs::knee_controllers::step:
                    return controller_defs::step::num_parameter;
                case (uint8_t)config_defs::knee_controllers::calibr_manager:
                    return controller_defs::calibr_manager::num_parameter;
                default:
                    return 0;
            }
        }
        case (uint8_t)config_defs::JointType::ankle:
        {
            switch (controller_id)
            {
                case (uint8_t)config_defs::ankle_controllers::disabled:
                case (uint8_t)config_defs::ankle_controllers::zero_torque:
                    return controller_defs::zero_torque::num_parameter;
                case (uint8_t)config_defs::ankle_controllers::pjmc:
                    return controller_defs::proportional_joint_moment::num_parameter;
                case (uint8_t)config_defs::ankle_controllers::zhang_collins:
                    return controller_defs::zhang_collins::num_parameter;
                case (uint8_t)config_defs::ankle_controllers::constant_torque:
                    return controller_defs::constant_torque::num_parameter;
                case (uint8_t)config_defs::ankle_controllers::trec:
                    return controller_defs::trec::num_parameter;
                case (uint8_t)config_defs::ankle_controllers::calibr_manager:
                    return controller_defs::calibr_manager::num_parameter;
                case (uint8_t)config_defs::ankle_controllers::chirp:
                    return controller_defs::chirp::num_parameter;
                case (uint8_t)config_defs::ankle_controllers::step:
                    return controller_defs::step::num_parameter;
                case (uint8_t)config_defs::ankle_controllers::spv2:
                    return controller_defs::spv2::num_parameter;
                case (uint8_t)config_defs::ankle_controllers::pjmc_plus:
                    return controller_defs::pjmc_plus::num_parameter;
                case (uint8_t)config_defs::ankle_controllers::spline:
                    return controller_defs::spline::num_parameter;
                default:
                    return 0;
            }
        }
        case (uint8_t)config_defs::JointType::elbow:
        {
            switch (controller_id)
            {
                case (uint8_t)config_defs::elbow_controllers::disabled:
                case (uint8_t)config_defs::elbow_controllers::zero_torque:
                    return controller_defs::zero_torque::num_parameter;
                case (uint8_t)config_defs::elbow_controllers::elbow_min_max:
                    return controller_defs::elbow_min_max::num_parameter;
                case (uint8_t)config_defs::elbow_controllers::calibr_manager:
                    return controller_defs::calibr_manager::num_parameter;
                case (uint8_t)config_defs::elbow_controllers::chirp:
                    return controller_defs::chirp::num_parameter;
                case (uint8_t)config_defs::elbow_controllers::step:
                    return controller_defs::step::num_parameter;
                default:
                    return 0;
            }
        }
        case (uint8_t)config_defs::JointType::arm_1:
        {
            switch (controller_id)
            {
                case (uint8_t)config_defs::arm_1_controllers::disabled:
                case (uint8_t)config_defs::arm_1_controllers::zero_torque:
                    return controller_defs::zero_torque::num_parameter;
                case (uint8_t)config_defs::arm_1_controllers::constant_torque:
                    return controller_defs::constant_torque::num_parameter;
                case (uint8_t)config_defs::arm_1_controllers::spline:
                    return controller_defs::spline::num_parameter;
                default:
                    return 0;
            }
        }
        case (uint8_t)config_defs::JointType::arm_2:
        {
            switch (controller_id)
            {
                case (uint8_t)config_defs::arm_2_controllers::disabled:
                case (uint8_t)config_defs::arm_2_controllers::zero_torque:
                    return controller_defs::zero_torque::num_parameter;
                case (uint8_t)config_defs::arm_2_controllers::constant_torque:
                    return controller_defs::constant_torque::num_parameter;
                case (uint8_t)config_defs::arm_2_controllers::spline:
                    return controller_defs::spline::num_parameter;
                default:
                    return 0;
            }
        }
        default:
            return 0;
    }
}

bool ControllerData::get_parameter_bounds_for(
    config_defs::JointType joint,
    uint8_t controller_id,
    uint8_t parameter_index,
    float* min_value,
    float* max_value,
    bool* integer_only)
{
    switch ((uint8_t)joint)
    {
        case (uint8_t)config_defs::JointType::hip:
        {
            switch (controller_id)
            {
                case (uint8_t)config_defs::hip_controllers::disabled:
                case (uint8_t)config_defs::hip_controllers::zero_torque:
                    return bounds_for_zero_torque(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::hip_controllers::franks_collins_hip:
                    return bounds_for_franks_collins_hip(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::hip_controllers::constant_torque:
                    return bounds_for_constant_torque(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::hip_controllers::chirp:
                    return bounds_for_chirp(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::hip_controllers::step:
                    return bounds_for_step(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::hip_controllers::phmc:
                    return bounds_for_phmc(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::hip_controllers::calibr_manager:
                    return bounds_for_calibr_manager(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::hip_controllers::spline:
                    return bounds_for_spline(parameter_index, min_value, max_value, integer_only);
                default:
                    return false;
            }
        }
        case (uint8_t)config_defs::JointType::knee:
        {
            switch (controller_id)
            {
                case (uint8_t)config_defs::knee_controllers::disabled:
                case (uint8_t)config_defs::knee_controllers::zero_torque:
                    return bounds_for_zero_torque(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::knee_controllers::constant_torque:
                    return bounds_for_constant_torque(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::knee_controllers::chirp:
                    return bounds_for_chirp(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::knee_controllers::step:
                    return bounds_for_step(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::knee_controllers::calibr_manager:
                    return bounds_for_calibr_manager(parameter_index, min_value, max_value, integer_only);
                default:
                    return false;
            }
        }
        case (uint8_t)config_defs::JointType::ankle:
        {
            switch (controller_id)
            {
                case (uint8_t)config_defs::ankle_controllers::disabled:
                case (uint8_t)config_defs::ankle_controllers::zero_torque:
                    return bounds_for_zero_torque(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::ankle_controllers::pjmc:
                    return bounds_for_pjmc(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::ankle_controllers::zhang_collins:
                    return bounds_for_zhang_collins(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::ankle_controllers::constant_torque:
                    return bounds_for_constant_torque(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::ankle_controllers::trec:
                    return bounds_for_trec(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::ankle_controllers::calibr_manager:
                    return bounds_for_calibr_manager(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::ankle_controllers::chirp:
                    return bounds_for_chirp(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::ankle_controllers::step:
                    return bounds_for_step(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::ankle_controllers::spv2:
                    return bounds_for_spv2(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::ankle_controllers::pjmc_plus:
                    return bounds_for_pjmc_plus(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::ankle_controllers::spline:
                    return bounds_for_spline(parameter_index, min_value, max_value, integer_only);
                default:
                    return false;
            }
        }
        case (uint8_t)config_defs::JointType::elbow:
        {
            switch (controller_id)
            {
                case (uint8_t)config_defs::elbow_controllers::disabled:
                case (uint8_t)config_defs::elbow_controllers::zero_torque:
                    return bounds_for_zero_torque(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::elbow_controllers::elbow_min_max:
                    return bounds_for_elbow_min_max(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::elbow_controllers::calibr_manager:
                    return bounds_for_calibr_manager(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::elbow_controllers::chirp:
                    return bounds_for_chirp(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::elbow_controllers::step:
                    return bounds_for_step(parameter_index, min_value, max_value, integer_only);
                default:
                    return false;
            }
        }
        case (uint8_t)config_defs::JointType::arm_1:
        {
            switch (controller_id)
            {
                case (uint8_t)config_defs::arm_1_controllers::disabled:
                case (uint8_t)config_defs::arm_1_controllers::zero_torque:
                    return bounds_for_zero_torque(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::arm_1_controllers::constant_torque:
                    return bounds_for_constant_torque(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::arm_1_controllers::spline:
                    return bounds_for_spline(parameter_index, min_value, max_value, integer_only);
                default:
                    return false;
            }
        }
        case (uint8_t)config_defs::JointType::arm_2:
        {
            switch (controller_id)
            {
                case (uint8_t)config_defs::arm_2_controllers::disabled:
                case (uint8_t)config_defs::arm_2_controllers::zero_torque:
                    return bounds_for_zero_torque(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::arm_2_controllers::constant_torque:
                    return bounds_for_constant_torque(parameter_index, min_value, max_value, integer_only);
                case (uint8_t)config_defs::arm_2_controllers::spline:
                    return bounds_for_spline(parameter_index, min_value, max_value, integer_only);
                default:
                    return false;
            }
        }
        default:
            return false;
    }
}
