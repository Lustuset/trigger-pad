#include <Arduino.h>
#include "serial_handler/serial_handler.h"
#include "routine/routine.h"

void setup() {
  SERIAL_HANDLER::setup();
  ROUTINE::setup();
}

void loop() {
  static unsigned long lastTime = 0;
  unsigned long currentTime = millis();
  unsigned long deltaTime = currentTime - lastTime;
  lastTime = currentTime;

  SERIAL_HANDLER::loop(deltaTime);
  ROUTINE::loop(deltaTime);
}