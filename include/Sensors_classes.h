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

class VarResSensor : public Sensor {
private:
    uint8_t maxValue;
public:
    VarResSensor(uint8_t pin, uint8_t maxValue);
    uint16_t readRawValue() override;
    uint8_t getScaledResistance();
};

class TemperatureHumiditySensor : public Sensor, public dth11Sensor {
public:
    TemperatureHumiditySensor(uint8_t pin);
    uint16_t readRawValue() override;
    uint16_t readValueHumidity();
};

class LdrSensor : public Sensor {
public:
    enum LRD_STATE_T { LDR_STATE_LIGHT, LDR_STATE_DARK };
    LdrSensor(uint8_t pin);
    uint16_t readRawValue() override;
    LRD_STATE_T getLdrState();
};

#endif
