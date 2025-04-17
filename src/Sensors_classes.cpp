#include "Sensors_classes.h"

/**
 * @brief Constructor initializes the sensor pin.
 * @param pin Pin number where the sensor is connected.
 */
Sensor::Sensor(uint8_t pin) : pin(pin) {
    pinMode(pin, INPUT);
}

/**
 * @brief Get the pin number where the sensor is connected.
 * @return The pin number.
 */
uint8_t Sensor::getPin() const {
    return pin;
}

/**
 * @brief Constructor initializes the variable resistance sensor.
 * @param pin The input pin connected to the sensor.
 * @param maxValue The maximum mapped value (default should not exceed 100).
 */
VarResSensor::VarResSensor(uint8_t pin, uint8_t maxValue) : Sensor(pin), maxValue(maxValue) {}

/**
 * @brief Read the raw ADC value from the sensor.
 * @return ADC value between 0-4095.
 */
uint16_t VarResSensor::readRawValue() {
    return analogRead(getPin());
}

/**
 * @brief Scale the ADC value to a percentage (0-100%).
 * @return Scaled value based on ADC reading.
 */
uint8_t VarResSensor::getScaledResistance() {
    uint16_t adc = analogRead(getPin());
    if (maxValue > 100) maxValue = 100;
    return map(adc, 0, 4095, 0, maxValue);
}

/**
 * @brief Constructor initializes the temperature and humidity sensor.
 * @param pin The input pin connected to the sensor.
 */
TemperatureHumiditySensor::TemperatureHumiditySensor(uint8_t pin) : Sensor(pin), dth11Sensor(pin) {}

/**
 * @brief Read the temperature value.
 * @return Temperature in degrees Celsius.
 */
uint16_t TemperatureHumiditySensor::readRawValue() {
    return dth11Sensor::dthReadTemp();
}

/**
 * @brief Read the humidity value.
 * @return Humidity percentage.
 */
uint16_t TemperatureHumiditySensor::readValueHumidity() {
    return dth11Sensor::dhtReadHum();
}

/**
 * @brief Constructor initializes the LDR sensor.
 * @param pin The input pin connected to the sensor.
 */
LdrSensor::LdrSensor(uint8_t pin) : Sensor(pin) {}

/**
 * @brief Read raw LDR sensor state (digital output).
 * @return Digital value (HIGH or LOW).
 */
uint16_t LdrSensor::readRawValue() {
    return digitalRead(getPin());
}

/**
 * @brief Get the LDR sensor state.
 * @return LDR_STATE_LIGHT or LDR_STATE_DARK based on reading.
 */
LdrSensor::LRD_STATE_T LdrSensor::getLdrState() {
    return (LRD_STATE_T)readRawValue();
}
