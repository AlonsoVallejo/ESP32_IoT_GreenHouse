#include "client_classes.h"
#include <Arduino.h>
#include "LogMgr.h"
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
    } else if (payload[payload.length() - 1] != '{') {
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

    LogSerial("Preparing to send payload: ", false);
    LogSerialn(payload, false); // Log the payload for debugging

    for (int retry = 0; retry < 3; retry++) { // Retry up to 3 times
        String fullUrl = String(serverUrl) + "updateData"; // Append the endpoint to the base URL
        if (serverUrl[strlen(serverUrl) - 1] != '/') {
            fullUrl = String(serverUrl) + "/updateData"; // Ensure the URL is correctly formatted
        }

        http.begin(fullUrl); // Use the full URL
        http.setTimeout(5000);
        http.addHeader("Content-Type", "application/json");

        int httpResponseCode = http.POST(payload);
        if (httpResponseCode > 0) {
            // HTTP POST successful
            LogSerial("HTTP POST successful, response code: ", true);
            LogSerialn(String(httpResponseCode), true);
            break; // Exit retry loop
        } else {
            // HTTP POST failed
            LogSerial("HTTP POST failed, error: ", true);
            LogSerialn(http.errorToString(httpResponseCode).c_str(), true);
        }

        http.end();
    }

    payload = "{}"; // Reset the payload after sending
}

/**
 * @brief Sends default settings to the server if no settings exist.
 * @param maxLevel Default max level percentage.
 * @param minLevel Default min level percentage.
 * @param hotTemp Default hot temperature.
 * @param lowHumidity Default low humidity.
 */
void ServerClient::sendDefaultSettings(uint8_t maxLevel, uint8_t minLevel, uint8_t hotTemp, uint8_t lowHumidity) {
    String payload = "{";
    payload += "\"defaultSettings\": {";
    payload += "\"maxLevel\": " + String(maxLevel) + ",";
    payload += "\"minLevel\": " + String(minLevel) + ",";
    payload += "\"hotTemperature\": " + String(hotTemp) + ",";
    payload += "\"lowHumidity\": " + String(lowHumidity);
    payload += "}}";

    HTTPClient http;
    String fullUrl = String(serverUrl) + "saveDefaultSettings"; // Append the endpoint to the base URL
    if (serverUrl[strlen(serverUrl) - 1] != '/') {
        fullUrl = String(serverUrl) + "/saveDefaultSettings"; // Ensure the URL is correctly formatted
    }

    http.begin(fullUrl); // Use the full URL
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(payload);
    if (httpResponseCode > 0) {
        LogSerial("Default settings response: ", true);
        LogSerialn(String(httpResponseCode), true);
    } else {
        LogSerial("Failed to send default settings: ", true);
        LogSerialn(http.errorToString(httpResponseCode).c_str(), true);
    }

    http.end();
}

/**
 * @brief Closes the HTTP connection (minimal cleanup).
 */
void ServerClient::closeConnection() {
    HTTPClient http;
    http.end(); // Ensure connection cleanup
}

/**
 * @brief Returns the server URL.
 * @return The server URL.
 */
const char* ServerClient::getServerUrl() {
    return serverUrl;
}