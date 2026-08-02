#ifndef UARTMSG_H
#define UARTMSG_H

#define UART_MSG_T_MAX_DATA_LEN 128
#include "Arduino.h"
#include "Logger.h"

typedef struct
{
  uint8_t command;
  uint8_t joint_id;
  float data[UART_MSG_T_MAX_DATA_LEN];
  uint8_t len;
} UART_msg_t;

namespace UART_msg_t_utils
{
    static void print_msg(UART_msg_t msg)
    {
        logger::println("UART_command_utils::print_msg->Msg: ");
        logger::print(msg.command); logger::print("\t");
        logger::print(msg.joint_id); logger::print("\t");
        logger::print(msg.len); logger::println();
        for (int i=0; i<msg.len; i++)
        {
           logger::print(msg.data[i]); logger::print(", ");
        }
        logger::println();
    }
};


#endif