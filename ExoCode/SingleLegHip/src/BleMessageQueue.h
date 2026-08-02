/**
 * @file BleMessageQueue.h
 * @author Chance Cuddeback
 * @brief Defines a FIFO queue for the BLEMessage type. The maximum size is specified in the cpp. 
 * @date 2022-08-29
 * 
 */

#ifndef BLEMESSAGEQUEUE_H
#define BLEMESSAGEQUEUE_H

#include "BleMessage.h"


namespace ble_queue
{

/**
 * @brief Pop a message off of the FIFO queue
 * 
 * @return BleMessage Head of queue
 */
BleMessage pop();

/**
 * @brief Put a message on the FIFO queue
 * 
 * @param msg Pointer to message you would like to place on the queue. You do not need to maintain the lifetime of msg. 
 */
void push(BleMessage* msg);

/**
 * @brief Check the size of the FIFO queue.
 * 
 * @return int The ammount of messages on the queue. One indexed. 
 */
int size();

/**
 * @brief Search the queue for a matching message. 
 * 
 * @param msg Message to search for. 
 * @return int Number of identical messages found
 */
int check_for(BleMessage msg);

/**
 * @brief Forcibly empty the queue.
 * 
 */
void clear();

};

#endif