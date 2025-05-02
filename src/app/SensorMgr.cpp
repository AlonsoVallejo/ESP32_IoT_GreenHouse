#include "SensorMgr.h"

/**
 * @brief Constructs the SensorManager object and initializes all sensors.
 * @param levelSensor Pointer to the AnalogSensor object for the level sensor.
 * @param tempHumSensor Pointer to the Dht11TempHumSens object.
 * @param pirSensor Pointer to the DigitalSensor object for the PIR sensor.
 * @param lightSensor Pointer to the DigitalSensor object for the light sensor.
 * @param buttonSelector Pointer to the DigitalSensor object for the button selector.
 */
SensorManager::SensorManager(AnalogSensor* levelSensor, Dht11TempHumSens* tempHumSensor,
                             DigitalSensor* pirSensor, DigitalSensor* lightSensor, DigitalSensor* buttonSelector)
    : levelSensor(levelSensor), tempHumSensor(tempHumSensor), pirSensor(pirSensor),
      lightSensor(lightSensor), buttonSelector(buttonSelector) {}

/**
 * @brief Reads the level sensor and updates the internal value.
 */
void SensorManager::readLevelSensor() {
    levelValue = levelSensor->readRawValue();
}

/**
 * @brief Reads the temperature and humidity sensors and updates the internal values.
 */
void SensorManager::readDht11TempHumSens() {
    temperature = tempHumSensor->readValueTemperature();
    humidity = tempHumSensor->readValueHumidity();
}

/**
 * @brief Reads the PIR sensor and updates the internal value.
 */
void SensorManager::readPirSensor() {
    pirValue = pirSensor->readRawValue();
}

/**
 * @brief Reads the light sensor and updates the internal value.
 */
void SensorManager::readLightSensor() {
    lightValue = lightSensor->readRawValue();
}

/**
 * @brief Reads the button selector and updates the internal value.
 */
void SensorManager::readButtonSelector() {
    buttonValue = buttonSelector->readRawValue();
}

/**
 * @brief Gets the last recorded value of the level sensor.
 * @return The raw ADC value of the level sensor.
 */
uint16_t SensorManager::getLevelSensorValue() const {
    return levelValue;
}

/**
 * @brief Gets the last recorded temperature value.
 * @return The temperature in degrees Celsius.
 */
double SensorManager::getTemperature() const {
    return temperature;
}

/**
 * @brief Gets the last recorded humidity value.
 * @return The humidity percentage.
 */
double SensorManager::getHumidity() const {
    return humidity;
}

/**
 * @brief Gets the last recorded value of the PIR sensor.
 * @return The digital state of the PIR sensor (true for HIGH, false for LOW).
 */
bool SensorManager::getPirSensorValue() const {
    return pirValue;
}

/**
 * @brief Gets the last recorded value of the light sensor.
 * @return The digital state of the light sensor (true for HIGH, false for LOW).
 */
bool SensorManager::getLightSensorValue() const {
    return lightValue;
}

/**
 * @brief Gets the last recorded value of the button selector.
 * @return The digital state of the button selector (true for HIGH, false for LOW).
 */
bool SensorManager::getButtonSelectorValue() const {
    return buttonValue;
}

/**
 * @brief Gets the pointer to the Dht11TempHumSens object.
 * @return Pointer to the Dht11TempHumSens object.
 */
Dht11TempHumSens* SensorManager::getTempHumSensor() const {
    return tempHumSensor;
}