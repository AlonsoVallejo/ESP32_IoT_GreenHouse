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
    void prepareData(const String& key, const String& value);
    void sendPayload();
    void sendDefaultSettings(uint8_t maxLevel, uint8_t minLevel, uint8_t hotTemp, uint8_t lowHumidity);
    void closeConnection();
    void resetServer();
};

#endif // CLIENT_CLASSES_H
