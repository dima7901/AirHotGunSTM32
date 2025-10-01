#include <Arduino.h>
#include "globals.h"
#include "control.h"
#include "sensor.h"
#include "display.h"
#include "pid.h"
#include "eeprom.h"

void setup() {
    // Инициализация пинов
    pinMode(RELAY_POWER, OUTPUT);
    pinMode(RELAY_HEATER, OUTPUT);
    pinMode(LED_POWER, OUTPUT);
    pinMode(LED_HEATER, OUTPUT);
    pinMode(BUZZER, OUTPUT);
    pinMode(FAN_PWM, OUTPUT);
    pinMode(HALL_SENSOR, INPUT_PULLUP);
    pinMode(2, INPUT);
    attachInterrupt(digitalPinToInterrupt(2), handleZeroCrossing, RISING);
    
    // Кнопки
    pinMode(BTN_PRESET1, INPUT_PULLUP);
    pinMode(BTN_PRESET2, INPUT_PULLUP);
    pinMode(BTN_PRESET3, INPUT_PULLUP);
    pinMode(BTN_TEMP_UP, INPUT_PULLUP);
    pinMode(BTN_TEMP_DOWN, INPUT_PULLUP);
    pinMode(BTN_FLOW_UP, INPUT_PULLUP);
    pinMode(BTN_FLOW_DOWN, INPUT_PULLUP);
    pinMode(BTN_POWER, INPUT_PULLUP);
    
    // Энкодер
    pinMode(ENC_A, INPUT_PULLUP);
    pinMode(ENC_B, INPUT_PULLUP);
    pinMode(ENC_SW, INPUT_PULLUP);
    encLastStateA = digitalRead(ENC_A);
    
    // Инициализация модулей
    initSensor();       // Инициализация датчика
    initDisplay();      // Инициализация дисплея
    loadSettings();     // Загрузка из EEPROM
    
    // Начальное состояние
    digitalWrite(RELAY_POWER, LOW);
    digitalWrite(RELAY_HEATER, LOW);
    digitalWrite(LED_POWER, LOW);
    analogWrite(FAN_PWM, 0);
    
    playBuzzer(BUZZER_ON, 200);
}

void loop() {
    unsigned long currentMillis = millis();
    
    // Основная логика
    handlePower();
    
    if (systemPowered) {
        // Обновление данных
        readTemperature();      // Чтение датчика температуры
        updatePID();           // Расчет PID
        
        handleGunSensor();
        handleEncoder();
        handleButtons();
        handlePresets();
        
        if (!inMenuMode) {
            updateHeater();
            updateFan();
            checkAutoOff();
        }
        
        if (coolingProcess) {
            handleCooling();
        }
        
        // Обновление дисплея
        updateDisplayData();
    }
    
    delay(10); 
}
