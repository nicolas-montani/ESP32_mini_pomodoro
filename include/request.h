#pragma once

#include <Arduino.h>

/**
 * Initializes the WiFi connection and HTTP client for making requests.
 * Must be called before using any other request functions.
 *
 * @param ssid WiFi network name
 * @param password WiFi password
 * @return true if WiFi connected successfully, false otherwise
 */
bool request_init(const char* ssid, const char* password);

/**
 * Fetches the mensa menu from mensa-hsg.vercel.com/menu.json
 * and displays it in the Serial terminal.
 *
 * @return true if request was successful, false otherwise
 */
bool request_fetch_mensa_menu();

/**
 * Checks if WiFi is currently connected.
 *
 * @return true if connected, false otherwise
 */
bool request_is_wifi_connected();

/**
 * Gets the current WiFi signal strength (RSSI).
 *
 * @return RSSI value in dBm
 */
int request_get_wifi_rssi();
