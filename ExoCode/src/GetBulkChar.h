#ifndef GetBulkChar_h
#define GetBulkChar_h

#if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)
//#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)

// Doubled (5000 -> 10000) so the Nano can buffer the controller-parameter
// preamble that now carries both name rows and value rows for every active
// controller (see ListCtrlParams::ctrl_param_array_gen).
const size_t MAX_MESSAGE_SIZE = 10000;
extern char rxBuffer_bulkStr[MAX_MESSAGE_SIZE];
void readSingleMessageBlocking();
#endif
#endif