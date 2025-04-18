#include <Arduino.h>
#include "OledDisplay.h"
#include "ESP32_shield.h"
#include "Sensors_classes.h"
#include "Actuators_classes.h"

using namespace std;

#define SENSOR_POT_PIN        (SHIELD_POTENTIOMETER_VP)
#define SENSOR_HUM_TEMP_PIN   (SHIELD_DAC1_D25)
#define SENSOR_LDR_PIN        (SHIELD_PUSHB3_D34) 
#define ACTUATOR_LED_PWM_PIN  (SHIELD_LED4_D14)
#define ACTUATOR_RELAY1_PIN   (SHIELD_RELAY1_D4)

/* Defined times for each task in ms */
#define SUBTASK_INTERVAL_100_MS  (100)
#define SUBTASK_INTERVAL_500_MS  (500)   
#define SUBTASK_INTERVAL_5000_MS (5000)  

#define DISPLAY_INTERVAL_300_MS   (300)   

#define SENSOR_POT_OPENCKT_V (3975) // ADC value for open circuit
#define SENSOR_POT_STG_V     (124) // ADC value for short circuit

#define SENSOR_MAX_TEMP_C (50)

#define SENSOR_POT_FAIL_OPEN  (0xFFFF)
#define SENSOR_POT_FAIL_SHORT (0x0000)

#define LED_NO_FAIL_INDICATE     (0x00) 
#define LED_FAIL_INDICATE        (0x01) 

/* Struct to store all sensor, actuator, and display-related data */
struct SystemData {
  /* Objects */
  AnalogSensor* potSensor;
  TemperatureHumiditySensor* tempHumSensor;
  Actuator* ledInd;
  OledDisplay* oledDisplay;
  DigitalSensor* lightSensor;
  Actuator* relay1;
};

void TaskReadSensors(void* pvParameters) {
    SystemData* data = (SystemData*)pvParameters;

    for (;;) {
        /* upadte sensor input values */
        data->potSensor->readRawValue();
        data->lightSensor->readRawValue();
        data->tempHumSensor->readValueTemperature();
        data->tempHumSensor->readValueHumidity();

        vTaskDelay(pdMS_TO_TICKS(100)); // Read sensors every 100ms
    }
}

void TaskProcessData(void* pvParameters) {
    SystemData* data = (SystemData*)pvParameters;

    for (;;) {
        uint16_t potValue = data->potSensor->getSensorValue();
        if (potValue >= SENSOR_POT_OPENCKT_V || potValue <= SENSOR_POT_STG_V) {
            data->ledInd->SetOutState(LED_FAIL_INDICATE);
        } else {
            data->ledInd->SetOutState(LED_NO_FAIL_INDICATE);
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Process data every 100ms
    }
}

void TaskControlActuators(void* pvParameters) {
  SystemData* data = (SystemData*)pvParameters;
  unsigned long lastSetAct_500ms = 0;
  bool ledfailstate = false;
  unsigned long currentMillis = 0;

  for (;;) {
    unsigned long currentMillis = millis();

    /* Toggling LED indicator in case of any failure */
    if(data->ledInd->getOutstate() == LED_FAIL_INDICATE) {
      if(currentMillis - lastSetAct_500ms >= SUBTASK_INTERVAL_500_MS) {
        /* togggle LED every 500ms in case of Pot failure */
        lastSetAct_500ms = currentMillis;
        ledfailstate = !ledfailstate;
        data->ledInd->setActuatorState(ledfailstate);
      }
    } else {
      data->ledInd->setActuatorState(LOW);
    }

    // Activate relay based on light sensor value state
    data->relay1->setActuatorState(data->lightSensor->getSensorValue());

    vTaskDelay(pdMS_TO_TICKS(100)); // Update actuators every 100ms
  }
}

/* Task: Update display with sensor data */
void TaskDisplay(void* pvParameters) {
  SystemData* data = (SystemData*) pvParameters;

  double prevTemperature = 0;
  double prevHumidity = 0;
  uint16_t prevPotVoltage = 0;

  for (;;) {
      // Check if any value has changed
      uint16_t potValue = data->potSensor->getSensorValue();
      if (potValue != prevPotVoltage) {
          if(potValue >= SENSOR_POT_OPENCKT_V) {
              data->oledDisplay->SetdisplayData(0, 0, "PotVoltage: OPEN");
          } else if (potValue <= SENSOR_POT_STG_V) {
              data->oledDisplay->SetdisplayData(0, 0, "PotVoltage: SHORT");
          } else {
              data->oledDisplay->SetdisplayData(0, 0, "PotVoltage: ");
              data->oledDisplay->SetdisplayData(75, 0, data->potSensor->getVoltage());
              data->oledDisplay->SetdisplayData(105, 0, "V");
          }
          prevPotVoltage = potValue; 
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

      data->oledDisplay->PrintdisplayData();
      
      vTaskDelay(pdMS_TO_TICKS(DISPLAY_INTERVAL_300_MS));  /* Refresh display every 300ms */
    }
}

void setup() {
  Serial.begin(9600);

  /* Allocate memory for struct dynamically */
  SystemData* systemData = new SystemData {
      new AnalogSensor(SENSOR_POT_PIN),
      new TemperatureHumiditySensor(SENSOR_HUM_TEMP_PIN),
      new Actuator(ACTUATOR_LED_PWM_PIN),
      new OledDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_ADDRESS),
      new DigitalSensor(SENSOR_LDR_PIN),
      new Actuator(ACTUATOR_RELAY1_PIN)
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