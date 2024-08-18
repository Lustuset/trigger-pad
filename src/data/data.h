#ifndef DATA_h
#define DATA_h

#include <Arduino.h>

namespace DATA {
    const uint8_t MAX_SUPPORTED_DATA_VERSION = 0x01;

    struct RoutineMeta {
        uint8_t buttonPin;
        uint16_t length;
    };

    struct Meta {
        uint8_t dataVersion;

        uint8_t defaultPinStates[32]; // 32 * 8 = 256 which is the maximum number of pins

        uint8_t routineCount;
        RoutineMeta *routineMetaList;
    };

    Meta* readMeta();
    void writeMeta();
    uint8_t readRoutineByte(unsigned int routineIndex, uint16_t byteIndex);
    bool writeRoutineByte(unsigned int routineIndex, uint16_t byteIndex, uint8_t value);

    void initializeEEPROM();
    void factoryReset();
    void dump();
}


#endif