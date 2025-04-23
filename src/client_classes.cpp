#include "client_classes.h"
#include <Arduino.h>


/**
 * @brief Constructor for ServerClient class
 * @param serverUrl URL of the backend server
 * @param ssid WiFi network SSID
 * @param password WiFi network password
 */
ServerClient::ServerClient(const char* serverUrl, const char* ssid, const char* password)
    : serverUrl(serverUrl), ssid(ssid), password(password), payload("{}") {}

/**
 * @brief Establishes WiFi connection
 */
void ServerClient::connectWiFi() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(pdMS_TO_TICKS(500)); // Wait while trying to connect
    }
}

/**
 * @brief Get the current status of the WiFi connection
 * @return  True if connected, false otherwise
 */
bool ServerClient::GetWiFiStatus() {
    return WiFi.status() == WL_CONNECTED ? true : false;
}

/**
 * @brief Adds a key-value pair to the JSON payload
 * @param key The name of the data field
 * @param value The corresponding value to store
 */
void ServerClient::prepareData(const String& key, const String& value) {
    if (payload == "{}") { 
        payload = "{"; // Start JSON object
    } else {
        payload += ","; // Separate multiple fields
    }
    payload += "\"" + key + "\": \"" + value + "\""; // Append key-value pair
}

/**
 * @brief Sends JSON payload to the server via HTTP POST request
 */
void ServerClient::sendPayload() {
    payload += "}"; // Close the JSON object
    HTTPClient http;
    http.begin(serverUrl);
    http.setTimeout(5000); // 5-second timeout
    http.addHeader("Content-Type", "application/json");
    
    int httpResponseCode = http.POST(payload);
    if (httpResponseCode <= 0) {
        Serial.println("HTTP POST failed");
    } 
    http.end();
    payload = "{}"; // Reset payload
}

/**
 * @brief Closes the HTTP connection (minimal cleanup)
 */
void ServerClient::closeConnection() {
    HTTPClient http;
    http.end(); // Ensure connection cleanup
}
