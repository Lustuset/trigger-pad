#include "data.h"
#include "config.h"
#include "utils/utils.h"
#include <Arduino.h>
#include <EEPROM.h>

const int META_OFFSET = 0;
const int META_DATA_VERSION_OFFSET = META_OFFSET;
const int META_SIZE = 16;
const int DEFAULT_PIN_STATES_OFFSET = META_OFFSET + META_SIZE;
const int DEFAULT_PIN_STATES_SIZE = 32;
const int ROUTINE_COUNT_OFFSET = DEFAULT_PIN_STATES_OFFSET + DEFAULT_PIN_STATES_SIZE;
const int ROUTINE_COUNT_SIZE = 1;
const int ROUTINE_META_LIST_OFFSET = ROUTINE_COUNT_OFFSET + ROUTINE_COUNT_SIZE;
const int ROUTINE_META_SIZE = 3;

const uint8_t DEFAULT_EEPROM_VALUE = 0xFF;

namespace DATA {
    Meta *_meta = nullptr;
    RoutineMeta _routineMetaList[CONFIG_MAX_ROUTINES];
    int _routineOffsetList[CONFIG_MAX_ROUTINES];

    void _calculateRoutineOffsetList();

    Meta* readMeta() {
        if (_meta != nullptr) {
            return _meta;
        }

        DEBUG_PRINTLN("Read meta");

        if (EEPROM.read(META_DATA_VERSION_OFFSET) == DEFAULT_EEPROM_VALUE) {
            DEBUG_PRINTLN("Uninitialized EEPROM found");
            initializeEEPROM();
        }

        _meta = new Meta();
        _meta->dataVersion = EEPROM.read(META_DATA_VERSION_OFFSET);
        DEBUG_PRINT("Data version: ");
        DEBUG_PRINTLN(_meta->dataVersion);

        _meta->routineCount = EEPROM.read(ROUTINE_COUNT_OFFSET);
        if (_meta->routineCount == DEFAULT_EEPROM_VALUE || _meta->routineCount > CONFIG_MAX_ROUTINES) {
            _meta->routineCount = 0;
        }
        DEBUG_PRINT("Routine count: ");
        DEBUG_PRINTLN(_meta->routineCount);

        DEBUG_PRINT("Default pin state: ");
        for (int i = 0; i < DEFAULT_PIN_STATES_SIZE; i++) {
            _meta->defaultPinStates[i] = EEPROM.read(DEFAULT_PIN_STATES_OFFSET + i);
            DEBUG_PRINT(byteToBin(_meta->defaultPinStates[i]));
        }
        DEBUG_PRINTLN();

        DEBUG_PRINTLN("Routine meta list:");
        _meta->routineMetaList = _routineMetaList;
        for (int i = 0; i < _meta->routineCount; i++) {
            RoutineMeta *routineMeta = &_routineMetaList[i];
            routineMeta->buttonPin = EEPROM.read(ROUTINE_META_LIST_OFFSET + i * ROUTINE_META_SIZE);
            routineMeta->length = EEPROM.read(ROUTINE_META_LIST_OFFSET + i * ROUTINE_META_SIZE + 1) << 8 |
                EEPROM.read(ROUTINE_META_LIST_OFFSET + i * ROUTINE_META_SIZE + 2);
            
            DEBUG_PRINT("Routine ");
            DEBUG_PRINT(i);
            DEBUG_PRINT(" button pin: ");
            DEBUG_PRINT(routineMeta->buttonPin);
            DEBUG_PRINT(" length: ");
            DEBUG_PRINTLN(routineMeta->length);
        }

        _calculateRoutineOffsetList();

        DEBUG_PRINTLN("Read meta done");

        return _meta;
    }

    void writeMeta() {
        DEBUG_PRINTLN("Write meta");

        EEPROM.write(META_DATA_VERSION_OFFSET, _meta->dataVersion);
        DEBUG_PRINT("Data version: ");
        DEBUG_PRINTLN(_meta->dataVersion);

        EEPROM.write(ROUTINE_COUNT_OFFSET, _meta->routineCount);
        DEBUG_PRINT("Routine count: ");
        DEBUG_PRINTLN(_meta->routineCount);

        DEBUG_PRINT("Default pin state: ");
        for (int i = 0; i < DEFAULT_PIN_STATES_SIZE; i++) {
            EEPROM.write(DEFAULT_PIN_STATES_OFFSET + i, _meta->defaultPinStates[i]);
            DEBUG_PRINT(byteToBin(_meta->defaultPinStates[i]));
        }
        DEBUG_PRINTLN();
        
        DEBUG_PRINTLN("Routine meta list:");
        for (int i = 0; i < _meta->routineCount; i++) {
            RoutineMeta *routineMeta = &_routineMetaList[i];
            EEPROM.write(ROUTINE_META_LIST_OFFSET + i * ROUTINE_META_SIZE, routineMeta->buttonPin);
            EEPROM.write(ROUTINE_META_LIST_OFFSET + i * ROUTINE_META_SIZE + 1, routineMeta->length >> 8);
            EEPROM.write(ROUTINE_META_LIST_OFFSET + i * ROUTINE_META_SIZE + 2, routineMeta->length & 0xFF);

            DEBUG_PRINT("Routine ");
            DEBUG_PRINT(i);
            DEBUG_PRINT(" button pin: ");
            DEBUG_PRINT(routineMeta->buttonPin);
            DEBUG_PRINT(" length: ");
            DEBUG_PRINTLN(routineMeta->length);
        }

        _calculateRoutineOffsetList();

        DEBUG_PRINTLN("Write meta done");
    }

    void _calculateRoutineOffsetList() {
        DEBUG_PRINTLN("Calculate routine offset list");

        int offset = ROUTINE_META_LIST_OFFSET + _meta->routineCount * ROUTINE_META_SIZE;
        for (int i = 0; i < _meta->routineCount; i++) {
            _routineOffsetList[i] = offset;
            offset += _routineMetaList[i].length;

            DEBUG_PRINT("Routine ");
            DEBUG_PRINT(i);
            DEBUG_PRINT(" offset: ");
            DEBUG_PRINTLN(_routineOffsetList[i]);
        }
    }

    uint8_t readRoutineByte(unsigned int routineIndex, uint16_t byteIndex) {
        if (_meta == nullptr) {
            DEBUG_PRINTLN("E: Meta is null");
            return 0;
        }

        if (routineIndex >= _meta->routineCount) {
            DEBUG_PRINT("E: Routine index out of bounds, ");
            DEBUG_PRINT(routineIndex);
            DEBUG_PRINT("/");
            DEBUG_PRINTLN(_meta->routineCount);
            return 0;
        }

        if (byteIndex >= _routineMetaList[routineIndex].length) {
            DEBUG_PRINT("E: Byte index out of bounds, ");
            DEBUG_PRINT(byteIndex);
            DEBUG_PRINT("/");
            DEBUG_PRINTLN(_routineMetaList[routineIndex].length);
            return 0;
        }

        uint8_t value = EEPROM.read(_routineOffsetList[routineIndex] + byteIndex);

        DEBUG_PRINT("Reading routine ");
        DEBUG_PRINT(routineIndex);
        DEBUG_PRINT(" at ");
        DEBUG_PRINT(byteIndex);
        DEBUG_PRINT(" (");
        DEBUG_PRINT(_routineOffsetList[routineIndex] + byteIndex);
        DEBUG_PRINT("): ");
        DEBUG_PRINTLN(byteToHex(value));

        return value;
    }

    bool writeRoutineByte(unsigned int routineIndex, uint16_t byteIndex, uint8_t value) {
        if (_meta == nullptr) {
            DEBUG_PRINTLN("E: Meta is null");
            return false;
        }

        if (routineIndex >= _meta->routineCount) {
            DEBUG_PRINT("E: Routine index out of bounds, ");
            DEBUG_PRINT(routineIndex);
            DEBUG_PRINT("/");
            DEBUG_PRINTLN(_meta->routineCount);
            return false;
        }

        if (byteIndex >= _routineMetaList[routineIndex].length) {
            DEBUG_PRINT("E: Byte index out of bounds");
            DEBUG_PRINT(byteIndex);
            DEBUG_PRINT("/");
            DEBUG_PRINTLN(_routineMetaList[routineIndex].length);
            return false;
        }

        EEPROM.write(_routineOffsetList[routineIndex] + byteIndex, value);
        return true;
    }

    void initializeEEPROM() {
        DEBUG_PRINTLN("Initialize EEPROM");

        DEBUG_PRINT("Clearing EEPROM: ");
        for (uint16_t i = 0; i < EEPROM.length(); i++) {
            EEPROM.write(i, DEFAULT_EEPROM_VALUE);

            if (i % 256 == 0) {
                DEBUG_PRINT('.');
            }
        }
        DEBUG_PRINTLN();
        DEBUG_PRINTLN("Cleared EEPROM");

        EEPROM.write(META_DATA_VERSION_OFFSET, MAX_SUPPORTED_DATA_VERSION);
        DEBUG_PRINTLN("Cleared meta");

        for (int i = DEFAULT_PIN_STATES_OFFSET; i < DEFAULT_PIN_STATES_OFFSET + DEFAULT_PIN_STATES_SIZE; i++) {
            EEPROM.write(i, 0b00000000);
        }
        DEBUG_PRINTLN("Cleared default pin states");

        EEPROM.write(ROUTINE_COUNT_OFFSET, 0);
        DEBUG_PRINTLN("Cleared routine count");

        delete _meta;
        _meta = nullptr;
        DEBUG_PRINTLN("Cleared meta object");

        for (int i = 0; i < CONFIG_MAX_ROUTINES; i++) {
            _routineMetaList[i].buttonPin = 0;
            _routineMetaList[i].length = 0;
            _routineOffsetList[i] = 0;
        }
        DEBUG_PRINTLN("Cleared routine meta list");

        readMeta();
        DEBUG_PRINTLN("Reconstructed meta object");

        DEBUG_PRINTLN("Initialize EEPROM done");
    }

    void factoryReset() {
        DEBUG_PRINTLN("Factory reset");

        for (uint16_t i = 0; i < EEPROM.length(); i++) {
            EEPROM.write(i, DEFAULT_EEPROM_VALUE);

            DEBUG_PRINT(i + 1);
            DEBUG_PRINT("/");
            DEBUG_PRINTLN(EEPROM.length());
        }

        DEBUG_PRINTLN("Factory reset done");
    }

    void dump() {
        char buffer[3];
        for (unsigned int i = 0; i < EEPROM.length(); i++) {
            sprintf(buffer, "%02X", EEPROM.read(i));
            Serial.print(buffer);
            Serial.print(" ");
        }
        Serial.println();
        Serial.println("Done");
    }
}