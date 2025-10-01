#ifndef CONTROL_H
#define CONTROL_H

#include <Arduino.h>

// Функции управления
void handlePower();
void handleGunSensor();
void handleEncoder();
void handleButtons();
void handlePresets();
void updateHeater();
void updateFan();
void checkAutoOff();
void handleCooling();
void playBuzzer(int freq, int duration);

#endif
