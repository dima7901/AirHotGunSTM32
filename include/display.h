#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

void initDisplay();
void updateDisplayData();
void NextionSendNum(String field, int value);
void NextionSendStr(String field, String value);

#endif
