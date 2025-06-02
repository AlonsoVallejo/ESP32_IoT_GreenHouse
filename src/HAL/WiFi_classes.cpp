#include "WiFi_classes.h"
#include "LogMgr.h" 
#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>

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
    return ssid.c_str();
}

/**
 * @brief Get the password of the WiFi network.
 * @return The password of the WiFi network.
 */
const char* WiFiManager::getPassword() {
    return password.c_str();
}

/**
 * @brief Set the SSID of the WiFi network.
 * @param ssid The new SSID of the WiFi network.
 */
void WiFiManager::setSSID(const char* ssid) {
    this->ssid = ssid;
}

/**
 * @brief Set the password of the WiFi network.
 * @param password The new password of the WiFi network.
 */
void WiFiManager::setPassword(const char* password) {
    this->password = password;
}

/**
 * @brief Disconnects from the WiFi network.
 */
void WiFiManager::disconnectWiFi() {
    WiFi.disconnect();
    LogSerialn("WiFi disconnected", true);
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

/**
 * @brief Scans for available WiFi networks and returns a list of SSIDs.
 * @return A vector of SSIDs of available WiFi networks.
 */
std::vector<String> WiFiManager::scanNetworks() {
    std::vector<String> ssidList;
    int16_t total_networks = WiFi.scanNetworks();
    for (uint8_t curr = 0; curr < total_networks; ++curr) {
        ssidList.push_back(WiFi.SSID(curr));
    }
    return ssidList;
}

/**
 * @brief Connects to a specified WiFi network.
 * @param ssid The SSID of the WiFi network.
 * @param password The password of the WiFi network.
 * @return True if connection is successful, false otherwise.
 */
bool WiFiManager::connectToNetwork(const String& ssid, const String& password) {
    LogSerialn("Connecting to SSID: " + ssid, true);

    WiFi.disconnect(true); // Disconnect from any previous network
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());

    unsigned long startAttemptTime = millis();
    const unsigned long timeout = 5000;

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
        delay(200);
    }

    if (WiFi.status() == WL_CONNECTED) {
        LogSerialn("WiFi connected!", true);
        this->ssid = ssid;
        this->password = password;
        saveCredentials(ssid, password);
        return true;
    } else {
        LogSerialn("WiFi connection failed.", true);
        return false;
    }
}

/**
 * @brief Saves WiFi credentials to persistent storage.
 * @param ssid The SSID of the WiFi network.
 * @param password The password of the WiFi network.
 */
void WiFiManager::saveCredentials(const String& ssid, const String& password) {
    Preferences prefs;
    prefs.begin("wifi", false);
    prefs.putString("ssid", ssid);
    prefs.putString("password", password);
    prefs.end();
}

/**
 * @brief Loads WiFi credentials from persistent storage.
 * @param ssid Reference to store the loaded SSID.
 * @param password Reference to store the loaded password.
 * @return True if credentials are successfully loaded, false otherwise.
 */
bool WiFiManager::loadCredentials(String& ssid, String& password) {
    Preferences prefs;
    prefs.begin("wifi", true);
    ssid = prefs.getString("ssid", "");
    password = prefs.getString("password", "");
    prefs.end();
    return (ssid.length() > 0 && password.length() > 0);
}