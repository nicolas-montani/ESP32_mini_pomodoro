#pragma once

#include <Arduino.h>

/**
 * Initializes the pins used by the Mario theme controller.
 * Must be called before any play/loop helpers that interact with the hardware.
 */
void buzzer_music_mario_init(uint8_t buzzerPin = 3);

/**
 * Executes one iteration of the sensor-driven Mario theme loop.
 * Call this repeatedly inside your main loop.
 */
void buzzer_music_mario_loop_iteration();

/**
 * Plays the Mario overworld theme once using the configured pins.
 */
void buzzer_music_mario_play_overworld();

/**
 * Plays the Mario underworld theme once using the configured pins.
 */
void buzzer_music_mario_play_underworld();
