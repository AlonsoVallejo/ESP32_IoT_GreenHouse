#include "Actuators_classes.h"

/**
 * @brief Constructor initializes the actuator pin as an output.
 * @param out_pin Pin number where the actuator is connected.
 */
Actuator::Actuator(uint8_t out_pin) : out_pin(out_pin) {
    pinMode(out_pin, OUTPUT);
}

/**
 * @brief Set PWM duty cycle for controlling actuator intensity.
 *        Maps the percentage value (0-100%) to PWM range (0-255).
 * @param intensity Percentage value for duty cycle (0 to 100).
 */
void Actuator::SetPwmDutyCycle(uint8_t intensity) {
    if (intensity >= 0 && intensity <= 100) {
        uint8_t pwm_val = map(intensity, 0, 100, 0, 255);
        analogWrite(out_pin, pwm_val);
    }
}

/**
 * @brief Set the actuator's output state (HIGH or LOW).
 * @param state 0 for LOW, 1 for HIGH.
 */
void Actuator::SetOutState(uint8_t state) {
    digitalWrite(out_pin, state ? HIGH : LOW);
}

/**
 * @brief Get the pin number where the actuator is connected.
 * @return The pin number.
 */
uint8_t Actuator::getPin() const {
    return out_pin;
}
