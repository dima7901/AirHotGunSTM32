#include <Arduino.h>
#include "globals.h"
#include "pid.h"

// PID параметры для быстрого нагрева
double Kp = 8.0, Ki = 0.8, Kd = 2.0;  // Агрессивные коэффициенты
double errorSum = 0, lastError = 0;
unsigned long lastPIDTime = 0;

// Для цифрового управления
int cyclesToSkip = 0;    // Пропускать полупериоды
int currentCycle = 0;

void updatePID() {
    if (!heatingActive || !gunInHand || coolingProcess) {
        pidPowerOutput = 0;
        errorSum = 0;
        lastError = 0;
        return;
    }
    
    unsigned long now = millis();
    double dt = (now - lastPIDTime) / 1000.0;
    
    if (dt >= 0.05) { // PID каждые 50мс (20 раз в секунду)
        double error = targetTemperature - currentTemperature;
        
        // Агрессивный интеграл только при больших ошибках
        if (abs(error) > 10) {
            errorSum += error * dt;
            errorSum = constrain(errorSum, -50, 50); // Анти-windup
        } else {
            errorSum *= 0.95; // Плавно сбрасываем интеграл
        }
        
        double dError = (error - lastError) / dt;
        
        // PID расчет
        double output = Kp * error + Ki * errorSum + Kd * dError;
        pidPowerOutput = constrain(output, 0, 100);
        
        // Преобразуем в цифровое управление
        // 100% = не пропускаем полупериоды, 0% = все пропускаем
        cyclesToSkip = map(pidPowerOutput, 0, 100, 10, 0);
        currentCycle = 0;
        
        lastError = error;
        lastPIDTime = now;
    }
}

// Вызывается в прерывании детектора нуля (100Гц)
void handleZeroCrossing() {
    currentCycle++;
    
    if (currentCycle >= cyclesToSkip) {
        // Включаем нагрев в этом полупериоде
        digitalWrite(RELAY_HEATER, HIGH);
        heatingThisCycle = true;
        currentCycle = 0;
    }
}

// Вызывается через 1-2мс после детектора нуля
void triggerHeater() {
    if (heatingThisCycle) {
        digitalWrite(RELAY_HEATER, LOW); // Выключаем после задержки
        heatingThisCycle = false;
    }
}