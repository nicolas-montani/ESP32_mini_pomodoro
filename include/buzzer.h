#pragma once

#include <Arduino.h>

/**
 * Initializes the buzzer pin.
 * Must be called before playing any sounds.
 *
 * @param pin The pin connected to the buzzer (default: 13)
 */
void buzzer_init(uint8_t pin = 13);

/**
 * Plays a short "happy" jingle variation.
 */
void buzzer_play_sound_happy1();

/**
 * Plays an alternate short "happy" jingle variation.
 */
void buzzer_play_sound_happy2();

/**
 * Plays a short "sad" jingle variation.
 */
void buzzer_play_sound_sad1();

/**
 * Plays an alternate short "sad" jingle variation.
 */
void buzzer_play_sound_sad2();

/**
 * Plays the startup sound when the device powers on.
 */
void buzzer_play_sound_turn_on();

/**
 * Plays the Mario overworld theme once on the configured buzzer pin.
 */
void buzzer_music_mario_play_overworld();

/**
 * Plays the Mario underworld theme once on the configured buzzer pin.
 */
void buzzer_music_mario_play_underworld();
