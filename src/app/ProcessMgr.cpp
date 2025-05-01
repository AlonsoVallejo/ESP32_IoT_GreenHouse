#include "ProcessMgr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <Arduino.h>

#define SENSOR_PIR_COOL_DOWN_TIME (5000) 

/**
 * @brief Handles the activation and deactivation of the lamp based on PIR sensor and light sensor states.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 */
void handleLampActivation(SystemData* data) {
    static unsigned long lastPirTriggerTime = 0;
    static bool presenceDetected = false;
    static bool pirWentLow = false;

    unsigned long currentMillis = millis();

    if (xSemaphoreTake(xSystemDataMutex, portMAX_DELAY)) {
        bool lightState = data->lightSensor->getSensorValue();
        bool pirState = data->pirSensor->getSensorValue();
        bool lampState = data->lamp->getOutstate();

        if (pirState) {
            /* If PIR detects presence, activate Lamp immediately */
            presenceDetected = true;
            pirWentLow = false; /** Reset cooldown tracking */
            data->lamp->SetOutState(true);
            lastPirTriggerTime = currentMillis; /** Reset PIR cooldown timer */
        } else if (lightState && !lampState) {
            /* If it's dark AND Lamp is OFF, activate Lamp */
            data->lamp->SetOutState(true);
        } else if (!lightState && !presenceDetected) { 
            /* If light sensor detects LIGHT and PIR is NOT detecting presence, turn Lamp OFF immediately */
            presenceDetected = false;
            pirWentLow = false;
            data->lamp->SetOutState(false);
        } else if (!pirState && presenceDetected && !pirWentLow) {
            /* If PIR stopped detecting presence, start cooldown */
            pirWentLow = true;
            lastPirTriggerTime = currentMillis;
        } else if (pirWentLow && (currentMillis - lastPirTriggerTime >= SENSOR_PIR_COOL_DOWN_TIME)) { 
            /* If PIR has been LOW for cooldown time, only turn Lamp OFF if light sensor does NOT require it to stay ON */
            presenceDetected = false;
            pirWentLow = false;
        /* Only turn Lamp OFF if light sensor reports brightness */
        if (!lightState) {
            data->lamp->SetOutState(false);
        }
        } else {
            /* Do nothing */
        }

        data->PirPresenceDetected = presenceDetected;
        xSemaphoreGive(xSystemDataMutex);
    }
}

/**
 * @brief Handles the activation and deactivation of the pump based on water level sensor readings.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 */
void handlePumpActivation(SystemData* data) {
    static bool pumpState = false;

    if (xSemaphoreTake(xSystemDataMutex, portMAX_DELAY)) {
        uint16_t levelValue = data->levelSensor->getSensorValue();
        uint16_t levelPercentage = ((levelValue - (SENSOR_LVL_STG_V + SENSOR_LVL_THRESHOLD_V)) * 100) /
                                    ((SENSOR_LVL_OPENCKT_V - SENSOR_LVL_THRESHOLD_V) - (SENSOR_LVL_STG_V + SENSOR_LVL_THRESHOLD_V));

        if (levelValue >= SENSOR_LVL_OPENCKT_V || levelValue <= SENSOR_LVL_STG_V) {
            data->ledInd->SetOutState(LED_FAIL_INDICATE);
            pumpState = false;
        } else {
            data->ledInd->SetOutState(LED_NO_FAIL_INDICATE);
            if (levelPercentage <= data->minLevelPercentage) {
                pumpState = true;
            } else if (levelPercentage >= data->maxLevelPercentage) {
                pumpState = false;
            }
        }

        if (levelPercentage >= 100) {
            levelPercentage = 100;
        }

        data->levelPercentage = levelPercentage;
        data->pump->SetOutState(pumpState);
        xSemaphoreGive(xSystemDataMutex);
    }
}

/**
 * @brief Handles the activation and deactivation of the irrigator based on temperature and humidity sensor readings.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 */
void handleIrrigatorControl(SystemData* data) {
    static bool irrigatorState = false;

    if (xSemaphoreTake(xSystemDataMutex, portMAX_DELAY)) {
        double temperature = data->tempHumSensor->getTemperature();
        double humidity = data->tempHumSensor->getHumidity();

        if (temperature >= 0 && temperature <= 100 && humidity >= 0 && humidity <= 100) {
            if (temperature >= data->hotTemperature && humidity <= data->lowHumidity) {
                if (!irrigatorState) {
                    data->irrigator->SetOutState(true);
                    irrigatorState = true;
                }
            } else if (temperature < data->hotTemperature - 2 || humidity > data->lowHumidity + 5) {
                if (irrigatorState) {
                    data->irrigator->SetOutState(false);
                    irrigatorState = false;
                }
            }
        }

        xSemaphoreGive(xSystemDataMutex);
    }
}