#include <Arduino.h>
#include "pomodoro.h"
#include "monitor.h"
#include "ultrasound.h"
#include "buzzer.h"
#include "shaking.h"
#include "request.h"
#include "lights.h"
#include "gambling.h"

namespace {
PomodoroState lastPomodoroState = POMODORO_IDLE;

void updateLightsForState(PomodoroState state) {
  switch (state) {
    case POMODORO_WORK:
      lights_show_work();
      break;
    case POMODORO_SHORT_BREAK:
    case POMODORO_LONG_BREAK:
      lights_show_break();
      break;
    default:
      lights_show_idle();
      break;
  }
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
const unsigned long shakingCooldown = 500; // 500ms cooldown between triggers

// Mode selection (when idle)
IdleMode selectedMode = MODE_WORK;

// Mensa menu mode variables
bool mensaMenuMode = false;
int mensaMenuIndex = 0;
int mensaMenuTotal = 0;
bool gamblingMode = false;

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
  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);
  buzzer_music_mario_init(BUZZER_PIN);
  lights_init();
  // Debug: blink both LEDs during startup.
  lights_blink_both();
  buzzer_play_sound_turn_on(BUZZER_PIN);
  gambling_init();

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
    buzzer_play_sound_happy1(BUZZER_PIN);

    // Fetch mensa menu
    Serial.println(">>> Fetching Mensa Menu...");
    request_fetch_mensa_menu();
    Serial.println();
  } else {
    Serial.println(">>> WiFi connection failed! Continuing without WiFi...\n");
    // Play sad sound on failed connection
    buzzer_play_sound_sad1(BUZZER_PIN);
  }
  
  //monitor_roboeyes_show_return() ;
  //monitor_roboeyes_show_lost();


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
  updateLightsForState(lastPomodoroState);

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
  // Update the pomodoro timer
  pomodoro_update();

  // Read button states
  bool button1State = digitalRead(BUTTON1_PIN);
  bool button2State = digitalRead(BUTTON2_PIN);

  PomodoroState currentState = pomodoro_get_state();
  if (currentState != lastPomodoroState) {
    updateLightsForState(currentState);
    lastPomodoroState = currentState;
  }

  // Check if both buttons are pressed (for exiting menu/gambling modes)
  if (mensaMenuMode && button1State == LOW && button2State == LOW) {
    if (millis() - lastButton1Press > debounceDelay && millis() - lastButton2Press > debounceDelay) {
      bool wasGambling = gamblingMode || gambling_choice_pending();
      if (wasGambling) {
        Serial.println("\n=== Exiting Gambling Mode ===");
      } else {
        Serial.println("\n=== Exiting Mensa Menu Mode ===");
      }
      mensaMenuMode = false;
      gambling_reset();
      gamblingMode = false;

      // Restore previous screen state
      if (currentState == POMODORO_IDLE) {
        monitor_show_idle_screen(selectedMode, pomodoro_get_completed_count());
      } else {
        monitor_show_running_screen(pomodoro_get_state(),
                                    pomodoro_get_time_remaining(),
                                    pomodoro_get_completed_count());
      }
      // Ensure lights reflect the active Pomodoro state.
      updateLightsForState(currentState);
      lastPomodoroState = currentState;

      lastButton1Press = millis();
      lastButton2Press = millis();
      button1LastState = button1State;
      button2LastState = button2State;
      // Skip further button handling this loop to avoid false triggers.
      return;
    }
  }
  // Button 1: Previous menu item (in mensa mode) or Start/Pause (normal mode)
  else if (button1State == LOW && button1LastState == HIGH) {
    if (millis() - lastButton1Press > debounceDelay) {

      if (gamblingMode && gambling_choice_pending()) {
        bool win = false;
        if (gambling_register_choice(GamblingChoice::Red, &win)) {
          Serial.println("\n=== Gambling Choice: RED ===");
          lights_show_work();
          monitor_gambling_show_result(GamblingChoice::Red, win);
          if (win) {
            Serial.println("Result: WIN! Mario theme incoming...");
            buzzer_music_mario_play_overworld();
          } else {
            Serial.println("Result: LOSS. Better luck next time.");
            buzzer_play_sound_sad1(BUZZER_PIN);
          }
          delay(1500);
          gambling_reset();
          gamblingMode = false;
          if (mensaMenuMode) {
            monitor_show_mensa_menu(mensaMenuIndex, mensaMenuTotal);
          }
          PomodoroState refreshedState = pomodoro_get_state();
          updateLightsForState(refreshedState);
          lastPomodoroState = refreshedState;
        }
        lastButton1Press = millis();
        button1LastState = button1State;
        return;
      }

      if (mensaMenuMode) {
        // Navigate to previous menu item
        if (mensaMenuIndex > 0) {
          mensaMenuIndex--;
          Serial.print("Previous menu item: ");
          Serial.println(mensaMenuIndex + 1);
          monitor_show_mensa_menu(mensaMenuIndex, mensaMenuTotal);
        }
      } else if (currentState == POMODORO_IDLE) {
        // Start selected mode
        if (selectedMode == MODE_WORK) {
          Serial.println("\n=== Starting Work Session ===");
          pomodoro_start_work();
          lastPomodoroState = pomodoro_get_state();
          updateLightsForState(lastPomodoroState);
          // Measure initial distance when work session starts
          ultrasound_measure_initial_distance();
          // Reset ultrasound monitoring
          ultrasoundCheckCount = 0;
          userLost = false;
          lastUltrasoundCheck = millis();
        } else {
          Serial.println("\n=== Starting Break ===");
          pomodoro_start_break();
          lastPomodoroState = pomodoro_get_state();
          updateLightsForState(lastPomodoroState);
        }
        monitor_show_running_screen(pomodoro_get_state(),
                                    pomodoro_get_time_remaining(),
                                    pomodoro_get_completed_count());
      } else if (currentState == POMODORO_WORK ||
                 currentState == POMODORO_SHORT_BREAK ||
                 currentState == POMODORO_LONG_BREAK) {
        // Pause
        Serial.println("\n=== Pausing Timer ===");
        pomodoro_pause();
        lastPomodoroState = pomodoro_get_state();
        updateLightsForState(lastPomodoroState);
        monitor_show_running_screen(pomodoro_get_state(),
                                    pomodoro_get_time_remaining(),
                                    pomodoro_get_completed_count());
      } else if (currentState == POMODORO_PAUSED) {
        // Resume
        Serial.println("\n=== Resuming Timer ===");
        pomodoro_resume();
        lastPomodoroState = pomodoro_get_state();
        updateLightsForState(lastPomodoroState);
        // Reset lost flag when resuming
        userLost = false;
        monitor_show_running_screen(pomodoro_get_state(),
                                    pomodoro_get_time_remaining(),
                                    pomodoro_get_completed_count());
      }

      lastButton1Press = millis();
    }
  }
  button1LastState = button1State;

  // Button 2: Next menu item (in mensa mode) or Toggle mode/Reset (normal mode)
  if (button2State == LOW && button2LastState == HIGH) {
    if (millis() - lastButton2Press > debounceDelay) {

      if (gamblingMode && gambling_choice_pending()) {
        bool win = false;
        if (gambling_register_choice(GamblingChoice::Black, &win)) {
          Serial.println("\n=== Gambling Choice: BLACK ===");
          lights_show_break();
          monitor_gambling_show_result(GamblingChoice::Black, win);
          if (win) {
            Serial.println("Result: WIN! Mario theme incoming...");
            buzzer_music_mario_play_overworld();
          } else {
            Serial.println("Result: LOSS. Better luck next time.");
            buzzer_play_sound_sad1(BUZZER_PIN);
          }
          delay(1500);
          gambling_reset();
          gamblingMode = false;
          if (mensaMenuMode) {
            monitor_show_mensa_menu(mensaMenuIndex, mensaMenuTotal);
          }
          PomodoroState refreshedState = pomodoro_get_state();
          updateLightsForState(refreshedState);
          lastPomodoroState = refreshedState;
        }
        lastButton2Press = millis();
        button2LastState = button2State;
        return;
      }

      if (mensaMenuMode) {
        // Navigate to next menu item
        if (mensaMenuIndex < mensaMenuTotal - 1) {
          mensaMenuIndex++;
          Serial.print("Next menu item: ");
          Serial.println(mensaMenuIndex + 1);
          monitor_show_mensa_menu(mensaMenuIndex, mensaMenuTotal);
        }
      } else if (currentState == POMODORO_IDLE) {
        // Toggle between WORK and BREAK
        if (selectedMode == MODE_WORK) {
          selectedMode = MODE_BREAK;
          Serial.println("\n=== Mode: BREAK ===");
        } else {
          selectedMode = MODE_WORK;
          Serial.println("\n=== Mode: WORK ===");
        }
        monitor_show_idle_screen(selectedMode, pomodoro_get_completed_count());
      } else {
        // Reset to idle when running
        Serial.println("\n=== Resetting Timer ===");
        pomodoro_reset();
        lastPomodoroState = pomodoro_get_state();
        updateLightsForState(lastPomodoroState);
        // Reset lost flag when resetting timer
        userLost = false;
        monitor_show_idle_screen(selectedMode, pomodoro_get_completed_count());
      }

      lastButton2Press = millis();
    }
  }
  button2LastState = button2State;

  // Check if timer finished
  if (pomodoro_is_finished()) {
    Serial.println("\n=== Timer Finished! ===");
    buzzer_play_sound_happy1(BUZZER_PIN);
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
          updateLightsForState(lastPomodoroState);
          Serial.println("Timer paused due to user out of range");

          // Play sad tone to indicate user left the workspace
          buzzer_play_sound_sad1(BUZZER_PIN);

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
          updateLightsForState(lastPomodoroState);
          Serial.println("Timer resumed - user back in range");

          // Play happy tone to celebrate the user's return
          buzzer_play_sound_happy1(BUZZER_PIN);

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
  if (shakingDetected && (millis() - lastShakingTrigger >= shakingCooldown) && !mensaMenuMode) {
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
  else if (shakingDetected && (millis() - lastShakingTrigger >= shakingCooldown) && mensaMenuMode && !gamblingMode && !gambling_choice_pending()) {
    shakingDetected = false;
    lastShakingTrigger = millis();

    Serial.println("\n=== Gambling Mode Activated ===");
    Serial.println("Shake detected while in menu.");
    Serial.println("Press BTN1 for RED or BTN2 for BLACK to place your bet.");
    gambling_start();
    gamblingMode = true;
    monitor_gambling_show_intro();
    lights_blink_both(2, 120, 120);
  }

  // Update display periodically (only when not in mensa menu mode)
  if (!mensaMenuMode) {
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
