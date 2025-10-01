#ifndef PID_H
#define PID_H

#include <Arduino.h>

void updatePID();
void setPIDParameters(double kp, double ki, double kd);
void handleZeroCrossing();    // Прерывание детектора нуля
void triggerHeater();         // Включение нагрева

#endif
