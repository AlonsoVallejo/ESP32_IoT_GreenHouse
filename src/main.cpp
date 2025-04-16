#include <Arduino.h>
#include "OledDisplay.h"
#include "dht11_sens.h"

using namespace std;

/* ESP32 GPIOS */
#define DTH11_SENS_PIN    (13) 
#define LIGHT_SENS_PIN    (36)
#define LED12_PWM_PIN     (12)
#define TEMP_HUM_SENS_PIN (25)

/* Defined times for each task in ms */
#define TEMP_HUM_INTERVAL_5000_MS (5000)  
#define POT_INTERVAL_100_MS       (100)   
#define DISPLAY_INTERVAL_300_MS   (300)   

class Sensor {
private:
  uint8_t pin;
public:
  Sensor(uint8_t pin) : pin(pin) {
    pinMode(pin, INPUT);
  }

  virtual uint16_t readValue() = 0;

  uint8_t getPin() const {
    return pin;
  }

};

class VarResSensor : public Sensor{
private: 
  uint8_t maxValue;
public:
VarResSensor(uint8_t pin, uint8_t maxValue) : Sensor(pin), maxValue(maxValue) {}

  uint16_t readValue() override {
    uint16_t adc = analogRead(getPin());
    if( maxValue >= 100)  maxValue = 100;
    return map(adc, 0, 4095, 0, maxValue);
  }
};

class TemperatureHumiditySensor : public Sensor , public dth11Sensor {
private:
  uint8_t pin;
public:
  TemperatureHumiditySensor(uint8_t pin) : Sensor(pin), dth11Sensor(pin) {}

  uint16_t readValue() override {
    return dth11Sensor::dthReadTemp();
  }

  uint16_t readValueHumidity() {
    return dth11Sensor::dhtReadHum();
  }
};

class Actuator {
private: 
  uint8_t out_pin;
public:
  Actuator(uint8_t out_pin) {
    this->out_pin = out_pin;
    pinMode(out_pin, OUTPUT);
  }

  void SetLedIntensity(uint8_t intensity) {
    uint8_t pwm_val = map(intensity, 0, 100, 0, 255);
    analogWrite(out_pin, pwm_val);
    delay(5);
  }

  uint8_t getPin() const {
    return out_pin;
  }
};

/* Struct to store all sensor, actuator, and display-related data */
struct SystemData {
  VarResSensor* potSensor;
  TemperatureHumiditySensor* tempHumSensor;
  Actuator* led;
  OledDisplay* oledDisplay;
  uint16_t potValue;
  uint16_t temperature;
  uint16_t humidity;
};

void TaskReadSensors(void *pvParameters) {
  SystemData* data = (SystemData*) pvParameters;
  
  unsigned long lastPotRead = 0;
  unsigned long lastTempHumRead = 0;

  for (;;) {
      unsigned long currentMillis = millis();

      /* Read potentiometer every 1 sec */
      if (currentMillis - lastPotRead >= POT_INTERVAL_100_MS) {
          lastPotRead = currentMillis;
          data->potValue = data->potSensor->readValue();
      }

      /* Read temperature & humidity every 5 sec */
      if (currentMillis - lastTempHumRead >= TEMP_HUM_INTERVAL_5000_MS) {
          lastTempHumRead = currentMillis;
          data->temperature = data->tempHumSensor->readValue();
          data->humidity = data->tempHumSensor->readValueHumidity();
      }

      /** Prevent CPU overloading */
      vTaskDelay(pdMS_TO_TICKS(100));
  }
}

/* Task: Process sensor data */
void TaskProcessData(void* pvParameters) {
  SystemData* data = (SystemData*) pvParameters;
  for (;;) {
      // Delay to allow periodic processing
      vTaskDelay(pdMS_TO_TICKS(100)); 
  }
}

/* Task: Control actuators (adjust LED intensity) */
void TaskControlActuators(void* pvParameters) {
  SystemData* data = (SystemData*) pvParameters;

  for (;;) {
      data->led->SetLedIntensity(data->potValue);
      vTaskDelay(pdMS_TO_TICKS(100));  /* Update actuators every 100ms */
  }
}

/* Task: Update display with sensor data */
void TaskDisplay(void* pvParameters) {
  SystemData* data = (SystemData*) pvParameters;

  uint16_t prevPotValue = 0;
  uint16_t prevTemperature = 0;
  uint16_t prevHumidity = 0;

  for (;;) {
      // Check if any value has changed
      if (data->potValue != prevPotValue || data->temperature != prevTemperature ||  data->humidity != prevHumidity) {

          if (data->potValue != prevPotValue) {
              data->oledDisplay->SetdisplayData(0, 0, "Led Int: ");
              data->oledDisplay->SetdisplayData(75, 0, data->potValue);
              data->oledDisplay->SetdisplayData(100, 0, "%");
              prevPotValue = data->potValue;  // Update previous value
          }

          if (data->temperature != prevTemperature) {
              data->oledDisplay->SetdisplayData(0, 10, "Temperature: ");
              data->oledDisplay->SetdisplayData(75, 10, data->temperature);
              data->oledDisplay->SetdisplayData(100, 10, "C");
              prevTemperature = data->temperature;  // Update previous value
          }

          if (data->humidity != prevHumidity) {
              data->oledDisplay->SetdisplayData(0, 20, "Humidity: ");
              data->oledDisplay->SetdisplayData(75, 20, data->humidity);
              data->oledDisplay->SetdisplayData(100, 20, "%");
              prevHumidity = data->humidity;  // Update previous value
          }

          data->oledDisplay->PrintdisplayData();
      }

      vTaskDelay(pdMS_TO_TICKS(DISPLAY_INTERVAL_300_MS));  /* Refresh display every 300ms */
  }
}

void setup() {
  Serial.begin(9600);

  /* Allocate memory for struct dynamically */
  SystemData* systemData = new SystemData {
      new VarResSensor(LIGHT_SENS_PIN, 100),
      new TemperatureHumiditySensor(TEMP_HUM_SENS_PIN),
      new Actuator(LED12_PWM_PIN),
      new OledDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_ADDRESS),
      0, 0, 0  /* Initialize sensor values */
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