#ifndef NEXTION_H
#define NEXTION_H

#include "globals.h"

void NextionSendNum(const char* var, int value);
void NextionSendCommand(const char* cmd);
void NextionSendString(const char* var, const char* str);
void NextionRefresh();

#endif
