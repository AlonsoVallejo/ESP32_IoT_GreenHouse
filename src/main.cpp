#include <Arduino.h>
#include "ESP32_shield.h"
#include "ProcessMgr.h"
#include "DisplayMgr.h"
#include "SrvClientMgr.h" 
#include "LogMgr.h"

using namespace std;

/* Current used GPIOs for sensors */
#define SENSOR_LVL_PIN          (SHIELD_POTENTIOMETER_VP_D36)
#define SENSOR_HUM_TEMP_PIN     (SHIELD_DAC1_D25)
#define SENSOR_LDR_PIN          (SHIELD_DIMMER_ZC_D5) 
#define SENSOR_PIR_PIN          (SHIELD_DHT11_D13)
#define SENSOR_PB_SELECT_PIN    (SHIELD_PUSHB1_D33)
#define SENSOR_PB_ESC_PIN       (SHIELD_PUSHB3_D34)
#define SENSOR_PB_UP_PIN        (SHIELD_PUSHB2_D35)
#define SENSOR_PB_DOWN_PIN      (SHIELD_PUSHB4_D32)
#define ACTUATOR_LED_FAULT_PIN  (SHIELD_LED4_D14)
#define ACTUATOR_IRRIGATOR_PIN  (SHIELD_RELAY1_D4)
#define ACTUATOR_PUMP_PIN       (SHIELD_RELAY2_D2)
#define ACTUATOR_LAMP_PIN       (SHIELD_LED3_D12)
#define OLED_DISPLAY_SCL_PIN    (SHIELD_OLED_SCL_D22)
#define OLED_DISPLAY_SDA_PIN    (SHIELD_OLED_SDA_D21)

#define SUBTASK_INTERVAL_100_MS  (100)
#define SUBTASK_INTERVAL_500_MS  (500)
#define SUBTASK_INTERVAL_1000_MS (1000)     
#define SUBTASK_INTERVAL_2000_MS (2000)  
#define SUBTASK_INTERVAL_15_S    (15000)  


#define TASK_CORE_0 (0)
#define TASK_CORE_1 (1)

const char* serverUrl = "http://192.168.100.9:3000/"; /* Base URL for the backend server */
const char* ssid = "MEGACABLE-2.4G-FAA4"; /* ESP32 WROOM32 works with 2.4GHz signals */
const char* password = "3kK4H6W48P";

SemaphoreHandle_t xSystemDataMutex;

void TaskReadSensors(void* pvParameters) {
    SystemData* data = (SystemData*)pvParameters;
    unsigned long lastTempHumReadTime = 0;
    unsigned long lastButtonPressTime = 0;

    uint8_t* settings[] = {
        &data->maxLevelPercentage,
        &data->minLevelPercentage,
        &data->hotTemperature,
        &data->lowHumidity
    };

    for (;;) {
        unsigned long currentMillis = millis();

        /* Update individual sensor values */
        data->sensorMgr->readLevelSensor();
        data->sensorMgr->readPirSensor();
        data->sensorMgr->readLightSensor();
        data->sensorMgr->readButtonSelector();
        data->sensorMgr->readButtonEsc();
        data->sensorMgr->readButtonUp();
        data->sensorMgr->readButtonDown();

        /* Read temperature and humidity periodically */
        if (currentMillis - lastTempHumReadTime >= SUBTASK_INTERVAL_2000_MS) {
            lastTempHumReadTime = currentMillis;
            data->sensorMgr->readDht11TempHumSens();
        }

        /* Protect shared variable access */
        if (xSemaphoreTake(xSystemDataMutex, portMAX_DELAY)) {
            bool SelectbuttonState = !(data->sensorMgr->getButtonSelectorValue()); /* (pressed = LOW, released = HIGH) */
            bool escButtonState = !(data->sensorMgr->getButtonEscValue()); /* (pressed = LOW, released = HIGH) */
            bool upButtonState = !(data->sensorMgr->getButtonUpValue()); /* (pressed = LOW, released = HIGH) */  
            bool downButtonState = !(data->sensorMgr->getButtonDownValue()); /* (pressed = LOW, released = HIGH) */

            if (SelectbuttonState && (currentMillis - lastButtonPressTime > 300)) {
                lastButtonPressTime = currentMillis;
                if (data->currentDisplayDataSelec == PB1_SELECT_DATA5) {
                    /* Navigate through settings menu */ 
                    data->currentSettingMenu++;
                    if (data->currentSettingMenu >= sizeof(settings) / sizeof(settings[0])) {
                        /* If last setting is reached, reset to default display */ 
                        data->currentDisplayDataSelec = PB1_SELECT_DATA1;
                        data->currentSettingMenu = 0;
                    }
                } else {
                    data->currentDisplayDataSelec = static_cast<pb1Selector>((data->currentDisplayDataSelec + 1) % PB1_SELECT_COUNT);
                }
            }

            if (escButtonState && (currentMillis - lastButtonPressTime > 300)) {
                lastButtonPressTime = currentMillis;
                if (data->currentDisplayDataSelec == PB1_SELECT_DATA5) {
                    /* Exit settings menu and save changes */ 
                    data->currentDisplayDataSelec = PB1_SELECT_DATA1;
                    data->currentSettingMenu = 0;
                }
            }

            if (data->currentDisplayDataSelec == PB1_SELECT_DATA5) {
                if (upButtonState && (currentMillis - lastButtonPressTime > 300)) {
                    lastButtonPressTime = currentMillis;
                    /* Increment the current setting value */
                    if (*settings[data->currentSettingMenu] < 100) {
                        (*settings[data->currentSettingMenu])++;
                    }
                }

                if (downButtonState && (currentMillis - lastButtonPressTime > 300)) {
                    lastButtonPressTime = currentMillis;
                    /* Decrement the current setting value */ 
                    if (*settings[data->currentSettingMenu] > 0) {
                        (*settings[data->currentSettingMenu])--;
                    }
                }
            }

            xSemaphoreGive(xSystemDataMutex);
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Delay for 100 ms
    }
}

void TaskProcessData(void* pvParameters) {
    SystemData* data = (SystemData*)pvParameters;

    for (;;) {
        /* Lamp activation logic */
        handleLampActivation(data);

        /* Pump activation logic */
        handlePumpActivation(data);

        /* Irrigator Control */
        handleIrrigatorControl(data);

        vTaskDelay(pdMS_TO_TICKS(100)); // Process data every 100ms
    }
}

void TaskControlActuators(void* pvParameters) {
    SystemData* data = (SystemData*)pvParameters;
    for (;;) {
        /* Apply internal states to hardware outputs */
        data->actuatorMgr->applyState();

        vTaskDelay(pdMS_TO_TICKS(100)); // Update actuators every 100ms
    }
}

/* Task: Update display with sensor data */
void TaskDisplay(void* pvParameters) {
    SystemData* data = (SystemData*)pvParameters;
    bool debug = true; // Enable or disable logging
    uint8_t* settings[] = {
        &data->maxLevelPercentage,
        &data->minLevelPercentage,
        &data->hotTemperature,
        &data->lowHumidity
    };
    
    for (;;) {
        switch (data->currentDisplayDataSelec) {
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
            case PB1_SELECT_DATA5:
                displaySettingsMenu(data, *settings[data->currentSettingMenu]);
                break;
        }

        data->oledDisplay->PrintdisplayData();

        LogSerial("Lvl: " + String(data->levelPercentage) + "%", debug);
        LogSerial(" Temp: " + String(data->sensorMgr->getTemperature()) + "C", debug);
        LogSerial(" Hum: " + String(data->sensorMgr->getHumidity()) + "%", debug);
        LogSerial(" ldr: " + String(data->sensorMgr->getLightSensorValue()), debug);
        LogSerial(" PIR: " + String(data->PirPresenceDetected), debug);
        LogSerial(" lamp: " + String(data->actuatorMgr->getLamp()->getOutstate()), debug);
        LogSerial(" Pump: " + String(data->actuatorMgr->getPump()->getOutstate()), debug);
        LogSerial(" lvlFlt: " + String(data->actuatorMgr->getLedIndicator()->getOutstate()), debug);
        LogSerialn(" Irgtr: " + String(data->actuatorMgr->getIrrigator()->getOutstate()), debug);
        
        vTaskDelay(pdMS_TO_TICKS(SUBTASK_INTERVAL_1000_MS));
    }
}

void TaskSendDataToServer(void* pvParameters) {
    SystemData* data = (SystemData*)pvParameters;
    bool wifiConnecting = false; /* Flag to track if WiFi connection is being attempted */ 
    bool wifiConnectedMessagePrinted = false; /* Flag to track if the "WiFi connected!" message has been printed */ 
    bool debug = true;
    pb1Selector previousDisplayDataSelec = data->currentDisplayDataSelec;
    static unsigned long lastSettingsFetchTime = 0;

    for (;;) {
        if (data->wifiManager->IsWiFiConnected()) {
            wifiConnecting = false; /* Reset the flag once WiFi is connected */ 
            unsigned long currentMillis = millis();
            static unsigned long lastSendTime = 0;
            
            /* Execute this every time wifi connection is restablished */
            if (!wifiConnectedMessagePrinted) {
                LogSerialn("WiFi connected! ESP32 IP Address: " + data->wifiManager->getWiFiLocalIp().toString(), debug);
                wifiConnectedMessagePrinted = true; 

                /* Check if settings exist in the database */
                if (!checkSettingsExistence(data, serverUrl)) {
                    /* Send default settings if they do not exist */
                    LogSerialn("Sending default settings to the backend...", debug);
                    sendDefaultSettings(data, serverUrl);
                }

                /* Fetch updated settings on initial connection */
                fetchUpdatedSettings(data, serverUrl);
            }

            /* Check if system settins have been manually modified */
            if (previousDisplayDataSelec == PB1_SELECT_DATA5 && data->currentDisplayDataSelec != PB1_SELECT_DATA5) {
                LogSerialn("Sending manual systems settings to the backend...", debug);
                LogSerial("maxlvl: " + String(data->maxLevelPercentage), debug);
                LogSerial(" minlvl: " + String(data->minLevelPercentage), debug);
                LogSerial(" hotTmp: " + String(data->hotTemperature), debug);
                LogSerialn(" lowHum: " + String(data->lowHumidity), debug);
                sendManualSettings(data, serverUrl); 
            }

            /* Periodically fetch updated settings */
            if ( (currentMillis - lastSettingsFetchTime >= SUBTASK_INTERVAL_15_S) && (data->currentDisplayDataSelec != PB1_SELECT_DATA5) ) {
                lastSettingsFetchTime = currentMillis;
                LogSerialn("Fetching system settings from server...", debug);
                fetchUpdatedSettings(data, serverUrl);
            } else if( data->currentDisplayDataSelec == PB1_SELECT_DATA5 ) {
                lastSettingsFetchTime = currentMillis;
            } else {
                /*  do nothing */
            }

            /* Update the previous state */
            previousDisplayDataSelec = data->currentDisplayDataSelec;
            /* Send data to Firebase server */
            if (currentMillis - lastSendTime >= SUBTASK_INTERVAL_15_S) {
                lastSendTime = currentMillis;

                LogSerialn("Sending Sensor data to server...", debug);
                data->SrvClient->prepareData("type", "sensors");
                data->SrvClient->prepareData("lvl", String(data->levelPercentage));
                data->SrvClient->prepareData("tmp", String(data->sensorMgr->getTemperature()));
                data->SrvClient->prepareData("hum", String(data->sensorMgr->getHumidity()));
                data->SrvClient->prepareData("ldr", String(data->sensorMgr->getLightSensorValue()));
                data->SrvClient->prepareData("pir", String(data->PirPresenceDetected));
                data->SrvClient->sendPayload();

                LogSerialn("Sending Actuators data to server...", debug);
                data->SrvClient->prepareData("type", "actuators");
                data->SrvClient->prepareData("lmp", String(data->actuatorMgr->getLamp()->getOutstate()));
                data->SrvClient->prepareData("pmp", String(data->actuatorMgr->getPump()->getOutstate()));
                data->SrvClient->prepareData("flt", String(data->actuatorMgr->getLedIndicator()->getOutstate()));
                data->SrvClient->prepareData("irr", String(data->actuatorMgr->getIrrigator()->getOutstate()));
                data->SrvClient->sendPayload();
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
        new SensorManager(
            new AnalogSensor(SENSOR_LVL_PIN),
            new Dht11TempHumSens(SENSOR_HUM_TEMP_PIN),
            new DigitalSensor(SENSOR_PIR_PIN),
            new DigitalSensor(SENSOR_LDR_PIN),
            new DigitalSensor(SENSOR_PB_SELECT_PIN),
            new DigitalSensor(SENSOR_PB_ESC_PIN),
            new DigitalSensor(SENSOR_PB_UP_PIN),
            new DigitalSensor(SENSOR_PB_DOWN_PIN)
        ),
        new ActuatorManager(
            new Actuator(ACTUATOR_LED_FAULT_PIN),
            new Actuator(ACTUATOR_IRRIGATOR_PIN),
            new Actuator(ACTUATOR_PUMP_PIN),
            new Actuator(ACTUATOR_LAMP_PIN)
        ),
        /* Object display */
        new OledDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_ADDRESS),
        /* WiFi object */
        new WiFiManager(ssid, password),
         /* Object client */
        nullptr, // SrvClient will be initialized later
        /* Variables */
        true,
        PB1_SELECT_DATA1,
        0,
        0,
        /** Dynamically updated settings */
        DFLT_MAX_LVL_PERCENTAGE,
        DFLT_MIN_LVL_PERCENTAGE,
        DFLT_SENSOR_HOT_TEMP_C,
        DFLT_SENSOR_LOW_HUMIDITY
    };

    /* Initialize the ServerSrvClient after systemData is fully constructed */ 
    systemData.SrvClient = new ServerClient(serverUrl, systemData.wifiManager);

    /* Initialize the display */ 
    systemData.oledDisplay->init();
    systemData.oledDisplay->clearAllDisplay();
    systemData.oledDisplay->setTextProperties(1, SSD1306_WHITE);

    /* Init DHT11 sensor */
    systemData.sensorMgr->getTempHumSensor()->dhtSensorInit();

    LogSerialn("Sensor/Actuator/Display/WiFi objects initialized", true);

    /* Core 0: Real-Time Peripheral and Logic */
    xTaskCreatePinnedToCore(TaskReadSensors, "ReadSensors", 4096, &systemData, 3, NULL, TASK_CORE_0);
    xTaskCreatePinnedToCore(TaskProcessData, "ProcessData", 2048, &systemData, 2, NULL, TASK_CORE_0);
    xTaskCreatePinnedToCore(TaskControlActuators, "ControlActuators", 2048, &systemData, 2, NULL, TASK_CORE_0);
    LogSerialn("Core 0: Sensor/Actuator tasks initialized", true);

    /* Core 1: Communication and Display */
    xTaskCreatePinnedToCore(TaskDisplay, "Display", 2048, &systemData, 2, NULL, TASK_CORE_1);
    xTaskCreatePinnedToCore(TaskSendDataToServer, "SendData", 4096, &systemData, 1, NULL, TASK_CORE_1);
    LogSerialn("Core 1: Display/SrvClient tasks initialized", true);
}

void loop() {
}