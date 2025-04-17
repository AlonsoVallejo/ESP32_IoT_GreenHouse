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

#define SENSOR_POT_OPENCKT_V (3.1)
#define SENSOR_POT_STG_V     (0.1)

#define SENSOR_MAX_TEMP_C (50)

#define LED_FAIL_INDICATE (0xFF) // LED state to indicate failure

/* Struct to store all sensor, actuator, and display-related data */
struct SystemData {
  /* Objects */
  VarResSensor* potSensor;
  TemperatureHumiditySensor* tempHumSensor;
  Actuator* led;
  OledDisplay* oledDisplay;
  LdrSensor* lightSensor;
  Actuator* relay1;
  /*variables */
  uint8_t ledState;
  double potVoltage;
  double temperature;
  double humidity;
  uint8_t ldrState;
};

void TaskReadSensors(void *pvParameters) {
  SystemData* data = (SystemData*) pvParameters;
  unsigned long lastReadSensor_100ms = 0;
  unsigned long lastReadSensor_5000ms = 0;

  for (;;) {
    unsigned long currentMillis = millis();

    /* Sensor to be read evey 100 ms */
    if (currentMillis - lastReadSensor_100ms >= SUBTASK_INTERVAL_100_MS) {
        lastReadSensor_100ms = currentMillis;
        data->potVoltage = data->potSensor->getVoltage();
        data->ldrState = data->lightSensor->getLdrState();
    }

    /* Sensor to be read evey 5000 ms */
    if (currentMillis - lastReadSensor_5000ms >= SUBTASK_INTERVAL_5000_MS) {
        lastReadSensor_5000ms = currentMillis;
        data->temperature = data->tempHumSensor->readValueTemperature();
        data->humidity = data->tempHumSensor->readValueHumidity();
    }

    /* Prevent CPU overloading */
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

/* Task: Process sensor data */
void TaskProcessData(void* pvParameters) {
  SystemData* data = (SystemData*)pvParameters;

  // Variables to store processed data
  double processedHumidity = 0;

  for (;;) {
    /* Scale potentiometer value for LED intensity */
    data->potVoltage = data->potSensor->getVoltage();
    if(data->potVoltage >= SENSOR_POT_OPENCKT_V) {
      data->potVoltage = SENSOR_POT_OPENCKT_V;
      data->ledState = LED_FAIL_INDICATE;
    } else if (data->potVoltage <= SENSOR_POT_STG_V) {
      data->potVoltage = SENSOR_POT_STG_V;
      data->ledState = LED_FAIL_INDICATE;
    } else {
      data->ledState = map(data->potSensor->readRawValue(), 0, 4095, 0, 100);
    }

    /* Apply a threshold to temperature */ 
    data->temperature = data->tempHumSensor->readValueTemperature();
    if (data->temperature > SENSOR_MAX_TEMP_C) {
      data->temperature = SENSOR_MAX_TEMP_C; 
    }

    /* Smooth humidity values (simple moving average) */ 
    static uint16_t previousHumidity = 0;
    processedHumidity = (data->tempHumSensor->readValueHumidity() + previousHumidity) / 2;
    previousHumidity = processedHumidity;
    data->humidity = processedHumidity;

    vTaskDelay(pdMS_TO_TICKS(100)); // Process data every 100ms
  }
}

/* Task: Control actuators */
void TaskControlActuators(void* pvParameters) {
  SystemData* data = (SystemData*)pvParameters;
  unsigned long lastSetAct_500ms = 0;
  bool ledfailstate = false;
  unsigned long currentMillis = 0;

  for (;;) {
    unsigned long currentMillis = millis();
    if(data->ledState == LED_FAIL_INDICATE) {
      if(currentMillis - lastSetAct_500ms >= SUBTASK_INTERVAL_500_MS) {
        /* togggle LED every 500ms in case of Pot failure */
        lastSetAct_500ms = currentMillis;
        ledfailstate = !ledfailstate;
        if(ledfailstate) {
          data->led->SetPwmDutyCycle(0); // Turn off LED
        } else {
          data->led->SetPwmDutyCycle(100); // Turn on LED
        }
      }
    } else {
      data->led->SetPwmDutyCycle(data->ledState);
    }

    // Use processed data for actuators
    data->relay1->SetOutState(data->lightSensor->getLdrState());

    vTaskDelay(pdMS_TO_TICKS(100)); // Update actuators every 100ms
  }
}

/* Task: Update display with sensor data */
void TaskDisplay(void* pvParameters) {
  SystemData* data = (SystemData*) pvParameters;

  uint8_t prevpotScaledValue = 0;
  double prevTemperature = 0;
  double prevHumidity = 0;
  double prevPotVoltage = 0.0;

  for (;;) {
      // Check if any value has changed
      if (data->potVoltage != prevPotVoltage) {
          if(prevPotVoltage >= SENSOR_POT_OPENCKT_V) {
              data->oledDisplay->SetdisplayData(0, 0, "PotVoltage: OPEN");
          } else if (prevPotVoltage <= SENSOR_POT_STG_V) {
              data->oledDisplay->SetdisplayData(0, 0, "PotVoltage: SHORT");
          } else {
              data->oledDisplay->SetdisplayData(0, 0, "PotVoltage: ");
              data->oledDisplay->SetdisplayData(75, 0, data->potVoltage);
              data->oledDisplay->SetdisplayData(105, 0, "V");
          }
          prevPotVoltage = data->potVoltage; 
      }

      if (data->temperature != prevTemperature) {
          data->oledDisplay->SetdisplayData(0, 10, "Temperature: ");
          data->oledDisplay->SetdisplayData(75, 10, data->temperature);
          data->oledDisplay->SetdisplayData(105, 10, "C");
          prevTemperature = data->temperature;  
      }

      if (data->humidity != prevHumidity) {
          data->oledDisplay->SetdisplayData(0, 20, "Humidity: ");
          data->oledDisplay->SetdisplayData(75, 20, data->humidity);
          data->oledDisplay->SetdisplayData(105, 20, "%");
          prevHumidity = data->humidity;  
      }

      data->oledDisplay->PrintdisplayData();
      
      vTaskDelay(pdMS_TO_TICKS(DISPLAY_INTERVAL_300_MS));  /* Refresh display every 300ms */
    }
}

void setup() {
  Serial.begin(9600);

  /* Allocate memory for struct dynamically */
  SystemData* systemData = new SystemData {
      new VarResSensor(SENSOR_POT_PIN, 100),
      new TemperatureHumiditySensor(SENSOR_HUM_TEMP_PIN),
      new Actuator(ACTUATOR_LED_PWM_PIN),
      new OledDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_ADDRESS),
      new LdrSensor(SENSOR_LDR_PIN),
      new Actuator(ACTUATOR_RELAY1_PIN),
      0, 0.0, 0, 0, 0 /* Initialize sensor values */
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