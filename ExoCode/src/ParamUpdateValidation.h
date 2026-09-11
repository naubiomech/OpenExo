#ifndef PARAM_UPDATE_VALIDATION_H
#define PARAM_UPDATE_VALIDATION_H

#include "Arduino.h"
#include "ControllerData.h"
#include "ExoData.h"
#include "JointData.h"
#include "Logger.h"
#include "ParseIni.h"

namespace param_update
{
    static const uint8_t k_expected_ble_fields = 4;
    static const uint8_t k_side_left = (uint8_t)config_defs::joint_id::left;
    static const uint8_t k_side_right = (uint8_t)config_defs::joint_id::right;
    static const uint8_t k_side_mask = k_side_left | k_side_right;

    enum class RejectionReason : uint8_t
    {
        accepted = 0,
        malformed = 1,
        joint_mismatch = 2,
        controller_mismatch = 3,
        invalid_index = 4,
        out_of_bounds = 5,
        not_integer = 6,
    };

    struct Request
    {
        uint8_t joint_id = 0;
        uint8_t controller_id = 0;
        uint8_t param_index = 0;
        float value = 0.0f;
    };

    inline static const char* reason_name(RejectionReason reason)
    {
        switch (reason)
        {
            case RejectionReason::accepted:
                return "accepted";
            case RejectionReason::malformed:
                return "malformed";
            case RejectionReason::joint_mismatch:
                return "joint_mismatch";
            case RejectionReason::controller_mismatch:
                return "controller_mismatch";
            case RejectionReason::invalid_index:
                return "invalid_index";
            case RejectionReason::out_of_bounds:
                return "out_of_bounds";
            case RejectionReason::not_integer:
                return "not_integer";
            default:
                return "unknown";
        }
    }

    inline static bool is_finite_float(float value)
    {
        return (value == value) && (value <= 3.402823e38f) && (value >= -3.402823e38f);
    }

    inline static bool try_float_to_uint8(float value, uint8_t* out)
    {
        if (out == NULL || !(value >= 0.0f) || !(value <= 255.0f))
        {
            return false;
        }

        uint8_t converted = (uint8_t)value;
        float diff = value - (float)converted;
        if (diff < 0.0f)
        {
            diff = -diff;
        }
        if (diff > 0.001f)
        {
            return false;
        }

        *out = converted;
        return true;
    }

    inline static bool is_integer_float(float value)
    {
        long converted = (long)value;
        float diff = value - (float)converted;
        if (diff < 0.0f)
        {
            diff = -diff;
        }
        return diff <= 0.001f;
    }

    inline static bool has_valid_side(uint8_t joint_id)
    {
        uint8_t side = joint_id & k_side_mask;
        return (side == k_side_left) || (side == k_side_right);
    }

    inline static bool has_valid_joint_type(uint8_t joint_id)
    {
        switch (joint_id & (uint8_t)(~k_side_mask))
        {
            case (uint8_t)config_defs::joint_id::hip:
            case (uint8_t)config_defs::joint_id::knee:
            case (uint8_t)config_defs::joint_id::ankle:
            case (uint8_t)config_defs::joint_id::elbow:
            case (uint8_t)config_defs::joint_id::arm_1:
            case (uint8_t)config_defs::joint_id::arm_2:
                return true;
            default:
                return false;
        }
    }

    inline static RejectionReason validate_request(
        ExoData* data,
        const Request& request,
        JointData** joint_out = NULL)
    {
        if (joint_out != NULL)
        {
            *joint_out = NULL;
        }

        if (data == NULL || !is_finite_float(request.value))
        {
            return RejectionReason::malformed;
        }

        if (!has_valid_side(request.joint_id) || !has_valid_joint_type(request.joint_id))
        {
            return RejectionReason::joint_mismatch;
        }

        JointData* j_data = data->get_joint_with(request.joint_id);
        if (j_data == NULL || !j_data->is_used || ((uint8_t)j_data->id != request.joint_id))
        {
            return RejectionReason::joint_mismatch;
        }

        bool request_is_left = ((request.joint_id & k_side_mask) == k_side_left);
        if (j_data->is_left != request_is_left)
        {
            return RejectionReason::joint_mismatch;
        }

        uint8_t param_count = ControllerData::get_parameter_length_for(
            j_data->controller.joint,
            request.controller_id);
        if (param_count == 0)
        {
            return RejectionReason::controller_mismatch;
        }

        if (request.param_index >= param_count || request.param_index >= controller_defs::max_parameters)
        {
            return RejectionReason::invalid_index;
        }

        float min_value = 0.0f;
        float max_value = 0.0f;
        bool integer_only = false;
        if (ControllerData::get_parameter_bounds_for(
                j_data->controller.joint,
                request.controller_id,
                request.param_index,
                &min_value,
                &max_value,
                &integer_only))
        {
            if (request.value < min_value || request.value > max_value)
            {
                return RejectionReason::out_of_bounds;
            }

            if (integer_only && !is_integer_float(request.value))
            {
                return RejectionReason::not_integer;
            }
        }

        if (joint_out != NULL)
        {
            *joint_out = j_data;
        }
        return RejectionReason::accepted;
    }

    inline static RejectionReason validate_forwardable_request(const Request& request)
    {
        if (!is_finite_float(request.value))
        {
            return RejectionReason::malformed;
        }

        if (!has_valid_side(request.joint_id) || !has_valid_joint_type(request.joint_id))
        {
            return RejectionReason::joint_mismatch;
        }

        if (request.param_index >= controller_defs::max_parameters)
        {
            return RejectionReason::invalid_index;
        }

        return RejectionReason::accepted;
    }

    inline static void log_rejection(const char* context, const Request& request, RejectionReason reason)
    {
        logger::print(context, LogLevel::Warn);
        logger::print(" rejected param update: joint=", LogLevel::Warn);
        logger::print(request.joint_id, LogLevel::Warn);
        logger::print(", controller=", LogLevel::Warn);
        logger::print(request.controller_id, LogLevel::Warn);
        logger::print(", index=", LogLevel::Warn);
        logger::print(request.param_index, LogLevel::Warn);
        logger::print(", value=", LogLevel::Warn);
        logger::print(request.value, LogLevel::Warn);
        logger::print(", reason=", LogLevel::Warn);
        logger::println(reason_name(reason), LogLevel::Warn);
    }
}

#endif
