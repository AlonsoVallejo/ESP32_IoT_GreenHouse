#ifndef LOG_MGR_H
#define LOG_MGR_H

#include <Arduino.h>

/**
 * @brief Logs data to the serial monitor.
 * @param data The string to log.
 * @param debug A flag to indicate whether to log the data or not.
 */
void LogSerial(String data, bool debug);

/**
 * @brief Logs data to the serial monitor with a newline at the end.
 * @param data The string to log.
 * @param debug A flag to indicate whether to log the data or not.
 */
void LogSerialn(String data, bool debug);

#endif // LOG_MGR_H