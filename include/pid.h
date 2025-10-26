#ifndef PID_H
#define PID_H

#include "globals.h"

uint8_t computePID(int16_t setpoint, int16_t input);
void resetPID();

#endif
