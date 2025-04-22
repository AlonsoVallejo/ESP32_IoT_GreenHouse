#include "client_classes.h"

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
    http.begin(serverUrl); // Initialize HTTP connection
    http.addHeader("Content-Type", "application/json"); // Set request header

    int httpResponseCode = http.POST(payload); // Send HTTP request
    http.end(); // Close connection

    payload = "{}"; // Reset payload after transmission
}

/**
 * @brief Closes the HTTP connection (minimal cleanup)
 */
void ServerClient::closeConnection() {
    HTTPClient http;
    http.end(); // Ensure connection cleanup
}
