#ifndef EEPROM_H
#define EEPROM_H

#include <Arduino.h>
#include "config.h"

void initEEPROM();
void loadSettings();
void saveSettings();
void savePresetsToEEPROM();
void loadPresetsFromEEPROM();

#endif
