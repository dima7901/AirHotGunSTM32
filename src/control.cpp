#include "control.h"
#include "pid.h" 
#include "storage.h"
#include "nextion.h"

bool powerButtonState = true; // текущее стабильное состояние кнопки питания

void beep(uint16_t freq, uint16_t duration) {
    tone(BUZZER_PIN, freq, duration);
}

void encoderISR() {
    static const int8_t tbl[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
    uint8_t a = digitalRead(ENCODER_A_PIN);
    uint8_t b = digitalRead(ENCODER_B_PIN);
    encoderState = (encoderState << 2) | (a << 1) | b;
    encoderDelta += tbl[encoderState & 0x0F];
}

void zeroCrossISR() {
    zeroCrossCount++;
    digitalWrite(HEATER_PIN, (zeroCrossCount * triacValue) > 255 ? HIGH : LOW);
}

void updateHeater(uint8_t output) {
    triacValue = output;
    zeroCrossCount = 0;
}

void updateHeaterLogic() {
    if (powerOn && handToolInHand) {
        int16_t error = setTemp - currentTemp;
        if (error > 150) {
            updateHeater(255);
        } else if (error > 50) {
            heaterOutput = computePID(setTemp, currentTemp);
            updateHeater(heaterOutput);
        } else {
            heaterOutput = computePID(setTemp, currentTemp);
            updateHeater(constrain(heaterOutput, 0, 200));
        }
    } else {
        updateHeater(0);
    }
}

// Унифицированная функция обработки удержания кнопок (для int16_t и uint8_t)
void handleButtonRepeat(uint32_t& pressStart, bool isPressed, int8_t direction,
                       int16_t& value, int16_t minVal, int16_t maxVal, int16_t step) {
    uint32_t now = millis();
    if (isPressed) {
        if (pressStart == 0) {
            pressStart = now;
            value = constrain(value + (direction * step), minVal, maxVal);
            lastActivity = now;
        } else if (now - pressStart > BUTTON_REPEAT_DELAY) {
            uint32_t repeatRate = max((uint32_t)20, BUTTON_ACCELERATE - (now - pressStart) / 30);
            if (now - lastActivity > repeatRate) {
                value = constrain(value + (direction * step), minVal, maxVal);
                lastActivity = now;
            }
        }
    } else {
        pressStart = 0;
    }
}

void handleButtonRepeat(uint32_t& pressStart, bool isPressed, int8_t direction,
                       uint8_t& value, uint8_t minVal, uint8_t maxVal, uint8_t step) {
    uint32_t now = millis();
    if (isPressed) {
        if (pressStart == 0) {
            pressStart = now;
            value = constrain(value + (direction * step), minVal, maxVal);
            lastActivity = now;
        } else if (now - pressStart > BUTTON_REPEAT_DELAY) {
            uint32_t repeatRate = max((uint32_t)20, BUTTON_ACCELERATE - (now - pressStart) / 30);
            if (now - lastActivity > repeatRate) {
                value = constrain(value + (direction * step), minVal, maxVal);
                lastActivity = now;
            }
        }
    } else {
        pressStart = 0;
    }
}

void updateFan() {
    if (powerOn && handToolInHand) {
        analogWrite(FAN_PWM_PIN, map(fanPercent, 0, 100, 0, 255));
    } else if (!powerOn && currentTemp > HOT_CUTOFF) {
        analogWrite(FAN_PWM_PIN, 128);
    } else if (!handToolInHand && currentTemp > HOT_CUTOFF) {
        int coolFan = map(currentTemp, HOT_CUTOFF, MAX_TEMP, 20, 100);
        coolFan = constrain(coolFan, 20, 100);
        analogWrite(FAN_PWM_PIN, map(coolFan, 0, 100, 0, 255));
    } else {
        analogWrite(FAN_PWM_PIN, 0);
    }
}

void updatePowerLED() {
    if (!powerOn) {
        digitalWrite(POWER_LED_PIN, LOW);
    } else {
        if (!handToolInHand) {
            static bool ledState = false;
            static uint32_t lastToggle = 0;
            uint32_t now = millis();
            if (now - lastToggle > 500) {
                ledState = !ledState;
                digitalWrite(POWER_LED_PIN, ledState);
                lastToggle = now;
            }
        } else {
            digitalWrite(POWER_LED_PIN, HIGH);
        }
    }
}

void handleGercon() {
    static bool lastGercon = true;
    uint32_t now = millis();
    // Предполагается: геркон замыкается на GND → LOW = на базе
    bool gercon = digitalRead(GERKON_PIN); // LOW = на базе
    if (gercon != lastGercon && (now - lastActivity > 50)) {
        handToolInHand = (gercon == HIGH); // HIGH = в руках
        lastGercon = gercon;
        lastActivity = now;
        if (powerOn) {
            beep(handToolInHand ? 2000 : 1000, 200);
        }
        if (handToolInHand) {
            resetPID();
        }
    }
}

void handlePowerButton() {
    static bool lastStableState = true;
    static uint32_t lastDebounceTime = 0;
    const uint32_t debounceDelay = 50; // 50 мс — надёжный дебаунс

    bool reading = digitalRead(POWER_BTN_PIN);

    if (reading != lastStableState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading != powerButtonState) {
            powerButtonState = reading;
            if (powerButtonState == LOW) { // нажатие (активный LOW)
                powerOn = !powerOn;
                lastActivity = millis();
                if (powerOn) {
                    beep(1500, 100); delay(110);
                    beep(2000, 100); delay(110);
                    beep(2500, 100);
                } else {
                    beep(800, 300);
                    menu = 0;
                    presetEditMode = false;
                    activePreset = -1;
                    updateHeater(0);
                    analogWrite(FAN_PWM_PIN, 0);
                    digitalWrite(POWER_LED_PIN, LOW);
                    saveSettings();
                }
            }
        }
    }
    lastStableState = reading;
}

void handleTempButtons() {
    if (!powerOn) return;
    bool upPressed = (digitalRead(TEMP_UP_PIN) == LOW);
    bool downPressed = (digitalRead(TEMP_DOWN_PIN) == LOW);

    if (presetEditMode && activePreset >= 0) {
        handleButtonRepeat(tempUpPressStart, upPressed, 1, presets[activePreset].temp, MIN_SET_TEMP, MAX_TEMP, 5);
        handleButtonRepeat(tempDownPressStart, downPressed, -1, presets[activePreset].temp, MIN_SET_TEMP, MAX_TEMP, 5);
        // Обновление дисплея
        if (activePreset == 0) NextionSendNum("p1temp", presets[0].temp);
        else if (activePreset == 1) NextionSendNum("p2temp", presets[1].temp);
        else if (activePreset == 2) NextionSendNum("p3temp", presets[2].temp);
    } else {
        handleButtonRepeat(tempUpPressStart, upPressed, 1, setTemp, MIN_SET_TEMP, MAX_TEMP, 5);
        handleButtonRepeat(tempDownPressStart, downPressed, -1, setTemp, MIN_SET_TEMP, MAX_TEMP, 5);
    }
}

void handleFanButtons() {
    if (!powerOn) return;
    bool upPressed = (digitalRead(FAN_UP_PIN) == LOW);
    bool downPressed = (digitalRead(FAN_DOWN_PIN) == LOW);

    if (presetEditMode && activePreset >= 0) {
        handleButtonRepeat(fanUpPressStart, upPressed, 1, presets[activePreset].fan, 0, 100, 1);
        handleButtonRepeat(fanDownPressStart, downPressed, -1, presets[activePreset].fan, 0, 100, 1);
        // Обновление дисплея
        if (activePreset == 0) NextionSendNum("p1fan", presets[0].fan);
        else if (activePreset == 1) NextionSendNum("p2fan", presets[1].fan);
        else if (activePreset == 2) NextionSendNum("p3fan", presets[2].fan);
    } else {
        handleButtonRepeat(fanUpPressStart, upPressed, 1, fanPercent, 0, 100, 1);
        handleButtonRepeat(fanDownPressStart, downPressed, -1, fanPercent, 0, 100, 1);
    }
}

void handleEncoder() {
    int8_t delta = encoderDelta;
    encoderDelta = 0;
    if (delta != 0 && powerOn) {
        lastActivity = millis();
        if (menu == 0) {
            if (presetEditMode && activePreset >= 0) {
                if (editParam == 0) {
                    presets[activePreset].temp += delta * 5;
                    presets[activePreset].temp = constrain(presets[activePreset].temp, MIN_SET_TEMP, MAX_TEMP);
                    if (activePreset == 0) NextionSendNum("p1temp", presets[0].temp);
                    else if (activePreset == 1) NextionSendNum("p2temp", presets[1].temp);
                    else if (activePreset == 2) NextionSendNum("p3temp", presets[2].temp);
                } else {
                    presets[activePreset].fan += delta;
                    presets[activePreset].fan = constrain(presets[activePreset].fan, 0, 100);
                    if (activePreset == 0) NextionSendNum("p1fan", presets[0].fan);
                    else if (activePreset == 1) NextionSendNum("p2fan", presets[1].fan);
                    else if (activePreset == 2) NextionSendNum("p3fan", presets[2].fan);
                }
            } else {
                if (editParam == 0) {
                    setTemp += delta * 5;
                    setTemp = constrain(setTemp, MIN_SET_TEMP, MAX_TEMP);
                } else {
                    fanPercent += delta;
                    fanPercent = constrain(fanPercent, 0, 100);
                }
            }
        } else if (menu == 2) {
            triacValue += delta * 5;
            triacValue = constrain(triacValue, 0, 255);
        }
    }
}

void handleEncoderButton() {
    static bool lastEncBtn = true;
    uint32_t now = millis();
    bool encBtn = digitalRead(ENCODER_SW_PIN);
    if (encBtn != lastEncBtn) {
        if (!encBtn) {
            encoderPressStart = now;
        } else {
            if (powerOn && (now - encoderPressStart < HOLD_TIME_SHORT_MS)) {
                if (menu == 0) {
                    editParam = 1 - editParam;
                    lastActivity = now;
                }
            }
        }
        lastEncBtn = encBtn;
    }
    if (!encBtn && powerOn && (now - encoderPressStart > HOLD_TIME_SHORT_MS)) {
        if (menu == 0) {
            menu = 1;
            beep(3000, 100);
            lastActivity = now;
        } else if (menu == 1 || menu == 2) {
            menu = 0;
            saveSettings();
            lastActivity = now;
        }
    }
}

void handlePresets() {
    if (!powerOn) return;
    uint32_t now = millis();
    uint8_t presetPins[NUM_PRESETS] = {PRESET1_PIN, PRESET2_PIN, PRESET3_PIN};
    for (int i = 0; i < NUM_PRESETS; i++) {
        bool pressed = (digitalRead(presetPins[i]) == LOW);
        if (pressed) {
            if (presetPressStart[i] == 0) {
                presetPressStart[i] = now;
            }
            if ((now - presetPressStart[i] > HOLD_TIME_LONG_MS) && !handToolInHand) {
                if (!presetEditMode || activePreset != i) {
                    presetEditMode = true;
                    activePreset = i;
                    editParam = 0;
                    beep(3000, 100);
                    lastActivity = now;
                } else {
                    presetEditMode = false;
                    activePreset = -1;
                    saveSettings();
                    beep(2000, 100);
                    lastActivity = now;
                }
            }
        } else {
            if (presetPressStart[i] != 0) {
                if ((now - presetPressStart[i] < HOLD_TIME_LONG_MS) && !presetEditMode) {
                    setTemp = presets[i].temp;
                    fanPercent = presets[i].fan;
                    activePreset = i;
                    beep(2000, 100);
                    lastActivity = now;
                }
                presetPressStart[i] = 0;
            }
        }
    }
}