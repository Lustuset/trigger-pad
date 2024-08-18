#ifndef ROUTINE_h
#define ROUTINE_h

#include <Arduino.h>

namespace ROUTINE {
    const uint8_t INSTRUCTION_HALT = 0x00;
    const uint8_t INSTRUCTION_PIN_LOW = 0x01;
    const uint8_t INSTRUCTION_PIN_HIGH = 0x02;
    const uint8_t INSTRUCTION_DELAY = 0x03;
    // ...
    const uint8_t INSTRUCTION_NOP = 0xFF;

    void setup();
    void loop(unsigned long delta);
}

#endif