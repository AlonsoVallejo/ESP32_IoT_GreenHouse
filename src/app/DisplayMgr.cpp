#include "DisplayMgr.h"
#include <Arduino.h>

void displayUserGuide(void) {
    
}

/**
 * @brief Displays footer with labels for next screen and settings.
 * @param oledDisplay Pointer to the OledDisplay object.
 * @param ltbottom Label for the left bottom corner.
 * @param midbottom Label for the middle bottom corner.
 * @param rtbottom Label for the right bottom corner.
 */
void displayFooter(OledDisplay* oledDisplay, const char* ltbottom, const char* midbottom, const char* rtbottom) {
    const uint16_t displayWidth = SCREEN_WIDTH;
    const uint16_t y = 50;

    uint16_t ltWidth = oledDisplay->getStringWidth(ltbottom);
    uint16_t midWidth = oledDisplay->getStringWidth(midbottom);
    uint16_t rtWidth = oledDisplay->getStringWidth(rtbottom);

    uint16_t ltX = 0;
    uint16_t midX = (displayWidth - midWidth) / 2;
    uint16_t rtX = displayWidth - rtWidth;

    oledDisplay->SetdisplayData(ltX, y, ltbottom);
    oledDisplay->SetdisplayData(midX, y, midbottom);
    oledDisplay->SetdisplayData(rtX, y, rtbottom);
}

/**
 * @brief Displays light sensor, PIR presence, and lamp state.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 */
void displayLightAndPresence(SystemData* data) {
    data->oledDisplay->SetdisplayData(0, 0, "Light Sensor: ");
    data->oledDisplay->SetdisplayData(80, 0, data->sensorMgr->getLightSensorValue() ? "Dark" : "Light");

    data->oledDisplay->SetdisplayData(0, 10, "Presence: ");
    data->oledDisplay->SetdisplayData(80, 10, data->PirPresenceDetected ? "YES" : "NO");

    data->oledDisplay->SetdisplayData(0, 20, "Lamp: ");
    data->oledDisplay->SetdisplayData(80, 20, data->actuatorMgr->getLamp()->getOutstate() ? "ON" : "OFF");

    displayFooter(data->oledDisplay, "Next", " ", "");
}

/**
 * @brief Displays water level and pump state.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 */
void displayWaterLevelAndPump(SystemData* data) {
    uint16_t levelValue = data->sensorMgr->getLevelSensorValue();

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
    data->oledDisplay->SetdisplayData(80, 10, data->actuatorMgr->getPump()->getOutstate() ? "ON" : "OFF");

    data->oledDisplay->SetdisplayData(0, 20, " ");
    data->oledDisplay->SetdisplayData(80, 20, " ");

    displayFooter(data->oledDisplay, "Next", " " , "Settings");
}

/**
 * @brief Displays temperature, humidity, and irrigator state.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 */
void displayTemperatureAndHumidity(SystemData* data) {
    data->oledDisplay->SetdisplayData(0, 0, "Temperature: ");
    data->oledDisplay->SetdisplayData(80, 0, data->sensorMgr->getTemperature());
    data->oledDisplay->SetdisplayData(105, 0, "C");

    data->oledDisplay->SetdisplayData(0, 10, "Humidity: ");
    data->oledDisplay->SetdisplayData(80, 10, data->sensorMgr->getHumidity());
    data->oledDisplay->SetdisplayData(105, 10, "%");

    data->oledDisplay->SetdisplayData(0, 20, "Irrigator: ");
    data->oledDisplay->SetdisplayData(80, 20, data->actuatorMgr->getIrrigator()->getOutstate() ? "ON" : "OFF");

    displayFooter(data->oledDisplay, "Next", " ", "Settings");
}

/**
 * @brief Displays WiFi status and connection information.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 */
void displayWiFiStatus(SystemData* data) {
    data->oledDisplay->SetdisplayData(0, 0, "WiFi: ");
    data->oledDisplay->SetdisplayData(0, 10, data->wifiManager->getSSID());
    data->oledDisplay->SetdisplayData(0, 20, "Status: ");
    data->oledDisplay->SetdisplayData(45, 20, data->wifiManager->IsWiFiConnected() ? "Connected" : "Disconnected");

    displayFooter(data->oledDisplay, "Next", " ", "Settings");
}

/**
 * @brief Displays the current selector state.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 * @param currentSettingMenu The current setting being displayed.
 * @param currentValue The current value of the setting.
 */
void displayLevelSettings(SystemData* data, uint8_t currentValue) {
    const char* settings[] = {
        "Max Level (%)",
        "Min Level (%)",
    };

    data->oledDisplay->SetdisplayData(0, 0, "System settings: ");
    data->oledDisplay->SetdisplayData(0, 10, settings[data->currentSettingMenu]);
    data->oledDisplay->SetdisplayData(0, 20, "Value:");
    data->oledDisplay->SetdisplayData(50, 20, currentValue);

    displayFooter(data->oledDisplay, "Param", "^ v", "save");
}

/**
 * @brief Displays the current selector state.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 * @param currentSettingMenu The current setting being displayed.
 * @param currentValue The current value of the setting.
 */
void displayTempHumSettings(SystemData* data, uint8_t currentValue) {
    const char* settings[] = {
        "Hot Temp (C)",
        "Low Humidity (%)",
    };
    
    data->oledDisplay->SetdisplayData(0, 0, "System settings: ");
    data->oledDisplay->SetdisplayData(0, 10, settings[data->currentSettingMenu]);
    data->oledDisplay->SetdisplayData(0, 20, "Value:");
    data->oledDisplay->SetdisplayData(50, 20, currentValue);

    displayFooter(data->oledDisplay, "Param", "^ v", "save");
}