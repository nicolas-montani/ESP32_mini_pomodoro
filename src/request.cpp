#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "request.h"

namespace {
  const char* mensaApiUrl = "https://mensa-hsg.vercel.app/menu.json";
  bool wifiInitialized = false;
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
  http.setTimeout(10000); // 10 second timeout

  Serial.println("Sending HTTP GET request...");
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    Serial.print("✓ HTTP Response Code: ");
    Serial.println(httpResponseCode);

    if (httpResponseCode == 200) {
      String payload = http.getString();
      Serial.println("\n=== Mensa Menu ===");
      Serial.println(payload);
      Serial.println("==================\n");

      // Optional: Parse JSON for better formatting
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        Serial.println("=== Parsed Menu ===");

        // Display date if available
        if (doc.containsKey("date")) {
          Serial.print("Date: ");
          Serial.println(doc["date"].as<String>());
        }

        // Display menu items
        if (doc.containsKey("menu") && doc["menu"].is<JsonArray>()) {
          JsonArray menu = doc["menu"].as<JsonArray>();
          int index = 1;

          for (JsonVariant item : menu) {
            Serial.print("\n[");
            Serial.print(index++);
            Serial.println("]");

            if (item.containsKey("category")) {
              Serial.print("  Category: ");
              Serial.println(item["category"].as<String>());
            }

            if (item.containsKey("title")) {
              Serial.print("  Title: ");
              Serial.println(item["title"].as<String>());
            }

            if (item.containsKey("description")) {
              Serial.print("  Description: ");
              Serial.println(item["description"].as<String>());
            }

            if (item.containsKey("price")) {
              Serial.print("  Price: CHF ");
              Serial.println(item["price"].as<String>());
            }
          }
        } else {
          Serial.println("No menu array found in JSON");
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
