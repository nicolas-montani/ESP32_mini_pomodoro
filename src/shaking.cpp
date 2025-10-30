#include <Arduino.h>
#include "shaking.h"

namespace {
  uint8_t shakingSensorPin = 14;  // Default pin (D14/GPIO14), configurable via shaking_init
}

void shaking_init(uint8_t sensorPin) {
  shakingSensorPin = sensorPin;
  pinMode(shakingSensorPin, INPUT_PULLUP);  // KY-002 is active LOW
}

void shaking_attach_interrupt(void (*callback)()) {
  // Attach interrupt on FALLING edge (when sensor goes from HIGH to LOW)
  // This triggers immediately when vibration is detected
  attachInterrupt(digitalPinToInterrupt(shakingSensorPin), callback, FALLING);
}

void shaking_detach_interrupt() {
  detachInterrupt(digitalPinToInterrupt(shakingSensorPin));
}

bool shaking_is_detected() {
  // KY-002 outputs LOW when vibration/shock is detected
  return digitalRead(shakingSensorPin) == LOW;
}

void shaking_wait_for_release() {
  // Wait until sensor returns to HIGH (no vibration)
  while (digitalRead(shakingSensorPin) == LOW) {
    delay(10);
  }
}

int shaking_get_raw_state() {
  return digitalRead(shakingSensorPin);
}
