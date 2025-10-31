#pragma once

#include <Arduino.h>

// Pin definition
#ifndef BUZZER_PIN
#define BUZZER_PIN 13  // Default buzzer pin
#endif

/**
 * Initializes the buzzer pin.
 * Must be called before playing any sounds.
 *
 * @param pin The pin connected to the buzzer (default: BUZZER_PIN)
 */
void buzzer_init(uint8_t pin = BUZZER_PIN);

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
