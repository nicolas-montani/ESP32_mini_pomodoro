#ifndef MONITOR_H
#define MONITOR_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "pomodoro.h"
#include "gambling.h"

// Display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Mode selection (when idle)
enum IdleMode {
  MODE_WORK,
  MODE_BREAK
};

// Initialize the monitor
bool monitor_init(int sda_pin, int scl_pin);

// Display functions
void monitor_show_idle_screen(IdleMode selectedMode, int completedCount);
void monitor_show_running_screen(PomodoroState state, unsigned long timeRemaining, int completedCount);
void monitor_show_finished_screen(int completedCount);
void monitor_show_boot_screen();
void monitor_show_mensa_menu(int currentIndex, int totalItems);
void monitor_gambling_show_intro();
void monitor_gambling_show_result(GamblingChoice choice, bool win);
void monitor_show_time_adjustment(const char* label, int minutes);

// Utility functions
String monitor_format_time(unsigned long seconds);
const char* monitor_get_banner_message();

// Show meme image
void monitor_show_meme();

// RoboEyes functions
void monitor_roboeyes_init();
void monitor_roboeyes_show_happy();
void monitor_roboeyes_show_sad();
void monitor_roboeyes_show_init();
void monitor_roboeyes_show_lost();
void monitor_roboeyes_show_return();
void monitor_roboeyes_show_shake();
void monitor_roboeyes_animate(int duration_ms);

#endif
