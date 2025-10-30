#include "pid.h"
#include "globals.h"
#include <math.h>

// ==== Константы PID ====
#define PID_DT             0.1f        // Интервал обновления (100 мс)
#define PID_OUTPUT_MAX     255.0f
#define PID_OUTPUT_MIN     0.0f
#define PID_MAX_CHANGE     8.0f        // Ограничение скорости изменения
#define PID_ERROR_DEADZONE 0.5f        // Зона нечувствительности
#define PID_INTEGRAL_DECAY 0.98f       // Затухание интеграла при малой ошибке

// ==== Пороговые зоны управления ====
#define PID_HEAT_FULL      60.0f       // Ошибка >60° — максимум
#define PID_HEAT_STRONG    15.0f       // Ошибка >15° — активный нагрев
#define PID_HEAT_NEAR       5.0f       // Ошибка >5° — осторожное приближение

// Основная функция PID — совместима со старым интерфейсом проекта
uint8_t computePID(int16_t setpoint, int16_t input) {
    static float last_error = 0.0f;
    static float last_output = 0.0f;
    static float last_derivative = 0.0f;

    // --- Преобразуем в float ---
    float f_setpoint = (float)setpoint;
    float f_input = (float)input;

    // --- Ошибка ---
    float error = f_setpoint - f_input;

    // --- Зона нечувствительности ---
    if (fabsf(error) < PID_ERROR_DEADZONE)
        error = 0.0f;

    // --- Производная с фильтрацией (подавляем шум) ---
    float derivative = (error - last_error) / PID_DT;
    derivative = 0.25f * derivative + 0.75f * last_derivative;

    // --- Интеграл с антидрейфом ---
    if (fabsf(error) > 0.5f) {
        pidIntegral += Ki * error * PID_DT;
    } else {
        pidIntegral *= PID_INTEGRAL_DECAY;
    }

    // --- Ограничение интеграла ---
    float integral_limit = PID_OUTPUT_MAX / fmaxf(Ki, 0.001f);
    if (pidIntegral > integral_limit) pidIntegral = integral_limit;
    if (pidIntegral < -integral_limit) pidIntegral = -integral_limit;

    // --- Адаптивный Ki (снижаем при приближении к цели) ---
    float adaptiveKi = Ki;
    if (fabsf(error) < 10.0f) adaptiveKi *= 0.5f;
    if (fabsf(error) < 3.0f)  adaptiveKi *= 0.2f;

    // --- Расчёт PID ---
    float output = Kp * error + adaptiveKi * pidIntegral + Kd * derivative;

    // --- Зонная логика управления ---
    if (error > PID_HEAT_FULL) {
        output = PID_OUTPUT_MAX;
    } else if (error > PID_HEAT_STRONG) {
        output = fminf(output, PID_OUTPUT_MAX * 0.95f);
    } else if (error > PID_HEAT_NEAR) {
        output = fminf(output, PID_OUTPUT_MAX * 0.75f);
    } else if (error < -5.0f) {
        output = 0.0f; // Перегрев — полностью выключаем
    } else {
        // --- Тонкая стабилизация ---
        float max_up = last_output + PID_MAX_CHANGE;
        float max_down = last_output - PID_MAX_CHANGE;

        if (output > max_up) output = max_up;
        if (output < max_down) output = max_down;

        // Минимальная мощность для поддержания
        output = fmaxf(output, 15.0f);
    }

    // --- Глобальное ограничение ---
    if (output > PID_OUTPUT_MAX) output = PID_OUTPUT_MAX;
    if (output < PID_OUTPUT_MIN) output = PID_OUTPUT_MIN;

    // --- Сохраняем значения ---
    last_error = error;
    last_output = output;
    last_derivative = derivative;

    return (uint8_t)output;
}

// Сброс PID (при старте, изменении режима и т.п.)
void resetPID() {
    pidIntegral = 0.0f;
}
