#ifndef SENSOR_H
#define SENSOR_H

#include "globals.h"

void updateTemperature();
bool checkThermocoupleError();
void calibrateSensor(int16_t knownTemp, int16_t measuredAdc);

#endif