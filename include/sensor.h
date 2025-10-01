#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>

void initSensor();
void readTemperature();
int readRawADC();
void setColdJunctionTemp(float temp);
void calibrateTemperature(float knownTemperature);

#endif