#include "globals.h"

// Инициализация глобальных переменных
bool powerOn = false;
bool handToolInHand = false;
uint32_t lastActivity = 0;

int16_t currentTemp = 0;
int16_t setTemp = 250;
int16_t rawAdc = 0;
uint8_t fanPercent = 60;

Preset presets[NUM_PRESETS] = {
    {250, 60},
    {300, 75},
    {200, 50}
};

// АГРЕССИВНЫЕ PID КОЭФФИЦИЕНТЫ ДЛЯ СКОРОСТИ И ТОЧНОСТИ
float Kp = 4.5f, Ki = 0.08f, Kd = 0.8f;
float pidLastError = 0;
float pidIntegral = 0;
uint8_t heaterOutput = 0;

volatile int8_t encoderDelta = 0;
volatile uint8_t encoderState = 0;

uint8_t menu = 0;
uint8_t editParam = 0;
int8_t activePreset = -1;
bool presetEditMode = false;

volatile uint8_t triacValue = 0;
volatile uint8_t zeroCrossCount = 0;

uint32_t encoderPressStart = 0;
uint32_t presetPressStart[NUM_PRESETS] = {0};
uint32_t tempUpPressStart = 0;
uint32_t tempDownPressStart = 0;
uint32_t fanUpPressStart = 0;
uint32_t fanDownPressStart = 0;
