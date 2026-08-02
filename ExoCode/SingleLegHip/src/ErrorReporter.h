/**
 * @file ErrorReporter.h
 * @author Chancelor Cuddeback
 * @brief Singleton class to report errors to the other microcontroller.
 * 
 */

#ifndef ERROR_REPORTER_H
#define ERROR_REPORTER_H

#include "UARTHandler.h"
#include "uart_commands.h"
#include "error_codes.h"
#include "ParseIni.h"
#include "UART_msg_t.h"

/**
 * @brief Singleton class to report errors to the other microcontroller.
 * 
 */
class ErrorReporter
{
    ErrorReporter() {};
    ~ErrorReporter() {};
public:
    /**
     * @brief Get the instance object
     * 
     * @return ErrorReporter* 
     */
    static ErrorReporter* get_instance()
    {
        static ErrorReporter* instance = new ErrorReporter();
        return instance;
    }

    /**
     * @brief Report an error to the other microcontroller.
     * 
     * @param error_code 
     * @param joint_id 
     */
    void report(ErrorCodes error_code, config_defs::joint_id joint_id)
    {
        UART_msg_t msg;
        msg.joint_id = static_cast<uint8_t>(joint_id);
        msg.data[(uint8_t)UART_command_enums::get_error_code::ERROR_CODE] = (float)static_cast<int>(error_code);
        UART_command_handlers::get_error_code(
            UARTHandler::get_instance(), nullptr, msg);
    }
};

#endif