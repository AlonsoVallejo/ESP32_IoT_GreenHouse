#include "ActuatorMgr.h"

/**
 * @brief Constructs the ActuatorManager object and initializes all actuators.
 * @param ledIndicator Pointer to the Actuator object for the LED indicator.
 * @param irrigator Pointer to the Actuator object for the irrigator.
 * @param pump Pointer to the Actuator object for the pump.
 * @param lamp Pointer to the Actuator object for the lamp.
 */
ActuatorManager::ActuatorManager(Actuator* ledIndicator, Actuator* irrigator, Actuator* pump, Actuator* lamp)
    : ledIndicator(ledIndicator), irrigator(irrigator), pump(pump), lamp(lamp) {}

/**
 * @brief Resets the LED indicator to LOW state.
 */
void ActuatorManager::resetLedIndicator() {
    ledIndicator->SetOutState(LOW); // Update internal state
}

/**
 * @brief Toggles the LED indicator state.
 * @param state The desired state (true for HIGH, false for LOW).
 */
void ActuatorManager::setLedIndicator(bool state) {
    ledIndicator->SetOutState(state);
}

/**
 * @brief Sets the state of the irrigator.
 * @param state The desired state (true for HIGH, false for LOW).
 */
void ActuatorManager::setIrrigatorState(bool state) {
    irrigator->SetOutState(state); // Update internal state
}

/**
 * @brief Sets the state of the pump.
 * @param state The desired state (true for HIGH, false for LOW).
 */
void ActuatorManager::setPumpState(bool state) {
    pump->SetOutState(state); // Update internal state
}

/**
 * @brief Sets the state of the lamp.
 * @param state The desired state (true for HIGH, false for LOW).
 */
void ActuatorManager::setLampState(bool state) {
    lamp->SetOutState(state); // Update internal state
}

/**
 * @brief Applies the internal states of all actuators to the hardware outputs.
 */
void ActuatorManager::applyState() {
    ledIndicator->setActuatorState(ledIndicator->getOutstate());
    irrigator->setActuatorState(irrigator->getOutstate());
    pump->setActuatorState(pump->getOutstate());
    lamp->setActuatorState(lamp->getOutstate());
}

/**
 * @brief Gets the LED indicator actuator.
 * @return Pointer to the LED indicator actuator.
 */
Actuator* ActuatorManager::getLedIndicator() const {
    return ledIndicator;
}

/**
 * @brief Gets the irrigator actuator.
 * @return Pointer to the irrigator actuator.
 */
Actuator* ActuatorManager::getIrrigator() const {
    return irrigator;
}

/**
 * @brief Gets the pump actuator.
 * @return Pointer to the pump actuator.
 */
Actuator* ActuatorManager::getPump() const {
    return pump;
}

/**
 * @brief Gets the lamp actuator.
 * @return Pointer to the lamp actuator.
 */
Actuator* ActuatorManager::getLamp() const {
    return lamp;
}