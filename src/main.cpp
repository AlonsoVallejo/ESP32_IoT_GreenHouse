#include <Arduino.h>
#include "ESP32_shield.h"
#include "ProcessMgr.h"
#include "DisplayMgr.h"
#include "LogMgr.h"

using namespace std;

#define SENSOR_LVL_PIN          (SHIELD_POTENTIOMETER_VP)
#define SENSOR_HUM_TEMP_PIN     (SHIELD_DAC1_D25)
#define SENSOR_LDR_PIN          (SHIELD_DIMMER_ZC_D5) 
#define SENSOR_PIR_PIN          (SHIELD_DHT11_D13)
#define SENSOR_PBSELECTOR_PIN   (SHIELD_PUSHB1_D33)
#define ACTUATOR_LED_FAULT_PIN  (SHIELD_LED4_D14)
#define ACTUATOR_IRRIGATOR_PIN  (SHIELD_RELAY1_D4)
#define ACTUATOR_PUMP_PIN       (SHIELD_RELAY2_D2)
#define ACTUATOR_LAMP_PIN       (SHIELD_LED3_D12)

#define SUBTASK_INTERVAL_100_MS  (100)
#define SUBTASK_INTERVAL_500_MS  (500)
#define SUBTASK_INTERVAL_1000_MS (1000)     
#define SUBTASK_INTERVAL_2_S     (2000)  
#define SUBTASK_INTERVAL_15_S    (15000)  

#define TASK_CORE_0 (0)
#define TASK_CORE_1 (1)

const char* serverUrl = "http://192.168.100.9:3000/updateData"; /* Your server API URL for POST data; TODO: Use firebase functions */
const char* ssid = "MEGACABLE-2.4G-FAA4"; /* ESP32 WROOM32 works with 2.4GHz signals */
const char* password = "3kK4H6W48P";

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
                /* Change display selector on button press */
                data->currentSelector = static_cast<pb1Selector>((data->currentSelector + 1) % sizeof(pb1Selector));
            }
            xSemaphoreGive(xSystemDataMutex);
        }

        /* Read temperature and humidity */
        if (currentMillis - lastTempHumReadTime >= SUBTASK_INTERVAL_2_S) {
            lastTempHumReadTime = currentMillis;
            double temperature = data->tempHumSensor->readValueTemperature();
            double humidity = data->tempHumSensor->readValueHumidity();
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Delay for 100 ms
    }
}

void TaskProcessData(void* pvParameters) {
    SystemData* data = (SystemData*)pvParameters;

    for (;;) {
        /* Handle Lamp activation logic */
        handleLampActivation(data);

        /* Handle pump activation logic */
        handlePumpActivation(data);

        /* Irrigator Control */
        handleIrrigatorControl(data);

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

    /* Update Irrigator activation */
    data->irrigator->setActuatorState(data->irrigator->getOutstate());

    /* Update pump activation */
    data->pump->setActuatorState(data->pump->getOutstate());

    /* Update lamp activation */
    data->lamp->setActuatorState(data->lamp->getOutstate());

    vTaskDelay(pdMS_TO_TICKS(100)); // Update actuators every 100ms
  }
}

/* Task: Update display with sensor data */
void TaskDisplay(void* pvParameters) {
    SystemData* data = (SystemData*)pvParameters;
    bool debug = true; // Enable or disable logging

    for (;;) {
        switch (data->currentSelector) {
            case PB1_SELECT_DATA1:
                displayLightAndPresence(data);
                break;

            case PB1_SELECT_DATA2:
                displayWaterLevelAndPump(data);
                break;

            case PB1_SELECT_DATA3:
                displayTemperatureAndHumidity(data);
                break;

            case PB1_SELECT_DATA4:
                displayWiFiStatus(data);
                break;
        }

        data->oledDisplay->PrintdisplayData();

        LogSerial("Lvl: " + String(data->levelPercentage) + "%", debug);
        LogSerial(" Temp: " + String(data->tempHumSensor->getTemperature()) + "C", debug);
        LogSerial(" Hum: " + String(data->tempHumSensor->getHumidity()) + "%", debug);
        LogSerial(" ldr: " + String(data->lightSensor->getSensorValue()), debug);
        LogSerial(" PIR: " + String(data->PirPresenceDetected), debug);
        LogSerial(" lamp: " + String(data->lamp->getOutstate()), debug);
        LogSerial(" Pump: " + String(data->pump->getOutstate()), debug);
        LogSerial(" lvlFlt: " + String(data->ledInd->getOutstate()), debug);
        LogSerialn(" Irgtr: " + String(data->irrigator->getOutstate()), debug);

        vTaskDelay(pdMS_TO_TICKS(SUBTASK_INTERVAL_1000_MS));
    }
}

void TaskSendDataToServer(void* pvParameters) {
    SystemData* data = (SystemData*)pvParameters;
    bool wifiConnecting = false; /* Flag to track if WiFi connection is being attempted */ 
    bool wifiConnectedMessagePrinted = false; /* Flag to track if the "WiFi connected!" message has been printed */ 
    bool debug = true;

    for (;;) {
        if (data->wifiManager->IsWiFiConnected()) {
            wifiConnecting = false; /* Reset the flag once WiFi is connected */ 
            unsigned long currentMillis = millis();
            static unsigned long lastSendTime = 0;
            
            if (!wifiConnectedMessagePrinted) {
                LogSerialn("WiFi connected! ESP32 IP Address: " + data->wifiManager->getWiFiLocalIp().toString(), debug);
                wifiConnectedMessagePrinted = true; 
            }

            /* Send data to Firebase server */
            if (currentMillis - lastSendTime >= SUBTASK_INTERVAL_15_S) {
                lastSendTime = currentMillis;

                LogSerialn("Sending Sensor data to server...", debug);
                data->client->prepareData("type", "sensors");
                data->client->prepareData("lvl", String(data->levelPercentage));
                data->client->prepareData("tmp", String(data->tempHumSensor->getTemperature()));
                data->client->prepareData("hum", String(data->tempHumSensor->getHumidity()));
                data->client->prepareData("ldr", String(data->lightSensor->getSensorValue()));
                data->client->prepareData("pir", String(data->PirPresenceDetected));
                data->client->sendPayload();

                LogSerialn("Sending Actuators data to server...", debug);
                data->client->prepareData("type", "actuators");
                data->client->prepareData("lmp", String(data->lamp->getOutstate()));
                data->client->prepareData("pmp", String(data->pump->getOutstate()));
                data->client->prepareData("flt", String(data->ledInd->getOutstate()));
                data->client->prepareData("irr", String(data->irrigator->getOutstate()));
                data->client->sendPayload();
            }
        } else {
            wifiConnectedMessagePrinted = false; /* Reset the flag when WiFi is disconnected */ 
            if (!wifiConnecting) {
                wifiConnecting = true; /* Set the flag to prevent multiple connection attempts */ 
                LogSerialn("WiFi disconnected! Attempting to reconnect...", debug);
                data->wifiManager->connectWiFi();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Keep task responsive
    }
}

void setup() {
    Serial.begin(9600);
    LogSerialn("Starting...", true);

    /* Get the reset reason for CPU0 */ 
    esp_reset_reason_t reason = esp_reset_reason();
    LogSerial("Reset reason: ", true);
    LogSerialn(getResetReasonString(reason), true);

    xSystemDataMutex = xSemaphoreCreateMutex();
    if (xSystemDataMutex == NULL) {
        LogSerialn("Failed to create mutex", true);
    }

    static SystemData systemData = {
        /* Sensors */
        new AnalogSensor(SENSOR_LVL_PIN),
        new TemperatureHumiditySensor(SENSOR_HUM_TEMP_PIN),
        new DigitalSensor(SENSOR_PIR_PIN),
        new DigitalSensor(SENSOR_LDR_PIN),
        new DigitalSensor(SENSOR_PBSELECTOR_PIN),
        /* Actuators */
        new Actuator(ACTUATOR_LED_FAULT_PIN),
        new Actuator(ACTUATOR_IRRIGATOR_PIN),
        new Actuator(ACTUATOR_PUMP_PIN),
        new Actuator(ACTUATOR_LAMP_PIN),
        /* Display */
        new OledDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_ADDRESS),
        /* WiFi */
        new WiFiManager(ssid, password),
        /* Client */
        nullptr, // Temporarily set to nullptr
        /* Variables */
        true,
        PB1_SELECT_DATA1,
        0
    };

    /* Initialize the ServerClient after systemData is fully constructed */ 
    systemData.client = new ServerClient(serverUrl, systemData.wifiManager);

    /* Initialize the display */ 
    systemData.oledDisplay->init();
    systemData.oledDisplay->clearAllDisplay();
    systemData.oledDisplay->setTextProperties(1, SSD1306_WHITE);

    /* Initialize the temperature and humidity sensor */ 
    systemData.tempHumSensor->dhtSensorInit();

    LogSerialn("Sensor/Actuator/Display/WiFi objects initialized", true);

    /* Core 0: Real-Time Peripheral and Logic */
    xTaskCreatePinnedToCore(TaskReadSensors, "ReadSensors", 4096, &systemData, 3, NULL, TASK_CORE_0);
    xTaskCreatePinnedToCore(TaskProcessData, "ProcessData", 2048, &systemData, 2, NULL, TASK_CORE_0);
    xTaskCreatePinnedToCore(TaskControlActuators, "ControlActuators", 2048, &systemData, 2, NULL, TASK_CORE_0);
    LogSerialn("Core 0: Sensor/Actuator tasks initialized", true);

    /* Core 1: Communication and Display */
    xTaskCreatePinnedToCore(TaskDisplay, "Display", 2048, &systemData, 2, NULL, TASK_CORE_1);
    xTaskCreatePinnedToCore(TaskSendDataToServer, "SendData", 4096, &systemData, 1, NULL, TASK_CORE_1);
    LogSerialn("Core 1: Display/Client tasks initialized", true);
}

void loop() {
}