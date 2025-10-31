#pragma once

#include <Arduino.h>

enum class GamblingChoice {
  Red,
  Black,
};

void gambling_init();

void gambling_start();

bool gambling_is_active();

bool gambling_choice_pending();

bool gambling_register_choice(GamblingChoice choice, bool* winOut);

void gambling_reset();
