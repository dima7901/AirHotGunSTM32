#ifndef CONFIG_H
#define CONFIG_H

#include "pins.h"

// Настройки температуры
#define TEMP_MIN 100
#define TEMP_MAX 500
#define FLOW_MIN 0
#define FLOW_MAX 100
#define TEMP_STEP 5
#define FLOW_STEP 5

// Защита и таймауты
#define COOLDOWN_TEMP 80
#define AUTO_OFF_TIMEOUT (10 * 60 * 1000) // 10 минут

// Зуммер
#define BUZZER_ON 1500
#define BUZZER_OFF 800
#define BUZZER_PICKUP 1200
#define BUZZER_RETURN 600
#define BUZZER_MENU 3000

// Пресеты по умолчанию
struct Preset {
    int temperature;
    int airflow;
};

const Preset DEFAULT_PRESETS[3] = {
    {300, 50},
    {350, 70},
    {400, 80}
};

#endif
