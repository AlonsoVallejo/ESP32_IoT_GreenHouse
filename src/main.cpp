#include <Arduino.h>
#include "OledDisplay.h"
#include "dht11_sens.h"

using namespace std;

#define DTH11_SENS_PIN    (13)
#define LIGHT_SENS_PIN    (36)
#define LED12_PWM_PIN     (12)
#define TEMP_HUM_SENS_PIN (25)

#define TEMP_HUM_INTERVAL (5000)  /* Temperature & humidity update every 5 sec */ 
#define POT_INTERVAL      (100)   /* Potentiometer update every 100 ms */
#define DISPLAY_INTERVAL  (300)   /* Display refresh every 500 ms */ 

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

struct TaskData {
  VarResSensor* potSensor;
  OledDisplay* oledDisplay;
  Actuator* led;
  TemperatureHumiditySensor* tempHumSensor;
  uint16_t potValue;
  uint16_t temperature;
  uint16_t humidity;
};

void TaskPotentiometer(void* pvParameters) {
  TaskData* data = (TaskData*) pvParameters;
  for (;;) {
      data->potValue = data->potSensor->readValue();
      data->led->SetLedIntensity(data->potValue);
      vTaskDelay(pdMS_TO_TICKS(POT_INTERVAL));
  }
}

void TaskTempHum(void* pvParameters) {
  TaskData* data = (TaskData*) pvParameters;
  for (;;) {
      data->temperature = data->tempHumSensor->readValue();
      data->humidity = data->tempHumSensor->readValueHumidity();
      vTaskDelay(pdMS_TO_TICKS(TEMP_HUM_INTERVAL));
  }
}

void TaskDisplay(void* pvParameters) {
  TaskData* data = (TaskData*) pvParameters;
  for (;;) {
      data->oledDisplay->clear();

      data->oledDisplay->SetdisplayData(0, 0, "Led Int: ");
      data->oledDisplay->SetdisplayData(50, 0, data->potValue);
      data->oledDisplay->SetdisplayData(70, 0, "%");

      data->oledDisplay->SetdisplayData(0, 10, "Temperature: ");
      data->oledDisplay->SetdisplayData(75, 10, data->temperature);
      data->oledDisplay->SetdisplayData(90, 10, "C");

      data->oledDisplay->SetdisplayData(0, 20, "Humidity: ");
      data->oledDisplay->SetdisplayData(75, 20, data->humidity);
      data->oledDisplay->SetdisplayData(90, 20, "%");

      data->oledDisplay->PrintdisplayData();
      vTaskDelay(pdMS_TO_TICKS(DISPLAY_INTERVAL));
  }
}

void setup() {
  Serial.begin(9600);

  TaskData* taskData = new TaskData {
      new VarResSensor(LIGHT_SENS_PIN, 100),
      new OledDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_ADDRESS),
      new Actuator(LED12_PWM_PIN),
      new TemperatureHumiditySensor(TEMP_HUM_SENS_PIN),
      0, 0, 0 // Initial sensor values
  };

  taskData->oledDisplay->init();
  taskData->oledDisplay->clear();
  taskData->oledDisplay->setTextProperties(1, SSD1306_WHITE);
  taskData->tempHumSensor->dhtSensorInit();

  xTaskCreate(TaskPotentiometer, "Potentiometer", 2048, taskData, 1, NULL);
  xTaskCreate(TaskTempHum, "TempHum", 2048, taskData, 1, NULL);
  xTaskCreate(TaskDisplay, "Display", 2048, taskData, 1, NULL);
}

void loop() {
  
}

