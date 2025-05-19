#ifndef CLIENT_CLASSES_H
#define CLIENT_CLASSES_H

#include <HTTPClient.h>
#include "WiFi_classes.h"

/**
 * @brief Class to manage client-server communication.
 */
class ServerClient {
private:
    const char* serverUrl;
    String payload;
    WiFiManager* wifiManager;

public:
    ServerClient(const char* serverUrl, WiFiManager* wifiManager);
    void closeConnection();
    const char* getServerUrl();
    void sendSysSettingsPayload(const String& settingsPayload);
    void sendSensActHistoryPayload(const String& sensActPayload);
};

#endif // CLIENT_CLASSES_H
