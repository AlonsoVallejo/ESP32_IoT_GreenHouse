#include <Arduino.h>
#include <iostream>
#include <vector>
#include <OledDisplay.h>

using namespace std;

#define TEMP_SENS_GPIO13 (13)
#define LIGHT_SENS_GPIO36 (36)

class Sensor {
private:
  uint8_t pin;
public:
  Sensor(uint8_t pin) : pin(pin) {}

  virtual uint16_t readValue() = 0;

  uint8_t getPin() const {
    return pin;
  }

};

class LightSensor : public Sensor{
private: 
  uint8_t maxValue;
public:
  LightSensor(uint8_t pin, uint8_t maxValue) : Sensor(pin), maxValue(maxValue) {}

  uint16_t readValue() override {
    uint16_t adc = analogRead(getPin());
    if( maxValue >= 100)  maxValue = 100;
    return map(adc, 0, 4095, 0, maxValue);
  }
};

void setup() {
  Serial.begin(115200);
}

void loop() {
  static LightSensor lightsens1(LIGHT_SENS_GPIO36, 100);
  static OledDisplay OledDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_ADDRESS);
  static bool init_setup = false; 
  
  if(!init_setup) {
    OledDisplay.init();
    OledDisplay.clear();
    OledDisplay.setTextProperties(1, SSD1306_WHITE);
    init_setup = true;
  }

  OledDisplay.SetdisplayData( 0, 0, "Light: ");
  OledDisplay.SetdisplayData(40, 0, lightsens1.readValue());
  OledDisplay.SetdisplayData( 60, 0, "%");
  OledDisplay.PrintdisplayData();
  OledDisplay.clear();
}