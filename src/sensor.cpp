#include "sensor.h"
#include "globals.h"

// БЫСТРЫЙ МЕДИАННЫЙ ФИЛЬТР (7 замеров вместо 15)
int16_t readMedianAdc(uint8_t n = 7) {
    static int16_t buf[7];
    for (int i = 0; i < n; i++) {
        buf[i] = analogRead(ADC_TEMP_PIN);
        delayMicroseconds(50); // Минимальная задержка между замерами
    }
    
    // БЫСТРАЯ СОРТИРОВКА ВСТАВКАМИ
    for (int i = 1; i < n; i++) {
        int16_t key = buf[i];
        int j = i - 1;
        while (j >= 0 && buf[j] > key) {
            buf[j + 1] = buf[j];
            j--;
        }
        buf[j + 1] = key;
    }
    return buf[n / 2];
}

// КАЛИБРОВОЧНАЯ ТАБЛИЦА ДЛЯ ТОЧНОСТИ
int16_t adcToTemp(int16_t adc) {
    if (adc >= MAX_SAFE_ADC) return MAX_TEMP;
    
    // ЛИНЕАРИЗАЦИЯ - заменить на реальную калибровку термопары
    static const int16_t tempTable[] = {
        20,  50, 100, 150, 200, 250, 300, 350, 380
    };
    static const int16_t adcTable[] = {
        0, 450, 950, 1450, 1950, 2450, 2950, 3450, 4000
    };
    
    for (int i = 0; i < 8; i++) {
        if (adc >= adcTable[i] && adc <= adcTable[i + 1]) {
            return map(adc, adcTable[i], adcTable[i + 1],
                      tempTable[i], tempTable[i + 1]);
        }
    }
    return MAX_TEMP;
}

void updateTemperature() {
    rawAdc = readMedianAdc();
    currentTemp = adcToTemp(rawAdc);
}

bool checkThermocoupleError() {
    return (rawAdc > MAX_SAFE_ADC);
}
