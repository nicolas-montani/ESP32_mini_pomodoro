#include <Arduino.h>
#include "buzzer.h"
#include "lights.h"

namespace {
// Default buzzer pin
uint8_t buzzerPin = 13;

// Light colors for different sound types
enum class LightColor {
  NONE,
  GREEN,
  RED,
  BOTH,
  ALTERNATE
};

struct ToneStep {
  int frequency;
  int durationMs;
  int pauseMs;
};

constexpr int NOTE_C5 = 523;
constexpr int NOTE_E5 = 659;
constexpr int NOTE_G5 = 784;
constexpr int NOTE_B5 = 988;
constexpr int NOTE_A4 = 440;
constexpr int NOTE_F4 = 349;
constexpr int NOTE_D4 = 294;
constexpr int NOTE_C4 = 262;
constexpr int NOTE_GS4 = 415;
constexpr int NOTE_FS4 = 370;

constexpr ToneStep happyTone1[] = {
    {NOTE_C5, 160, 40},
    {NOTE_E5, 160, 40},
    {NOTE_G5, 220, 80},
};

constexpr ToneStep happyTone2[] = {
    {NOTE_E5, 150, 30},
    {NOTE_G5, 150, 30},
    {NOTE_B5, 220, 100},
};

constexpr ToneStep sadTone1[] = {
    {NOTE_A4, 200, 50},
    {NOTE_F4, 200, 50},
    {NOTE_D4, 240, 120},
};

constexpr ToneStep sadTone2[] = {
    {NOTE_GS4, 180, 40},
    {NOTE_FS4, 200, 40},
    {NOTE_C4, 260, 140},
};

constexpr ToneStep turnOnTone[] = {
    {NOTE_C5, 150, 40},
    {NOTE_E5, 160, 40},
    {NOTE_G5, 220, 120},
};

template <size_t N>
void playSequence(const ToneStep (&steps)[N], LightColor lightColor = LightColor::NONE) {
  bool toggleState = false;

  for (const auto& step : steps) {
    // Turn on lights based on color
    switch (lightColor) {
      case LightColor::GREEN:
        light_green_on();
        break;
      case LightColor::RED:
        light_red_on();
        break;
      case LightColor::BOTH:
        light_both_on();
        break;
      case LightColor::ALTERNATE:
        if (toggleState) {
          light_green_on();
          light_red_off();
        } else {
          light_red_on();
          light_green_off();
        }
        toggleState = !toggleState;
        break;
      case LightColor::NONE:
      default:
        break;
    }

    tone(buzzerPin, step.frequency, step.durationMs);
    delay(step.durationMs);
    noTone(buzzerPin);

    // Turn off lights
    if (lightColor != LightColor::NONE) {
      light_both_off();
    }

    if (step.pauseMs > 0) {
      delay(step.pauseMs);
    }
  }

  // Ensure lights are off at the end
  light_both_off();
}
}  // namespace

// Initialize the buzzer
void buzzer_init(uint8_t pin) {
  buzzerPin = pin;
  pinMode(buzzerPin, OUTPUT);
  noTone(buzzerPin);
}

void buzzer_play_sound_happy1() {
  playSequence(happyTone1, LightColor::GREEN);
}

void buzzer_play_sound_happy2() {
  playSequence(happyTone2, LightColor::GREEN);
}

void buzzer_play_sound_sad1() {
  playSequence(sadTone1, LightColor::RED);
}

void buzzer_play_sound_sad2() {
  playSequence(sadTone2, LightColor::RED);
}

void buzzer_play_sound_turn_on() {
  playSequence(turnOnTone, LightColor::ALTERNATE);
}

// ===== Mario Music Notes =====
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_CS4 277
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978


// ===== Mario main theme =====
int marioMelody[] = {
  NOTE_E7, NOTE_E7, 0, NOTE_E7,
  0, NOTE_C7, NOTE_E7, 0,
  NOTE_G7, 0, 0,  0,
  NOTE_G6, 0, 0, 0,

  NOTE_C7, 0, 0, NOTE_G6,
  0, 0, NOTE_E6, 0,
  0, NOTE_A6, 0, NOTE_B6,
  0, NOTE_AS6, NOTE_A6, 0,

  NOTE_G6, NOTE_E7, NOTE_G7,
  NOTE_A7, 0, NOTE_F7, NOTE_G7,
  0, NOTE_E7, 0, NOTE_C7,
  NOTE_D7, NOTE_B6, 0, 0,

  NOTE_C7, 0, 0, NOTE_G6,
  0, 0, NOTE_E6, 0,
  0, NOTE_A6, 0, NOTE_B6,
  0, NOTE_AS6, NOTE_A6, 0,

  NOTE_G6, NOTE_E7, NOTE_G7,
  NOTE_A7, 0, NOTE_F7, NOTE_G7,
  0, NOTE_E7, 0, NOTE_C7,
  NOTE_D7, NOTE_B6, 0, 0
};
int marioTempo[] = {
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  9, 9, 9,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,

  9, 9, 9,
  12, 12, 12, 12,
  12, 12, 12, 12,
  12, 12, 12, 12,
};

// ===== Underworld theme =====
int underworld_melody[] = {
  NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4,
  NOTE_AS3, NOTE_AS4, 0,
  0,
  NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4,
  NOTE_AS3, NOTE_AS4, 0,
  0,
  NOTE_F3, NOTE_F4, NOTE_D3, NOTE_D4,
  NOTE_DS3, NOTE_DS4, 0,
  0,
  NOTE_F3, NOTE_F4, NOTE_D3, NOTE_D4,
  NOTE_DS3, NOTE_DS4, 0,
  0, NOTE_DS4, NOTE_CS4, NOTE_D4,
  NOTE_CS4, NOTE_DS4,
  NOTE_DS4, NOTE_GS3,
  NOTE_G3, NOTE_CS4,
  NOTE_C4, NOTE_FS4, NOTE_F4, NOTE_E3, NOTE_AS4, NOTE_A4,
  NOTE_GS4, NOTE_DS4, NOTE_B3,
  NOTE_AS3, NOTE_A3, NOTE_GS3,
  0, 0, 0
};
int underworld_tempo[] = {
  12, 12, 12, 12,
  12, 12, 6,
  3,
  12, 12, 12, 12,
  12, 12, 6,
  3,
  12, 12, 12, 12,
  12, 12, 6,
  3,
  12, 12, 12, 12,
  12, 12, 6,
  6, 18, 18, 18,
  6, 6,
  6, 6,
  6, 6,
  18, 18, 18, 18, 18, 18,
  10, 10, 10,
  10, 10, 10,
  3, 3, 3
};

// ===== Utility: safe buzz (handles frequency == 0) =====
void buzzer_play_music_with_light(int targetPin, long frequency, long lengthMs) {
  if (lengthMs <= 0) return;

  if (frequency <= 0) {
    // Rest: just wait
    delay(lengthMs);
    return;
  }

  long halfPeriodUs = 1000000L / frequency / 2L;   // microseconds
  long cycles = (frequency * lengthMs) / 1000L;    // total cycles

  for (long i = 0; i < cycles; i++) {
    digitalWrite(targetPin, HIGH);
    delayMicroseconds(halfPeriodUs);
    digitalWrite(targetPin, LOW);
    delayMicroseconds(halfPeriodUs);
  }
}

// ===== Play functions =====
void buzzer_music_mario_play_overworld() {
  int size = sizeof(marioMelody) / sizeof(int);
  for (int i = 0; i < size; i++) {
    int noteDuration = 1000 / marioTempo[i];      // e.g., 1000/4 = quarter
    buzzer_play_music_with_light(buzzerPin, marioMelody[i], noteDuration);
    delay((int)(noteDuration * 0.30));            // small gap
  }
}

void buzzer_music_mario_play_underworld() {
  int size = sizeof(underworld_melody) / sizeof(int);
  for (int i = 0; i < size; i++) {
    int noteDuration = 1000 / underworld_tempo[i];
    buzzer_play_music_with_light(buzzerPin, underworld_melody[i], noteDuration);
    delay((int)(noteDuration * 0.30));
  }
}
