#include <Arduino.h>
#include "globals.h"
#include "pins.h"
#include "eeprom.h"

// === ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ===

void handleMenuSelection() {
    switch (menuPosition) {
        case 0:
            // Настройка PID
            break;
        case 1:
            // Калибровка температуры
            break;
        case 2:
            // Настройка зуммера
            break;
    }
    playBuzzer(1000, 50);
}

// === ОСНОВНЫЕ ФУНКЦИИ УПРАВЛЕНИЯ ===

void handleEncoder() {
    static long oldPos = 0;
    static unsigned long encButtonPress = 0;
    static bool encButtonHolding = false;
    
    int encStateA = digitalRead(ENC_A);
    if (encStateA != encLastStateA) {
        if (digitalRead(ENC_B) != encStateA) {
            encoderPosition++;
        } else {
            encoderPosition--;
        }
        
        int change = (encoderPosition > oldPos) ? 1 : -1;
        oldPos = encoderPosition;
        
        if (editingPreset >= 0) {
            if (activeParameter == 0) {
                userPresets[editingPreset].temperature = 
                    constrain(userPresets[editingPreset].temperature + change * TEMP_STEP, TEMP_MIN, TEMP_MAX);
            } else {
                userPresets[editingPreset].airflow = 
                    constrain(userPresets[editingPreset].airflow + change * FLOW_STEP, FLOW_MIN, FLOW_MAX);
            }
        } else if (!inMenuMode) {
            if (activeParameter == 0) {
                targetTemperature = constrain(targetTemperature + change * TEMP_STEP, TEMP_MIN, TEMP_MAX);
            } else {
                targetAirFlow = constrain(targetAirFlow + change * FLOW_STEP, FLOW_MIN, FLOW_MAX);
            }
        } else {
            menuPosition = constrain(menuPosition + change, 0, 5);
        }
        updateDisplay = true;
    }
    encLastStateA = encStateA;
    
    bool encBtn = digitalRead(ENC_SW);
    
    if (encBtn == LOW) {
        if (encButtonPress == 0) {
            encButtonPress = millis();
        } else if (millis() - encButtonPress > 1000 && !encButtonHolding) {
            encButtonHolding = true;
            
            if (editingPreset >= 0) {
                editingPreset = -1;
                saveSettings();
                playBuzzer(BUZZER_MENU, 100);
            } else {
                inMenuMode = !inMenuMode;
                menuPosition = 0;
                playBuzzer(BUZZER_MENU, 100);
            }
            updateDisplay = true;
        }
    } else {
        if (encButtonPress > 0 && !encButtonHolding) {
            if (editingPreset >= 0) {
                activeParameter = !activeParameter;
            } else if (!inMenuMode) {
                activeParameter = !activeParameter;
            } else {
                handleMenuSelection();
            }
            playBuzzer(1000, 50);
            updateDisplay = true;
        }
        encButtonPress = 0;
        encButtonHolding = false;
    }
}

void handlePresets() {
    static unsigned long preset1Press = 0, preset2Press = 0, preset3Press = 0;
    
    if (digitalRead(BTN_PRESET1) == LOW) {
        if (preset1Press == 0) {
            targetTemperature = userPresets[0].temperature;
            targetAirFlow = userPresets[0].airflow;
            preset1Press = millis();
            playBuzzer(1000, 50);
            updateDisplay = true;
        } else if (millis() - preset1Press > 1000) {
            editingPreset = 0;
            activeParameter = 0;
            preset1Press = 0;
            playBuzzer(BUZZER_MENU, 100);
            updateDisplay = true;
        }
    } else {
        preset1Press = 0;
    }
    
    // PRESET 2 и 3 аналогично...
}

void handleButtons() {
    // Реализация кнопок...
}

void handlePower() {
    static bool lastPowerBtn = HIGH;
    bool currentPowerBtn = digitalRead(BTN_POWER);
    
    if (currentPowerBtn == LOW && lastPowerBtn == HIGH) {
        delay(50);
        currentPowerBtn = digitalRead(BTN_POWER);
        if (currentPowerBtn == LOW) {
            systemPowered = !systemPowered;
            
            if (systemPowered) {
                digitalWrite(RELAY_POWER, HIGH);
                digitalWrite(LED_POWER, HIGH);
                playBuzzer(BUZZER_ON, 200);
            } else {
                if (gunInHand) {
                    coolingProcess = true;
                    coolingStartTime = millis();
                    powerLedBlinking = true;
                    playBuzzer(BUZZER_MENU, 100);
                } else {
                    digitalWrite(RELAY_POWER, LOW);
                    digitalWrite(LED_POWER, LOW);
                    playBuzzer(BUZZER_OFF, 200);
                }
            }
            updateDisplay = true;
        }
    }
    lastPowerBtn = currentPowerBtn;
}

void handleGunSensor() {
    static bool lastGunState = HIGH;
    bool currentGunState = digitalRead(HALL_SENSOR);
    
    if (currentGunState != lastGunState) {
        delay(50);
        currentGunState = digitalRead(HALL_SENSOR);
        if (currentGunState != lastGunState) {
            gunInHand = currentGunState;
            
            if (gunInHand) {
                heatingActive = true;
                fanRunning = true;
                playBuzzer(BUZZER_PICKUP, 100);
            } else {
                heatingActive = false;
                coolingProcess = true;
                playBuzzer(BUZZER_RETURN, 100);
            }
            updateDisplay = true;
        }
    }
    lastGunState = currentGunState;
}

void updateHeater() {
    if (heatingActive && gunInHand && !coolingProcess) {
        digitalWrite(LED_HEATER, heatingThisCycle ? HIGH : LOW);
    } else {
        digitalWrite(RELAY_HEATER, LOW);
        digitalWrite(LED_HEATER, LOW);
        heatingThisCycle = false;
    }
}

void updateFan() {
    if (fanRunning) {
        int pwmValue = map(targetAirFlow, 0, 100, 0, 255);
        analogWrite(FAN_PWM, pwmValue);
    } else {
        analogWrite(FAN_PWM, 0);
    }
}

void checkAutoOff() {
    if (!gunInHand && !coolingProcess) {
        if (millis() - lastActivityTime > AUTO_OFF_TIMEOUT) {
            systemPowered = false;
            digitalWrite(RELAY_POWER, LOW);
            digitalWrite(LED_POWER, LOW);
            playBuzzer(BUZZER_OFF, 200);
        }
    } else {
        lastActivityTime = millis();
    }
}

void handleCooling() {
    if (currentTemperature <= COOLDOWN_TEMP) {
        coolingProcess = false;
        fanRunning = false;
        powerLedBlinking = false;
        digitalWrite(LED_POWER, LOW);
        
        if (!systemPowered) {
            digitalWrite(RELAY_POWER, LOW);
        }
    } else {
        fanRunning = true;
        powerLedBlinking = true;
        digitalWrite(LED_POWER, (millis() % 1000) < 500);
    }
}

void playBuzzer(int freq, int duration) {
    tone(BUZZER, freq, duration);
}