#ifndef PROCESS_MGR_H
#define PROCESS_MGR_H

#include "SystemData.h"

void handleLampActivation(SystemData* data);
void handlePumpActivation(SystemData* data);
void handleIrrigatorControl(SystemData* data);

#endif // PROCESS_MGR_H