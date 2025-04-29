#include "LogMgr.h"

/**
 * @brief Logs data to the serial monitor.
 * @param data The string to log.
 * @param debug A flag to indicate whether to log the data or not.
 */
void LogSerial(String data, bool debug) {
    if (debug) {
        Serial.print(data);
    }
}

/**
 * @brief Logs data to the serial monitor with a newline at the end.
 * @param data The string to log.
 * @param debug A flag to indicate whether to log the data or not.
 */
void LogSerialn(String data, bool debug) {
    if (debug) {
        Serial.println(data);
    }
}

/**
 * @brief Converts the reset reason enum to a string.
 * @param reason The reset reason enum value.
 * @return A string representation of the reset reason.
 */
const char* getResetReasonString(esp_reset_reason_t reason) {
    switch (reason) {
      case ESP_RST_UNKNOWN:    return "Reset reason cannot be determined";
      case ESP_RST_POWERON:    return "Reset due to power-on event";
      case ESP_RST_EXT:        return "Reset by external pin (not applicable for ESP32)";
      case ESP_RST_SW:         return "Software reset via esp_restart";
      case ESP_RST_PANIC:      return "Software reset due to exception/panic";
      case ESP_RST_INT_WDT:    return "Reset due to interrupt watchdog";
      case ESP_RST_TASK_WDT:   return "Reset due to task watchdog";
      case ESP_RST_WDT:        return "Reset due to other watchdogs";
      case ESP_RST_DEEPSLEEP:  return "Reset after exiting deep sleep mode";
      case ESP_RST_BROWNOUT:   return "Brownout reset (software or hardware)";
      case ESP_RST_SDIO:       return "Reset over SDIO";
      default:                 return "Unknown reset reason";
    }
  }