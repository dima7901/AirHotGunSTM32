#ifndef PINS_H
#define PINS_H

// Кнопки
#define BTN_PRESET1     PA0
#define BTN_PRESET2     PA1
#define BTN_PRESET3     PA2
#define BTN_TEMP_UP     PA3
#define BTN_TEMP_DOWN   PA4
#define BTN_FLOW_UP     PA5
#define BTN_FLOW_DOWN   PA6
#define BTN_POWER       PA7

// Энкодер
#define ENC_A           PB0
#define ENC_B           PB1
#define ENC_SW          PB2

// Реле и управление
#define RELAY_POWER     PB10
#define RELAY_HEATER    PC14  // Исправленный пин!
#define LED_POWER       PB12
#define LED_HEATER      PB13
#define BUZZER          PB14
#define FAN_PWM         PA8

// Датчики
#define THERMOCOUPLE_ADC     PA9  // Термопара K-типа
#define ZERO_CROSS_DETECTOR  PB5  // Детектор нуля для прерывания
#define HALL_SENSOR     PA10


#endif
