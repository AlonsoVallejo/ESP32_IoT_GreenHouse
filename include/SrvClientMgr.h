#ifndef SRV_CLIENT_MGR_H
#define SRV_CLIENT_MGR_H

#include "SystemData.h"

void fetchUpdatedSettings(SystemData* data);
bool checkJsonSettingsExistence(SystemData* data);
void sendSensActHistory(SystemData* data);
void sendSystemSettings(SystemData* data);

#endif // SRV_CLIENT_MGR_H