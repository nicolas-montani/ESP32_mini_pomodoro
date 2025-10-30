#ifndef ULTRASOUND_H
#define ULTRASOUND_H

#include <Arduino.h>

// Pin definitions
#define TRIG_PIN 19
#define ECHO_PIN 18

// Initialize ultrasound sensor
void ultrasound_init();

// Measure and save the initial distance
void ultrasound_measure_initial_distance();

// Take a single measurement and store it (for continuous monitoring)
void ultrasound_take_single_measurement();

// Store a measurement at a specific index (0, 1, or 2)
void ultrasound_store_measurement(int index, float measurement);

// Take 3 measurements (1 per second) and save them in an array
void ultrasound_measure_distance();

// Compare the 3 measurements - returns true if NOT all the same, false if exactly the same
bool ultrasound_compare_distances();

// Check if average of 3 measurements is within 25cm range of initial measurement
bool ultrasound_compare_range();

// Get the most recent single distance measurement
float ultrasound_get_single_measurement();

// Get initial distance
float ultrasound_get_initial_distance();

// Get the average of the last 3 measurements
float ultrasound_get_average_distance();

#endif
