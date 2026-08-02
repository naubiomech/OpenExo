/**
 * @file SystemReset.h
 * @brief Helpers for triggering a MCU reset across supported boards.
 */
#ifndef SYSTEM_RESET_H
#define SYSTEM_RESET_H

#include "Arduino.h"

#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)
#define CPU_RESTART_ADDR (uint32_t *)0xE000ED0C
#define CPU_RESTART_VAL 0x5FA0004
#define CPU_RESTART (*CPU_RESTART_ADDR = CPU_RESTART_VAL)
#elif defined(ARDUINO_ARDUINO_NANO33BLE) || defined(ARDUINO_NANO_RP2040_CONNECT)
extern "C" void NVIC_SystemReset(void);
#endif

inline void exo_system_reset()
{
#if defined(ARDUINO_TEENSY36) || defined(ARDUINO_TEENSY41)
    CPU_RESTART;
#elif defined(ARDUINO_ARDUINO_NANO33BLE) || defined(ARDUINO_NANO_RP2040_CONNECT)
    NVIC_SystemReset();
#endif
}

#endif
