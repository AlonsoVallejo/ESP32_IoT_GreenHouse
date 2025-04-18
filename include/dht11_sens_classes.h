#ifndef DHT11_SENS_CLASSES_H  // Prevent multiple inclusions
#define DHT11_SENS_CLASSES_H

#include <DHT.h>

class dth11Sensor {
private:
    DHT TempHumSens;
public:
    dth11Sensor(uint8_t pin);

    void dhtSensorInit();
    double dthReadTemp();
    double dhtReadHum();
};

#endif
