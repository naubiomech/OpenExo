/**
 * @file BleMessage.h
 * @author Chance Cuddeback
 * @brief Defines the BleMessage class used to hold command-data pairs exchanged between the GUI.
 * @date 2022-08-22
 *
 */

#ifndef BLEMESSAGE_H
#define BLEMESSAGE_H

static const int _max_size = 10;

class BleMessage
{
public:
    /**
     * @brief Construct a new Ble Message object
     *
     */
    BleMessage();

    /**
     * @brief Set the message back to its defaults
     *
     */
    void clear();

    /**
     * @brief Sets its values equal to another BleMessage
     *
     * @param n Message to copy
     */
    void copy(BleMessage *n);

    //GUI command
    char command = 0;

    //Number of parameters to expect with the command
    int expecting = 0;

    //Variable to indicate the message has all of its data
    bool is_complete = false;

    //Array to hold the message parameters
    float data[_max_size] = {0};

    /**
     * @brief Print the message values to the serial monitor
     *
     * @param msg Message to print
     */
    static void print(BleMessage msg);

    /**
     * @brief Check if two messages are matching
     *
     * @param msg1 One of the messages to check
     * @param msg2 One of the messages to check
     * @return int One if the messages match, Zero if they dont
     */
    static int matching(BleMessage msg1, BleMessage msg2);

private:
    //Current index of the data array
    int _size = 0;
};

#endif
