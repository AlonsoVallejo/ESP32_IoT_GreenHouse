#include "Sensors_classes.h"

/**
 * @brief Initializes the sensor pin as an input.
 * @param pin Pin number where the sensor is connected.
 */
Sensor::Sensor(uint8_t pin) : pin(pin) {
    pinMode(pin, INPUT);
}

/**
 * @brief Retrieves the pin number of the sensor.
 * @return The pin number.
 */
uint8_t Sensor::getPin() const {
    return pin;
}

/**
 * @brief Initializes an analog sensor.
 * @param pin The input pin connected to the sensor.
 */
AnalogSensor::AnalogSensor(uint8_t pin) : Sensor(pin), AdcValue(0) {}

/**
 * @brief Reads the raw ADC value from the sensor.
 * @return ADC value between 0-4095.
 */
uint16_t AnalogSensor::readRawValue() {
    AdcValue = analogRead(getPin());
    return AdcValue;
}

/**
 * @brief Converts the ADC reading to voltage.
 * @return Corresponding voltage value.
 */
double AnalogSensor::getVoltage() {
    return (AdcValue * 3.3) / 4095.0;
}

/**
 * @brief Gets the last recorded sensor value.
 * @return ADC value.
 */
uint16_t AnalogSensor::getSensorValue() const {
    return AdcValue;
}

/**
 * @brief Initializes a temperature and humidity sensor.
 * @param pin The input pin connected to the sensor.
 */
TemperatureHumiditySensor::TemperatureHumiditySensor(uint8_t pin) 
    : Sensor(pin), dth11Sensor(pin), temperature(0), humidity(0) {}

/**
 * @brief Reads the sensor but returns a default value (base class compliance).
 * @return Placeholder value.
 */
uint16_t TemperatureHumiditySensor::readRawValue() {
    return 0xFFFF;
}

/**
 * @brief Reads the humidity measurement from the sensor.
 * @return Humidity percentage.
 */
double TemperatureHumiditySensor::readValueHumidity() {
    humidity = dth11Sensor::dhtReadHum();
    return humidity;
}

/**
 * @brief Reads the temperature measurement from the sensor.
 * @return Temperature in degrees Celsius.
 */
double TemperatureHumiditySensor::readValueTemperature() {
    temperature = dth11Sensor::dthReadTemp();
    return temperature;
}

/**
 * @brief Retrieves the last recorded temperature value.
 * @return Temperature in degrees Celsius.
 */
double TemperatureHumiditySensor::getTemperature() const {
    return temperature;
}

/**
 * @brief Retrieves the last recorded humidity value.
 * @return Humidity percentage.
 */
double TemperatureHumiditySensor::getHumidity() const {
    return humidity;
}

/**
 * @brief Initializes a digital sensor.
 * @param pin The input pin connected to the sensor.
 */
DigitalSensor::DigitalSensor(uint8_t pin) : Sensor(pin), SensorState(0) {}

/**
 * @brief Reads the digital state of the sensor.
 * @return Digital value (HIGH or LOW).
 */
uint16_t DigitalSensor::readRawValue() {
    SensorState = digitalRead(getPin());
    return SensorState;
}

/**
 * @brief Retrieves the last recorded digital state.
 * @return Digital state (HIGH or LOW).
 */
uint8_t DigitalSensor::getSensorValue() const {
    return SensorState;
}