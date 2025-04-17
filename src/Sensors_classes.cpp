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
 * @brief . Get the voltage value from the ADC reading.
 * @return Voltage value based on ADC reading.
 */
double VarResSensor::getVoltage() {
  uint16_t adc = analogRead(getPin());
  return (adc * 3.3) / 4095.0; // Assuming a 3.3V reference voltage
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
    return 0xFFFF; /* To complain base class */
}

/**
 * @brief Read the humidity value.
 * @return Humidity percentage.
 */
double TemperatureHumiditySensor::readValueHumidity() {
    return dth11Sensor::dhtReadHum();
}

/**
 * @brief Read the temperature value.
 * @return Temperature in degrees Celsius.
 */
double TemperatureHumiditySensor::readValueTemperature() {
  return dth11Sensor::dthReadTemp();
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
