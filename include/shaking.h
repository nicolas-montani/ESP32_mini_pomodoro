#pragma once

#include <Arduino.h>

/**
 * Initializes the KY-002 vibration/shock sensor with interrupt support.
 * Must be called before using any other shaking functions.
 *
 * @param sensorPin The digital pin connected to the KY-002 sensor
 */
void shaking_init(uint8_t sensorPin);

/**
 * Attaches an interrupt handler that triggers immediately when shaking is detected.
 * This provides instant response without polling delays.
 *
 * @param callback Function to call when shaking is detected (ISR-safe)
 */
void shaking_attach_interrupt(void (*callback)());

/**
 * Detaches the interrupt handler.
 */
void shaking_detach_interrupt();

/**
 * Checks if the KY-002 sensor has detected a shake/vibration.
 * The sensor outputs LOW when vibration is detected.
 *
 * @return true if shake/vibration is detected, false otherwise
 */
bool shaking_is_detected();

/**
 * Waits until the shake/vibration is no longer detected.
 * Useful for debouncing and avoiding multiple triggers.
 */
void shaking_wait_for_release();

/**
 * Gets the current raw state of the sensor pin.
 *
 * @return LOW if vibration detected, HIGH otherwise
 */
int shaking_get_raw_state();
