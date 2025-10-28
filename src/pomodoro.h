#ifndef POMODORO_H
#define POMODORO_H

#include <Arduino.h>

// Pomodoro timer states
enum PomodoroState {
  POMODORO_IDLE,
  POMODORO_WORK,
  POMODORO_SHORT_BREAK,
  POMODORO_LONG_BREAK,
  POMODORO_PAUSED
};

// Default durations (in seconds)
#define WORK_DURATION 1500      // 25 minutes
#define SHORT_BREAK_DURATION 300 // 5 minutes
#define LONG_BREAK_DURATION 900  // 15 minutes
#define POMODOROS_UNTIL_LONG_BREAK 4

// Initialize the pomodoro timer
void pomodoro_init();

// Start a work session
void pomodoro_start_work();

// Start a break (short or long based on completed pomodoros)
void pomodoro_start_break();

// Pause the current timer
void pomodoro_pause();

// Resume the timer
void pomodoro_resume();

// Reset the timer
void pomodoro_reset();

// Update the timer (call this in loop)
void pomodoro_update();

// Get current state
PomodoroState pomodoro_get_state();

// Get time remaining in seconds
unsigned long pomodoro_get_time_remaining();

// Get completed pomodoros count
int pomodoro_get_completed_count();

// Check if timer has finished
bool pomodoro_is_finished();

// Get state as string
String pomodoro_get_state_string();

// Format time as MM:SS
String pomodoro_format_time(unsigned long seconds);

#endif
