#include "storage.h"

// Временное решение с RAM - заменить на Flash
static struct SettingsData {
    Preset presets[NUM_PRESETS];
    float Kp, Ki, Kd;
} ramSettings;

void storageSetup() {
    // Загрузка при старте
    loadSettings();
}

bool saveSettings() {
    // Сохраняем в RAM
    for (int i = 0; i < NUM_PRESETS; i++) {
        ramSettings.presets[i] = presets[i];
    }
    ramSettings.Kp = Kp;
    ramSettings.Ki = Ki;
    ramSettings.Kd = Kd;
    return true;
}

bool loadSettings() {
    // Загружаем из RAM (при первом запуске - сброс к заводским)
    static bool firstLoad = true;
    if (firstLoad) {
        firstLoad = false;
        resetToDefaults();
        return false;
    }
    
    for (int i = 0; i < NUM_PRESETS; i++) {
        presets[i] = ramSettings.presets[i];
    }
    Kp = ramSettings.Kp;
    Ki = ramSettings.Ki;
    Kd = ramSettings.Kd;
    return true;
}

void resetToDefaults() {
    Preset defaultPresets[NUM_PRESETS] = {
        {250, 60},
        {300, 75},
        {200, 50}
    };
    
    for (int i = 0; i < NUM_PRESETS; i++) {
        presets[i] = defaultPresets[i];
    }
    
    // АГРЕССИВНЫЕ КОЭФФИЦИЕНТЫ ПО УМОЛЧАНИЮ
    Kp = 4.5f;
    Ki = 0.08f;
    Kd = 0.8f;
    
    saveSettings();
}
