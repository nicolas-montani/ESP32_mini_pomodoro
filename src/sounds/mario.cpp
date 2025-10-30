#include <Arduino.h>

#include "mario.h"

// ===== Notes (as provided) =====
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
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
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

// ===== Pins =====
namespace {

uint8_t marioBuzzerPin = 13;         // buzzer (configurable via buzzer_music_mario_init)
constexpr uint8_t LED_PIN    = 13;  // LED indicator
constexpr uint8_t SHOCK_PIN  = 10;  // vibration sensor (digital)

}

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
void buzz(int targetPin, long frequency, long lengthMs) {
  if (lengthMs <= 0) return;

  if (frequency <= 0) {
    // Rest: keep LED off and just wait
    digitalWrite(LED_PIN, LOW);
    delay(lengthMs);
    return;
  }

  digitalWrite(LED_PIN, HIGH);
  long halfPeriodUs = 1000000L / frequency / 2L;   // microseconds
  long cycles = (frequency * lengthMs) / 1000L;    // total cycles

  for (long i = 0; i < cycles; i++) {
    digitalWrite(targetPin, HIGH);
    delayMicroseconds(halfPeriodUs);
    digitalWrite(targetPin, LOW);
    delayMicroseconds(halfPeriodUs);
  }
  digitalWrite(LED_PIN, LOW);
}

// ===== Play functions =====
void buzzer_music_mario_play_overworld() {
  int size = sizeof(marioMelody) / sizeof(int);
  for (int i = 0; i < size; i++) {
    int noteDuration = 1000 / marioTempo[i];      // e.g., 1000/4 = quarter
    buzz(marioBuzzerPin, marioMelody[i], noteDuration);
    delay((int)(noteDuration * 0.30));            // small gap
  }
}

void buzzer_music_mario_play_underworld() {
  int size = sizeof(underworld_melody) / sizeof(int);
  for (int i = 0; i < size; i++) {
    int noteDuration = 1000 / underworld_tempo[i];
    buzz(marioBuzzerPin, underworld_melody[i], noteDuration);
    delay((int)(noteDuration * 0.30));
  }
}

// ===== Setup & Loop (sensor-triggered) =====
void buzzer_music_mario_init(uint8_t buzzerPin) {
  marioBuzzerPin = buzzerPin;
  pinMode(marioBuzzerPin, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(SHOCK_PIN, INPUT_PULLUP); // active LOW
  digitalWrite(LED_PIN, LOW);
  // Serial.begin(9600); // optional for debugging
}

void buzzer_music_mario_loop_iteration() {
  bool triggered = (digitalRead(SHOCK_PIN) == LOW);  // LOW = vibration detected

  if (triggered) {
    // Optional: Serial.println("Triggered: playing tunes");
    buzzer_music_mario_play_overworld();
    buzzer_music_mario_play_overworld();
    buzzer_music_mario_play_underworld();

    // Cooldown to avoid chatter from the vibration sensor
    delay(300);

    // Wait for sensor to release
    while (digitalRead(SHOCK_PIN) == LOW) {
      delay(10);
    }
  }

  // Idle behaviour (LED off)
  digitalWrite(LED_PIN, LOW);
}
