#ifndef DISPLAY_MGR_H
#define DISPLAY_MGR_H

#include "SystemData.h"

/**
 * @brief Displays light sensor, PIR presence, and lamp state.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 */
void displayLightAndPresence(SystemData* data);

/**
 * @brief Displays water level and pump state.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 */
void displayWaterLevelAndPump(SystemData* data);

/**
 * @brief Displays temperature, humidity, and irrigator state.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 */
void displayTemperatureAndHumidity(SystemData* data);

/**
 * @brief Displays WiFi status and connection information.
 * @param data Pointer to the SystemData structure containing sensor and actuator objects.
 */
void displayWiFiStatus(SystemData* data);

#endif // DISPLAY_MGR_H