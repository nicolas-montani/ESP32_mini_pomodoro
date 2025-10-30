#pragma once

#include <Arduino.h>

/**
 * Plays a short "happy" jingle variation on the provided buzzer pin.
 */
void buzzer_play_sound_happy1(int buzzerPin);

/**
 * Plays an alternate short "happy" jingle variation on the provided buzzer pin.
 */
void buzzer_play_sound_happy2(int buzzerPin);

/**
 * Plays a short "sad" jingle variation on the provided buzzer pin.
 */
void buzzer_play_sound_sad1(int buzzerPin);

/**
 * Plays an alternate short "sad" jingle variation on the provided buzzer pin.
 */
void buzzer_play_sound_sad2(int buzzerPin);

/**
 * Initializes the buzzer pin for Mario theme music.
 * Must be called before playing any Mario music.
 *
 * @param buzzerPin The pin connected to the buzzer (default: 13)
 */
void buzzer_music_mario_init(uint8_t buzzerPin = 13);

/**
 * Plays the Mario overworld theme once on the configured buzzer pin.
 */
void buzzer_music_mario_play_overworld();

/**
 * Plays the Mario underworld theme once on the configured buzzer pin.
 */
void buzzer_music_mario_play_underworld();
