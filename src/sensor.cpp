#include <Arduino.h>
#include "globals.h"
#include "sensor.h"

// Калибровочные коэффициенты
float tempCalibrationOffset = 0.0;
float tempCalibrationGain = 1.0;
float amplifierGain = 100.0;        // Коэффициент усиления усилителя
float vRef = 3.3;                   // Опорное напряжение АЦП
float coldJunctionTemp = 25.0;      // Температура холодного спая (пока фиксированная)

// Полиномиальные коэффициенты для K-типа (аппроксимация NIST)
const float KTYPE_COEFFS[] = {
    0.0,           // a0
    25.08355,      // a1  
    0.07860106,    // a2
    -0.2503131,    // a3
    0.0831527,     // a4
    -0.01228034,   // a5
    0.0009804036,  // a6
    -0.0000441303, // a7
    0.000001057734,// a8
    -0.00000001052755 // a9
};

void initSensor() {
    analogReadResolution(12); // 12-bit для STM32
    // Можно добавить инициализацию встроенного датчика температуры MCU
    // для компенсации холодного спая
}

// Преобразование милливольтов в температуру для K-типа
float convertKTypeMillivoltsToCelsius(float millivolts) {
    float temp = 0.0;
    float mv = millivolts * 1000.0; // переводим в микровольты для точности
    
    // Полиномиальная аппроксимация (работает для диапазона 0-500°C)
    for (int i = 0; i < 10; i++) {
        temp += KTYPE_COEFFS[i] * pow(mv, i);
    }
    
    return temp;
}

// Чтение сырого значения АЦП
int readRawADC() {
    long sum = 0;
    for(int i = 0; i < 16; i++) {
        sum += analogRead(THERMOCOUPLE_ADC);
        delay(1);
    }
    return sum / 16;
}

void readTemperature() {
    int adcValue = readRawADC();
    
    // Преобразование АЦП в напряжение
    float voltage = (adcValue / 4095.0) * vRef;
    
    // Учет усиления усилителя
    float thermocoupleVoltage = voltage / amplifierGain;
    
    // Преобразование в температуру
    float temperature = convertKTypeMillivoltsToCelsius(thermocoupleVoltage);
    
    // Компенсация холодного спая
    float compensatedTemperature = temperature + coldJunctionTemp;
    
    // Применяем калибровку
    compensatedTemperature = (compensatedTemperature + tempCalibrationOffset) * tempCalibrationGain;
    
    currentTemperature = constrain(compensatedTemperature, 0, 500);
}

// Функция для установки температуры холодного спая
void setColdJunctionTemp(float temp) {
    coldJunctionTemp = temp;
}

// Функция для калибровки (вызовем позже через меню)
void calibrateTemperature(float knownTemperature) {
    float error = knownTemperature - currentTemperature;
    tempCalibrationOffset += error;
}