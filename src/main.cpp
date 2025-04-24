#include <Arduino.h>
#include "Sensors_classes.h"
#include "Actuators_classes.h"
#include "OledDisplay_classes.h"
#include "client_classes.h"
#include "ESP32_shield.h"
#include <esp_system.h> 

using namespace std;

#define SENSOR_LVL_PIN        (SHIELD_POTENTIOMETER_VP)
#define SENSOR_HUM_TEMP_PIN   (SHIELD_DAC1_D25)
#define SENSOR_LDR_PIN        (SHIELD_PUSHB3_D34) 
#define SENSOR_PIR_PIN        (SHIELD_DHT11_D13)
#define SENSOR_PBSELECTOR_PIN (SHIELD_PUSHB2_D35)
#define ACTUATOR_LED_PWM_PIN  (SHIELD_LED4_D14)
#define ACTUATOR_RELAY1_PIN   (SHIELD_RELAY1_D4)
#define ACTUATOR_RELAY2_PIN   (SHIELD_RELAY2_D2)

#define SUBTASK_INTERVAL_100_MS  (100)
#define SUBTASK_INTERVAL_500_MS  (500)
#define SUBTASK_INTERVAL_1000_MS (1000)     
#define SUBTASK_INTERVAL_3_S     (3000)  
#define SUBTASK_INTERVAL_30_S    (30000)  

#define SENSOR_LVL_OPENCKT_V (3975) // ADC value for open circuit
#define SENSOR_LVL_STG_V     (124)  // ADC value for short circuit
#define SENSOR_LVL_THRESHOLD_V (50) // ADC Threshold for level sensor

#define MAX_LVL_PERCENTAGE (90) 
#define MIN_LVL_PERCENTAGE (20) 

#define SENSOR_MAX_TEMP_C (50)

#define SENSOR_LVL_FAIL_OPEN  (0xFFFF)
#define SENSOR_LVL_FAIL_SHORT (0x0000)

#define LED_NO_FAIL_INDICATE     (0x00) 
#define LED_FAIL_INDICATE        (0x01) 

#define SENSOR_PIR_COOL_DOWN_TIME (5000) 

#define TASK_CORE_0 (0)
#define TASK_CORE_1 (1)

const char* serverUrl = "http://192.168.100.9:3000/updateData"; /* IP of localhost */
const char* ssid = "MEGACABLE-2.4G-FAA4"; /* ESP32 WROOM32 works with 2.4GHz signals */
const char* password = "3kK4H6W48P";

enum pb1Selector{
  PB1_SELECT_DATA1, /* Display light, Pir and relay1 data */
  PB1_SELECT_DATA2, /* Display level and relay2 data */
  PB1_SELECT_DATA3, /* Display temperature and humidity data */
  PB1_SELECT_DATA4, /* Display wifi status */
};

/* Struct to store all sensor, actuator, and display-related data */
/* Do not forget to update the setup() init */
struct SystemData {
   /* Object Sensors */
  AnalogSensor* levelSensor;
  TemperatureHumiditySensor* tempHumSensor;
  DigitalSensor* pirSensor;
  DigitalSensor* lightSensor;
  DigitalSensor* buttonSelector;
  /* Object Actuators */
  Actuator* ledInd;
  Actuator* relay1;
  Actuator* relay2;
  /* Object display */
  OledDisplay* oledDisplay;
  /* Object client */
  ServerClient* client;
  /* Variables */
  bool PirPresenceDetected;
  pb1Selector currentSelector;
  uint16_t levelPercentage;
};

SemaphoreHandle_t xSystemDataMutex;

void TaskReadSensors(void* pvParameters) {
    SystemData* data = (SystemData*)pvParameters;
    unsigned long lastTempHumReadTime = 0;
    unsigned long lastButtonPressTime = 0;

    for (;;) {
        unsigned long currentMillis = millis();

        /* Update sensor values */
        data->levelSensor->readRawValue();
        data->lightSensor->readRawValue();
        data->pirSensor->readRawValue();
        data->buttonSelector->readRawValue();

        /* Protect shared variable access */
        if (xSemaphoreTake(xSystemDataMutex, portMAX_DELAY)) {
            bool buttonState = !(data->buttonSelector->getSensorValue());
            if (buttonState && (currentMillis - lastButtonPressTime > 300)) {
                lastButtonPressTime = currentMillis;
                data->currentSelector = static_cast<pb1Selector>((data->currentSelector + 1) % sizeof(pb1Selector));
            }
            xSemaphoreGive(xSystemDataMutex);
        }

        /* Read temperature and humidity */
        if (currentMillis - lastTempHumReadTime >= SUBTASK_INTERVAL_3_S) {
            lastTempHumReadTime = currentMillis;
            data->tempHumSensor->readValueTemperature();
            data->tempHumSensor->readValueHumidity();
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void TaskProcessData(void* pvParameters) {
    SystemData* data = (SystemData*)pvParameters;
    unsigned long lastPirTriggerTime = 0;
    bool presenceDetected = false;
    bool pirWentLow = false;

    for (;;) {
      unsigned long currentMillis = millis();

      /* Handle relay1 activation logic */
      bool lightState = data->lightSensor->getSensorValue();
      bool pirState = data->pirSensor->getSensorValue();
      bool relayState = data->relay1->getOutstate();

      /* If PIR detects presence, activate relay immediately */
      if (pirState) {
        presenceDetected = true;
        pirWentLow = false; /** Reset cooldown tracking */
        data->relay1->SetOutState(true);
        lastPirTriggerTime = currentMillis; /** Reset PIR cooldown timer */
      } else if (lightState && !relayState) {
         /* If it's dark AND relay is OFF, activate relay */
        data->relay1->SetOutState(true);
      } else if (!lightState && !presenceDetected) { 
        /* If light sensor detects LIGHT and PIR is NOT detecting presence, turn relay OFF immediately */
        presenceDetected = false;
        pirWentLow = false;
        data->relay1->SetOutState(false);
      } else if (!pirState && presenceDetected && !pirWentLow) {
        /* If PIR stopped detecting presence, start cooldown */
        pirWentLow = true;
        lastPirTriggerTime = currentMillis;
      } else if (pirWentLow && (currentMillis - lastPirTriggerTime >= SENSOR_PIR_COOL_DOWN_TIME)) { 
         /* If PIR has been LOW for cooldown time, only turn relay OFF if light sensor does NOT require it to stay ON */
        presenceDetected = false;
        pirWentLow = false;
        /* Only turn relay OFF if light sensor reports brightness */
        if (!lightState) {
          data->relay1->SetOutState(false);
        }
      } else {

      }

      /* Handle relay2 activation logic */
      uint16_t levelValue = data->levelSensor->getSensorValue();
      uint16_t levelPercentage = ((levelValue - (SENSOR_LVL_STG_V + SENSOR_LVL_THRESHOLD_V)) * 100) / 
                                ((SENSOR_LVL_OPENCKT_V - SENSOR_LVL_THRESHOLD_V) - (SENSOR_LVL_STG_V + SENSOR_LVL_THRESHOLD_V));
      static bool relay2State = false;
      
      /** Check if the level sensor value is in an invalid range */
      if (levelValue >= SENSOR_LVL_OPENCKT_V || levelValue <= SENSOR_LVL_STG_V) {
        /** Sensor failure detected—turn relay OFF to prevent misactivation */
        data->ledInd->SetOutState(LED_FAIL_INDICATE);
        relay2State = false;
      } else {
        data->ledInd->SetOutState(LED_NO_FAIL_INDICATE);
        if (levelPercentage <= MIN_LVL_PERCENTAGE) {
          /* Activate relay2 when level percentage is low */
          relay2State = true;
        } else if (levelPercentage >= MAX_LVL_PERCENTAGE) {
          /* Deactivate relay2 when level percentage has reach the max*/
          relay2State = false;
        } else {
          /* Do nothing */
        }
      }
      if(levelPercentage >= 100) {
        levelPercentage = 100; // Ensure level percentage does not exceed 100%
      } 

      /** Protect shared variable access */
      if (xSemaphoreTake(xSystemDataMutex, portMAX_DELAY)) {
          data->PirPresenceDetected = presenceDetected;
          data->levelPercentage = levelPercentage;
          xSemaphoreGive(xSystemDataMutex);
      }

      /** Apply relay2 state */
      data->relay2->SetOutState(relay2State); 

      vTaskDelay(pdMS_TO_TICKS(100)); // Process data every 100ms
    }
}

void TaskControlActuators(void* pvParameters) {
  SystemData* data = (SystemData*)pvParameters;
  unsigned long lastSetAct_500ms = 0;
  bool ledfailstate = false;

  for (;;) {
    unsigned long currentMillis = millis();

    /* Toggling LED indicator in case of any failure */
    if(data->ledInd->getOutstate() == LED_FAIL_INDICATE) {
      if(currentMillis - lastSetAct_500ms >= SUBTASK_INTERVAL_500_MS) {
        /* togggle LED every 500ms in case of Lvl failure */
        lastSetAct_500ms = currentMillis;
        ledfailstate = !ledfailstate;
        data->ledInd->setActuatorState(ledfailstate);
      }
    } else {
      data->ledInd->setActuatorState(LOW);
    }

    /* Update relay activation */
    data->relay1->setActuatorState(data->relay1->getOutstate());

    /* Update relay2 activation */
    data->relay2->setActuatorState(data->relay2->getOutstate());

    vTaskDelay(pdMS_TO_TICKS(100)); // Update actuators every 100ms
  }
}

/* Task: Update display with sensor data */
void TaskDisplay(void* pvParameters) {
  SystemData* data = (SystemData*)pvParameters;
  uint16_t levelValue = 0;
  uint16_t levelPercentage = 0;

  for (;;) {
      switch (data->currentSelector) {
          case PB1_SELECT_DATA1:
              data->oledDisplay->SetdisplayData(0, 0, "Light Sensor: ");
              data->oledDisplay->SetdisplayData(80, 0, data->lightSensor->getSensorValue() ? "Dark" : "Light");

              data->oledDisplay->SetdisplayData(0, 10, "Prencese: ");
              data->oledDisplay->SetdisplayData(80, 10, data->PirPresenceDetected ? "YES" : "NO");

              data->oledDisplay->SetdisplayData(0, 20, "Lamp: ");
              data->oledDisplay->SetdisplayData(80, 20, data->relay1->getOutstate() ? "ON" : "OFF");
          break;

          case PB1_SELECT_DATA2:
              data->oledDisplay->SetdisplayData(0, 0, "Water Level: ");
              levelValue = data->levelSensor->getSensorValue();
              if (levelValue >= SENSOR_LVL_OPENCKT_V) {
                  data->oledDisplay->SetdisplayData(80, 0, "OPEN");
              } else if (levelValue <= SENSOR_LVL_STG_V + SENSOR_LVL_THRESHOLD_V) {
                  data->oledDisplay->SetdisplayData(80, 0, "SHORT");
              } else if (levelValue >= SENSOR_LVL_OPENCKT_V - SENSOR_LVL_THRESHOLD_V) {
                // Ensure 100% is displayed just before OPEN
                data->oledDisplay->SetdisplayData(80, 0, "100%");
              } else {
                    data->oledDisplay->SetdisplayData(80,  0, data->levelPercentage);
                    data->oledDisplay->SetdisplayData(105, 0, "%");
                }
                data->oledDisplay->SetdisplayData(0,  10, "Pump: ");
                data->oledDisplay->SetdisplayData(80, 10, data->relay2->getOutstate() ? "ON" : "OFF");
                data->oledDisplay->SetdisplayData(0,  20, " ");
                data->oledDisplay->SetdisplayData(80, 20, " ");
          break;

          case PB1_SELECT_DATA3:
              data->oledDisplay->SetdisplayData(0,   0,  "Temperature: ");
              data->oledDisplay->SetdisplayData(80,  0,  data->tempHumSensor->getTemperature());
              data->oledDisplay->SetdisplayData(105, 0,  "C");

              data->oledDisplay->SetdisplayData(0,   10, "Humidity: ");
              data->oledDisplay->SetdisplayData(80,  10, data->tempHumSensor->getHumidity());
              data->oledDisplay->SetdisplayData(105, 10, "%");

              data->oledDisplay->SetdisplayData(0,   20, " ");
              data->oledDisplay->SetdisplayData(80,  20, " ");
          break;

          case PB1_SELECT_DATA4:
              data->oledDisplay->SetdisplayData(0,   0, "Wifi: ");
              data->oledDisplay->SetdisplayData(0,   10, ssid);
              data->oledDisplay->SetdisplayData(0,   20, "Status: ");
              data->oledDisplay->SetdisplayData(45,   20, data->client->IsWiFiConnected() ? "Connected" : "Disconnected");
          break;
      }

      Serial.print("Level: " + String(data->levelPercentage) + "%");
      Serial.print(" Temperature: " + String(data->tempHumSensor->getTemperature()) + "C");
      Serial.print(" Humidity: " + String(data->tempHumSensor->getHumidity()) + "%");
      Serial.print(" Light: " + String(data->lightSensor->getSensorValue()));
      Serial.print(" PIR: " + String(data->PirPresenceDetected));
      Serial.print(" Lamp: " + String(data->relay1->getOutstate()));
      Serial.print(" Pump: " + String(data->relay2->getOutstate()));
      Serial.println(" Fault: " + String(data->ledInd->getOutstate()));

      data->oledDisplay->PrintdisplayData();

      vTaskDelay(pdMS_TO_TICKS(SUBTASK_INTERVAL_1000_MS)); // Update display every 1 second
  }
}

void TaskSendDataToServer(void* pvParameters) {
    SystemData* data = (SystemData*)pvParameters;
    bool wifiConnecting = false; /* Flag to track if WiFi connection is being attempted */ 
    bool wifiConnectedMessagePrinted = false; /* Flag to track if the "WiFi connected!" message has been printed */ 

    for (;;) {
        if (data->client->IsWiFiConnected()) {
            wifiConnecting = false; /* Reset the flag once WiFi is connected */ 
            unsigned long currentMillis = millis();
            static unsigned long lastSendTime = 0;
            
            if (!wifiConnectedMessagePrinted) {
              Serial.println("WiFi connected!");
              wifiConnectedMessagePrinted = true; 
            }

            /* Send data to Firebase server */
            if (currentMillis - lastSendTime >= SUBTASK_INTERVAL_30_S) {
                lastSendTime = currentMillis;

                Serial.println("Sending Sensor data to server...");
                data->client->prepareData("type", "sensors");
                data->client->prepareData("lvl", String(data->levelPercentage));
                data->client->prepareData("tmp", String(data->tempHumSensor->getTemperature()));
                data->client->prepareData("hum", String(data->tempHumSensor->getHumidity()));
                data->client->prepareData("ldr", String(data->lightSensor->getSensorValue()));
                data->client->prepareData("pir", String(data->PirPresenceDetected));
                data->client->sendPayload();

                Serial.println("Sending Actuators data to server...");
                data->client->prepareData("type", "actuators");
                data->client->prepareData("lmp", String(data->relay1->getOutstate()));
                data->client->prepareData("pmp", String(data->relay2->getOutstate()));
                data->client->prepareData("flt", String(data->ledInd->getOutstate()));
                data->client->sendPayload();
            }
        } else {
            wifiConnectedMessagePrinted = false; /* Reset the flag when WiFi is disconnected */ 
            if (!wifiConnecting) {
                wifiConnecting = true; /* Set the flag to prevent multiple connection attempts */ 
                Serial.println("WiFi disconnected! Attempting to reconnect...");
                data->client->connectWiFi();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Keep task responsive
    }
}

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

void setup() {
  Serial.begin(9600);

  Serial.println("Starting...");

  // Get the reset reason for CPU0
  esp_reset_reason_t reason = esp_reset_reason();
  Serial.print("Reset reason: ");
  Serial.println(getResetReasonString(reason));

  xSystemDataMutex = xSemaphoreCreateMutex();
  if (xSystemDataMutex == NULL) {
      Serial.println("Failed to create mutex");
  }

  static SystemData systemData = {
    /* Sensors */
    new AnalogSensor(SENSOR_LVL_PIN),
    new TemperatureHumiditySensor(SENSOR_HUM_TEMP_PIN),
    new DigitalSensor(SENSOR_PIR_PIN),
    new DigitalSensor(SENSOR_LDR_PIN),
    new DigitalSensor(SENSOR_PBSELECTOR_PIN),
    /* Actuators */
    new Actuator(ACTUATOR_LED_PWM_PIN),
    new Actuator(ACTUATOR_RELAY1_PIN),
    new Actuator(ACTUATOR_RELAY2_PIN),
    /* Display */
    new OledDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_ADDRESS),
    /* Client */
    new ServerClient(serverUrl, ssid, password),
    /* Variables */
    true,
    PB1_SELECT_DATA1,
    0
  };

  systemData.oledDisplay->init();
  systemData.oledDisplay->clearAllDisplay();
  systemData.oledDisplay->setTextProperties(1, SSD1306_WHITE);
  systemData.tempHumSensor->dhtSensorInit();
  
  Serial.println("Sensor/Actuator/Display/WiFi objects initialized");

  /* Core 0: Real-Time Peripheral and Logic */
  xTaskCreatePinnedToCore(TaskReadSensors, "ReadSensors", 2048, &systemData, 3, NULL, TASK_CORE_0);
  xTaskCreatePinnedToCore(TaskProcessData, "ProcessData", 2048, &systemData, 2, NULL, TASK_CORE_0);
  xTaskCreatePinnedToCore(TaskControlActuators, "ControlActuators", 2048, &systemData, 2, NULL, TASK_CORE_0);
  Serial.println("Core 0: Sensor/Actuator tasks initialized");

  /* Core 1: Communication and Display */
  xTaskCreatePinnedToCore(TaskDisplay, "Display", 2048, &systemData, 2, NULL, TASK_CORE_1);
  xTaskCreatePinnedToCore(TaskSendDataToServer, "SendData", 4096, &systemData, 1, NULL, TASK_CORE_1);
  Serial.println("Core 1: Display/Client tasks initialized");
}

/* Empty loop since FreeRTOS manages execution in tasks */
void loop() {

}