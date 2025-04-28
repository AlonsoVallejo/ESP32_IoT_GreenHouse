#include <Arduino.h>
#include <dht11_sens_classes.h>

/**
 * @brief Constructor for the DHT11 sensor wrapper class.
 * @param pin The input pin connected to the DHT11 sensor.
 */
dth11Sensor::dth11Sensor(uint8_t pin) : pin(pin) {}

/**
 * @brief Initializes the DHT11 sensor.
 * This method must be called before attempting to read temperature or humidity.
 */
void dth11Sensor::dhtSensorInit() {
    TempHumSens.setup(pin, DHTesp::DHT11); // Initialize the DHT11 sensor
}

/**
 * @brief Reads the temperature from the DHT11 sensor.
 * @return Temperature in degrees Celsius as a float. Returns a default value if the value is invalid.
 */
float dth11Sensor::readTemperature() {
    float temperature = TempHumSens.getTemperature();
    if (isnan(temperature)) {
        return -1.0; // Default value for invalid temperature
    }
    return temperature;
}

/**
 * @brief Reads the humidity from the DHT11 sensor.
 * @return Humidity percentage as a float. Returns a default value if the value is invalid.
 */
float dth11Sensor::readHumidity() {
    float humidity = TempHumSens.getHumidity();
    if (isnan(humidity)) {
        return -1.0; // Default value for invalid humidity
    }
    return humidity;
}
