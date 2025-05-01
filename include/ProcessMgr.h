#ifndef PROCESS_MGR_H
#define PROCESS_MGR_H

#include "SystemData.h"

#define DFLT_MAX_LVL_PERCENTAGE (90) /* default value for max level */
#define DFLT_MIN_LVL_PERCENTAGE (20) /* default value for min level */

#define DFLT_SENSOR_HOT_TEMP_C   (30) /* default value for hot temperature */
#define DFLT_SENSOR_LOW_HUMIDITY (15) /* default value for low humidity */ 

void handleLampActivation(SystemData* data);
void handlePumpActivation(SystemData* data);
void handleIrrigatorControl(SystemData* data);

#endif // PROCESS_MGR_H