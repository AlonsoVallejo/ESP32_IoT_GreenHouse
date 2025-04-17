#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>
#include "dht11_sens.h"

class Sensor {
protected:
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
    uint16_t getSensorValue() const;
};

class TemperatureHumiditySensor : public Sensor, public dth11Sensor {
private:
    double temperature; 
    double humidity;    
public:
    TemperatureHumiditySensor(uint8_t pin);
    uint16_t readRawValue() override;
    double readValueTemperature();
    double readValueHumidity();
    double getTemperature() const;
    double getHumidity() const;
};

class DigitalSensor : public Sensor {
private:
    uint8_t SensorState;
public:
    DigitalSensor(uint8_t pin);
    uint16_t readRawValue() override;
    uint8_t getSensorValue() const;
};

#endif
