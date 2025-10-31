#pragma once

#include <Arduino.h>

/**
 * Prepares the LED GPIOs and ensures both lights start off.
 */
void lights_init();

struct LightsState {
  bool redOn;
  bool greenOn;
};

/**
 * Turns on the red light connected to GPIO 27.
 */
void lights_red_on();

/**
 * Turns off the red light connected to GPIO 27.
 */
void lights_red_off();

/**
 * Turns on the green light connected to GPIO 26.
 */
void lights_green_on();

/**
 * Turns off the green light connected to GPIO 26.
 */
void lights_green_off();

/**
 * Turns both lights off.
 */
void lights_off();

/**
 * Shows "work" mode: red on, green off.
 */
void lights_show_work();

/**
 * Shows "break" mode: green on, red off.
 */
void lights_show_break();

/**
 * Shows the idle state: both lights off.
 */
void lights_show_idle();

/**
 * Blinks both lights together during startup or diagnostics.
 *
 * @param times Number of blink cycles.
 * @param onMs Duration of the ON phase in milliseconds.
 * @param offMs Duration of the OFF phase in milliseconds (ignored after last blink).
 */
void lights_blink_both(uint8_t times = 3, unsigned long onMs = 150, unsigned long offMs = 150);

/**
 * Captures the current LED state (on/off for each color).
 */
LightsState lights_capture_state();

/**
 * Restores a previously captured LED state.
 */
void lights_apply_state(const LightsState& state);
