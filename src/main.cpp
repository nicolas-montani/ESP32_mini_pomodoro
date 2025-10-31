#include <Arduino.h>
#include "pomodoro.h"
#include "monitor.h"
#include "ultrasound.h"
#include "buzzer.h"
#include "shaking.h"
#include "request.h"
#include "gambling.h"
#include "lights.h"

extern IdleMode selectedMode;

namespace {
PomodoroState lastPomodoroState = POMODORO_IDLE;
bool settingsMode = false;
bool settingsForWork = true;
int settingsMinutes = 0;
bool settingsRequireRelease = false;
const unsigned long doubleClickInterval = 350;
bool button2SingleClickPending = false;
unsigned long button2FirstClickTime = 0;
unsigned long settingsReentryBlockUntil = 0;

const char* settingsLabel() {
  return settingsForWork ? "WORK" : "BREAK";
}

void showSettingsScreen() {
  monitor_show_time_adjustment(settingsLabel(), settingsMinutes);
}

void enterSettingsMode(IdleMode tab) {
  if (pomodoro_get_state() != POMODORO_IDLE) {
    return;
  }
  settingsMode = true;
  settingsRequireRelease = true;
  settingsForWork = (tab == MODE_WORK);
  button2SingleClickPending = false;

  if (settingsForWork) {
    settingsMinutes = static_cast<int>(pomodoro_get_work_duration() / 60);
  } else {
    settingsMinutes = static_cast<int>(pomodoro_get_short_break_duration() / 60);
  }

  Serial.print("Entering timer settings for ");
  Serial.println(settingsLabel());
  showSettingsScreen();
}

void adjustSettingsBy(int deltaMinutes) {
  if (!settingsMode) {
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
  Serial.print(settingsLabel());
  Serial.print(" duration to ");
  Serial.print(settingsMinutes);
  Serial.println(" minutes");
  showSettingsScreen();
}

void confirmSettings() {
  if (!settingsMode) {
    return;
  }

  unsigned long seconds = static_cast<unsigned long>(settingsMinutes) * 60UL;

  if (settingsForWork) {
    pomodoro_set_work_duration(seconds);
  } else {
    pomodoro_set_short_break_duration(seconds);
  }

  Serial.print("Timer settings saved: ");
  Serial.print(settingsLabel());
  Serial.print(" set to ");
  Serial.print(settingsMinutes);
  Serial.println(" minutes");

  selectedMode = settingsForWork ? MODE_WORK : MODE_BREAK;
  settingsMode = false;
  settingsRequireRelease = false;
  button2SingleClickPending = false;
  settingsReentryBlockUntil = millis() + 400; // short cooldown before re-entry
  monitor_show_idle_screen(selectedMode, pomodoro_get_completed_count());
  lastPomodoroState = POMODORO_IDLE;
}
}  // namespace

// Pin definitions
#define SDA_PIN 21
#define SCL_PIN 22
#define BUTTON1_PIN 5  // Start/Pause button
#define BUTTON2_PIN 4  // Toggle Work/Break OR Next/Reset when running
#ifndef BUZZER_PIN
#define BUZZER_PIN 13  // Default buzzer pin; override when wiring differs
#endif
#define SHAKING_PIN 14  // KY-002 vibration/shock sensor (moved from 12 due to boot strapping)

// WiFi credentials
const char* WIFI_SSID = "Self_Destruction_Device";
const char* WIFI_PASSWORD = "123456789";

// Button state variables
bool button1LastState = HIGH;
bool button2LastState = HIGH;
unsigned long lastButton1Press = 0;
unsigned long lastButton2Press = 0;
const unsigned long debounceDelay = 200;

// Display update
unsigned long lastDisplayUpdate = 0;
const unsigned long displayUpdateInterval = 500; // Update every 500ms for running screen
const unsigned long idleDisplayUpdateInterval = 100; // Update every 100ms for idle screen (for smooth scrolling)

// Ultrasound monitoring
unsigned long lastUltrasoundCheck = 0;
const unsigned long ultrasoundCheckInterval = 1000; // Check every 1 second
int ultrasoundCheckCount = 0;
float ultrasoundMeasurements[3] = {0.0, 0.0, 0.0};
bool userLost = false;

// Shaking sensor monitoring (interrupt-based)
volatile bool shakingDetected = false;
unsigned long lastShakingTrigger = 0;
const unsigned long shakingCooldown = 2000; // 2s cooldown between triggers for menu/gambling

// Mode selection (when idle)
IdleMode selectedMode = MODE_WORK;

// Mensa menu mode variables
bool mensaMenuMode = false;
int mensaMenuIndex = 0;
int mensaMenuTotal = 0;
bool gamblingMode = false;

static void handleButton2SingleAction(PomodoroState currentState) {
  if (settingsMode) {
    adjustSettingsBy(5);
    return;
  }

  if (mensaMenuMode) {
    if (mensaMenuIndex < mensaMenuTotal - 1) {
      mensaMenuIndex++;
      Serial.print("Next menu item: ");
      Serial.println(mensaMenuIndex + 1);
      monitor_show_mensa_menu(mensaMenuIndex, mensaMenuTotal);
    }
    return;
  }

  if (currentState == POMODORO_IDLE) {
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

  Serial.println("\n=== Resetting Timer ===");
  pomodoro_reset();
  lastPomodoroState = pomodoro_get_state();
  userLost = false;
  monitor_show_idle_screen(selectedMode, pomodoro_get_completed_count());
}

// Interrupt handler for shaking sensor (must be IRAM_ATTR for ESP32)
void IRAM_ATTR onShakingDetected() {
  shakingDetected = true;
}



void setup() {
  Serial.begin(115200);
  delay(500); // Give serial time to initialize

  Serial.println("\n\n=================================");
  Serial.println("   ESP32 Pomodoro Timer v1.0");
  Serial.println("=================================\n");

  // Initialize pins FIRST
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);

  // Initialize buzzer
  buzzer_init(BUZZER_PIN);
  buzzer_play_sound_turn_on();

  // Initialize LED lights
  lights_init();

  // Initialize monitor (display) early
  if (!monitor_init(SDA_PIN, SCL_PIN)) {
    Serial.println("Failed to initialize monitor!");
    for(;;);
  }

  // Initialize RoboEyes (display is ready now)
  monitor_roboeyes_init();

  // Show RoboEyes init animation
  monitor_roboeyes_show_init();

  // comment out for suprise
  //monitor_show_meme();
  //delay(3000);

  // Connect to WiFi AFTER display initialization
  Serial.println(">>> Initializing WiFi...");
  if (request_init(WIFI_SSID, WIFI_PASSWORD)) {
    Serial.println(">>> WiFi connected successfully!\n");
    // Play happy sound on successful connection
    buzzer_play_sound_happy1();
    light_alternate_3sec();
    // Fetch mensa menu
    Serial.println(">>> Fetching Mensa Menu...");
    request_fetch_mensa_menu();
    Serial.println();
  } else {
    Serial.println(">>> WiFi connection failed! Continuing without WiFi...\n");
    // Play sad sound on failed connection
    buzzer_play_sound_sad1();
  }
  
  //monitor_roboeyes_show_return() ;
  //monitor_roboeyes_show_lost();

  gambling_init();
  // Initialize ultrasound sensor
  ultrasound_init();

  // Measure initial distance after start animation
  ultrasound_measure_initial_distance();

  // Initialize shaking sensor with interrupt (instant response!)
  shaking_init(SHAKING_PIN);
  shaking_attach_interrupt(onShakingDetected);

  // Initialize pomodoro timer
  pomodoro_init();
  
  lastPomodoroState = pomodoro_get_state();

  Serial.println("Pomodoro Timer Initialized");
  Serial.println("BTN1 (D5): Start/Pause");
  Serial.println("BTN2 (D4): Toggle Mode / Next/Reset");


  // Show idle screen
  monitor_show_idle_screen(selectedMode, pomodoro_get_completed_count());

  // Initialize button states
  button1LastState = digitalRead(BUTTON1_PIN);
  button2LastState = digitalRead(BUTTON2_PIN);
}

void loop() {
  pomodoro_update();
  unsigned long now = millis();

  bool button1State = digitalRead(BUTTON1_PIN);
  bool button2State = digitalRead(BUTTON2_PIN);

  PomodoroState currentState = pomodoro_get_state();

  if (currentState != lastPomodoroState) {
    lastPomodoroState = currentState;
  }

  if (settingsMode && shakingDetected) {
    shakingDetected = false;
  }

  bool gamblingChoiceActive = gamblingMode && gambling_choice_pending();
  bool bothPressed = (button1State == LOW && button2State == LOW);
  bool bothDebounced = (now - lastButton1Press > debounceDelay &&
                        now - lastButton2Press > debounceDelay);

  bool doubleClickContext = (!settingsMode &&
                             !mensaMenuMode &&
                             !gamblingMode &&
                             !gambling_choice_pending() &&
                             currentState == POMODORO_IDLE);
  if (doubleClickContext && now < settingsReentryBlockUntil) {
    doubleClickContext = false;
  }

  if (button2SingleClickPending) {
    if (!doubleClickContext || (now - button2FirstClickTime > doubleClickInterval)) {
      button2SingleClickPending = false;
      handleButton2SingleAction(currentState);
      currentState = pomodoro_get_state();
      gamblingChoiceActive = gamblingMode && gambling_choice_pending();
      doubleClickContext = (!settingsMode &&
                            !mensaMenuMode &&
                            !gamblingMode &&
                            !gambling_choice_pending() &&
                            currentState == POMODORO_IDLE);
      if (doubleClickContext && now < settingsReentryBlockUntil) {
        doubleClickContext = false;
      }
    }
  }

  if (settingsMode && settingsRequireRelease && button1State == HIGH && button2State == HIGH) {
    settingsRequireRelease = false;
  }

  if (settingsMode && !settingsRequireRelease && bothPressed && bothDebounced) {
    confirmSettings();
    lastButton1Press = now;
    lastButton2Press = now;
    button1LastState = button1State;
    button2LastState = button2State;
    return;
  }

  if (!settingsMode &&
      (mensaMenuMode || gamblingMode || gambling_choice_pending()) &&
      bothPressed && bothDebounced) {
    bool wasGambling = gamblingMode || gambling_choice_pending();
    if (wasGambling) {
      Serial.println("\n=== Exiting Gambling Mode ===");
    } else {
      Serial.println("\n=== Exiting Mensa Menu Mode ===");
    }
    mensaMenuMode = false;
    gambling_reset();
    gamblingMode = false;

    settingsReentryBlockUntil = now + 400;
    if (currentState == POMODORO_IDLE) {
      monitor_show_idle_screen(selectedMode, pomodoro_get_completed_count());
    } else {
      monitor_show_running_screen(pomodoro_get_state(),
                                  pomodoro_get_time_remaining(),
                                  pomodoro_get_completed_count());
    }
    lastPomodoroState = currentState;
    button2SingleClickPending = false;

    lastButton1Press = now;
    lastButton2Press = now;
    button1LastState = button1State;
    button2LastState = button2State;
    return;
  }

  if (!settingsMode && !mensaMenuMode && !gamblingMode &&
      !gambling_choice_pending() && currentState == POMODORO_IDLE &&
      now >= settingsReentryBlockUntil &&
      bothPressed && bothDebounced) {
    enterSettingsMode(selectedMode);
    lastButton1Press = now;
    lastButton2Press = now;
    button1LastState = button1State;
    button2LastState = button2State;
    return;
  }

  if (button1State == LOW && button1LastState == HIGH &&
      now - lastButton1Press > debounceDelay) {
    if (settingsMode) {
      adjustSettingsBy(-5);
    } else if (gamblingChoiceActive) {
      bool win = false;
      if (gambling_register_choice(GamblingChoice::Red, &win)) {
        Serial.println("\n=== Gambling Choice: RED ===");

        monitor_gambling_show_result(GamblingChoice::Red, win);
        if (win) {
          Serial.println("Result: WIN! Mario theme incoming...");
          buzzer_music_mario_play_overworld();
        } else {
          Serial.println("Result: LOSS. Better luck next time.");
          buzzer_play_sound_sad1();
        }
        delay(1500);
        gambling_reset();
        gamblingMode = false;
        if (mensaMenuMode) {
          monitor_show_mensa_menu(mensaMenuIndex, mensaMenuTotal);
        }
        PomodoroState refreshedState = pomodoro_get_state();
        lastPomodoroState = refreshedState;
      }
    } else if (mensaMenuMode) {
      if (mensaMenuIndex > 0) {
        mensaMenuIndex--;
        Serial.print("Previous menu item: ");
        Serial.println(mensaMenuIndex + 1);
        monitor_show_mensa_menu(mensaMenuIndex, mensaMenuTotal);
      }
    } else if (currentState == POMODORO_IDLE) {
      if (selectedMode == MODE_WORK) {
        Serial.println("\n=== Starting Work Session ===");
        pomodoro_start_work();
        lastPomodoroState = pomodoro_get_state();
        ultrasound_measure_initial_distance();
        ultrasoundCheckCount = 0;
        userLost = false;
        lastUltrasoundCheck = now;
      } else {
        Serial.println("\n=== Starting Break ===");
        pomodoro_start_break();
        lastPomodoroState = pomodoro_get_state();
      }
      monitor_show_running_screen(pomodoro_get_state(),
                                  pomodoro_get_time_remaining(),
                                  pomodoro_get_completed_count());
    } else if (currentState == POMODORO_WORK ||
               currentState == POMODORO_SHORT_BREAK ||
               currentState == POMODORO_LONG_BREAK) {
      Serial.println("\n=== Pausing Timer ===");
      pomodoro_pause();
      lastPomodoroState = pomodoro_get_state();
      monitor_show_running_screen(pomodoro_get_state(),
                                  pomodoro_get_time_remaining(),
                                  pomodoro_get_completed_count());
    } else if (currentState == POMODORO_PAUSED) {
      Serial.println("\n=== Resuming Timer ===");
      pomodoro_resume();
      lastPomodoroState = pomodoro_get_state();
      userLost = false;
      monitor_show_running_screen(pomodoro_get_state(),
                                  pomodoro_get_time_remaining(),
                                  pomodoro_get_completed_count());
    }

    lastButton1Press = now;
    currentState = pomodoro_get_state();
    gamblingChoiceActive = gamblingMode && gambling_choice_pending();
  }
  button1LastState = button1State;

  if (button2State == LOW && button2LastState == HIGH &&
      now - lastButton2Press > debounceDelay) {
    if (settingsMode) {
      adjustSettingsBy(5);
    } else if (gamblingChoiceActive) {
      bool win = false;
      if (gambling_register_choice(GamblingChoice::Black, &win)) {
        Serial.println("\n=== Gambling Choice: BLACK ===");

        monitor_gambling_show_result(GamblingChoice::Black, win);
        if (win) {
          Serial.println("Result: WIN! Mario theme incoming...");
          buzzer_music_mario_play_overworld();
        } else {
          Serial.println("Result: LOSS. Better luck next time.");
          buzzer_play_sound_sad1();
        }
        delay(1500);
        gambling_reset();
        gamblingMode = false;
        if (mensaMenuMode) {
          monitor_show_mensa_menu(mensaMenuIndex, mensaMenuTotal);
        }
          PomodoroState refreshedState = pomodoro_get_state();
          lastPomodoroState = refreshedState;
        }
    } else if (mensaMenuMode) {
      if (mensaMenuIndex < mensaMenuTotal - 1) {
        mensaMenuIndex++;
        Serial.print("Next menu item: ");
        Serial.println(mensaMenuIndex + 1);
        monitor_show_mensa_menu(mensaMenuIndex, mensaMenuTotal);
      }
    } else if (doubleClickContext && now >= settingsReentryBlockUntil) {
      if (button2SingleClickPending && (now - button2FirstClickTime <= doubleClickInterval)) {
        button2SingleClickPending = false;
        enterSettingsMode(selectedMode);
      } else {
        button2SingleClickPending = true;
        button2FirstClickTime = now;
      }
    } else {
      handleButton2SingleAction(currentState);
      button2SingleClickPending = false;
    }

    lastButton2Press = now;
    currentState = pomodoro_get_state();
    gamblingChoiceActive = gamblingMode && gambling_choice_pending();
    doubleClickContext = (!settingsMode &&
                          !mensaMenuMode &&
                          !gamblingMode &&
                          !gambling_choice_pending() &&
                          currentState == POMODORO_IDLE);
    if (doubleClickContext && now < settingsReentryBlockUntil) {
      doubleClickContext = false;
    }
  }
  button2LastState = button2State;

  // Check if timer finished
  if (pomodoro_is_finished()) {
    Serial.println("\n=== Timer Finished! ===");
    buzzer_play_sound_happy1();
    monitor_show_finished_screen(pomodoro_get_completed_count());
    delay(3000);
    monitor_show_idle_screen(selectedMode, pomodoro_get_completed_count());
    // Reset ultrasound monitoring when timer finishes
    userLost = false;
  }

  // Ultrasound monitoring during work session (both when user is present and lost)
  if (!mensaMenuMode && (currentState == POMODORO_WORK || currentState == POMODORO_PAUSED)) {
    if (millis() - lastUltrasoundCheck >= ultrasoundCheckInterval) {
      // Take a single measurement (fast, no delays)
      ultrasound_take_single_measurement();
      float currentDistance = ultrasound_get_single_measurement();

      // Store the measurement in our rolling array (we need 3 measurements to compare range)
      int arrayIndex = ultrasoundCheckCount % 3;
      ultrasound_store_measurement(arrayIndex, currentDistance);
      ultrasoundCheckCount++;

      Serial.print("Distance check #");
      Serial.print(ultrasoundCheckCount);
      Serial.print(": ");
      Serial.print(currentDistance);
      Serial.println(" cm");

      // After collecting 3 measurements, check if user is within range
      if (ultrasoundCheckCount >= 3) {
        // Check if the average is within 25cm of initial distance
        bool withinRange = ultrasound_compare_range();

        if (!withinRange && !userLost) {
          // User just went out of range
          Serial.println("!!! USER OUT OF RANGE !!!");
          userLost = true;

          // Pause the pomodoro timer
          pomodoro_pause();
          lastPomodoroState = pomodoro_get_state();
          Serial.println("Timer paused due to user out of range");

          // Play sad tone to indicate user left the workspace
          buzzer_play_sound_sad1();

          // Show the "lost" animation
          monitor_roboeyes_show_lost();

          // Return to running screen after animation (showing paused state)
          monitor_show_running_screen(pomodoro_get_state(),
                                      pomodoro_get_time_remaining(),
                                      pomodoro_get_completed_count());
        } else if (withinRange && userLost) {
          // User returned to range
          Serial.println("!!! USER RETURNED TO RANGE !!!");
          userLost = false;

          // Show the "return" animation
          monitor_roboeyes_show_return();

          // Resume the pomodoro timer
          pomodoro_resume();
          lastPomodoroState = pomodoro_get_state();
          Serial.println("Timer resumed - user back in range");

          // Play happy tone to celebrate the user's return
          buzzer_play_sound_happy1();

          // Return to running screen after animation
          monitor_show_running_screen(pomodoro_get_state(),
                                      pomodoro_get_time_remaining(),
                                      pomodoro_get_completed_count());
        }

        // Reset count to continue monitoring (keep rolling)
        ultrasoundCheckCount = 0;
      }

      lastUltrasoundCheck = millis();
    }
  }

  // Check shaking sensor (interrupt-based - instant response!)
  if (!settingsMode && shakingDetected && (millis() - lastShakingTrigger >= shakingCooldown) && !mensaMenuMode && !gamblingMode) {
    shakingDetected = false; // Reset flag

    Serial.println("!!! SHAKING DETECTED (INSTANT) !!!");

    // Show shake animation on display
    monitor_roboeyes_show_shake();

    // Enter mensa menu mode
    mensaMenuMode = true;
    mensaMenuIndex = 0;
    mensaMenuTotal = request_get_menu_count();

    Serial.println("Entering Mensa Menu Mode");
    Serial.print("Total menu items: ");
    Serial.println(mensaMenuTotal);

    // Show the first menu item
    monitor_show_mensa_menu(mensaMenuIndex, mensaMenuTotal);

    // Update last trigger time for cooldown
    lastShakingTrigger = millis();
  }
  else if (!settingsMode && shakingDetected && (millis() - lastShakingTrigger >= shakingCooldown) && mensaMenuMode && !gamblingMode && !gambling_choice_pending()) {
    shakingDetected = false;
    lastShakingTrigger = millis();

    Serial.println("\n=== Gambling Mode Activated ===");
    Serial.println("Shake detected while in menu.");
    Serial.println("Press BTN1 for RED or BTN2 for BLACK to place your bet.");
    gambling_start();
    gamblingMode = true;
    monitor_gambling_show_intro();
  }

  // Update display periodically (only when not in mensa menu mode)
  if (!settingsMode && !mensaMenuMode) {
    if (currentState == POMODORO_IDLE) {
      // Idle screen: update more frequently for smooth scrolling
      if (millis() - lastDisplayUpdate >= idleDisplayUpdateInterval) {
        monitor_show_idle_screen(selectedMode, pomodoro_get_completed_count());
        lastDisplayUpdate = millis();
      }
    } else if (currentState == POMODORO_WORK ||
               currentState == POMODORO_SHORT_BREAK ||
               currentState == POMODORO_LONG_BREAK ||
               currentState == POMODORO_PAUSED) {
      // Running screen: update every 500ms
      if (millis() - lastDisplayUpdate >= displayUpdateInterval) {
        monitor_show_running_screen(pomodoro_get_state(),
                                    pomodoro_get_time_remaining(),
                                    pomodoro_get_completed_count());
        lastDisplayUpdate = millis();
      }
    }
  }

  delay(10);
}
