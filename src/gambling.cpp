#include "gambling.h"
#include "monitor.h"
#include "buzzer.h"

#if defined(ESP32)
#include <esp_timer.h>
#endif

// Timing constant
const unsigned long GAMBLING_RESULT_DISPLAY_MS = 1500;

namespace {
bool active = false;
bool awaitingChoice = false;
}

void gambling_init() {
#if defined(ESP32)
  randomSeed(static_cast<unsigned long>(esp_timer_get_time()));
#else
  randomSeed(static_cast<unsigned long>(micros() ^ millis()));
#endif
  gambling_reset();
}

void gambling_start() {
  active = true;
  awaitingChoice = true;
}

bool gambling_is_active() {
  return active;
}

bool gambling_choice_pending() {
  return active && awaitingChoice;
}

bool gambling_register_choice(GamblingChoice choice, bool* winOut) {
  if (!gambling_choice_pending()) {
    return false;
  }

  (void)choice;
  bool win = (random(0, 2) == 1);
  awaitingChoice = false;
  active = false;

  if (winOut != nullptr) {
    *winOut = win;
  }

  return true;
}

void gambling_reset() {
  active = false;
  awaitingChoice = false;
}

void gambling_handle_result(GamblingChoice choice, bool win) {
  Serial.print("\n=== Gambling Choice: ");
  Serial.print(choice == GamblingChoice::Red ? "RED" : "BLACK");
  Serial.println(" ===");

  monitor_gambling_show_result(choice, win);

  if (win) {
    Serial.println("Result: WIN! Mario theme incoming...");
    buzzer_music_mario_play_overworld();
  } else {
    Serial.println("Result: LOSS. Better luck next time.");
    buzzer_play_sound_sad1();
  }

  delay(GAMBLING_RESULT_DISPLAY_MS);
  gambling_reset();
}
