#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// I2C PIN DEFINITIONS (for display)
// ============================================================================

#ifndef SDA_PIN
#define SDA_PIN 21  // I2C Data pin
#endif

#ifndef SCL_PIN
#define SCL_PIN 22  // I2C Clock pin
#endif

// ============================================================================
// BUTTON PIN DEFINITIONS
// ============================================================================

#ifndef BUTTON1_PIN
#define BUTTON1_PIN 5  // Start/Pause button
#endif

#ifndef BUTTON2_PIN
#define BUTTON2_PIN 4  // Toggle Work/Break OR Next/Reset when running
#endif

// ============================================================================
// WIFI CONFIGURATION
// ============================================================================

// WiFi credentials can be overridden by defining them before including this file
#ifndef WIFI_SSID
#define WIFI_SSID "Self_Destruction_Device"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "123456789"
#endif

#endif // CONFIG_H
