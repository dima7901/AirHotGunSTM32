#ifndef CONTROL_H
#define CONTROL_H

#include "globals.h"

void beep(uint16_t freq, uint16_t duration);
void encoderISR();
void zeroCrossISR();
void updateHeater(uint8_t output);
// ДОБАВИТЬ в control.h после существующей handleButtonRepeat:
void handleButtonRepeat(uint32_t& pressStart, bool buttonState, int8_t direction, 
                       uint8_t& value, uint8_t minVal, uint8_t maxVal, uint8_t step);
void updateFan();
void updatePowerLED();
void updateHeaterLogic();
void handlePowerButton();
void handleTempButtons();
void handleFanButtons();
void handleEncoder();
void handleEncoderButton();
void handlePresets();
void handleGercon();

#endif
