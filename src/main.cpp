#include <Arduino.h>
#include "config.h"
#include "pomodoro.h"
#include "monitor.h"
#include "ultrasound.h"
#include "buzzer.h"
#include "shaking.h"
#include "request.h"
#include "gambling.h"
#include "lights.h"

// ============================================================================
// CONSTANTS
// ============================================================================

// Timing constants
const unsigned long DEBOUNCE_DELAY_MS = 200;
const unsigned long DOUBLE_CLICK_INTERVAL_MS = 350;
const unsigned long SETTINGS_REENTRY_COOLDOWN_MS = 400;
const unsigned long DISPLAY_UPDATE_INTERVAL_MS = 500;
const unsigned long IDLE_DISPLAY_UPDATE_INTERVAL_MS = 100;
const unsigned long ULTRASOUND_CHECK_INTERVAL_MS = 1000;
const unsigned long SHAKING_COOLDOWN_MS = 2000;
const unsigned long FINISHED_SCREEN_DISPLAY_MS = 3000;

// ============================================================================
// ENUMS
// ============================================================================

enum class AppMode {
  NORMAL,
  SETTINGS,
  MENSA_MENU,
  GAMBLING
};

// ============================================================================
// STATE VARIABLES
// ============================================================================

namespace {
// Application mode
AppMode currentAppMode = AppMode::NORMAL;
IdleMode selectedMode = MODE_WORK;
PomodoroState lastPomodoroState = POMODORO_IDLE;

// Settings mode state
bool settingsForWork = true;
int settingsMinutes = 0;
bool settingsRequireRelease = false;
unsigned long settingsReentryBlockUntil = 0;

// Button state
bool button1LastState = HIGH;
bool button2LastState = HIGH;
unsigned long lastButton1Press = 0;
unsigned long lastButton2Press = 0;
bool button2SingleClickPending = false;
unsigned long button2FirstClickTime = 0;

// Display state
unsigned long lastDisplayUpdate = 0;

// Ultrasound monitoring
unsigned long lastUltrasoundCheck = 0;
int ultrasoundCheckCount = 0;
bool isUserLost = false;

// Shaking sensor
volatile bool shakingDetected = false;
unsigned long lastShakingTrigger = 0;

// Mensa menu state
int mensaMenuIndex = 0;
int mensaMenuTotal = 0;

// ============================================================================
// HELPER FUNCTIONS - Settings
// ============================================================================

const char* getSettingsLabel() {
  return settingsForWork ? "WORK" : "BREAK";
}

void showSettingsScreen() {
  monitor_show_time_adjustment(getSettingsLabel(), settingsMinutes);
}

void enterSettingsMode(IdleMode mode) {
  if (pomodoro_get_state() != POMODORO_IDLE) {
    return;
  }

  currentAppMode = AppMode::SETTINGS;
  settingsRequireRelease = true;
  settingsForWork = (mode == MODE_WORK);
  button2SingleClickPending = false;

  if (settingsForWork) {
    settingsMinutes = static_cast<int>(pomodoro_get_work_duration() / 60);
  } else {
    settingsMinutes = static_cast<int>(pomodoro_get_short_break_duration() / 60);
  }

  Serial.print("Entering timer settings for ");
  Serial.println(getSettingsLabel());
  showSettingsScreen();
}

void adjustSettingsBy(int deltaMinutes) {
  if (currentAppMode != AppMode::SETTINGS) {
    return;
  }

  const int minVal = settingsForWork ? 10 : 5;
  const int maxVal = settingsForWork ? 90 : 20;

  int newValue = settingsMinutes + deltaMinutes;
  if (newValue > maxVal) {
    newValue = minVal;
  } else if (newValue < minVal) {
    newValue = maxVal;
  }

  settingsMinutes = newValue;
  Serial.print("Adjusted ");
  Serial.print(getSettingsLabel());
  Serial.print(" duration to ");
  Serial.print(settingsMinutes);
  Serial.println(" minutes");
  showSettingsScreen();
}

void confirmSettings() {
  if (currentAppMode != AppMode::SETTINGS) {
    return;
  }

  unsigned long seconds = static_cast<unsigned long>(settingsMinutes) * 60UL;

  if (settingsForWork) {
    pomodoro_set_work_duration(seconds);
  } else {
    pomodoro_set_short_break_duration(seconds);
  }

  Serial.print("Timer settings saved: ");
  Serial.print(getSettingsLabel());
  Serial.print(" set to ");
  Serial.print(settingsMinutes);
  Serial.println(" minutes");

  selectedMode = settingsForWork ? MODE_WORK : MODE_BREAK;
  currentAppMode = AppMode::NORMAL;
  settingsRequireRelease = false;
  button2SingleClickPending = false;
  settingsReentryBlockUntil = millis() + SETTINGS_REENTRY_COOLDOWN_MS;
  monitor_show_idle_screen(selectedMode, pomodoro_get_completed_count());
  lastPomodoroState = POMODORO_IDLE;
}

// ============================================================================
// HELPER FUNCTIONS - Button Actions
// ============================================================================

void handleButton2SingleAction(PomodoroState currentState) {
  if (currentAppMode == AppMode::SETTINGS) {
    adjustSettingsBy(5);
    return;
  }

  if (currentAppMode == AppMode::MENSA_MENU) {
    if (mensaMenuIndex < mensaMenuTotal - 1) {
      mensaMenuIndex++;
      Serial.print("Next menu item: ");
      Serial.println(mensaMenuIndex + 1);
      monitor_show_mensa_menu(mensaMenuIndex, mensaMenuTotal);
    }
    return;
  }

  if (currentState == POMODORO_IDLE) {
    // Toggle between work and break mode
    if (selectedMode == MODE_WORK) {
      selectedMode = MODE_BREAK;
      Serial.println("\n=== Mode: BREAK ===");
    } else {
      selectedMode = MODE_WORK;
      Serial.println("\n=== Mode: WORK ===");
    }
    monitor_show_idle_screen(selectedMode, pomodoro_get_completed_count());
    return;
  }

  // Reset timer when running
  Serial.println("\n=== Resetting Timer ===");
  pomodoro_reset();
  lastPomodoroState = pomodoro_get_state();
  isUserLost = false;
  monitor_show_idle_screen(selectedMode, pomodoro_get_completed_count());
}

void startPomodoroSession() {
  if (selectedMode == MODE_WORK) {
    Serial.println("\n=== Starting Work Session ===");
    pomodoro_start_work();
    ultrasound_measure_initial_distance();
    ultrasoundCheckCount = 0;
    isUserLost = false;
    lastUltrasoundCheck = millis();
  } else {
    Serial.println("\n=== Starting Break ===");
    pomodoro_start_break();
  }

  lastPomodoroState = pomodoro_get_state();
  monitor_show_running_screen(pomodoro_get_state(),
                              pomodoro_get_time_remaining(),
                              pomodoro_get_completed_count());
}

void pausePomodoro() {
  Serial.println("\n=== Pausing Timer ===");
  pomodoro_pause();
  lastPomodoroState = pomodoro_get_state();
  monitor_show_running_screen(pomodoro_get_state(),
                              pomodoro_get_time_remaining(),
                              pomodoro_get_completed_count());
}

void resumePomodoro() {
  Serial.println("\n=== Resuming Timer ===");
  pomodoro_resume();
  lastPomodoroState = pomodoro_get_state();
  isUserLost = false;
  monitor_show_running_screen(pomodoro_get_state(),
                              pomodoro_get_time_remaining(),
                              pomodoro_get_completed_count());
}

// ============================================================================
// HELPER FUNCTIONS - Mode Management
// ============================================================================

void exitSpecialModes() {
  bool wasGambling = (currentAppMode == AppMode::GAMBLING);

  if (wasGambling) {
    Serial.println("\n=== Exiting Gambling Mode ===");
    gambling_reset();
  } else {
    Serial.println("\n=== Exiting Mensa Menu Mode ===");
  }

  currentAppMode = AppMode::NORMAL;
  settingsReentryBlockUntil = millis() + SETTINGS_REENTRY_COOLDOWN_MS;
  button2SingleClickPending = false;

  // Reset shaking flag to prevent stale triggers
  shakingDetected = false;
  lastShakingTrigger = millis(); // Reset cooldown timer

  PomodoroState currentState = pomodoro_get_state();
  if (currentState == POMODORO_IDLE) {
    monitor_show_idle_screen(selectedMode, pomodoro_get_completed_count());
  } else {
    monitor_show_running_screen(pomodoro_get_state(),
                                pomodoro_get_time_remaining(),
                                pomodoro_get_completed_count());
  }
  lastPomodoroState = currentState;
}

bool isInSpecialMode() {
  return currentAppMode != AppMode::NORMAL;
}

bool isGamblingActive() {
  return currentAppMode == AppMode::GAMBLING && gambling_choice_pending();
}

bool canEnterDoubleClickContext(unsigned long now) {
  PomodoroState currentState = pomodoro_get_state();
  bool isIdle = (currentState == POMODORO_IDLE);
  bool noSpecialMode = (currentAppMode == AppMode::NORMAL);
  bool cooldownExpired = (now >= settingsReentryBlockUntil);

  return isIdle && noSpecialMode && cooldownExpired;
}

// ============================================================================
// INTERRUPT HANDLERS
// ============================================================================

void IRAM_ATTR onShakingDetected() {
  shakingDetected = true;
}

// ============================================================================
// BUTTON HANDLING
// ============================================================================

void handleButton1Press(unsigned long now, PomodoroState currentState) {
  bool button1State = digitalRead(BUTTON1_PIN);

  if (button1State == LOW && button1LastState == HIGH &&
      now - lastButton1Press > DEBOUNCE_DELAY_MS) {

    if (currentAppMode == AppMode::SETTINGS) {
      adjustSettingsBy(-5);
    }
    else if (isGamblingActive()) {
      bool win = false;
      if (gambling_register_choice(GamblingChoice::Red, &win)) {
        gambling_handle_result(GamblingChoice::Red, win);
        currentAppMode = AppMode::MENSA_MENU;
        monitor_show_mensa_menu(mensaMenuIndex, mensaMenuTotal);
        lastPomodoroState = pomodoro_get_state();
      }
    }
    else if (currentAppMode == AppMode::MENSA_MENU) {
      if (mensaMenuIndex > 0) {
        mensaMenuIndex--;
        Serial.print("Previous menu item: ");
        Serial.println(mensaMenuIndex + 1);
        monitor_show_mensa_menu(mensaMenuIndex, mensaMenuTotal);
      }
    }
    else if (currentState == POMODORO_IDLE) {
      startPomodoroSession();
    }
    else if (currentState == POMODORO_WORK ||
             currentState == POMODORO_SHORT_BREAK ||
             currentState == POMODORO_LONG_BREAK) {
      pausePomodoro();
    }
    else if (currentState == POMODORO_PAUSED) {
      resumePomodoro();
    }

    lastButton1Press = now;
  }

  button1LastState = button1State;
}

void handleButton2Press(unsigned long now, PomodoroState currentState) {
  bool button2State = digitalRead(BUTTON2_PIN);

  if (button2State == LOW && button2LastState == HIGH &&
      now - lastButton2Press > DEBOUNCE_DELAY_MS) {

    if (currentAppMode == AppMode::SETTINGS) {
      adjustSettingsBy(5);
    }
    else if (isGamblingActive()) {
      bool win = false;
      if (gambling_register_choice(GamblingChoice::Black, &win)) {
        gambling_handle_result(GamblingChoice::Black, win);
        currentAppMode = AppMode::MENSA_MENU;
        monitor_show_mensa_menu(mensaMenuIndex, mensaMenuTotal);
        lastPomodoroState = pomodoro_get_state();
      }
    }
    else if (currentAppMode == AppMode::MENSA_MENU) {
      if (mensaMenuIndex < mensaMenuTotal - 1) {
        mensaMenuIndex++;
        Serial.print("Next menu item: ");
        Serial.println(mensaMenuIndex + 1);
        monitor_show_mensa_menu(mensaMenuIndex, mensaMenuTotal);
      }
    }
    else if (canEnterDoubleClickContext(now)) {
      // Handle double-click for settings
      if (button2SingleClickPending &&
          (now - button2FirstClickTime <= DOUBLE_CLICK_INTERVAL_MS)) {
        button2SingleClickPending = false;
        enterSettingsMode(selectedMode);
      } else {
        button2SingleClickPending = true;
        button2FirstClickTime = now;
      }
    }
    else {
      handleButton2SingleAction(currentState);
      button2SingleClickPending = false;
    }

    lastButton2Press = now;
  }

  button2LastState = button2State;
}

void handleButton2SingleClickTimeout(unsigned long now, PomodoroState currentState) {
  if (!button2SingleClickPending) {
    return;
  }

  bool doubleClickExpired = !canEnterDoubleClickContext(now) ||
                           (now - button2FirstClickTime > DOUBLE_CLICK_INTERVAL_MS);

  if (doubleClickExpired) {
    button2SingleClickPending = false;
    handleButton2SingleAction(currentState);
  }
}

void handleBothButtonsPressed(unsigned long now, PomodoroState currentState) {
  bool button1State = digitalRead(BUTTON1_PIN);
  bool button2State = digitalRead(BUTTON2_PIN);
  bool bothPressed = (button1State == LOW && button2State == LOW);
  bool bothDebounced = (now - lastButton1Press > DEBOUNCE_DELAY_MS &&
                        now - lastButton2Press > DEBOUNCE_DELAY_MS);

  if (!bothPressed || !bothDebounced) {
    return;
  }

  // Settings mode: confirm settings
  if (currentAppMode == AppMode::SETTINGS && !settingsRequireRelease) {
    confirmSettings();
    lastButton1Press = now;
    lastButton2Press = now;
    button1LastState = button1State;
    button2LastState = button2State;
    return;
  }

  // Exit special modes (Mensa Menu or Gambling)
  if (isInSpecialMode() && currentAppMode != AppMode::SETTINGS) {
    exitSpecialModes();
    lastButton1Press = now;
    lastButton2Press = now;
    button1LastState = button1State;
    button2LastState = button2State;
    return;
  }

  // Enter settings mode
  if (currentAppMode == AppMode::NORMAL &&
      currentState == POMODORO_IDLE &&
      now >= settingsReentryBlockUntil) {
    enterSettingsMode(selectedMode);
    lastButton1Press = now;
    lastButton2Press = now;
    button1LastState = button1State;
    button2LastState = button2State;
    return;
  }
}

void handleSettingsButtonRelease() {
  if (currentAppMode != AppMode::SETTINGS || !settingsRequireRelease) {
    return;
  }

  bool button1State = digitalRead(BUTTON1_PIN);
  bool button2State = digitalRead(BUTTON2_PIN);

  if (button1State == HIGH && button2State == HIGH) {
    settingsRequireRelease = false;
  }
}

void handleSettingsShakingBlock() {
  if (currentAppMode == AppMode::SETTINGS && shakingDetected) {
    shakingDetected = false;
  }
}

// ============================================================================
// MONITORING FUNCTIONS
// ============================================================================

void handleTimerCompletion() {
  if (!pomodoro_is_finished()) {
    return;
  }

  Serial.println("\n=== Timer Finished! ===");
  buzzer_play_sound_happy1();
  monitor_show_finished_screen(pomodoro_get_completed_count());
  delay(FINISHED_SCREEN_DISPLAY_MS);
  monitor_show_idle_screen(selectedMode, pomodoro_get_completed_count());
  isUserLost = false;
}

void handleUltrasoundMonitoring(unsigned long now, PomodoroState currentState) {
  bool shouldMonitor = (currentAppMode != AppMode::MENSA_MENU) &&
                       (currentState == POMODORO_WORK || currentState == POMODORO_PAUSED);

  if (!shouldMonitor || (now - lastUltrasoundCheck < ULTRASOUND_CHECK_INTERVAL_MS)) {
    return;
  }

  ultrasound_take_single_measurement();
  float currentDistance = ultrasound_get_single_measurement();

  int arrayIndex = ultrasoundCheckCount % 3;
  ultrasound_store_measurement(arrayIndex, currentDistance);
  ultrasoundCheckCount++;

  Serial.print("Distance check #");
  Serial.print(ultrasoundCheckCount);
  Serial.print(": ");
  Serial.print(currentDistance);
  Serial.println(" cm");

  if (ultrasoundCheckCount >= 3) {
    bool withinRange = ultrasound_compare_range();

    if (!withinRange && !isUserLost) {
      // User left workspace
      Serial.println("!!! USER OUT OF RANGE !!!");
      isUserLost = true;
      pomodoro_pause();
      lastPomodoroState = pomodoro_get_state();
      Serial.println("Timer paused due to user out of range");
      buzzer_play_sound_sad1();
      monitor_roboeyes_show_lost();
      monitor_show_running_screen(pomodoro_get_state(),
                                  pomodoro_get_time_remaining(),
                                  pomodoro_get_completed_count());
    }
    else if (withinRange && isUserLost) {
      // User returned
      Serial.println("!!! USER RETURNED TO RANGE !!!");
      isUserLost = false;
      monitor_roboeyes_show_return();
      pomodoro_resume();
      lastPomodoroState = pomodoro_get_state();
      Serial.println("Timer resumed - user back in range");
      buzzer_play_sound_happy1();
      monitor_show_running_screen(pomodoro_get_state(),
                                  pomodoro_get_time_remaining(),
                                  pomodoro_get_completed_count());
    }

    ultrasoundCheckCount = 0;
  }

  lastUltrasoundCheck = now;
}

void handleShakingSensor(unsigned long now) {
  if (!shakingDetected || (now - lastShakingTrigger < SHAKING_COOLDOWN_MS)) {
    return;
  }

  shakingDetected = false;

  // Enter Mensa Menu mode
  if (currentAppMode == AppMode::NORMAL) {
    Serial.println("!!! SHAKING DETECTED (INSTANT) !!!");
    monitor_roboeyes_show_shake();

    currentAppMode = AppMode::MENSA_MENU;
    mensaMenuIndex = 0;
    mensaMenuTotal = request_get_menu_count();

    Serial.println("Entering Mensa Menu Mode");
    Serial.print("Total menu items: ");
    Serial.println(mensaMenuTotal);

    monitor_show_mensa_menu(mensaMenuIndex, mensaMenuTotal);
    lastShakingTrigger = now;
    return;
  }

  // Activate Gambling mode
  if (currentAppMode == AppMode::MENSA_MENU && !gambling_choice_pending()) {
    Serial.println("\n=== Gambling Mode Activated ===");
    Serial.println("Shake detected while in menu.");
    Serial.println("Press BTN1 for RED or BTN2 for BLACK to place your bet.");
    gambling_start();
    currentAppMode = AppMode::GAMBLING;
    monitor_gambling_show_intro();
    lastShakingTrigger = now;
    shakingDetected = false; // Ensure flag is cleared
  }
}

void updateDisplay(unsigned long now, PomodoroState currentState) {
  // Don't update display when in special modes (they manage their own display)
  if (currentAppMode == AppMode::SETTINGS ||
      currentAppMode == AppMode::MENSA_MENU ||
      currentAppMode == AppMode::GAMBLING) {
    return;
  }

  if (currentState == POMODORO_IDLE) {
    if (now - lastDisplayUpdate >= IDLE_DISPLAY_UPDATE_INTERVAL_MS) {
      monitor_show_idle_screen(selectedMode, pomodoro_get_completed_count());
      lastDisplayUpdate = now;
    }
  }
  else if (currentState == POMODORO_WORK ||
           currentState == POMODORO_SHORT_BREAK ||
           currentState == POMODORO_LONG_BREAK ||
           currentState == POMODORO_PAUSED) {
    if (now - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL_MS) {
      monitor_show_running_screen(pomodoro_get_state(),
                                  pomodoro_get_time_remaining(),
                                  pomodoro_get_completed_count());
      lastDisplayUpdate = now;
    }
  }
}

}  

// ============================================================================
// INITIALIZATION FUNCTIONS
// ============================================================================

void initializeSerial() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n\n=================================");
  Serial.println("   ESP32 Pomodoro Timer v1.0");
  Serial.println("=================================\n");
}

void initializeInputPins() {
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  button1LastState = digitalRead(BUTTON1_PIN);
  button2LastState = digitalRead(BUTTON2_PIN);
}

void initializeOutputs() {
  buzzer_init();  // Uses BUZZER_PIN from buzzer.h
  buzzer_play_sound_turn_on();
  lights_init();
}

void initializeDisplay() {
  if (!monitor_init()) {  // Uses SDA_PIN and SCL_PIN from monitor.h
    Serial.println("Failed to initialize monitor!");
    for(;;);
  }
  monitor_roboeyes_init();
  monitor_roboeyes_show_init();
}

void initializeWiFi() {
  Serial.println(">>> Initializing WiFi...");
  if (request_init(WIFI_SSID, WIFI_PASSWORD)) {
    Serial.println(">>> WiFi connected successfully!\n");
    buzzer_play_sound_happy1();
    light_alternate_3sec();
    Serial.println(">>> Fetching Mensa Menu...");
    request_fetch_mensa_menu();
    Serial.println();
  } else {
    Serial.println(">>> WiFi connection failed! Continuing without WiFi...\n");
    buzzer_play_sound_sad1();
  }
}

void initializeSensors() {
  gambling_init();
  ultrasound_init();
  ultrasound_measure_initial_distance();
  shaking_init();  // Uses SHAKING_PIN from shaking.h
  shaking_attach_interrupt(onShakingDetected);
}

void initializePomodoro() {
  pomodoro_init();
  lastPomodoroState = pomodoro_get_state();
  Serial.println("Pomodoro Timer Initialized");
  Serial.println("BTN1 (D5): Start/Pause");
  Serial.println("BTN2 (D4): Toggle Mode / Next/Reset");
}

void showInitialScreen() {
  monitor_show_idle_screen(selectedMode, pomodoro_get_completed_count());
}

// ============================================================================
// MAIN FUNCTIONS
// ============================================================================

void setup() {
  initializeSerial();
  initializeInputPins();
  initializeOutputs();
  initializeDisplay();
  initializeWiFi();
  initializeSensors();
  initializePomodoro();
  showInitialScreen();
}

void loop() {
  pomodoro_update();
  unsigned long now = millis();
  PomodoroState currentState = pomodoro_get_state();

  // Update last state tracking
  if (currentState != lastPomodoroState) {
    lastPomodoroState = currentState;
  }

  // Handle button inputs
  handleSettingsShakingBlock();
  handleSettingsButtonRelease();
  handleBothButtonsPressed(now, currentState);
  handleButton2SingleClickTimeout(now, currentState);
  handleButton1Press(now, currentState);
  handleButton2Press(now, currentState);

  // Handle monitoring and events
  handleTimerCompletion();
  handleUltrasoundMonitoring(now, currentState);
  handleShakingSensor(now);
  updateDisplay(now, currentState);

  delay(10);
}
