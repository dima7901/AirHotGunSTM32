#include "pid.h"
#include "globals.h"
#include <math.h>

// УСКОРЕННЫЙ ПИД С АНТИВИНДАПОМ И НЕЛИНЕЙНОСТЬЮ
uint8_t computePID(int16_t setpoint, int16_t input) {
    const float dt = 0.05f; // Уменьшил период в 2 раза (20 Гц)
    float error = (float)(setpoint - input);
    
    // АНТИВИНДАП - ограничение интегральной составляющей
    float newIntegral = pidIntegral + error * dt;
    if (fabs(error) < 50.0f) { // Интеграл только в зоне ±50°C
        pidIntegral = constrain(newIntegral, -200.0f, 200.0f);
    } else {
        pidIntegral = 0; // Сброс интеграла при большом отклонении
    }
    
    float derivative = (error - pidLastError) / dt;
    
    // АГРЕССИВНЫЕ КОЭФФИЦИЕНТЫ ДЛЯ БЫСТРОЙ РЕАКЦИИ
    float output = Kp * error + Ki * pidIntegral + Kd * derivative;
    
    pidLastError = error;
    
    // НЕЛИНЕЙНОСТЬ ДЛЯ МОЛНИЕНОСНОГО НАГРЕВА
    if (error > 80.0f) output += 30.0f; // Дополнительный толчок
    if (error > 150.0f) output = 255;   // Максимальный нагрев при большом отклонении
    
    // ПЛАВНОЕ ОГРАНИЧЕНИЕ ДЛЯ СТАБИЛЬНОСТИ
    return (uint8_t)constrain((int)output, 0, 255);
}

void resetPID() {
    pidIntegral = 0;
    pidLastError = 0;
}
