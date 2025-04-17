#ifndef ACTUATOR_H
#define ACTUATOR_H

#include <Arduino.h>

class Actuator {
private:
    uint8_t out_pin; 
public:
    Actuator(uint8_t out_pin);
    void SetPwmDutyCycle(uint8_t dutycycle);
    void SetOutState(uint8_t state);
    uint8_t getPin() const;
};

#endif
