#include <Arduino.h>
#include <dht11_sens.h>

dth11Sensor::dth11Sensor(uint8_t pin) : TempHumSens(pin, DHT11) {}

void dth11Sensor::dhtSensorInit() {
    TempHumSens.begin();
}

uint8_t dth11Sensor::dthReadTemp(){
    return (uint8_t)TempHumSens.readTemperature();
}

uint8_t dth11Sensor::dhtReadHum(){
    return (uint8_t)TempHumSens.readHumidity();
}
