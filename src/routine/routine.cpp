#include "routine.h"
#include "data/data.h"
#include "config.h"
#include "utils/utils.h"
#include <Arduino.h>

#define read(routineIndex, index) DATA::readRoutineByte(routineIndex, index); \
    index++;

namespace ROUTINE {
    long _timers[CONFIG_MAX_ROUTINES];
    int _indices[CONFIG_MAX_ROUTINES];

    void _runRoutine(uint8_t routineIndex, DATA::Meta *meta, unsigned long delta);
    void _detectButtonPress(int routineIndex, DATA::Meta *meta);

    void setup() {
        DEBUG_PRINTLN("Routine setup");

        DATA::Meta *meta = DATA::readMeta();

        for (int i = 0; i < meta->routineCount; i++) {
            DATA::RoutineMeta *routineMeta = &meta->routineMetaList[i];
            pinMode(routineMeta->buttonPin, INPUT);

            DEBUG_PRINT("Routine ");
            DEBUG_PRINT(i);
            DEBUG_PRINT(" button pin: ");
            DEBUG_PRINTLN(routineMeta->buttonPin);
        }

        DEBUG_PRINTLN("Setting default pin states");
        for (int i = 0; i < 32; i++) {
            uint8_t pinByte = meta->defaultPinStates[i];
            for (int j = 0; j < 8; j++) {
                if (!validatePin(i * 8 + j)) {
                    continue;
                }
                
                uint8_t pinState = (pinByte >> j) & 0x01;
                digitalWrite(i * 8 + j, pinState);

                DEBUG_PRINT("Pin ");
                DEBUG_PRINT(i * 8 + j);
                DEBUG_PRINT(" state: ");
                DEBUG_PRINTLN(pinState);
            }
        }

        DEBUG_PRINTLN("Initializing timers and indices");
        for (int i = 0; i < CONFIG_MAX_ROUTINES; i++) {
            _timers[i] = 0;
            _indices[i] = -1;
        }

        DEBUG_PRINTLN("Routine setup done");
    }

    void loop(unsigned long delta) {
        DATA::Meta *meta = DATA::readMeta();
        for (int routineIndex = 0; routineIndex < CONFIG_MAX_ROUTINES; routineIndex++) {
            _runRoutine(routineIndex, meta, delta);
            _detectButtonPress(routineIndex, meta);
        }
    }

    void _runRoutine(uint8_t routineIndex, DATA::Meta *meta, unsigned long delta) {
        if (_indices[routineIndex] < 0) {
            return;
        }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
        if (_indices[routineIndex] >= meta->routineMetaList[routineIndex].length) {
#pragma GCC diagnostic pop
            DEBUG_PRINT("Routine ");
            DEBUG_PRINT(routineIndex);
            DEBUG_PRINTLN(" finished");
            _indices[routineIndex] = -1;
            return;
        }

        if (_timers[routineIndex] > 0) {
            _timers[routineIndex] -= delta;
#if CONFIG_DEBUG_ROUTINE_TIMERS == true
            DEBUG_PRINT("Routine ");
            DEBUG_PRINT(routineIndex);
            DEBUG_PRINT(" timer: ");
            DEBUG_PRINTLN(_timers[routineIndex]);
#endif
            return;
        }

        uint8_t instruction = read(routineIndex, _indices[routineIndex]);
        uint8_t arg1;
        switch (instruction) {
            case INSTRUCTION_HALT:
                _indices[routineIndex] = -1;
                DEBUG_PRINT("Routine ");
                DEBUG_PRINT(routineIndex);
                DEBUG_PRINTLN(" finished");
                break;
            case INSTRUCTION_PIN_LOW:
                arg1 = read(routineIndex, _indices[routineIndex]);
                digitalWrite(arg1, LOW);
                break;
            case INSTRUCTION_PIN_HIGH:
                arg1 = read(routineIndex, _indices[routineIndex]);
                digitalWrite(arg1, HIGH);
                break;
            case INSTRUCTION_DELAY:
                arg1 = read(routineIndex, _indices[routineIndex]);
                _timers[routineIndex] = (long)arg1 * (long)1000;
                DEBUG_PRINT("Routine ");
                DEBUG_PRINT(routineIndex);
                DEBUG_PRINT(" delay: ");
                DEBUG_PRINT(arg1);
                DEBUG_PRINT("s (");
                DEBUG_PRINT(_timers[routineIndex]);
                DEBUG_PRINTLN("ms)");
                break;
            case INSTRUCTION_NOP:
                break;
        }
    }

    void _detectButtonPress(int routineIndex, DATA::Meta *meta) {
        if (routineIndex >= meta->routineCount) {
            return;
        }

        if (_indices[routineIndex] >= 0) {
            return;
        }

        if (digitalRead(meta->routineMetaList[routineIndex].buttonPin) == HIGH) {
            DEBUG_PRINT("Routine ");
            DEBUG_PRINT(routineIndex);
            DEBUG_PRINTLN(" button pressed");

            _timers[routineIndex] = 0;
            _indices[routineIndex] = 0;
        }
    }
}