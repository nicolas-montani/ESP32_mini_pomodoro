#include "lights.h"

namespace {
constexpr uint8_t RED_PIN = 27;
constexpr uint8_t GREEN_PIN = 26;

bool initialized = false;
bool currentRedState = false;
bool currentGreenState = false;

void ensureInitialized() {
  if (initialized) {
    return;
  }
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  initialized = true;
}

void setPin(uint8_t pin, bool on) {
  ensureInitialized();
  digitalWrite(pin, on ? HIGH : LOW);
  if (pin == RED_PIN) {
    currentRedState = on;
  } else if (pin == GREEN_PIN) {
    currentGreenState = on;
  }
}
}  // namespace

void lights_init() {
  ensureInitialized();
  lights_off();
}

void lights_red_on() {
  setPin(RED_PIN, true);
}

void lights_red_off() {
  setPin(RED_PIN, false);
}

void lights_green_on() {
  setPin(GREEN_PIN, true);
}

void lights_green_off() {
  setPin(GREEN_PIN, false);
}

void lights_off() {
  lights_red_off();
  lights_green_off();
}

void lights_show_work() {
  lights_red_on();
  lights_green_off();
}

void lights_show_break() {
  lights_red_off();
  lights_green_on();
}

void lights_show_idle() {
  lights_off();
}

LightsState lights_capture_state() {
  ensureInitialized();
  return {currentRedState, currentGreenState};
}

void lights_apply_state(const LightsState& state) {
  setPin(RED_PIN, state.redOn);
  setPin(GREEN_PIN, state.greenOn);
}

void lights_blink_both(uint8_t times, unsigned long onMs, unsigned long offMs) {
  LightsState saved = lights_capture_state();
  for (uint8_t i = 0; i < times; ++i) {
    lights_red_on();
    lights_green_on();
    delay(onMs);
    lights_off();
    if (i + 1 < times) {
      delay(offMs);
    }
  }
  lights_apply_state(saved);
}
