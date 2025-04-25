#include <Arduino.h>
#include <dht11_sens_classes.h>

/**
 * @brief Constructor for the DHT11 sensor wrapper class.
 * @param pin The input pin connected to the DHT11 sensor.
 */
dth11Sensor::dth11Sensor(uint8_t pin) : TempHumSens(pin, DHT11) {}

/**
 * @brief Initializes the DHT11 sensor.
 * This method must be called before attempting to read temperature or humidity.
 */
void dth11Sensor::dhtSensorInit() {
    TempHumSens.begin();
}

/**
 * @brief Reads the temperature from the DHT11 sensor.
 * Protects against invalid (NaN) values.
 * @return Temperature in degrees Celsius as a double. Returns -999.0 if the value is invalid.
 */
double dth11Sensor::dthReadTemp() {
    double temperature = (double)TempHumSens.readTemperature();
    if (isnan(temperature)) {
        return -999.0; // Return an error value
    }
    return temperature;
}

/**
 * @brief Reads the humidity from the DHT11 sensor.
 * @return Humidity percentage as a double. Returns -999.0 if the value is invalid.
 */
double dth11Sensor::dhtReadHum() {
    double humidity = (double)TempHumSens.readHumidity();
    if (isnan(humidity)) {
        return -999.0; // Return an error value
    }
    return humidity;
}
