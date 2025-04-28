#include "DisplayMgr.h"
#include <Arduino.h>

/**
 * @brief Displays light sensor, PIR presence, and lamp state.
 */
void displayLightAndPresence(SystemData* data) {
    data->oledDisplay->SetdisplayData(0, 0, "Light Sensor: ");
    data->oledDisplay->SetdisplayData(80, 0, data->lightSensor->getSensorValue() ? "Dark" : "Light");

    data->oledDisplay->SetdisplayData(0, 10, "Presence: ");
    data->oledDisplay->SetdisplayData(80, 10, data->PirPresenceDetected ? "YES" : "NO");

    data->oledDisplay->SetdisplayData(0, 20, "Lamp: ");
    data->oledDisplay->SetdisplayData(80, 20, data->lamp->getOutstate() ? "ON" : "OFF");
}

/**
 * @brief Displays water level and pump state.
 */
void displayWaterLevelAndPump(SystemData* data) {
    uint16_t levelValue = data->levelSensor->getSensorValue();

    data->oledDisplay->SetdisplayData(0, 0, "Water Level: ");
    if (levelValue >= SENSOR_LVL_OPENCKT_V) {
        data->oledDisplay->SetdisplayData(80, 0, "OPEN");
    } else if (levelValue <= SENSOR_LVL_STG_V + SENSOR_LVL_THRESHOLD_V) {
        data->oledDisplay->SetdisplayData(80, 0, "SHORT");
    } else if (levelValue >= SENSOR_LVL_OPENCKT_V - SENSOR_LVL_THRESHOLD_V) {
        data->oledDisplay->SetdisplayData(80, 0, "100%");
    } else {
        data->oledDisplay->SetdisplayData(80, 0, data->levelPercentage);
        data->oledDisplay->SetdisplayData(105, 0, "%");
    }

    data->oledDisplay->SetdisplayData(0, 10, "Pump: ");
    data->oledDisplay->SetdisplayData(80, 10, data->pump->getOutstate() ? "ON" : "OFF");

    data->oledDisplay->SetdisplayData(0, 20, " ");
    data->oledDisplay->SetdisplayData(80, 20, " ");
}

/**
 * @brief Displays temperature, humidity, and irrigator state.
 */
void displayTemperatureAndHumidity(SystemData* data) {
    data->oledDisplay->SetdisplayData(0, 0, "Temperature: ");
    data->oledDisplay->SetdisplayData(80, 0, data->tempHumSensor->getTemperature());
    data->oledDisplay->SetdisplayData(105, 0, "C");

    data->oledDisplay->SetdisplayData(0, 10, "Humidity: ");
    data->oledDisplay->SetdisplayData(80, 10, data->tempHumSensor->getHumidity());
    data->oledDisplay->SetdisplayData(105, 10, "%");

    data->oledDisplay->SetdisplayData(0, 20, "Irrigator: ");
    data->oledDisplay->SetdisplayData(80, 20, data->irrigator->getOutstate() ? "ON" : "OFF");
}

/**
 * @brief Displays WiFi status and connection information.
 */
void displayWiFiStatus(SystemData* data) {
    data->oledDisplay->SetdisplayData(0, 0, "WiFi: ");
    data->oledDisplay->SetdisplayData(0, 10, data->wifiManager->getSSID());
    data->oledDisplay->SetdisplayData(0, 20, "Status: ");
    data->oledDisplay->SetdisplayData(45, 20, data->wifiManager->IsWiFiConnected() ? "Connected" : "Disconnected");
}