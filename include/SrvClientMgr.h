#ifndef SRV_CLIENT_MGR_H
#define SRV_CLIENT_MGR_H

#include "SystemData.h"

void fetchUpdatedSettings(SystemData* data, const char* serverUrl);
bool checkSettingsExistence(SystemData* data, const char* serverUrl);
void sendDefaultSettings(SystemData* data, const char* serverUrl);
void sendManualSettings(SystemData* data, const char* serverUrl);

#endif // SRV_CLIENT_MGR_H