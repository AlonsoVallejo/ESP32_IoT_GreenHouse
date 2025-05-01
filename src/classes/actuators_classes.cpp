#include "Actuators_classes.h"

/**
 * @brief Constructor initializes the actuator pin as an output.
 * @param out_pin Pin number where the actuator is connected.
 */
Actuator::Actuator(uint8_t out_pin) : out_pin(out_pin), ActuatorState(0) {
    Serial.println("Initializing Actuator on pin: " + String(out_pin));
    pinMode(out_pin, OUTPUT);
}

/**
 * @brief Set PWM duty cycle to control the actuator's intensity.
 * @param dutycycle Percentage value for duty cycle (0 to 100).
 */
void Actuator::SetPwmDutyCycle(uint8_t dutycycle) {
    if (dutycycle >= 0 && dutycycle <= 100) {
        uint8_t pwm_val = map(dutycycle, 0, 100, 0, 255);
        analogWrite(out_pin, pwm_val); // Writes PWM signal to actuator
    }
}

/**
 * @brief Update the internal state of the actuator.
 * @param state 0 for LOW, 1 for HIGH.
 */
void Actuator::SetOutState(uint8_t state) {
    ActuatorState = state; // Stores the actuator state internally
}

/**
 * @brief Get the last stored state of the actuator.
 * @return The internally stored actuator state (0 or 1).
 */
uint8_t Actuator::getOutstate() const {
    return ActuatorState; // Retrieves the stored state
}

/**
 * @brief Set the actuator state and update the GPIO output.
 * @param state The state to set (0 or 1).
 */
void Actuator::setActuatorState(uint8_t state) {
    digitalWrite(out_pin, state ? HIGH : LOW); // Updates GPIO output
}

/**
 * @brief Get the pin number where the actuator is connected.
 * @return The pin number assigned to the actuator.
 */
uint8_t Actuator::getPin() const {
    return out_pin;
}
