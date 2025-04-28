#ifndef WIFI_CLASSES_H
#define WIFI_CLASSES_H

#include <WiFi.h>
#include <IPAddress.h>

/**
 * @brief Class to manage WiFi functionalities.
 */
class WiFiManager {
private:
    const char* ssid;
    const char* password;

public:
    WiFiManager(const char* ssid, const char* password);
    const char* getSSID();
    void connectWiFi();
    bool IsWiFiConnected();
    IPAddress getWiFiLocalIp();
};

#endif // WIFI_CLASSES_H