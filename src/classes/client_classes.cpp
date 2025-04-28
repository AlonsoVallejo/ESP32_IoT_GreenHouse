#include "client_classes.h"
#include <Arduino.h>

/**
 * @brief Constructor for ServerClient class.
 * @param serverUrl URL of the backend server.
 * @param wifiManager Pointer to the WiFiManager instance.
 */
ServerClient::ServerClient(const char* serverUrl, WiFiManager* wifiManager)
    : serverUrl(serverUrl), wifiManager(wifiManager), payload("{}") {}

/**
 * @brief Adds a key-value pair to the JSON payload.
 * @param key The name of the data field.
 * @param value The corresponding value to store.
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
 * @brief Sends JSON payload to the server via HTTP POST request.
 */
void ServerClient::sendPayload() {
    payload += "}"; // Close the JSON object
    HTTPClient http;

    for (int retry = 0; retry < 3; retry++) { // Retry up to 3 times
        http.begin(serverUrl);
        http.setTimeout(5000);
        http.addHeader("Content-Type", "application/json");

        int httpResponseCode = http.POST(payload);
        if (httpResponseCode > 0) {
            // HTTP POST successful
            Serial.print("HTTP POST successful, response code: ");
            Serial.println(httpResponseCode);
            break; // Exit retry loop
        } else {
            // HTTP POST failed
            Serial.print("HTTP POST failed, error: ");
            Serial.println(http.errorToString(httpResponseCode).c_str());
        }

        http.end(); // Close the connection
        vTaskDelay(pdMS_TO_TICKS(2000)); // Wait 2 seconds before retrying
    }

    http.end(); // Ensure connection cleanup
    payload = "{}"; // Reset payload
}

/**
 * @brief Closes the HTTP connection (minimal cleanup).
 */
void ServerClient::closeConnection() {
    HTTPClient http;
    http.end(); // Ensure connection cleanup
}
