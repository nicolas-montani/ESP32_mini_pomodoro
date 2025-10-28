// Example usage of the ultrasound module
// This file demonstrates how to use all the ultrasound functions

#include <Arduino.h>
#include "ultrasound.h"

void example_usage() {
  Serial.begin(115200);

  // 1. Initialize the ultrasound sensor
  ultrasound_init();
  Serial.println("Ultrasound sensor initialized");
  delay(1000);

  // 2. Measure and save the initial distance
  Serial.println("\n=== Step 1: Measuring initial distance ===");
  ultrasound_measure_initial_distance();

  float initial = ultrasound_get_initial_distance();
  Serial.print("Saved initial distance: ");
  Serial.print(initial);
  Serial.println(" cm");

  delay(2000);

  // 3. Take 3 measurements (1 per second each)
  Serial.println("\n=== Step 2: Taking 3 measurements ===");
  ultrasound_measure_distance();

  float average = ultrasound_get_average_distance();
  Serial.print("Average of 3 measurements: ");
  Serial.print(average);
  Serial.println(" cm");

  delay(1000);

  // 4. Compare if the 3 measurements are the same or different
  Serial.println("\n=== Step 3: Comparing measurements ===");
  bool areDifferent = ultrasound_compare_distances();
  if (areDifferent) {
    Serial.println("Result: Measurements are DIFFERENT");
  } else {
    Serial.println("Result: Measurements are EXACTLY the same");
  }

  delay(1000);

  // 5. Check if average is within 1m range of initial
  Serial.println("\n=== Step 4: Checking range ===");
  bool withinRange = ultrasound_compare_range();
  if (withinRange) {
    Serial.println("Result: Within 1m (100cm) range of initial measurement");
  } else {
    Serial.println("Result: Outside 1m (100cm) range of initial measurement");
  }

  Serial.println("\n=== Example complete ===\n");
}

// Example in a typical Arduino setup/loop structure
void setup() {
  Serial.begin(115200);
  ultrasound_init();

  delay(2000);
  Serial.println("Starting ultrasound example...");

  // Measure initial distance on startup
  ultrasound_measure_initial_distance();
}

void loop() {
  // Example: Press a button to trigger measurements
  // if (buttonPressed) {
  //   ultrasound_measure_distance();  // Takes 3 seconds
  //
  //   if (ultrasound_compare_distances()) {
  //     Serial.println("Measurements varied");
  //   }
  //
  //   if (ultrasound_compare_range()) {
  //     Serial.println("Still in range!");
  //   } else {
  //     Serial.println("Too far from initial position!");
  //   }
  // }

  delay(100);
}
