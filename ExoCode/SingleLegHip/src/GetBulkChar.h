#ifndef GetBulkChar_h
#define GetBulkChar_h

#if defined(ARDUINO_ARDUINO_NANO33BLE) | defined(ARDUINO_NANO_RP2040_CONNECT)
//#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)

// Must be large enough to hold the full controller payload sent by Teensy.
// The names+values controller list is larger than the older names-only payload.
const size_t MAX_MESSAGE_SIZE = 25000;
extern char rxBuffer_bulkStr[MAX_MESSAGE_SIZE];
void readSingleMessageBlocking();
#endif
#endif
