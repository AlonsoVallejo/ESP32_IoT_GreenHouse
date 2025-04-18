#ifndef ACTUATOR__CLASSES_H
#define ACTUATOR__CLASSES_H

#include <Arduino.h>

class Actuator {
private:
    uint8_t out_pin;
    uint8_t ActuatorState;
public:
    Actuator(uint8_t out_pin);
    void SetPwmDutyCycle(uint8_t dutycycle);
    void SetOutState(uint8_t state);
    uint8_t getOutstate() const;
    void setActuatorState(uint8_t state);
    uint8_t getPin() const;
};

#endif
