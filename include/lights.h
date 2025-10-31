#ifndef LIGHTS_H
#define LIGHTS_H

// Pin definitions
// NOTE: GPIO 34 and 35 are INPUT-ONLY on ESP32! Use these safe output pins instead:
#define GREEN_LED_PIN 33  // Safe output pin
#define RED_LED_PIN 32    // Safe output pin

// Function declarations
void lights_init();
void light_green_on();
void light_green_off();
void light_red_on();
void light_red_off();
void light_both_on();
void light_both_off();
void light_alternate_3sec();

#endif // LIGHTS_H
