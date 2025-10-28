#include <Arduino.h>
#include "pomodoro.h"
#include "monitor.h"
#include "ultrasound.h"

// Pin definitions
#define SDA_PIN 21
#define SCL_PIN 22
#define BUTTON1_PIN 5  // Start/Pause button
#define BUTTON2_PIN 4  // Toggle Work/Break OR Next/Reset when running

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

// Mode selection (when idle)
IdleMode selectedMode = MODE_WORK;

void setup() {
  Serial.begin(115200);

  // Initialize monitor (display)
  if (!monitor_init(SDA_PIN, SCL_PIN)) {
    Serial.println("Failed to initialize monitor!");
    for(;;);
  }

  // comment out for suprise
  //monitor_show_meme();
  //delay(3000);

  // Initialize RoboEyes
  monitor_roboeyes_init();

  monitor_roboeyes_show_init();
  //monitor_roboeyes_show_return() ;
  //monitor_roboeyes_show_lost();


  // Initialize ultrasound sensor
  ultrasound_init();

  // Measure initial distance after start animation
  ultrasound_measure_initial_distance();

  // Initialize pins
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);

  // Initialize pomodoro timer
  pomodoro_init();

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

  // Button 1: Start/Pause
  if (button1State == LOW && button1LastState == HIGH) {
    if (millis() - lastButton1Press > debounceDelay) {

      if (currentState == POMODORO_IDLE) {
        // Start selected mode
        if (selectedMode == MODE_WORK) {
          Serial.println("\n=== Starting Work Session ===");
          pomodoro_start_work();
          // Measure initial distance when work session starts
          ultrasound_measure_initial_distance();
          // Reset ultrasound monitoring
          ultrasoundCheckCount = 0;
          userLost = false;
          lastUltrasoundCheck = millis();
        } else {
          Serial.println("\n=== Starting Break ===");
          pomodoro_start_break();
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
        monitor_show_running_screen(pomodoro_get_state(),
                                    pomodoro_get_time_remaining(),
                                    pomodoro_get_completed_count());
      } else if (currentState == POMODORO_PAUSED) {
        // Resume
        Serial.println("\n=== Resuming Timer ===");
        pomodoro_resume();
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

  // Button 2: Toggle mode when idle, Next/Reset when running
  if (button2State == LOW && button2LastState == HIGH) {
    if (millis() - lastButton2Press > debounceDelay) {

      if (currentState == POMODORO_IDLE) {
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
    monitor_show_finished_screen(pomodoro_get_completed_count());
    delay(3000);
    monitor_show_idle_screen(selectedMode, pomodoro_get_completed_count());
    // Reset ultrasound monitoring when timer finishes
    userLost = false;
  }

  // Ultrasound monitoring during work session (both when user is present and lost)
  if (currentState == POMODORO_WORK || currentState == POMODORO_PAUSED) {
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
          Serial.println("Timer paused due to user out of range");

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
          Serial.println("Timer resumed - user back in range");

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

  // Update display periodically
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

  delay(10);
}
