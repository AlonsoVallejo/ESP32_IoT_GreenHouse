#ifndef CLIENT_CLASSES_H
#define CLIENT_CLASSES_H

#include <WiFi.h>
#include <HTTPClient.h>

class ServerClient {
public:
    ServerClient(const char* serverUrl, const char* ssid, const char* password);
    void connectWiFi();
    void prepareData(const String& key, const String& value);
    void sendPayload();
    void closeConnection();
    
private:
    const char* serverUrl;
    const char* ssid;
    const char* password;
    String payload;
};

#endif // CLIENT_CLASSES_H
