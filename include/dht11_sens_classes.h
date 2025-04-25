#ifndef DHT11_SENS_CLASSES_H
#define DHT11_SENS_CLASSES_H

#include "../lib/DTH11/src/DHTesp.h"

class dth11Sensor {
private:
    DHTesp TempHumSens;
    uint8_t pin; 
public:
    dth11Sensor(uint8_t pin);

    void dhtSensorInit();
    float readTemperature(); 
    float readHumidity();    
};

#endif
