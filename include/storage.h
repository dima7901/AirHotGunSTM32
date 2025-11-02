#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include <EEPROM.h>

void storageSetup();
bool savePresets();  // Сохраняет только пресеты
bool loadPresets();  // Загружает только пресеты
void resetToDefaults();

#endif