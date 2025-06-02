#include "client_classes.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include "LogMgr.h"
#include <HTTPClient.h>

/**
 * @brief Constructor for ServerClient class.
 * @param serverUrl URL of the backend server.
 * @param wifiManager Pointer to the WiFiManager instance.
 */
ServerClient::ServerClient(const char* serverUrl, WiFiManager* wifiManager)
    : serverUrl(serverUrl), wifiManager(wifiManager), payload("{}") {}

/**
 * @brief Sends a JSON payload with settings to the server at /updateSettings.
 * @param settingsPayload The JSON string to send.
 */
void ServerClient::sendSysSettingsPayload(const String& settingsPayload) {
    HTTPClient http;
    String fullUrl = String(serverUrl) + "updateSettings";
    if (serverUrl[strlen(serverUrl) - 1] != '/') {
        fullUrl = String(serverUrl) + "/updateSettings";
    }

    LogSerial("Sending settings payload: ", false);
    LogSerialn(settingsPayload, false);

    http.begin(fullUrl);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(settingsPayload);

    if (httpResponseCode > 0) {
        LogSerial("Settings POST successful, response code: ", true);
        LogSerialn(String(httpResponseCode), true);
    } else {
        LogSerial("Settings POST failed, error: ", true);
        LogSerialn(http.errorToString(httpResponseCode).c_str(), true);
    }
    http.end();
}

/**
 * @brief Sends a JSON payload with sensor and actuator history to the server at /updateSensActHistory.
 * @param sensActPayload The JSON string to send.
 */
void ServerClient::sendSensActHistoryPayload(const String& sensActPayload) {
    HTTPClient http;
    String fullUrl = String(serverUrl) + "updateSensActHistory";
    if (serverUrl[strlen(serverUrl) - 1] != '/') {
        fullUrl = String(serverUrl) + "/updateSensActHistory";
    }

    LogSerial("Sending sensor/actuator history payload: ", false);
    LogSerialn(sensActPayload, false);

    http.begin(fullUrl);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(sensActPayload);

    if (httpResponseCode > 0) {
        LogSerial("SensActHistory POST successful, response code: ", true);
        LogSerialn(String(httpResponseCode), true);
    } else {
        LogSerial("SensActHistory POST failed, error: ", true);
        LogSerialn(http.errorToString(httpResponseCode).c_str(), true);
    }
    http.end();
}

/**
 * @brief Closes the HTTP connection (minimal cleanup).
 */
void ServerClient::closeConnection() {
    HTTPClient http;
    http.end(); /* Ensure connection cleanup */
}

/**
 * @brief Returns the server URL.
 * @return The server URL.
 */
const char* ServerClient::getServerUrl() {
    return serverUrl;
}