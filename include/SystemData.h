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

/* Enum for button selector states */
enum pb1Selector {
    PB1_SELECT_DATA1, /* Display light, PIR, and Lamp data */
    PB1_SELECT_DATA2, /* Display level and Pump data */
    PB1_SELECT_DATA3, /* Display temperature, humidity and irrigator data */
    PB1_SELECT_DATA4, /* Display WiFi status */
    PB1_SELECT_COUNT, /* Before this, the screens shall change every time select button is pressed */
    PB1_SELECT_DATA5, /* Level Settings menu */
    PB1_SELECT_DATA6, /* Temperature and Humidity Settings menu */
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