#include "sensor.h"
#include "globals.h"
#include "nextion.h"
#include <math.h>

// --- Калибровка ---
// Используем реальную точку: 2710 АЦП = 350°C (проверено по термопаре)
// Остальные точки рассчитаны линейно для диапазона 50–450°C
// Если нужно — потом уточни реальные АЦП, я помогу пересчитать

int16_t calPoints[][2] = {
    {155,   20}, 
    {384,   50},    // низ (~50°C)
    {1548,  200},   // средняя (~200°C)
    {2322,  300},   // ближе к 300°C (по старой формуле)
    {2709,  350},   // твоя реальная калибровка
    {3483,  450},
    {3870,  500}   // верхний предел (~450°C)
}; 

// --- Быстрый медианный фильтр для стабильности показаний ---
int16_t readMedianAdc(uint8_t n = 7) {
    static int16_t buf[7];
    for (int i = 0; i < n; i++) {
        buf[i] = analogRead(ADC_TEMP_PIN);
        delayMicroseconds(50);
    }

    // сортировка вставками
    for (int i = 1; i < n; i++) {
        int16_t key = buf[i];
        int j = i - 1;
        while (j >= 0 && buf[j] > key) {
            buf[j + 1] = buf[j];
            j--;
        }
        buf[j + 1] = key;
    }

    return buf[n / 2]; // медиана
}

// --- Интерполяция по калибровочным точкам ---
int16_t adcToTemp(int16_t adc) {
    const uint8_t nPoints = sizeof(calPoints) / sizeof(calPoints[0]);

    // Защита от выхода за диапазон
    if (adc <= calPoints[0][0])
        return calPoints[0][1];
    if (adc >= calPoints[nPoints - 1][0])
        return calPoints[nPoints - 1][1];

    // Поиск диапазона для интерполяции
    for (int i = 0; i < nPoints - 1; i++) {
        if (adc >= calPoints[i][0] && adc <= calPoints[i + 1][0]) {
            float t = (float)(adc - calPoints[i][0]) /
                      (float)(calPoints[i + 1][0] - calPoints[i][0]);
            return calPoints[i][1] +
                   t * (calPoints[i + 1][1] - calPoints[i][1]);
        }
    }

    return 0; // fallback, теоретически не должен сработать
}

// --- Основная функция обновления температуры ---
void updateTemperature() {
    rawAdc = readMedianAdc();
    currentTemp = adcToTemp(rawAdc);

    // Отправляем значения на экран
    NextionSendNum("adc", rawAdc);
    NextionSendNum("temp", currentTemp);
}

// --- Проверка перегрева ---
bool checkThermocoupleError() {
    return (rawAdc > MAX_SAFE_ADC);
}
