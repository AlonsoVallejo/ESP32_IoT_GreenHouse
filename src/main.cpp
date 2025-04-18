#include <Arduino.h>
#include "OledDisplay_classes.h"
#include "ESP32_shield.h"
#include "Sensors_classes.h"
#include "Actuators_classes.h"

using namespace std;

#define SENSOR_LVL_PIN        (SHIELD_POTENTIOMETER_VP)
#define SENSOR_HUM_TEMP_PIN   (SHIELD_DAC1_D25)
#define SENSOR_LDR_PIN        (SHIELD_PUSHB3_D34) 
#define SENSOR_PIR_PIN        (SHIELD_DHT11_D13)
#define SENSOR_PBSELECTOR_PIN (SHIELD_PUSHB2_D35)
#define ACTUATOR_LED_PWM_PIN  (SHIELD_LED4_D14)
#define ACTUATOR_RELAY1_PIN   (SHIELD_RELAY1_D4)
#define ACTUATOR_RELAY2_PIN   (SHIELD_RELAY2_D2)

/* Defined times for each task in ms */
#define SUBTASK_INTERVAL_100_MS  (100)
#define SUBTASK_INTERVAL_500_MS  (500)   
#define SUBTASK_INTERVAL_3000_MS (3000)  

#define DISPLAY_INTERVAL_300_MS   (300)   

#define SENSOR_LVL_OPENCKT_V (3975) // ADC value for open circuit
#define SENSOR_LVL_STG_V     (124) // ADC value for short circuit
#define SENSOR_LVL_THRESHOLD_V (50) // Threshold for level sensor

#define MAX_LVL_PERCENTAGE (90) 
#define MIN_LVL_PERCENTAGE (20) 

#define SENSOR_MAX_TEMP_C (50)

#define SENSOR_LVL_FAIL_OPEN  (0xFFFF)
#define SENSOR_LVL_FAIL_SHORT (0x0000)

#define LED_NO_FAIL_INDICATE     (0x00) 
#define LED_FAIL_INDICATE        (0x01) 

#define SENSOR_PIR_COOL_DOWN_TIME (5000) 

enum pb1Selector{
  PB1_SELECT_DATA1, /* Display light, Pir and relay1 data */
  PB1_SELECT_DATA2, /* Display level and relay2 data */
  PB1_SELECT_DATA3, /* Display temperature and humidity data */
};

/* Struct to store all sensor, actuator, and display-related data */
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
  /* Variables */
  bool PirPresenceDetected;
  pb1Selector currentSelector;
};

void TaskReadSensors(void* pvParameters) {
  SystemData* data = (SystemData*)pvParameters;
  unsigned long lastTempHumReadTime = 0; /** Tracks last temp/humidity read time */
  unsigned long lastButtonPressTime = 0;

  for (;;) {
      unsigned long currentMillis = millis(); /** Get current time */

      /* Update level, light, and PIR sensor values every 100ms */
      data->levelSensor->readRawValue();
      data->lightSensor->readRawValue();
      data->pirSensor->readRawValue();
      data->buttonSelector->readRawValue();

      /* push button selector input is inverted */
      bool buttonState = !(data->buttonSelector->getSensorValue());
 
      /* Check for button press with debounce */ 
      if (buttonState && (currentMillis - lastButtonPressTime > 300)) { /* 300ms debounce */ 
          lastButtonPressTime = currentMillis;

          data->currentSelector = static_cast<pb1Selector>((data->currentSelector + 1) % 3);
      }

      /* Read temperature and humidity every 3 seconds */
      if (currentMillis - lastTempHumReadTime >= SUBTASK_INTERVAL_3000_MS) {
          lastTempHumReadTime = currentMillis;
          data->tempHumSensor->readValueTemperature();
          data->tempHumSensor->readValueHumidity();
      }

      vTaskDelay(pdMS_TO_TICKS(100)); /** Maintain 100ms loop timing */
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
      data->PirPresenceDetected = presenceDetected;

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
          relay2State = true;
        }  
        else if (levelPercentage >= MAX_LVL_PERCENTAGE) {
          relay2State = false;
        }
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
                    levelPercentage = ((levelValue - SENSOR_LVL_STG_V) * 100) / (SENSOR_LVL_OPENCKT_V - SENSOR_LVL_STG_V);
                    data->oledDisplay->SetdisplayData(80, 0, levelPercentage);
                    data->oledDisplay->SetdisplayData(105,0, "%");
                }
                data->oledDisplay->SetdisplayData(0, 10, "Pump: ");
                data->oledDisplay->SetdisplayData(80, 10, data->relay2->getOutstate() ? "ON" : "OFF");
                data->oledDisplay->SetdisplayData(0, 20, " ");
                data->oledDisplay->SetdisplayData(80, 20," ");
          break;

          case PB1_SELECT_DATA3:
              data->oledDisplay->SetdisplayData(0, 0, "Temperature: ");
              data->oledDisplay->SetdisplayData(80, 0, data->tempHumSensor->getTemperature());
              data->oledDisplay->SetdisplayData(105,0, "C");
              data->oledDisplay->SetdisplayData(0, 10, "Humidity: ");
              data->oledDisplay->SetdisplayData(80, 10, data->tempHumSensor->getHumidity());
              data->oledDisplay->SetdisplayData(105,10, "%");
              data->oledDisplay->SetdisplayData(0, 20, " ");
              data->oledDisplay->SetdisplayData(80, 20," ");
          break;
      }

      data->oledDisplay->PrintdisplayData();

      vTaskDelay(pdMS_TO_TICKS(DISPLAY_INTERVAL_300_MS)); // Refresh display every 300ms
  }
}

void setup() {
  Serial.begin(9600);

  /* Allocate memory for struct dynamically */
  SystemData* systemData = new SystemData {
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
      /* Display user data*/
      new OledDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_ADDRESS),
       /* variables */
      true,
      PB1_SELECT_DATA1
  };

  systemData->oledDisplay->init();
  systemData->oledDisplay->clearAllDisplay();
  systemData->oledDisplay->setTextProperties(1, SSD1306_WHITE);
  systemData->tempHumSensor->dhtSensorInit();

  /* Create FreeRTOS tasks, passing systemData struct instead of using global variables */
  xTaskCreate(TaskReadSensors, "ReadSensors", 2048, systemData, 2, NULL);
  xTaskCreate(TaskProcessData, "ProcessData", 2048, systemData, 2, NULL);  
  xTaskCreate(TaskControlActuators, "ControlActuators", 2048, systemData, 1, NULL);
  xTaskCreate(TaskDisplay, "Display", 2048, systemData, 1, NULL);
}

/* Empty loop since FreeRTOS manages execution in tasks */
void loop() {

}