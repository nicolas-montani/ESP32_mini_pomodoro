#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "request.h"

namespace {
  const char* mensaApiUrl = "https://mensa-hsg.vercel.app/menu.json";
  bool wifiInitialized = false;
  MensaMenuItem menuItems[MAX_MENU_ITEMS];
  int menuItemCount = 0;
}

bool request_init(const char* ssid, const char* password) {
  Serial.println("\n=== Connecting to WiFi ===");
  Serial.print("SSID: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Wait for connection (30 second timeout)
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 60) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✓ WiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal Strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    wifiInitialized = true;
    return true;
  } else {
    Serial.println("\n✗ WiFi Connection Failed!");
    wifiInitialized = false;
    return false;
  }
}

bool request_fetch_mensa_menu() {
  if (!wifiInitialized || WiFi.status() != WL_CONNECTED) {
    Serial.println("✗ WiFi not connected! Call request_init() first.");
    return false;
  }

  Serial.println("\n=== Fetching Mensa Menu ===");
  Serial.print("URL: ");
  Serial.println(mensaApiUrl);

  HTTPClient http;
  http.begin(mensaApiUrl);

  // Increase timeout to 20 seconds for slow connections
  http.setTimeout(20000);

  // Set connection timeout separately
  http.setConnectTimeout(10000);

  // Add user agent to avoid potential blocking
  http.addHeader("User-Agent", "ESP32-Mensa-Client/1.0");
  http.addHeader("Accept", "application/json");

  Serial.println("Sending HTTP GET request...");
  Serial.println("(This may take a few seconds...)");

  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    Serial.print("✓ HTTP Response Code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode == 200) {
      String payload = http.getString();
      Serial.println("\n=== Mensa Menu ===");
      Serial.println(payload);
      Serial.println("==================\n");

      // Parse JSON and store menu items
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        Serial.println("=== Parsed Menu ===");

        // The API returns a flat array of menu items
        if (doc.is<JsonArray>()) {
          JsonArray items = doc.as<JsonArray>();
          int count = items.size();
          Serial.print("Found ");
          Serial.print(count);
          Serial.println(" menu items\n");

          // Store menu items (up to MAX_MENU_ITEMS)
          menuItemCount = 0;
          int index = 0;
          for (JsonVariant item : items) {
            if (index >= MAX_MENU_ITEMS) {
              Serial.println("⚠ Menu limit reached, storing first 20 items only");
              break;
            }

            // Store in global array
            menuItems[index].date = item["date"].as<String>();
            menuItems[index].weekday = item["weekday"].as<String>();
            menuItems[index].title = item["title"].as<String>();
            menuItems[index].price_chf = item["price_chf"].as<String>();
            menuItems[index].source = item["source"].as<String>();

            // Print to serial
            Serial.print("[");
            Serial.print(index + 1);
            Serial.println("]");
            Serial.print("Day: ");
            Serial.println(menuItems[index].weekday);
            Serial.print("Date: ");
            Serial.println(menuItems[index].date);
            Serial.print("Title: ");
            Serial.println(menuItems[index].title);
            Serial.print("Price: CHF ");
            Serial.println(menuItems[index].price_chf);
            Serial.println("---");

            index++;
            menuItemCount++;
          }

          Serial.print("✓ Stored ");
          Serial.print(menuItemCount);
          Serial.println(" menu items in memory");
        } else {
          Serial.println("✗ Expected JSON array but got different format");
        }

        Serial.println("==================");
      } else {
        Serial.print("✗ JSON Parsing Failed: ");
        Serial.println(error.c_str());
      }

      http.end();
      return true;
    } else {
      Serial.println("✗ Unexpected HTTP response code");
    }
  } else {
    Serial.print("✗ HTTP Request Failed: ");
    Serial.println(http.errorToString(httpResponseCode));
  }

  http.end();
  return false;
}

bool request_is_wifi_connected() {
  return wifiInitialized && (WiFi.status() == WL_CONNECTED);
}

int request_get_wifi_rssi() {
  if (request_is_wifi_connected()) {
    return WiFi.RSSI();
  }
  return 0;
}

MensaMenuItem* request_get_menu_items() {
  return menuItems;
}

int request_get_menu_count() {
  return menuItemCount;
}
