#include "control.h"
#include "pid.h" 
#include "storage.h"
#include "nextion.h"
#include "globals.h"

bool powerButtonState = true;
bool lastHandToolState = true;
bool coolingMode = false;
bool fanWasActive = false;
const uint32_t AUTO_POWER_OFF_TIME = 10 * 60 * 1000; // 10 минут в миллисекундах

void beep(uint16_t freq, uint16_t duration) {
    tone(BUZZER_PIN, freq, duration);
}

void checkAutoPowerOff() {
    if (powerOn && !handToolInHand) {
        uint32_t now = millis();
        if (now - lastActivity > (uint32_t)AUTO_SHUTDOWN_MIN * 60000UL) {
            powerOn = false;
            coolingMode = false;
            fanWasActive = false;
            menu = 0;
            presetEditMode = false;
            activePreset = -1;
            updateHeater(0);
            analogWrite(FAN_PWM_PIN, 0);
            digitalWrite(POWER_LED_PIN, LOW);
            beep(600, 200);
            
        }
    }
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
    // ЖЕЛЕЗОБЕТОННОЕ ПРАВИЛО: нагрев ВКЛЮЧЕН только когда станция ВКЛ и фен В РУКАХ
    if (powerOn && handToolInHand) {
        // ВКЛЮЧАЕМ НАГРЕВ - сбрасываем флаг охлаждения
        fanWasActive = false;
        
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
        // Нагрев ВЫКЛЮЧЕН когда на подставке или станция ВЫКЛ
        updateHeater(0);
    }
}

void updateFan() {
    if (coolingMode) {
        // Режим охлаждения перед выключением - 100%
        analogWrite(FAN_PWM_PIN, 255);
        fanWasActive = true;
    } else if (powerOn && handToolInHand) {
        // Рабочий режим - ВСЕГДА работает по установленной скорости
        uint8_t fanSpeed = map(constrain(fanPercent, 30, 100), 0, 100, 80, 255);
        analogWrite(FAN_PWM_PIN, fanSpeed);
    } else if (powerOn && !handToolInHand) {
        // Режим ожидания на подставке - охлаждение до 80°C
        if (currentTemp > COOLING_TEMP && !fanWasActive) {
            // Включаем охлаждение только если еще не остывали
            analogWrite(FAN_PWM_PIN, 255);
        } else if (currentTemp <= COOLING_TEMP) {
            // Остыли до 80°C - выключаем вентилятор и ставим флаг
            analogWrite(FAN_PWM_PIN, 0);
            fanWasActive = true;
        }
    } else if (!powerOn && currentTemp > COOLING_TEMP) {
        // Охлаждение после выключения
        analogWrite(FAN_PWM_PIN, 255);
        fanWasActive = true;
    } else if (currentTemp > HOT_CUTOFF) {
        // Аварийное охлаждение
        analogWrite(FAN_PWM_PIN, 255);
        fanWasActive = true;
    } else {
        analogWrite(FAN_PWM_PIN, 0);
    }
}

void updatePowerLED() {
    if (!powerOn) {
        digitalWrite(POWER_LED_PIN, LOW);
    } else {
        if (!handToolInHand) {
            // Мигание при установке на базу (режим паузы)
            static bool ledState = false;
            static uint32_t lastToggle = 0;
            uint32_t now = millis();
            if (now - lastToggle > 500) {
                ledState = !ledState;
                digitalWrite(POWER_LED_PIN, ledState);
                lastToggle = now;
            }
        } else {
            // Постоянное свечение при работе в руках
            digitalWrite(POWER_LED_PIN, HIGH);
        }
    }
}

// Унифицированная функция обработки удержания кнопок
void handleButtonRepeat(uint32_t& pressStart, bool isPressed, int8_t direction,
                       int16_t& value, int16_t minVal, int16_t maxVal, int16_t step) {
    if (!powerOn) return; // Управление заблокировано ТОЛЬКО при powerOff
    
    uint32_t now = millis();
    if (isPressed) {
        if (pressStart == 0) {
            pressStart = now;
            value = constrain(value + (direction * step), minVal, maxVal);
            lastActivity = now; // Сбрасываем таймер авто-выключения
            beep(1000, 50);
        } else {
            uint32_t holdTime = now - pressStart;
            if (holdTime > BUTTON_REPEAT_DELAY) {
                uint32_t repeatRate = BUTTON_ACCELERATE;
                
                // Ускорение при длительном удержании
                if (holdTime > 2000) repeatRate = 80;
                if (holdTime > 4000) repeatRate = 40;
                if (holdTime > 6000) repeatRate = 20;
                
                if (now - lastActivity > repeatRate) {
                    value = constrain(value + (direction * step), minVal, maxVal);
                    lastActivity = now; // Сбрасываем таймер авто-выключения
                }
            }
        }
    } else {
        if (pressStart != 0) {
            pressStart = 0;
        }
    }
}

void handleButtonRepeat(uint32_t& pressStart, bool isPressed, int8_t direction,
                       uint8_t& value, uint8_t minVal, uint8_t maxVal, uint8_t step) {
    if (!powerOn) return; // Управление заблокировано ТОЛЬКО при powerOff
    
    uint32_t now = millis();
    if (isPressed) {
        if (pressStart == 0) {
            pressStart = now;
            value = constrain(value + (direction * step), minVal, maxVal);
            lastActivity = now; // Сбрасываем таймер авто-выключения
            beep(1000, 50);
        } else {
            uint32_t holdTime = now - pressStart;
            if (holdTime > BUTTON_REPEAT_DELAY) {
                uint32_t repeatRate = BUTTON_ACCELERATE;
                
                if (holdTime > 2000) repeatRate = 80;
                if (holdTime > 4000) repeatRate = 40;
                if (holdTime > 6000) repeatRate = 20;
                
                if (now - lastActivity > repeatRate) {
                    value = constrain(value + (direction * step), minVal, maxVal);
                    lastActivity = now; // Сбрасываем таймер авто-выключения
                }
            }
        }
    } else {
        if (pressStart != 0) {
            pressStart = 0;
        }
    }
}

void handleGercon() {
    if (!powerOn) return;

    bool raw = digitalRead(GERKON_PIN); // LOW = на базе, HIGH = в руках

    // Обнаружено изменение состояния
    if (raw != gerkonRaw) {
        gerkonRaw = raw;
        gerkonPending = true;
        gerkonDebounceTime = millis() + 200; // как в оригинале — 200 мс
    }

    // Проверяем, прошло ли время дребезга
    if (gerkonPending && millis() >= gerkonDebounceTime) {
        if (gerkonDebounced != gerkonRaw) {
            gerkonDebounced = gerkonRaw;
            handToolInHand = gerkonDebounced; // только здесь обновляем!

            // Сигнал смены режима
            beep(handToolInHand ? 2000 : 1000, 200);
            lastActivity = millis();

            if (handToolInHand) {
                resetPID();
            }
        }
        gerkonPending = false;
    }
}

void handlePowerButton() {
    static bool lastStableState = true;
    static uint32_t lastDebounceTime = 0;
    const uint32_t debounceDelay = 50;

    bool reading = digitalRead(POWER_BTN_PIN);

    if (reading != lastStableState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading != powerButtonState) {
            powerButtonState = reading;
            if (powerButtonState == LOW) {
                if (coolingMode) {
                    // В режиме охлаждения кнопка не реагирует
                    return;
                }
                
                if (powerOn && currentTemp > COOLING_TEMP) {
                    // Запуск режима охлаждения
                    coolingMode = true;
                    powerOn = false;
                    updateHeater(0);
                    presetEditMode = false;
                    activePreset = -1;
                    menu = 0;
                    beep(500, 300);
                } else {
                    // Обычное включение/выключение
                    powerOn = !powerOn;
                    coolingMode = false;
                    fanWasActive = false;
                    lastActivity = millis(); // Сбрасываем таймер авто-выключения
                    
                    if (powerOn) {
                        // Включение
                        beep(1500, 100); delay(110);
                        beep(2000, 100); delay(110);
                        beep(2500, 100);
                        resetPID();
                    } else {
                        // Выключение
                        beep(800, 300);
                        menu = 0;
                        presetEditMode = false;
                        activePreset = -1;
                        updateHeater(0);
                        
                    }
                }
            }
        }
    }
    lastStableState = reading;
}

void checkCoolingMode() {
    if (coolingMode && currentTemp <= COOLING_TEMP) {
        // Охлаждение завершено
        coolingMode = false;
        analogWrite(FAN_PWM_PIN, 0);
        beep(800, 200);
    }
}

void handleTempButtons() {
    if (!powerOn) return; // Работает только при включенной станции
    
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
    if (!powerOn) return; // Работает только при включенной станции
    
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
    if (!powerOn) return; // Работает только при включенной станции
    
        int8_t delta = (encoderDelta > 0) ? 1 : (encoderDelta < 0 ? -1 : 0);
        encoderDelta = 0;
    
    if (delta != 0) {
        lastActivity = millis(); // Сбрасываем таймер авто-выключения
        
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
                    int16_t newFan = fanPercent + delta;
                    if (newFan < 0) newFan = 0;
                    if (newFan > 100) newFan = 100;
                    fanPercent = newFan;
                }
            }
            beep(1200, 30);
        } else if (menu == 2) {
            triacValue += delta * 5;
            triacValue = constrain(triacValue, 0, 255);
        }
    }
}

void handleEncoderButton() {
    static bool lastEncBtn = true;
    static uint32_t lastEncBtnTime = 0;
    uint32_t now = millis();
    
    if (!powerOn) return; // Работает только при включенной станции
    
    if (now - lastEncBtnTime < 50) return;
    
    bool encBtn = digitalRead(ENCODER_SW_PIN);
    
    if (encBtn != lastEncBtn) {
        lastEncBtnTime = now;
        
        if (!encBtn) {
            encoderPressStart = now;
        } else if (encoderPressStart != 0) {
            uint32_t pressDuration = now - encoderPressStart;
            
            if (pressDuration < HOLD_TIME_SHORT_MS) {
                // Короткое нажатие - переключение параметра
                if (menu == 0) {
                    editParam = 1 - editParam;
                    lastActivity = now; // Сбрасываем таймер авто-выключения
                    beep(1500, 100);
                }
            }
            encoderPressStart = 0;
        }
        
        lastEncBtn = encBtn;
    }
    
    // Обработка длительного удержания
    if (!encBtn && encoderPressStart != 0) {
        uint32_t holdTime = now - encoderPressStart;
        
        if (holdTime > HOLD_TIME_SHORT_MS && menu == 0) {
            // Длительное удержание - переход в меню
            menu = 1;
            beep(3000, 100);
            lastActivity = now; // Сбрасываем таймер авто-выключения
            encoderPressStart = 0;
        } else if (holdTime > HOLD_TIME_SHORT_MS && (menu == 1 || menu == 2)) {
            // Длительное удержание в меню - выход и сохранение
            menu = 0;
            //savePresets();
            lastActivity = now; // Сбрасываем таймер авто-выключения
            beep(2000, 100);
            encoderPressStart = 0;
        }
    }
}

void handlePresets() {
    if (!powerOn) return; // Работает только при включенной станции
    
    uint32_t now = millis();
    uint8_t presetPins[NUM_PRESETS] = {PRESET1_PIN, PRESET2_PIN, PRESET3_PIN};
    
    for (int i = 0; i < NUM_PRESETS; i++) {
        bool pressed = (digitalRead(presetPins[i]) == LOW);
        
        if (pressed) {
            if (presetPressStart[i] == 0) {
                presetPressStart[i] = now;
            }
            
            if ((now - presetPressStart[i] > HOLD_TIME_LONG_MS)) {
                // Длительное удержание - вход/выход из режима редактирования
                if (!presetEditMode) {
                    presetEditMode = true;
                    activePreset = i;
                    editParam = 0;
                    beep(3000, 100);
                    lastActivity = now; // Сбрасываем таймер авто-выключения
                } else {
                    presetEditMode = false;
                    activePreset = -1;
                    savePresets();
                    beep(2000, 100);
                    lastActivity = now; // Сбрасываем таймер авто-выключения
                }
                presetPressStart[i] = 0;
            }
        } else {
            if (presetPressStart[i] != 0) {
                // Короткое нажатие - применение пресета
                if ((now - presetPressStart[i] < HOLD_TIME_LONG_MS) && !presetEditMode) {
                    setTemp = presets[i].temp;
                    fanPercent = presets[i].fan;
                    activePreset = i;
                    beep(2000, 100);
                    lastActivity = now; // Сбрасываем таймер авто-выключения
                }
                presetPressStart[i] = 0;
            }
        }
    }
}
