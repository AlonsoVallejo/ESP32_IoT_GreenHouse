#ifndef ESP32_SHIELD_H
#define ESP32_SHIELD_H

// Optocoupled Inputs
#define SHIELD_OPTOIN1_D26    (26)
#define SHIELD_OPTOIN2_D27    (27)

// Relay Outputs
#define SHIELD_RELAY1_D4      (4)
#define SHIELD_RELAY2_D2      (2)

// Push Buttons
#define SHIELD_PUSHB1_D33     (33)
#define SHIELD_PUSHB2_D35     (35)
#define SHIELD_PUSHB3_D34     (34)
#define SHIELD_PUSHB4_D32     (32)

// Buzzer
#define SHIELD_BUZZER_D15     (15)

// Potentiometer
#define SHIELD_POTENTIOMETER_VP  (36) // VP corresponds to GPIO36 on ESP32

// Temperature Sensor (NTC 10K)
#define SHIELD_TEMP_SENSOR_VN (39) // VN corresponds to GPIO39 on ESP32

// MOSFET Outputs
#define SHIELD_MOSFET1_D23    (23)
#define SHIELD_MOSFET2_D19    (19)

// AC Dimmer
#define SHIELD_DIMMER_ZC_D5   (5)
#define SHIELD_DIMMER_TRIAC_D18  (18)

// User LEDs
#define SHIELD_LED1_D15       (15)
#define SHIELD_LED2_D13       (13)
#define SHIELD_LED3_D12       (12)
#define SHIELD_LED4_D14       (14)

// Real-Time Clock (DS1307)
#define SHIELD_RTC_SCL_D22    (22)
#define SHIELD_RTC_SDA_D21    (21)
#define SHIELD_RTC_SOUT_D32   (32)

// OLED Display (I2C)
#define SHIELD_OLED_SCL_D22   (22)
#define SHIELD_OLED_SDA_D21   (21)

// General I2C Header
#define SHIELD_I2C_SCL_D22    (22)
#define SHIELD_I2C_SDA_D21    (21)

// Serial Communication (COM Header)
#define SHIELD_COM_RX2_D16    (16)
#define SHIELD_COM_TX2_D17    (17)

// DAC1 Output
#define SHIELD_DAC1_D25       (25)

// DHT11 Sensor
#define SHIELD_DHT11_D13      (13)

#endif