#include <Arduino.h>

#include "happy_sad.h"

namespace {
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

template <size_t N>
void playSequence(int buzzerPin, const ToneStep (&steps)[N]) {
  for (const auto& step : steps) {
    tone(buzzerPin, step.frequency, step.durationMs);
    delay(step.durationMs + step.pauseMs);
    noTone(buzzerPin);
  }
}
}  // namespace

void buzzer_sound_ToneHappy1(int buzzerPin) {
  playSequence(buzzerPin, happyTone1);
}

void buzzer_sound_ToneHappy2(int buzzerPin) {
  playSequence(buzzerPin, happyTone2);
}

void buzzer_sound_ToneSad1(int buzzerPin) {
  playSequence(buzzerPin, sadTone1);
}

void buzzer_sound_ToneSad2(int buzzerPin) {
  playSequence(buzzerPin, sadTone2);
}
