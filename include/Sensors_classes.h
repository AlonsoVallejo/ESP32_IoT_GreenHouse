#ifndef SENSOR__CLASSES_H
#define SENSOR__CLASSES_H

#include <Arduino.h>
#include "../lib/DTH11/src/DHTesp.h"

class Sensor {
private:
    uint8_t pin; 
public:
    Sensor(uint8_t pin);
    virtual uint16_t readRawValue() = 0;
    uint8_t getPin() const;
};

class AnalogSensor : public Sensor {
private:
    uint16_t AdcValue; 
public:
    AnalogSensor(uint8_t pin);
    uint16_t readRawValue() override;
    double getVoltage();
};

class Dht11TempHumSens : public Sensor {
private:
    DHTesp TempHumSens; 
    double temperature; 
    double humidity;    
public:
    Dht11TempHumSens(uint8_t pin);
    uint16_t readRawValue() override;
    double readValueTemperature();
    double readValueHumidity();
    double getTemperature() const;
    double getHumidity() const;
    void dhtSensorInit();
};

class DigitalSensor : public Sensor {
private:
    uint8_t SensorState;
public:
    DigitalSensor(uint8_t pin);
    uint16_t readRawValue() override;
};

#endif
