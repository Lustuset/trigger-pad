#include "utils/utils.h"
#include "config.h"
#include <Arduino.h>

char* byteToBin(uint8_t byte, char* buffer) {
    for (int i = 0; i < 8; i++) {
        buffer[7 - i] = (byte & (1 << i)) ? '1' : '0';
    }
    buffer[8] = '\0';
    return buffer;
}

char* byteToBin(uint8_t byte) {
    static char buffer[9];
    return byteToBin(byte, buffer);
}

char* byteToHex(uint8_t byte, char* buffer) {
    sprintf(buffer, "\\x%02X", byte);
    return buffer;
}

char* byteToHex(uint8_t byte) {
    static char buffer[5];
    return byteToHex(byte, buffer);
}

char* representByte(uint8_t byte, char* buffer) {
    if (byte == '\\') {
        sprintf(buffer, "%s", "\\\\");
    } else if (isprint(byte)) {
        sprintf(buffer, "%c", byte);
    } else {
        switch (byte) {
            case '\0':
                sprintf(buffer, "%s", "\\0");
                break;
            case '\n':
                sprintf(buffer, "%s", "\\n");
                break;
            case '\r':
                sprintf(buffer, "%s", "\\r");
                break;
            case '\t':
                sprintf(buffer, "%s", "\\t");
                break;
            case '\v':
                sprintf(buffer, "%s", "\\v");
                break;
            case '\a':
                sprintf(buffer, "%s", "\\a");
                break;
            case '\b':
                sprintf(buffer, "%s", "\\b");
                break;
            case '\f':
                sprintf(buffer, "%s", "\\f");
                break;
            default:
                byteToHex(byte, buffer);
                break;
        }
    }
    return buffer;
}

char* representByte(uint8_t byte) {
    static char buffer[5];
    return representByte(byte, buffer);
}

bool validatePin(uint8_t pin) {
#if ARDUINO_AVR_UNO
    if (pin <= 0 && pin >= 13) {
        return true;
    } else if (pin <= A0 && pin >= A5) {
        return true;
    }
    return false;
#else
    pinMode(LED_BUILTIN, INPUT);
    while (true) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);
        delay(500);
    }
#endif
}