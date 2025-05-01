#ifndef SENSOR_MGR_H
#define SENSOR_MGR_H

#include "Sensors_classes.h"

class SensorManager {
public:
    SensorManager(AnalogSensor* levelSensor, TemperatureHumiditySensor* tempHumSensor,
                  DigitalSensor* pirSensor, DigitalSensor* lightSensor, DigitalSensor* buttonSelector);

    void readLevelSensor();
    void readTemperatureHumiditySensor();
    void readPirSensor();
    void readLightSensor();
    void readButtonSelector();

    uint16_t getLevelSensorValue() const;
    double getTemperature() const;
    double getHumidity() const;
    bool getPirSensorValue() const;
    bool getLightSensorValue() const;
    bool getButtonSelectorValue() const;

    TemperatureHumiditySensor* getTempHumSensor() const;

private:
    AnalogSensor* levelSensor;
    TemperatureHumiditySensor* tempHumSensor;
    DigitalSensor* pirSensor;
    DigitalSensor* lightSensor;
    DigitalSensor* buttonSelector;

    uint16_t levelValue;
    double temperature;
    double humidity;
    bool pirValue;
    bool lightValue;
    bool buttonValue;
};

#endif // SENSOR_MGR_H