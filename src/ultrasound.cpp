#include "ultrasound.h"

// Global variables
static float initialDistance = 0.0;
static float measurements[3] = {0.0, 0.0, 0.0};
static float lastSingleMeasurement = 0.0;

// Initialize ultrasound sensor pins
void ultrasound_init() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);
}

// Internal helper function to measure distance once
static float measureOnce() {
  // Clear trigger pin
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // Send 10us pulse
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Read echo pin with timeout
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout

  // Calculate distance in cm
  if (duration == 0) {
    return -1.0; // No echo received
  }

  float distance = duration * 0.034 / 2.0;
  return distance;
}

// Measure and save the initial distance
void ultrasound_measure_initial_distance() {
  initialDistance = measureOnce();
  lastSingleMeasurement = initialDistance;

  Serial.print("Initial distance measured: ");
  Serial.print(initialDistance);
  Serial.println(" cm");
}

// Take a single measurement and store it (for continuous monitoring)
void ultrasound_take_single_measurement() {
  float measurement = measureOnce();
  lastSingleMeasurement = measurement;
}

// Store a measurement at a specific index (0, 1, or 2)
void ultrasound_store_measurement(int index, float measurement) {
  if (index >= 0 && index < 3) {
    measurements[index] = measurement;
  }
}

// Take 3 measurements (1 per second) and save them in an array
void ultrasound_measure_distance() {
  Serial.println("Taking 3 measurements (1 per second)...");

  for (int i = 0; i < 3; i++) {
    measurements[i] = measureOnce();
    lastSingleMeasurement = measurements[i];

    Serial.print("Measurement ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(measurements[i]);
    Serial.println(" cm");

    // Wait 1 second between measurements (except after last one)
    if (i < 2) {
      delay(1000);
    }
  }
}

// Compare the 3 measurements
// Returns true if NOT all the same (different)
// Returns false if exactly the same
bool ultrasound_compare_distances() {
  // Check if all three measurements are exactly equal
  if (measurements[0] == measurements[1] &&
      measurements[1] == measurements[2]) {
    Serial.println("All measurements are exactly the same");
    return false; // All the same
  } else {
    Serial.println("Measurements are different");
    return true; // Not the same
  }
}

// Check if average of 3 measurements is within 25cm range of initial measurement
bool ultrasound_compare_range() {
  // Check if any measurement is -1.0 (no echo received)
  if (measurements[0] == -1.0 || measurements[1] == -1.0 || measurements[2] == -1.0) {
    Serial.println("One or more measurements failed (no echo) - out of range");
    return false;
  }

  // Check if initial distance is -1.0
  if (initialDistance == -1.0) {
    Serial.println("Initial distance invalid - out of range");
    return false;
  }

  // Calculate average of the 3 measurements
  float average = (measurements[0] + measurements[1] + measurements[2]) / 3.0;

  Serial.print("Average of 3 measurements: ");
  Serial.print(average);
  Serial.println(" cm");

  Serial.print("Initial distance: ");
  Serial.print(initialDistance);
  Serial.println(" cm");

  // Calculate difference from initial distance
  float difference = abs(average - initialDistance);

  Serial.print("Difference from initial: ");
  Serial.print(difference);
  Serial.println(" cm");

  // Check if within 25cm range
  if (difference <= 25.0) {
    Serial.println("Within 25cm range of initial measurement");
    return true;
  } else {
    Serial.println("Outside 25cm range of initial measurement");
    return false;
  }
}

// Get the most recent single distance measurement
float ultrasound_get_single_measurement() {
  return lastSingleMeasurement;
}

// Get initial distance
float ultrasound_get_initial_distance() {
  return initialDistance;
}

// Get the average of the last 3 measurements
float ultrasound_get_average_distance() {
  return (measurements[0] + measurements[1] + measurements[2]) / 3.0;
}
