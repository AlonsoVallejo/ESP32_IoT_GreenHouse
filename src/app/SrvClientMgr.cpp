#include "SrvClientMgr.h"
#include <HTTPClient.h>
#include "LogMgr.h"
#include "ProcessMgr.h"

/**
 * @brief Extracts a value from a JSON string by key.
 * @param json The JSON string.
 * @param key The key to search for.
 * @return The value as a string, or an empty string if the key is not found.
 */
String extractJsonValue(const String& json, const String& key) {
    String searchKey = "\"" + key + "\":";
    int startIndex = json.indexOf(searchKey);
    if (startIndex == -1) {
        return ""; // Key not found
    }

    startIndex += searchKey.length();
    int endIndex = json.indexOf(",", startIndex);
    if (endIndex == -1) {
        endIndex = json.indexOf("}", startIndex); // Handle the last key-value pair
    }

    String value = json.substring(startIndex, endIndex);
    value.trim(); // Remove any extra spaces or newlines
    value.replace("\"", ""); // Remove quotes if the value is a string
    return value;
}

/**
 * @brief Fetch updated settings from the server and update the SystemData structure.
 * @param data Pointer to the SystemData structure to update.
 * @param serverUrl URL of the backend server.
 */
void fetchUpdatedSettings(SystemData* data, const char* serverUrl) {
    HTTPClient http;
    String fullUrl = String(serverUrl) + "getSettings"; /* Append the endpoint to the base URL */

    http.begin(fullUrl); /* Use the full URL */
    http.setTimeout(5000);

    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
        String response = http.getString();
        LogSerial("Fetched updated settings: ", false);
        LogSerialn(response, false);

        /* Extract and update dynamic settings in SystemData */
        String maxLevelStr = extractJsonValue(response, "maxLevel");
        String minLevelStr = extractJsonValue(response, "minLevel");
        String hotTempStr = extractJsonValue(response, "hotTemperature");
        String lowHumidityStr = extractJsonValue(response, "lowHumidity");

        if (!maxLevelStr.isEmpty()) {
            data->maxLevelPercentage = maxLevelStr.toInt();
        }
        if (!minLevelStr.isEmpty()) {
            data->minLevelPercentage = minLevelStr.toInt();
        }
        if (!hotTempStr.isEmpty()) {
            data->hotTemperature = hotTempStr.toInt();
        }
        if (!lowHumidityStr.isEmpty()) {
            data->lowHumidity = lowHumidityStr.toInt();
        }
    } else {
        LogSerial("Failed to fetch updated settings: ", true);
        LogSerialn(http.errorToString(httpResponseCode).c_str(), true);
    }

    http.end();
}

/**
 * @brief Checks if the 'settings' JSON package exists in the database.
 * @param data Pointer to the SystemData structure.
 * @param serverUrl URL of the backend server.
 * @return True if the settings exist, false otherwise.
 */
bool checkSettingsExistence(SystemData* data, const char* serverUrl) {
    HTTPClient http;
    String fullUrl = String(serverUrl) + "getSettings"; /* Append the endpoint to the base URL */

    http.begin(fullUrl); /* Use the full URL */
    http.setTimeout(5000);

    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
        String response = http.getString();
        LogSerial("Settings existence check response: ", true);
        LogSerialn(response, true);

        /* Check if the response contains an error or is empty */
        if (response.indexOf("\"error\"") != -1 || response.isEmpty() || response == "null") {
            return false; /* Settings do not exist */
        }

        return true; /* Settings exist */
    } else {
        LogSerial("Failed to check settings existence: ", true);
        LogSerialn(http.errorToString(httpResponseCode).c_str(), true);
    }

    http.end();
    return false; /* Assume settings do not exist if the request fails */
}

/**
 * @brief Sends default settings to the server.
 * @param data Pointer to the SystemData structure.
 * @param serverUrl URL of the backend server.
 */
void sendDefaultSettings(SystemData* data, const char* serverUrl) {
    HTTPClient http;
    String fullUrl = String(serverUrl) + "saveDefaultSettings"; /* Append the endpoint to the base URL */

    String payload = "{";
    payload += "\"defaultSettings\": {";
    payload += "\"maxLevel\": " + String(DFLT_MAX_LVL_PERCENTAGE) + ",";
    payload += "\"minLevel\": " + String(DFLT_MIN_LVL_PERCENTAGE) + ",";
    payload += "\"hotTemperature\": " + String(DFLT_SENSOR_HOT_TEMP_C) + ",";
    payload += "\"lowHumidity\": " + String(DFLT_SENSOR_LOW_HUMIDITY);
    payload += "}}";

    http.begin(fullUrl); /* Use the full URL */
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