#ifndef SENSOR_MGR_H
#define SENSOR_MGR_H

#include "Sensors_classes.h"

class SensorManager {
public:
    SensorManager(AnalogSensor* levelSensor, 
                  Dht11TempHumSens* tempHumSensor, 
                  DigitalSensor* pirSensor, 
                  DigitalSensor* lightSensor, 
                  DigitalSensor* buttonSelector, 
                  DigitalSensor* buttonEsc, 
                  DigitalSensor* buttonUp, 
                  DigitalSensor* buttonDown);

    void readLevelSensor();
    void readDht11TempHumSens();
    void readPirSensor();
    void readLightSensor();
    void readButtonSelector();
    void readButtonEsc();
    void readButtonUp();
    void readButtonDown();

    uint16_t getLevelSensorValue() const;
    double getTemperature() const;
    double getHumidity() const;
    bool getPirSensorValue() const;
    bool getLightSensorValue() const;
    bool getButtonSelectorValue() const;
    bool getButtonEscValue() const;
    bool getButtonUpValue() const;  
    bool getButtonDownValue() const;

    Dht11TempHumSens* getTempHumSensor() const;

private:
    AnalogSensor* levelSensor;
    Dht11TempHumSens* tempHumSensor;
    DigitalSensor* pirSensor;
    DigitalSensor* lightSensor;
    DigitalSensor* buttonSelector;
    DigitalSensor* buttonEsc;
    DigitalSensor* buttonUp;
    DigitalSensor* buttonDown;

    uint16_t levelValue;
    double temperature;
    double humidity;
    bool pirValue;
    bool lightValue;
    bool buttonValue;
    bool buttonEscValue;
    bool buttonUpValue;
    bool buttonDownValue;
    bool buttonSelectorValue;
};

#endif // SENSOR_MGR_H