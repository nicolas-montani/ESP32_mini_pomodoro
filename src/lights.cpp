#include <Arduino.h>
#include "lights.h"

// Initialize the LED pins
void lights_init() {
    Serial.println("Initializing LEDs...");

    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);

    // Initial test: quick blink to confirm LEDs are working
    light_both_on();
    delay(200);
    light_both_off();
    delay(100);
    light_both_on();
    delay(200);
    light_both_off();

    Serial.println("LEDs initialized successfully!");
}

// Turn green LED on
void light_green_on() {
    digitalWrite(GREEN_LED_PIN, HIGH);
}

// Turn green LED off
void light_green_off() {
    digitalWrite(GREEN_LED_PIN, LOW);
}

// Turn red LED on
void light_red_on() {
    digitalWrite(RED_LED_PIN, HIGH);
}

// Turn red LED off
void light_red_off() {
    digitalWrite(RED_LED_PIN, LOW);
}

// Turn both LEDs on
void light_both_on() {
    light_green_on();
    light_red_on();
}

// Turn both LEDs off
void light_both_off() {
    light_green_off();
    light_red_off();
}

// Alternate between LEDs for 3 seconds (0.5s each)
void light_alternate_3sec() {
    unsigned long startTime = millis();
    bool greenState = true;

    while (millis() - startTime < 3000) {
        if (greenState) {
            light_green_on();
            light_red_off();
        } else {
            light_green_off();
            light_red_on();
        }

        delay(500); // Half a second
        greenState = !greenState;
    }

    // Turn both off when done
    light_both_off();
}
