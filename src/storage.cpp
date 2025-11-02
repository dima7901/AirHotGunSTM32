#include "storage.h"
#include "globals.h"

// Адреса в EEPROM
#define EEPROM_PRESETS_ADDR 0

void storageSetup() {
    // EEPROM инициализируется автоматически
}

bool savePresets() {
    uint16_t addr = EEPROM_PRESETS_ADDR;
    
    // Сохраняем только пресеты
    for (int i = 0; i < NUM_PRESETS; i++) {
        EEPROM.put(addr, presets[i]);
        addr += sizeof(Preset);
    }
    
    return true;
}

bool loadPresets() {
    uint16_t addr = EEPROM_PRESETS_ADDR;
    
    // Загружаем только пресеты
    for (int i = 0; i < NUM_PRESETS; i++) {
        EEPROM.get(addr, presets[i]);
        addr += sizeof(Preset);
        
        // Проверка валидности данных
        if (presets[i].temp < MIN_SET_TEMP || presets[i].temp > MAX_TEMP) {
            presets[i].temp = 300 + i * 50;
        }
        if (presets[i].fan > 100) {
            presets[i].fan = 50 + i * 10;
        }
    }
    
    return true;
}

void resetToDefaults() {
    // Сброс пресетов к значениям по умолчанию
    Preset defaultPresets[NUM_PRESETS] = {
        {300, 50},
        {350, 60}, 
        {400, 70}
    };
    
    for (int i = 0; i < NUM_PRESETS; i++) {
        presets[i] = defaultPresets[i];
    }
    
    // Сохраняем defaults в EEPROM
    savePresets();
}
