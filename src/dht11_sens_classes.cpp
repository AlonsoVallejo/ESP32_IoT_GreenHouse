#include <Arduino.h>
#include <dht11_sens_classes.h>

dth11Sensor::dth11Sensor(uint8_t pin) : TempHumSens(pin, DHT11) {}

void dth11Sensor::dhtSensorInit() {
    TempHumSens.begin();
}

double dth11Sensor::dthReadTemp(){
    return (double)TempHumSens.readTemperature();
}

double dth11Sensor::dhtReadHum(){
    return (double)TempHumSens.readHumidity();
}
