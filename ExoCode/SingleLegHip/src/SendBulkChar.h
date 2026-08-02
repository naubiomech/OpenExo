#ifndef SendBulkChar_h
#define SendBulkChar_h



#if defined(ARDUINO_TEENSY36)  || defined(ARDUINO_TEENSY41)

#include <Arduino.h> // Necessary for Serial object
#include <string.h>  // Necessary for strlen

#include "ListCtrlParams.h"

void send_bulk_char();

#endif
#endif