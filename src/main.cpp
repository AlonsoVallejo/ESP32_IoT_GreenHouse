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
#define ACTUATOR_LED_PWM_PIN  (SHIELD_LED4_D14)
#define ACTUATOR_RELAY1_PIN   (SHIELD_RELAY1_D4)
#define ACTUATOR_RELAY2_PIN   (SHIELD_RELAY2_D2)

/* Defined times for each task in ms */
#define SUBTASK_INTERVAL_100_MS  (100)
#define SUBTASK_INTERVAL_500_MS  (500)   
#define SUBTASK_INTERVAL_5000_MS (5000)  

#define DISPLAY_INTERVAL_300_MS   (300)   

#define SENSOR_LVL_OPENCKT_V (3975) // ADC value for open circuit
#define SENSOR_LVL_STG_V     (124) // ADC value for short circuit
#define SENSOR_LVL_THRESHOLD_V (50) // Threshold for level sensor

#define SENSOR_MAX_TEMP_C (50)

#define SENSOR_LVL_FAIL_OPEN  (0xFFFF)
#define SENSOR_LVL_FAIL_SHORT (0x0000)

#define LED_NO_FAIL_INDICATE     (0x00) 
#define LED_FAIL_INDICATE        (0x01) 

#define SENSOR_PIR_COOL_DOWN_TIME (5000) 

/* Struct to store all sensor, actuator, and display-related data */
struct SystemData {
  /* Objects */
  AnalogSensor* levelSensor;
  TemperatureHumiditySensor* tempHumSensor;
  Actuator* ledInd;
  OledDisplay* oledDisplay;
  DigitalSensor* lightSensor;
  Actuator* relay1;
  DigitalSensor* pirSensor;
  Actuator* relay2;
  /* Variables */
  bool PirPresenceDetected;
};

void TaskReadSensors(void* pvParameters) {
    SystemData* data = (SystemData*)pvParameters;

    for (;;) {
        /* upadte sensor input values */
        data->levelSensor->readRawValue();
        data->lightSensor->readRawValue();
        data->tempHumSensor->readValueTemperature();
        data->tempHumSensor->readValueHumidity();
        data->pirSensor->readRawValue();
        vTaskDelay(pdMS_TO_TICKS(100)); // Read sensors every 100ms
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
      /** Calculate level percentage */
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
        
        /** If levelPercentage ≤ 18%, activate relay */
        if (levelPercentage <= 18) {
          relay2State = true;
        }  
        /** If levelPercentage reaches 82%, deactivate relay */
        else if (levelPercentage >= 82) {
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
  SystemData* data = (SystemData*) pvParameters;

  double prevTemperature = 0;
  double prevHumidity = 0;
  uint16_t prevLvlVoltage = 0;
  uint8_t prevLightSensor = 2;  /* To update value in first iteration */
  uint8_t prevPresenceSensor = 2; /* To update value in first iteration */
  uint16_t levelPercentage = 0;

  for (;;) {
      uint16_t levelValue = data->levelSensor->getSensorValue();
      if (levelValue != prevLvlVoltage) {
        if (levelValue >= SENSOR_LVL_OPENCKT_V) {
            // Display OPEN when the level value reaches or exceeds SENSOR_LVL_OPENCKT_V
            data->oledDisplay->SetdisplayData(0, 0, "LevelSensor: OPEN");
        } else if (levelValue <= SENSOR_LVL_STG_V + SENSOR_LVL_THRESHOLD_V) {
            // Display SHORT for values below the lower threshold
            data->oledDisplay->SetdisplayData(0, 0, "LevelSensor: SHORT");
        } else if (levelValue >= SENSOR_LVL_OPENCKT_V - SENSOR_LVL_THRESHOLD_V) {
            // Ensure 100% is displayed just before OPEN
            data->oledDisplay->SetdisplayData(0, 0, "LevelSensor: ");
            data->oledDisplay->SetdisplayData(75, 0, "100%");
            levelPercentage = 100;
        }  else {
          // Adjusted range for percentage calculation
          levelPercentage = ((levelValue - (SENSOR_LVL_STG_V + SENSOR_LVL_THRESHOLD_V)) * 100) / 
                            ((SENSOR_LVL_OPENCKT_V - SENSOR_LVL_THRESHOLD_V) - (SENSOR_LVL_STG_V + SENSOR_LVL_THRESHOLD_V));
          data->oledDisplay->SetdisplayData(0, 0, "LevelSensor: ");
          data->oledDisplay->SetdisplayData(75, 0, levelPercentage);
          data->oledDisplay->SetdisplayData(105, 0, "%");
      }

          prevLvlVoltage = levelValue; 
      }

      double temperature = data->tempHumSensor->getTemperature();
      if (data->tempHumSensor->getTemperature() != prevTemperature) {
          data->oledDisplay->SetdisplayData(0, 10, "Temperature: ");
          data->oledDisplay->SetdisplayData(75, 10, temperature);
          data->oledDisplay->SetdisplayData(105, 10, "C");
          prevTemperature = temperature;  
      }

      double HumidityValue = data->tempHumSensor->getHumidity();
      if (data->tempHumSensor->getHumidity() != prevHumidity) {
          data->oledDisplay->SetdisplayData(0, 20, "Humidity: ");
          data->oledDisplay->SetdisplayData(75, 20, HumidityValue);
          data->oledDisplay->SetdisplayData(105, 20, "%");
          prevHumidity = HumidityValue;  
      }

      uint16_t LightSensor = data->lightSensor->getSensorValue();
      if (prevLightSensor != LightSensor) {
          data->oledDisplay->SetdisplayData(0, 30, "LightSensor: ");
          data->oledDisplay->SetdisplayData(75, 30, LightSensor ? "Dark" : "Light");
          prevLightSensor = LightSensor;  
      }
      
      uint8_t PresenceSensor = data->PirPresenceDetected;
      if (prevPresenceSensor != PresenceSensor) {
          data->oledDisplay->SetdisplayData(0, 20, "Presence: ");
          data->oledDisplay->SetdisplayData(75, 20, PresenceSensor ? "YES" : "NO");
          prevPresenceSensor = PresenceSensor;  
      }
      
      data->oledDisplay->PrintdisplayData();

      vTaskDelay(pdMS_TO_TICKS(DISPLAY_INTERVAL_300_MS));  /* Refresh display every 300ms */
    }
}

void setup() {
  Serial.begin(9600);

  /* Allocate memory for struct dynamically */
  SystemData* systemData = new SystemData {
      new AnalogSensor(SENSOR_LVL_PIN),
      new TemperatureHumiditySensor(SENSOR_HUM_TEMP_PIN),
      new Actuator(ACTUATOR_LED_PWM_PIN),
      new OledDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_ADDRESS),
      new DigitalSensor(SENSOR_LDR_PIN),
      new Actuator(ACTUATOR_RELAY1_PIN),
      new DigitalSensor(SENSOR_PIR_PIN),
      new Actuator(ACTUATOR_RELAY2_PIN),
      true
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