#ifndef SYSTEM_DATA_H
#define SYSTEM_DATA_H

#include "SensorMgr.h"
#include "ActuatorMgr.h"
#include "OledDisplay_classes.h"
#include "client_classes.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#define SENSOR_LVL_OPENCKT_V   (3975) // ADC value for open circuit
#define SENSOR_LVL_STG_V       (124)  // ADC value for short circuit
#define SENSOR_LVL_THRESHOLD_V (50)  // ADC Threshold for level sensor

#define LED_NO_FAIL_INDICATE (0x00) 
#define LED_FAIL_INDICATE    (0x01) 

#define DEV_SW_VERSION "1.0.1" 

/* Enum for button selector states */
enum pb1Selector {
    SCREEN_LGT_PIR_LAMP_DATA, /* Display light, PIR, and Lamp data */
    SCREEN_LVL_PUMP_DATA,     /* Display level and Pump data */
    SCREEN_TEMP_HUM_IRR_DATA, /* Display temperature, humidity and irrigator data */
    SCREEN_WIFI_STATUS,       /* Display WiFi status */
    SCREEN_DEV_INFO,          /* Display device information */
    SCREEN_SELECT_NEXT,       /* Before this, the screens shall change every time select button is pressed */
    SCREEN_LVL_SETT_MENU,     /* Level Settings menu */
    SCREEN_TEMP_HUM_SETT_MENU,/* Temperature and Humidity Settings menu */
    SCREEN_WIFI_SETT_MENU,    /* WiFi settins main menu */
    SCREEN_WIFI_SETT_SUB_MENU,/* WiFi settins sub menu */
};

/* Struct to store all system-related data */
struct SystemData {
    /* Sensor Manager */
    SensorManager* sensorMgr;

    /* Actuator Manager */
    ActuatorManager* actuatorMgr;

    /* Object display */
    OledDisplay* oledDisplay;

    /* WiFi object */
    WiFiManager* wifiManager;

    /* Object client */
    ServerClient* SrvClient;

    /* Variables */
    bool PirPresenceDetected;
    pb1Selector currentDisplayDataSelec;
    uint8_t currentSettingMenu;
    uint16_t levelPercentage;

    /** Dynamically updated settings */
    uint8_t maxLevelPercentage; /* Updated max level percentage */
    uint8_t minLevelPercentage; /* Updated min level percentage */
    uint8_t hotTemperature;     /* Updated hot temperature */
    uint8_t lowHumidity;        /* Updated low humidity */
};

/* Declare the mutex globally so it can be used across files */
extern SemaphoreHandle_t xSystemDataMutex;

#endif // SYSTEM_DATA_H