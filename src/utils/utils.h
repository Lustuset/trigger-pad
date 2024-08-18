#ifndef UTILS_h
#define UTILS_h

#include "config.h"
#include <Arduino.h>

#if CONFIG_DEBUG == true
    #define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
    #if CONFIG_DEBUG_DISABLE_FORCE_FLUSH == true
        #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__); Serial.flush()
    #else
        #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
    #endif
#else
    #define DEBUG_PRINT(...)
    #define DEBUG_PRINTLN(...)
#endif

char* byteToBin(uint8_t byte, char* buffer);
char* byteToBin(uint8_t byte);

char* byteToHex(uint8_t byte, char* buffer);
char* byteToHex(uint8_t byte);

char* representByte(uint8_t byte, char* buffer);
char* representByte(uint8_t byte);

bool validatePin(uint8_t pin);

#endif