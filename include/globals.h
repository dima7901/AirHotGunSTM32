#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include "config.h"

// === Состояния системы ===
extern bool systemPowered;
extern bool heatingActive;
extern bool fanRunning;
extern bool gunInHand;
extern bool inMenuMode;
extern bool coolingProcess;
extern bool powerLedBlinking;
extern bool inMenuMode;    // В меню настроек
extern bool updateDisplay;   
extern bool heatingThisCycle;     // Состояние нагрева    

// === Режимы управления ===
extern int activeParameter;      // 0=Температура, 1=Воздух
extern int editingPreset;        // -1=нет, 0-2=редактируемый пресет
extern int menuPosition;         // Позиция в меню

// === Параметры ===
extern int currentTemperature;
extern int targetTemperature;
extern int targetAirFlow;
extern int pidPowerOutput;

// === Пресеты ===
extern Preset userPresets[3];

// === Таймеры ===
extern unsigned long lastActivityTime;
extern unsigned long coolingStartTime;

// === Энкодер ===
extern int encLastStateA;
extern long encoderPosition;

// === Функции ===
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
void handleZeroCrossing();        // Прерывание детектора нуля
void triggerHeater();             // Включение нагрева

#endif
