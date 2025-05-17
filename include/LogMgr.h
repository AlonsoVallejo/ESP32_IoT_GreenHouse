#ifndef LOG_MGR_H
#define LOG_MGR_H

#include <Arduino.h>
#include <esp_system.h> 

void LogSerial(String data, bool IsLog);
void LogSerialn(String data, bool IsLog);
const char* getResetReasonString(esp_reset_reason_t reason);

#endif // LOG_MGR_H