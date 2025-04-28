#include "WiFi_classes.h"
#include <Arduino.h>

/**
 * @brief Constructor for WiFiManager class.
 * @param ssid WiFi network SSID.
 * @param password WiFi network password.
 */
WiFiManager::WiFiManager(const char* ssid, const char* password)
    : ssid(ssid), password(password) {}

/**
 * @brief Get the SSID of the WiFi network.
 * @return The SSID of the WiFi network.
 */
const char* WiFiManager::getSSID() {
    return ssid;
}

/**
 * @brief Establishes WiFi connection.
 */
void WiFiManager::connectWiFi() {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
}

/**
 * @brief Get the current status of the WiFi connection.
 * @return True if connected, false otherwise.
 */
bool WiFiManager::IsWiFiConnected() {
    return WiFi.status() == WL_CONNECTED;
}

/**
 * @brief Get the local IP address of the ESP32.
 * @return The local IP address of the ESP32.
 */
IPAddress WiFiManager::getWiFiLocalIp() {
    return WiFi.localIP();
}