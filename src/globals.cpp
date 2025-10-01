#include "globals.h"

// === Состояния системы ===
bool systemPowered = false;
bool heatingActive = false;
bool fanRunning = false;
bool gunInHand = false;
bool inMenuMode = false;
bool coolingProcess = false;
bool powerLedBlinking = false;
bool updateDisplay = true;
bool heatingThisCycle = false;

// === Режимы управления ===
int activeParameter = 0;      // 0=Температура, 1=Воздух
int editingPreset = -1;       // -1=не редактируем пресет
int menuPosition = 0;         // Позиция в меню

// === Параметры ===
int currentTemperature = 25;
int targetTemperature = 300;
int targetAirFlow = 50;
int pidPowerOutput = 0;

// === Пресеты ===
Preset userPresets[3] = {
    {300, 50},
    {350, 70}, 
    {400, 80}
};

// === Таймеры ===
unsigned long lastActivityTime = 0;
unsigned long coolingStartTime = 0;

// === Энкодер ===
int encLastStateA = 0;
long encoderPosition = 0;