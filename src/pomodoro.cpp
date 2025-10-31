#include "pomodoro.h"

// Global state variables
static PomodoroState currentState = POMODORO_IDLE;
static PomodoroState pausedSourceState = POMODORO_IDLE;
static unsigned long timeRemaining = 0;
static unsigned long lastUpdateTime = 0;
static int completedPomodoros = 0;
static bool isRunning = false;
static bool timerFinished = false;
static unsigned long workDuration = WORK_DURATION;
static unsigned long shortBreakDuration = SHORT_BREAK_DURATION;
static unsigned long longBreakDuration = LONG_BREAK_DURATION;

// Initialize the pomodoro timer
void pomodoro_init() {
  currentState = POMODORO_IDLE;
  pausedSourceState = POMODORO_IDLE;
  timeRemaining = 0;
  lastUpdateTime = millis();
  completedPomodoros = 0;
  isRunning = false;
  timerFinished = false;

  Serial.println("Pomodoro timer initialized");
}

// Start a work session
void pomodoro_start_work() {
  currentState = POMODORO_WORK;
  timeRemaining = workDuration;
  lastUpdateTime = millis();
  isRunning = true;
  timerFinished = false;

  Serial.print("Starting work session (");
  Serial.print(workDuration / 60);
  Serial.println(" minutes)");
  Serial.print("Completed pomodoros: ");
  Serial.println(completedPomodoros);
}

// Start a break (short or long based on completed pomodoros)
void pomodoro_start_break() {
  // Determine if it's time for a long break
  if (completedPomodoros > 0 && completedPomodoros % POMODOROS_UNTIL_LONG_BREAK == 0) {
    currentState = POMODORO_LONG_BREAK;
    timeRemaining = longBreakDuration;
    Serial.print("Starting long break (");
    Serial.print(longBreakDuration / 60);
    Serial.println(" minutes)");
  } else {
    currentState = POMODORO_SHORT_BREAK;
    timeRemaining = shortBreakDuration;
    Serial.print("Starting short break (");
    Serial.print(shortBreakDuration / 60);
    Serial.println(" minutes)");
  }

  lastUpdateTime = millis();
  isRunning = true;
  timerFinished = false;
}

// Pause the current timer
void pomodoro_pause() {
  if (isRunning && currentState != POMODORO_IDLE) {
    isRunning = false;
    pausedSourceState = currentState;
    currentState = POMODORO_PAUSED;
    Serial.println("Timer paused");
  }
}

// Resume the timer
void pomodoro_resume() {
  if (currentState == POMODORO_PAUSED) {
    isRunning = true;
    lastUpdateTime = millis();
    if (pausedSourceState != POMODORO_IDLE) {
      currentState = pausedSourceState;
    } else {
      currentState = POMODORO_WORK;
    }
    pausedSourceState = POMODORO_IDLE;
    Serial.println("Timer resumed");
  }
}

// Reset the timer
void pomodoro_reset() {
  currentState = POMODORO_IDLE;
  timeRemaining = 0;
  isRunning = false;
  timerFinished = false;
  completedPomodoros = 0;
  pausedSourceState = POMODORO_IDLE;

  Serial.println("Timer reset");
  Serial.println("Pomodoros reset to 0");
}

// Update the timer (call this in loop)
void pomodoro_update() {
  if (!isRunning || currentState == POMODORO_IDLE) {
    return;
  }

  unsigned long currentTime = millis();
  unsigned long elapsed = currentTime - lastUpdateTime;

  // Update every second
  if (elapsed >= 1000) {
    lastUpdateTime = currentTime;

    if (timeRemaining > 0) {
      timeRemaining--;
    } else {
      // Timer finished
      timerFinished = true;
      isRunning = false;

      // Handle state transitions
      if (currentState == POMODORO_WORK) {
        completedPomodoros++;
        Serial.println("Work session completed!");
        Serial.print("Total completed pomodoros: ");
        Serial.println(completedPomodoros);
        currentState = POMODORO_IDLE;
        pausedSourceState = POMODORO_IDLE;
      } else if (currentState == POMODORO_SHORT_BREAK || currentState == POMODORO_LONG_BREAK) {
        Serial.println("Break completed!");
        currentState = POMODORO_IDLE;
        pausedSourceState = POMODORO_IDLE;
      }
    }
  }
}

// Get current state
PomodoroState pomodoro_get_state() {
  return currentState;
}

// Get time remaining in seconds
unsigned long pomodoro_get_time_remaining() {
  return timeRemaining;
}

// Get completed pomodoros count
int pomodoro_get_completed_count() {
  return completedPomodoros;
}

// Check if timer has finished
bool pomodoro_is_finished() {
  bool finished = timerFinished;
  if (finished) {
    timerFinished = false; // Reset the flag after reading
  }
  return finished;
}

// Get state as string
String pomodoro_get_state_string() {
  switch (currentState) {
    case POMODORO_IDLE:
      return "Idle";
    case POMODORO_WORK:
      return "Work";
    case POMODORO_SHORT_BREAK:
      return "Short Break";
    case POMODORO_LONG_BREAK:
      return "Long Break";
    case POMODORO_PAUSED:
      return "Paused";
    default:
      return "Unknown";
  }
}

void pomodoro_set_work_duration(unsigned long seconds) {
  if (seconds == 0) return;
  workDuration = seconds;
}

void pomodoro_set_short_break_duration(unsigned long seconds) {
  if (seconds == 0) return;
  shortBreakDuration = seconds;
}

void pomodoro_set_long_break_duration(unsigned long seconds) {
  if (seconds == 0) return;
  longBreakDuration = seconds;
}

unsigned long pomodoro_get_work_duration() {
  return workDuration;
}

unsigned long pomodoro_get_short_break_duration() {
  return shortBreakDuration;
}

unsigned long pomodoro_get_long_break_duration() {
  return longBreakDuration;
}

void pomodoro_set_time_remaining(unsigned long seconds) {
  timeRemaining = seconds;
  timerFinished = false;
  lastUpdateTime = millis();
}

bool pomodoro_is_running() {
  return isRunning;
}

PomodoroState pomodoro_get_paused_source_state() {
  return pausedSourceState;
}

// Format time as MM:SS
String pomodoro_format_time(unsigned long seconds) {
  unsigned long minutes = seconds / 60;
  unsigned long secs = seconds % 60;

  String timeStr = "";
  if (minutes < 10) timeStr += "0";
  timeStr += String(minutes);
  timeStr += ":";
  if (secs < 10) timeStr += "0";
  timeStr += String(secs);

  return timeStr;
}
