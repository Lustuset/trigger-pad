#include "serial_handler/serial_handler.h"
#include "config.h"
#include "data/data.h"
#include "routine/routine.h"
#include "utils/utils.h"
#include <Arduino.h>

namespace SERIAL_HANDLER {
    const uint8_t COMMAND_WRITE = 'w';
    const uint8_t COMMAND_READ = 'r';
    const uint8_t COMMAND_FACTORY_RESET = 'f';
    const uint8_t COMMAND_DUMP = 'd';
    const uint8_t COMMAND_PINS = 'p';

    const uint8_t COMMAND_WRITE_HALT = 'h';
    const uint8_t COMMAND_WRITE_PIN_LOW = 'L';
    const uint8_t COMMAND_WRITE_PIN_HIGH = 'H';
    const uint8_t COMMAND_WRITE_DELAY = 'd';
    const uint8_t COMMAND_WRITE_NOP = 'n';
    const uint8_t COMMAND_WRITE_UNDEFINED = '?';

    void _writeRoutines();
    void _readRoutines();
    int _readInt(int digits);
    void _writeInt(int value, int digits);
    void _writePins();
    uint8_t _readSkip();
    bool _writeRoutine(uint8_t routineIndex, uint16_t& byteIndex, uint8_t value);
    uint8_t _readRoutine(uint8_t routineIndex, uint16_t& byteIndex);

    void setup() {
        Serial.begin(9600);
        Serial.println("Serial ready");
        Serial.println("Version: 1");
        
        Serial.print("CONFIG_MAX_ROUTINES: ");
        Serial.println(CONFIG_MAX_ROUTINES);
    }

    void loop(unsigned long delta) {
        if (Serial.available() <= 0) {
            return;
        }

        uint8_t command = Serial.read();
        byteToHex(command);
        if (command == '\n' || command == '\r') {
            DEBUG_PRINT("Skipped: ");
            DEBUG_PRINTLN(representByte(command));
            return;
        }

        DEBUG_PRINT("Command: ");
        DEBUG_PRINTLN(representByte(command));

        switch (command) {
            case COMMAND_WRITE:
                _writeRoutines();
                break;
            case COMMAND_READ:
                _readRoutines();
                break;
            case COMMAND_FACTORY_RESET:
                DATA::factoryReset();
                break;
            case COMMAND_DUMP:
                DATA::dump();
                break;
            case COMMAND_PINS:
                _writePins();
                break;
            default:
                DEBUG_PRINTLN("Unknown command");
                break;
        }
    }

    void _writeRoutines() {
        DEBUG_PRINTLN("Write begins");

        DATA::Meta *meta = DATA::readMeta();

        DEBUG_PRINTLN("Routine count:");
        meta->routineCount = _readInt(2);

        for (int i = 0; i < meta->routineCount; i++) {
            DEBUG_PRINT("Routine ");
            DEBUG_PRINT(i);
            DEBUG_PRINTLN(" button pin:");
            int buttonPin = _readInt(3);

            DEBUG_PRINT("Routine ");
            DEBUG_PRINT(i);
            DEBUG_PRINTLN(" length:");
            int length = _readInt(3);

            meta->routineMetaList[i].buttonPin = buttonPin;
            meta->routineMetaList[i].length = length;
        }

        DATA::writeMeta();

        for (int i = 0; i < meta->routineCount; i++) {
            DEBUG_PRINT("Routine ");
            DEBUG_PRINT(i);
            DEBUG_PRINTLN(" instructions:");

            uint16_t index = 0;
            while (index < meta->routineMetaList[i].length) {
                DEBUG_PRINT("Routine ");
                DEBUG_PRINT(i);
                DEBUG_PRINT(" instruction (");
                DEBUG_PRINT(index);
                DEBUG_PRINT("/");
                DEBUG_PRINT(meta->routineMetaList[i].length);
                DEBUG_PRINTLN("):");

                uint8_t command = _readSkip();
                switch (command) {
                    case COMMAND_WRITE_HALT:
                        _writeRoutine(i, index, ROUTINE::INSTRUCTION_HALT);
                        break;
                    case COMMAND_WRITE_PIN_LOW:
                        _writeRoutine(i, index, ROUTINE::INSTRUCTION_PIN_LOW);
                        _writeRoutine(i, index, _readInt(3));
                        break;
                    case COMMAND_WRITE_PIN_HIGH:
                        _writeRoutine(i, index, ROUTINE::INSTRUCTION_PIN_HIGH);
                        _writeRoutine(i, index, _readInt(3));
                        break;
                    case COMMAND_WRITE_DELAY:
                        _writeRoutine(i, index, ROUTINE::INSTRUCTION_DELAY);
                        _writeRoutine(i, index, _readInt(3));
                        break;
                }
            }

            DEBUG_PRINTLN("Routine end");
        }

        DEBUG_PRINTLN("Write ends");
    }

    void _readRoutines() {
        DEBUG_PRINTLN("Read begins");

        DATA::Meta *meta = DATA::readMeta();

        DEBUG_PRINTLN("Routine count:");
        _writeInt(meta->routineCount, 2);

        for (int i = 0; i < meta->routineCount; i++) {
            DEBUG_PRINT("Routine ");
            DEBUG_PRINT(i);
            DEBUG_PRINTLN(" button pin:");
            _writeInt(meta->routineMetaList[i].buttonPin, 3);

            DEBUG_PRINT("Routine ");
            DEBUG_PRINT(i);
            DEBUG_PRINTLN(" length:");
            _writeInt(meta->routineMetaList[i].length, 3);
        }

        DEBUG_PRINTLN("Instructions:");
        for (int i = 0; i < meta->routineCount; i++) {
            DEBUG_PRINT("Routine ");
            DEBUG_PRINT(i);
            DEBUG_PRINTLN(" instructions:");

            uint16_t index = 0;
            while (index < meta->routineMetaList[i].length) {
                uint8_t instruction = _readRoutine(i, index);
                uint8_t arg1;
                switch (instruction) {
                    case ROUTINE::INSTRUCTION_HALT:
                        Serial.print(COMMAND_WRITE_HALT);
                        break;
                    case ROUTINE::INSTRUCTION_PIN_LOW:
                        Serial.print(COMMAND_WRITE_PIN_LOW);
                        arg1 = _readRoutine(i, index);
                        _writeInt(arg1, 3);
                        break;
                    case ROUTINE::INSTRUCTION_PIN_HIGH:
                        Serial.print(COMMAND_WRITE_PIN_HIGH);
                        arg1 = _readRoutine(i, index);
                        _writeInt(arg1, 3);
                        break;
                    case ROUTINE::INSTRUCTION_DELAY:
                        Serial.print(COMMAND_WRITE_DELAY);
                        arg1 = _readRoutine(i, index);
                        _writeInt(arg1, 3);
                        break;
                    case ROUTINE::INSTRUCTION_NOP:
                        Serial.print(COMMAND_WRITE_NOP);
                        break;
                    default:
                        Serial.print(COMMAND_WRITE_UNDEFINED);
                        break;
                }
            }
        }

        Serial.println();

        DEBUG_PRINTLN("Read ends");
    }

    void _writePins() {
        DEBUG_PRINTLN("Write pins");

        DATA::Meta *meta = DATA::readMeta();

        for (int i = 0; i < 32; i++) {
            uint8_t pinByte = _readInt(3);
            meta->defaultPinStates[i] = pinByte;

            DEBUG_PRINT(i + 1);
            DEBUG_PRINTLN("/32");
        }

        DATA::writeMeta();
    }

    int _readInt(int digits) {
        DEBUG_PRINT("Read int (");
        DEBUG_PRINT(digits);
        DEBUG_PRINTLN(" digits): ");

        int readDigits = 0;
        int value = 0;
        while (readDigits < digits) {
            while (Serial.available() <= 0) {
                continue;
            }

            uint8_t readByte = Serial.read();
            if (!isdigit(readByte)) {
                continue;
            }

            value = value * 10 + (readByte - '0');
            readDigits++;
        }

        DEBUG_PRINTLN(value);

        return value;
    }

    void _writeInt(int value, int digits) {
        // Calculate the number of digits in the integer
        int numberOfDigits = 0;
        int temp = value;
        do {
            temp /= 10;
            numberOfDigits++;
        } while (temp != 0);

        // Calculate how many zeros are needed to pad the number
        int paddingZeros = digits - numberOfDigits;

        // Write the padding zeros
        for (int i = 0; i < paddingZeros; i++) {
            Serial.print('0');
        }

        // Write the actual number
        Serial.print(value);
    }

    uint8_t _readSkip() {
        while (Serial.available() <= 0) {
            continue;
        }

        uint8_t readByte = Serial.read();
        while (readByte == '\n' || readByte == '\r') {
            while (Serial.available() <= 0) {
                continue;
            }
            readByte = Serial.read();
        }

        return readByte;
    }

    bool _writeRoutine(uint8_t routineIndex, uint16_t& byteIndex, uint8_t value) {
        bool result = DATA::writeRoutineByte(routineIndex, byteIndex, value);
        if (result) {
            byteIndex++;
        } else {
            DEBUG_PRINTLN("E: Write failed");
        }

        return result;
    }

    uint8_t _readRoutine(uint8_t routineIndex, uint16_t& byteIndex) {
        return DATA::readRoutineByte(routineIndex, byteIndex);
    }
}
