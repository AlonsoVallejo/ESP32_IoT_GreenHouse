#include "SrvClientMgr.h"
#include <HTTPClient.h>
#include "LogMgr.h"
#include "ProcessMgr.h"
#include <ArduinoJson.h>
#include <WiFi.h> // For ESP.getChipModel()

/**
 * @brief Fetch updated settings from the server and update the SystemData structure.
 * @param data Pointer to the SystemData structure to update.
 */
void fetchUpdatedSettings(SystemData* data) {
    uint64_t chipId = ESP.getEfuseMac();
    char chipIdStr[18];
    snprintf(chipIdStr, sizeof(chipIdStr), "%02X:%02X:%02X:%02X:%02X:%02X",
        (uint8_t)(chipId >> 40),
        (uint8_t)(chipId >> 32),
        (uint8_t)(chipId >> 24),
        (uint8_t)(chipId >> 16),
        (uint8_t)(chipId >> 8),
        (uint8_t)chipId);

    const char* serverUrl = data->SrvClient->getServerUrl();
    String fullUrl = String(serverUrl) + "getSettings?chipId=" + chipIdStr;

    HTTPClient http;
    http.begin(fullUrl);
    http.setTimeout(5000);

    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
        String response = http.getString();
        LogSerial("Fetched updated settings: ", false);
        LogSerialn(response, false);

        /* Parse JSON response */
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, response);
        if (!error) {
            if (doc["maxLevel"].is<int>()) {
                data->maxLevelPercentage = doc["maxLevel"];
            }
            if (doc["minLevel"].is<int>()) {
                data->minLevelPercentage = doc["minLevel"];
            }
            if (doc["hotTemperature"].is<int>()) {
                data->hotTemperature = doc["hotTemperature"];
            }
            if (doc["lowHumidity"].is<int>()) {
                data->lowHumidity = doc["lowHumidity"];
            }
        } else {
            LogSerial("Failed to parse settings JSON: ", true);
            LogSerialn(error.c_str(), true);
        }
    } else {
        LogSerial("Failed to fetch updated settings: ", true);
        LogSerialn(http.errorToString(httpResponseCode).c_str(), true);
    }

    http.end();
}

/**
 * @brief Checks if the 'settings' JSON package exists in the database for this device.
 * @param data Pointer to the SystemData structure.
 * @return True if the json settings packet exists, false otherwise.
 */
bool checkJsonSettingsExistence(SystemData* data) {
    uint64_t chipId = ESP.getEfuseMac();
    char chipIdStr[18];
    snprintf(chipIdStr, sizeof(chipIdStr), "%02X:%02X:%02X:%02X:%02X:%02X",
        (uint8_t)(chipId >> 40),
        (uint8_t)(chipId >> 32),
        (uint8_t)(chipId >> 24),
        (uint8_t)(chipId >> 16),
        (uint8_t)(chipId >> 8),
        (uint8_t)chipId);

    const char* serverUrl = data->SrvClient->getServerUrl();
    String fullUrl = String(serverUrl) + "getSettings?chipId=" + chipIdStr;

    HTTPClient http;
    http.begin(fullUrl);
    http.setTimeout(5000);

    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
        String response = http.getString();
        LogSerial("Settings existence check response: ", true);
        LogSerialn(response, true);

        /* Parse JSON response to check for error */
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, response);
        if (!error) {
            if (!doc["error"].isNull()) {
                return false;
            }
        }
        /* If response is not empty and not "null", settings exist */
        if (!response.isEmpty() && response != "null") {
            return true;
        }
    } else {
        LogSerial("Failed to check settings existence: ", true);
        LogSerialn(http.errorToString(httpResponseCode).c_str(), true);
    }

    http.end();
    return false;
}

/**
 * @brief Packs and sends sensor and actuator data in a single JSON under the ESP chip id.
 * @param data Pointer to the SystemData structure.
 */
void sendSensActHistory(SystemData* data) {
    uint64_t chipId = ESP.getEfuseMac();
    char chipIdStr[18];
    snprintf(chipIdStr, sizeof(chipIdStr), "%02X:%02X:%02X:%02X:%02X:%02X",
        (uint8_t)(chipId >> 40),
        (uint8_t)(chipId >> 32),
        (uint8_t)(chipId >> 24),
        (uint8_t)(chipId >> 16),
        (uint8_t)(chipId >> 8),
        (uint8_t)chipId);

    JsonDocument doc; /* Use static allocation as you requested */

    /* Create root object with chipId as key */
    JsonObject root = doc[chipIdStr].to<JsonObject>();

    /* Pack sensor data */
    JsonObject sensorData = root["sensorData"].to<JsonObject>();
    sensorData["lvl"] = data->levelPercentage;
    sensorData["tmp"] = data->sensorMgr->getTemperature();
    sensorData["hum"] = data->sensorMgr->getHumidity();
    sensorData["ldr"] = data->sensorMgr->getLightSensorValue() ? "1" : "0";
    sensorData["pir"] = data->PirPresenceDetected ? "1" : "0";

    /* Pack actuator data */
    JsonObject actuatorData = root["actuatorData"].to<JsonObject>();
    actuatorData["lmp"] = data->actuatorMgr->getLamp()->getOutstate() ? "1" : "0";
    actuatorData["pmp"] = data->actuatorMgr->getPump()->getOutstate() ? "1" : "0";
    actuatorData["flt"] = data->actuatorMgr->getLedIndicator()->getOutstate() ? "1" : "0";
    actuatorData["irr"] = data->actuatorMgr->getIrrigator()->getOutstate() ? "1" : "0";

    String payload;
    serializeJson(doc, payload);

    data->SrvClient->sendSensActHistoryPayload(payload);
}

/**
 * @brief Sends manual systems settings to the server.
 * @param data Pointer to the SystemData structure.
 * @param serverUrl URL of the backend server.
 */
void sendSystemSettings(SystemData* data) {
    uint64_t chipId = ESP.getEfuseMac();
    char chipIdStr[18];
    snprintf(chipIdStr, sizeof(chipIdStr), "%02X:%02X:%02X:%02X:%02X:%02X",
        (uint8_t)(chipId >> 40),
        (uint8_t)(chipId >> 32),
        (uint8_t)(chipId >> 24),
        (uint8_t)(chipId >> 16),
        (uint8_t)(chipId >> 8),
        (uint8_t)chipId);

    JsonDocument doc;

    /** Create root object with chipId as key */
    JsonObject root = doc[chipIdStr].to<JsonObject>();

    /** Pack settings */
    JsonObject settings = root["settings"].to<JsonObject>();
    settings["maxLevel"] = data->maxLevelPercentage;
    settings["minLevel"] = data->minLevelPercentage;
    settings["hotTemperature"] = data->hotTemperature;
    settings["lowHumidity"] = data->lowHumidity;

    String payload;
    serializeJson(doc, payload);

    data->SrvClient->sendSysSettingsPayload(payload);
}

