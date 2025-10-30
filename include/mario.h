#pragma once

#include <Arduino.h>

/**
 * Initializes the pins used by the Mario theme controller.
 * Must be called before any play/loop helpers that interact with the hardware.
 */
void marioInit(uint8_t buzzerPin = 3);

/**
 * Executes one iteration of the sensor-driven Mario theme loop.
 * Call this repeatedly inside your main loop.
 */
void marioLoopIteration();

/**
 * Plays the Mario overworld theme once using the configured pins.
 */
void playMario();

/**
 * Plays the Mario underworld theme once using the configured pins.
 */
void playUnderworld();
