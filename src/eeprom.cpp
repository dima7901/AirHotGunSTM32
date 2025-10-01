#include <Arduino.h>
#include "globals.h"
#include "eeprom.h"

void initEEPROM() {
    // Ничего не делаем
    Serial.println("EEPROM: Initialized (stub)");
}

void loadSettings() {
    loadPresetsFromEEPROM();
}

void saveSettings() {
    savePresetsToEEPROM();
}

void savePresetsToEEPROM() {
    // Временно ничего не сохраняем
    Serial.println("EEPROM: Save presets (stub)");
}

void loadPresetsFromEEPROM() {
    // Всегда загружаем значения по умолчанию
    for (int i = 0; i < 3; i++) {
        userPresets[i] = DEFAULT_PRESETS[i];
    }
    Serial.println("EEPROM: Loaded default presets");
}