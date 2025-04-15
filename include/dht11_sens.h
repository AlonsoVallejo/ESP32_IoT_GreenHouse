#ifndef DHT11_SENS_H  // Prevent multiple inclusions
#define DHT11_SENS_H

#include <DHT.h>

class dth11Sensor {
private:
    DHT TempHumSens;
public:
    dth11Sensor(uint8_t pin);

    void dhtSensorInit();
    uint8_t dthReadTemp();
    uint8_t dhtReadHum();
};

#endif
