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

#define SENSOR_TASK_STACK_SIZE   (4096)
#define PROCESS_TASK_STACK_SIZE  (2048)
#define ACTUATOR_TASK_STACK_SIZE (2048)
#define DISPLAY_TASK_STACK_SIZE  (2048)
#define SENDDATA_TASK_STACK_SIZE (4096)

#define SENSOR_TASK_PRIORITY     (3)
#define PROCESS_TASK_PRIORITY    (2)
#define ACTUATOR_TASK_PRIORITY   (2)
#define DISPLAY_TASK_PRIORITY    (2)
#define SENDDATA_TASK_PRIORITY   (1)

#define WIFI_RETRY_INTERVAL_MS  (10000)

#define TASK_CORE_0 (0)
#define TASK_CORE_1 (1)

SemaphoreHandle_t xSystemDataMutex;

void TaskReadSensors(void* pvParameters) {
    SystemData* data = (SystemData*)pvParameters;
    uint32_t lastTempHumReadTime = 0;
    uint32_t lastButtonPressTime = 0;

    for (;;) {
        uint32_t currentMillis = millis();

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
 
        vTaskDelay(pdMS_TO_TICKS(SUBTASK_INTERVAL_100_MS)); // Delay for button debounce
    }
}

void TaskProcessData(void* pvParameters) {
    SystemData* data = (SystemData*)pvParameters;

    for (;;) {
        /* Lamp activation logic */
        LampActivationCtrl(data);

        /* Pump activation logic */
        PumpActivationCtrl(data);

        /* Irrigator Control */
        IrrigatorActivationCtrl(data);

        /* Button control logic */
        pButtonsCtrl(data);

        vTaskDelay(pdMS_TO_TICKS(SUBTASK_INTERVAL_100_MS)); // Process data every 100ms
    }
}

void TaskControlActuators(void* pvParameters) {
    SystemData* data = (SystemData*)pvParameters;
    for (;;) {
        /* Apply internal states to hardware outputs */
        data->actuatorMgr->applyState();

        vTaskDelay(pdMS_TO_TICKS(SUBTASK_INTERVAL_100_MS)); // Update actuators every 100ms
    }
}

/* Task: Update display with sensor data */
void TaskDisplay(void* pvParameters) {
    SystemData* data = (SystemData*)pvParameters;
    bool IsLog = true; // Enable or disable logging
    uint8_t* Levelsettings[] = {
        &data->maxLevelPercentage,
        &data->minLevelPercentage,
    };
    uint8_t* TempHumsettings[] = {
        &data->hotTemperature,
        &data->lowHumidity
    };
   
    for (;;) {
        static uint32_t lastLogTime = 0;
        uint32_t currentMillis = millis();

        data->oledDisplay->clearAllDisplay();
        
        switch (data->currentDisplayDataSelec) {
            case SCREEN_LGT_PIR_LAMP_DATA:
                displayLightAndPresence(data);
                break;
            case SCREEN_LVL_PUMP_DATA:
                displayWaterLevelAndPump(data);
                break;
            case SCREEN_TEMP_HUM_IRR_DATA:
                displayTemperatureAndHumidity(data);
                break;
            case SCREEN_WIFI_STATUS:
                displayWiFiStatus(data);
                break;
            case SCREEN_DEV_INFO:
                displayDeviceInfo(data);
                break;
            case SCREEN_LVL_SETT_MENU:
                displayLevelSettings(data, *Levelsettings[data->currentSettingMenu]);
                break;
            case SCREEN_TEMP_HUM_SETT_MENU:
                displayTempHumSettings(data, *TempHumsettings[data->currentSettingMenu]);
                break;
            case SCREEN_WIFI_SETT_MENU:
            case SCREEN_WIFI_SETT_SUB_MENU:
                displayWiFiSettings(data);
                break;
            default:
                displayLightAndPresence(data);
                break;
        }

        data->oledDisplay->PrintdisplayData();

        if (currentMillis - lastLogTime >= SUBTASK_INTERVAL_1000_MS) {
            lastLogTime = currentMillis;
            LogSerial("Lvl: " + String(data->levelPercentage) + "%", IsLog);
            LogSerial(" Temp: " + String(data->sensorMgr->getTemperature()) + "C", IsLog);
            LogSerial(" Hum: " + String(data->sensorMgr->getHumidity()) + "%", IsLog);
            LogSerial(" ldr: " + String(data->sensorMgr->getLightSensorValue()), IsLog);
            LogSerial(" PIR: " + String(data->PirPresenceDetected), IsLog);
            LogSerial(" lamp: " + String(data->actuatorMgr->getLamp()->getOutstate()), IsLog);
            LogSerial(" Pump: " + String(data->actuatorMgr->getPump()->getOutstate()), IsLog);
            LogSerial(" lvlFlt: " + String(data->actuatorMgr->getLedIndicator()->getOutstate()), IsLog);
            LogSerialn(" Irgtr: " + String(data->actuatorMgr->getIrrigator()->getOutstate()), IsLog);
        }
        
        vTaskDelay(pdMS_TO_TICKS(SUBTASK_INTERVAL_100_MS));
    }
}

void TaskSendDataToServer(void* pvParameters) {
    SystemData* data = (SystemData*)pvParameters;
    bool wifiConnecting = false; /* Flag to track if WiFi connection is being attempted */ 
    bool wifiConnectedMessagePrinted = false; /* Flag to track if the "WiFi connected!" message has been printed */ 
    bool IsLog = true;
    pb1Selector previousDisplayDataSelec = data->currentDisplayDataSelec;
    static uint32_t lastSettingsFetchTime = 0;
    const char* serverUrl = data->SrvClient->getServerUrl();
    uint16_t customTaskDelay = 0;
    static uint32_t lastWifiAttempt = 0;
    const uint32_t wifiRetryInterval = WIFI_RETRY_INTERVAL_MS;

    for (;;) {
        if (strcmp(data->wifiManager->getSSID(), "DUMMY_WIFI_SSID") == 0 && strcmp(data->wifiManager->getPassword(), "DUMMY_WIFI_PASSWORD") == 0) {
            LogSerialn("WiFi credentials are dummy. Please set correct SSID and password.", IsLog);
            customTaskDelay = SUBTASK_INTERVAL_15_S;
        } else if (data->wifiManager->IsWiFiConnected()) {
            customTaskDelay = SUBTASK_INTERVAL_100_MS;
            wifiConnecting = false; /* Reset the flag once WiFi is connected */ 
            uint32_t currentMillis = millis();
            static uint32_t lastSendTime = 0;
            
            /* Execute this every time wifi connection is restablished */
            if (!wifiConnectedMessagePrinted) {
                LogSerialn("WiFi connected! ESP32 IP Address: " + data->wifiManager->getWiFiLocalIp().toString(), IsLog);
                wifiConnectedMessagePrinted = true; 

                /* Check if settings exist in the database */
                if (!checkSettingsExistence(data, serverUrl)) {
                    /* Send default settings if they do not exist */
                    LogSerialn("Sending default settings to the backend...", IsLog);
                    sendDefaultSettings(data, serverUrl);
                }

                /* Fetch updated settings on initial connection */
                fetchUpdatedSettings(data, serverUrl);
            }

            /* Check if system settins have been manually modified */
            if ( (previousDisplayDataSelec == SCREEN_LVL_SETT_MENU) && (data->currentDisplayDataSelec != SCREEN_LVL_SETT_MENU) || 
                 (previousDisplayDataSelec == SCREEN_TEMP_HUM_SETT_MENU) && (data->currentDisplayDataSelec != SCREEN_TEMP_HUM_SETT_MENU) ) {
                LogSerialn("Sending manual systems settings to the backend...", IsLog);
                LogSerial("maxlvl: " + String(data->maxLevelPercentage), IsLog);
                LogSerial(" minlvl: " + String(data->minLevelPercentage), IsLog);
                LogSerial(" hotTmp: " + String(data->hotTemperature), IsLog);
                LogSerialn(" lowHum: " + String(data->lowHumidity), IsLog);
                sendManualSettings(data, serverUrl); 
            }

            /* Periodically fetch updated settings */
            if ( (currentMillis - lastSettingsFetchTime >= SUBTASK_INTERVAL_15_S) && (data->currentDisplayDataSelec != SCREEN_LVL_SETT_MENU) ) {
                lastSettingsFetchTime = currentMillis;
                LogSerialn("Fetching system settings from server...", IsLog);
                fetchUpdatedSettings(data, serverUrl);
            } else if( data->currentDisplayDataSelec == SCREEN_LVL_SETT_MENU ) {
                lastSettingsFetchTime = currentMillis;
            } else {
                /*  do nothing */
            }

            /* Update the previous state */
            previousDisplayDataSelec = data->currentDisplayDataSelec;
            /* Send data to Firebase server */
            if (currentMillis - lastSendTime >= SUBTASK_INTERVAL_15_S) {
                lastSendTime = currentMillis;

                LogSerialn("Sending Sensor data to server...", IsLog);
                data->SrvClient->prepareData("type", "sensors");
                data->SrvClient->prepareData("lvl", String(data->levelPercentage));
                data->SrvClient->prepareData("tmp", String(data->sensorMgr->getTemperature()));
                data->SrvClient->prepareData("hum", String(data->sensorMgr->getHumidity()));
                data->SrvClient->prepareData("ldr", String(data->sensorMgr->getLightSensorValue()));
                data->SrvClient->prepareData("pir", String(data->PirPresenceDetected));
                data->SrvClient->sendPayload();

                LogSerialn("Sending Actuators data to server...", IsLog);
                data->SrvClient->prepareData("type", "actuators");
                data->SrvClient->prepareData("lmp", String(data->actuatorMgr->getLamp()->getOutstate()));
                data->SrvClient->prepareData("pmp", String(data->actuatorMgr->getPump()->getOutstate()));
                data->SrvClient->prepareData("flt", String(data->actuatorMgr->getLedIndicator()->getOutstate()));
                data->SrvClient->prepareData("irr", String(data->actuatorMgr->getIrrigator()->getOutstate()));
                data->SrvClient->sendPayload();
            }
        } else {
            customTaskDelay = SUBTASK_INTERVAL_100_MS;
            wifiConnectedMessagePrinted = false; /* Reset the flag when WiFi is disconnected */ 
            uint32_t now = millis();
            if (!wifiConnecting || (now - lastWifiAttempt > wifiRetryInterval)) {
                wifiConnecting = true; /* Set the flag to prevent multiple connection attempts */ 
                lastWifiAttempt = now;
                LogSerialn("WiFi disconnected! Attempting to reconnect...", IsLog);
                data->wifiManager->connectWiFi();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(customTaskDelay));
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

    const char* ssid = "DUMMY_WIFI_SSID"; /* ESP32 WROOM32 works with 2.4GHz signals */
    const char* password = "DUMMY_WIFI_PASSWORD"; /* WiFi password */
    const char* BackendServerUrl = "http://192.168.100.9:3000/"; /* Base URL for the backend server */

    static AnalogSensor analogSensor(SENSOR_LVL_PIN);
    static Dht11TempHumSens dht11Sensor(SENSOR_HUM_TEMP_PIN);
    static DigitalSensor pirSensor(SENSOR_PIR_PIN);
    static DigitalSensor ldrSensor(SENSOR_LDR_PIN);
    static DigitalSensor pbSelectSensor(SENSOR_PB_SELECT_PIN);
    static DigitalSensor pbEscSensor(SENSOR_PB_ESC_PIN);
    static DigitalSensor pbUpSensor(SENSOR_PB_UP_PIN);
    static DigitalSensor pbDownSensor(SENSOR_PB_DOWN_PIN);

    static SensorManager sensorManager(
        &analogSensor,
        &dht11Sensor,
        &pirSensor,
        &ldrSensor,
        &pbSelectSensor,
        &pbEscSensor,
        &pbUpSensor,
        &pbDownSensor
    );

    static Actuator ledFaultActuator(ACTUATOR_LED_FAULT_PIN);
    static Actuator irrigatorActuator(ACTUATOR_IRRIGATOR_PIN);
    static Actuator pumpActuator(ACTUATOR_PUMP_PIN);
    static Actuator lampActuator(ACTUATOR_LAMP_PIN);

    static ActuatorManager actuatorManager(
        &ledFaultActuator,
        &irrigatorActuator,
        &pumpActuator,
        &lampActuator
    );

    static OledDisplay oledDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_ADDRESS);
    static WiFiManager wifiManager(ssid, password);

    /* Try to load saved credentials and connect */ 
    String savedSsid, savedPassword;
    if (wifiManager.loadCredentials(savedSsid, savedPassword)) {
        LogSerialn("Loaded saved WiFi credentials for: " + savedSsid, true);
        wifiManager.setSSID(savedSsid.c_str());
        wifiManager.setPassword(savedPassword.c_str());
        wifiManager.connectWiFi();

    } else {
        LogSerialn("No saved WiFi credentials found.", true);
    }

    static ServerClient serverClient(BackendServerUrl, &wifiManager);

    static SystemData systemData = {
        &sensorManager,
        &actuatorManager,
        &oledDisplay,
        &wifiManager,
        &serverClient,
        /* Variables */
        true,
        SCREEN_LGT_PIR_LAMP_DATA,
        0,
        0,
        /** Dynamically updated settings */
        DFLT_MAX_LVL_PERCENTAGE,
        DFLT_MIN_LVL_PERCENTAGE,
        DFLT_SENSOR_HOT_TEMP_C,
        DFLT_SENSOR_LOW_HUMIDITY
    };

    /* Initialize the display */
    systemData.oledDisplay->init();
    systemData.oledDisplay->clearAllDisplay();
    systemData.oledDisplay->setTextProperties(1, SSD1306_WHITE);

    /* Init DHT11 sensor */
    systemData.sensorMgr->getTempHumSensor()->dhtSensorInit();

    LogSerialn("Sensor/Actuator/Display/WiFi objects initialized", true);

    /* Core 0: Real-Time Peripheral and Logic */
    xTaskCreatePinnedToCore(TaskReadSensors, "ReadSensors", SENSOR_TASK_STACK_SIZE, &systemData, SENSOR_TASK_PRIORITY, NULL, TASK_CORE_0);
    xTaskCreatePinnedToCore(TaskProcessData, "ProcessData", PROCESS_TASK_STACK_SIZE, &systemData, PROCESS_TASK_PRIORITY, NULL, TASK_CORE_0);
    xTaskCreatePinnedToCore(TaskControlActuators, "ControlActuators", ACTUATOR_TASK_STACK_SIZE, &systemData, ACTUATOR_TASK_PRIORITY, NULL, TASK_CORE_0);
    LogSerialn("Core 0: Sensor/Actuator tasks initialized", true);

    /* Core 1: Communication and Display */
    xTaskCreatePinnedToCore(TaskDisplay, "Display", DISPLAY_TASK_STACK_SIZE, &systemData, DISPLAY_TASK_PRIORITY, NULL, TASK_CORE_1);
    xTaskCreatePinnedToCore(TaskSendDataToServer, "SendData", SENDDATA_TASK_STACK_SIZE, &systemData, SENDDATA_TASK_PRIORITY, NULL, TASK_CORE_1);
    LogSerialn("Core 1: Display/SrvClient tasks initialized", true);
}

void loop() {
    // Empty loop
}