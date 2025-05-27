#ifndef WIFI_CLASSES_H
#define WIFI_CLASSES_H

#include <WiFi.h>
#include <IPAddress.h>
#include <vector>
#include <Preferences.h>

/**
 * @brief Class to manage WiFi functionalities.
 */
class WiFiManager {
private:
    String ssid;
    String password;

public:
    WiFiManager(const char* ssid, const char* password);
    const char* getSSID();
    const char* getPassword();
    void setSSID(const char* ssid);
    void setPassword(const char* password);
    void disconnectWiFi();
    bool IsWiFiConnected();
    IPAddress getWiFiLocalIp();
    std::vector<String> scanNetworks();
    bool connectToNetwork(const String& ssid, const String& password);
    void saveCredentials(const String& ssid, const String& password);
    bool loadCredentials(String& ssid, String& password);
};

#endif // WIFI_CLASSES_H