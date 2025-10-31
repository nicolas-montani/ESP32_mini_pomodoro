#include "gambling.h"

#if defined(ESP32)
#include <esp_timer.h>
#endif

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
