#ifndef DISPLAY_MGR_H
#define DISPLAY_MGR_H

#include "SystemData.h"

void displayLightAndPresence(SystemData* data);
void displayWaterLevelAndPump(SystemData* data);
void displayTemperatureAndHumidity(SystemData* data);
void displayWiFiStatus(SystemData* data);
void displayLevelSettings(SystemData* data, uint8_t currentValue);
void displayTempHumSettings(SystemData* data, uint8_t currentValue);


#endif // DISPLAY_MGR_H