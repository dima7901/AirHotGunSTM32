#include "globals.h"
#include "sensor.h"
#include "pid.h"
#include "nextion.h"
#include "storage.h"
#include "control.h"

void setup() {
    // Инициализация пинов
    pinMode(ADC_TEMP_PIN, INPUT_ANALOG);
    pinMode(ZERO_CROSS_PIN, INPUT);
    pinMode(HEATER_PIN, OUTPUT);
    pinMode(POWER_BTN_PIN, INPUT_PULLUP);
    pinMode(GERKON_PIN, INPUT);
    pinMode(ENCODER_A_PIN, INPUT);
    pinMode(ENCODER_B_PIN, INPUT);
    pinMode(ENCODER_SW_PIN, INPUT);
    pinMode(FAN_PWM_PIN, OUTPUT);
    pinMode(POWER_LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(TEMP_UP_PIN, INPUT_PULLUP);
    pinMode(TEMP_DOWN_PIN, INPUT_PULLUP);
    pinMode(FAN_UP_PIN, INPUT_PULLUP);
    pinMode(FAN_DOWN_PIN, INPUT_PULLUP);
    pinMode(PRESET1_PIN, INPUT_PULLUP);
    pinMode(PRESET2_PIN, INPUT_PULLUP);
    pinMode(PRESET3_PIN, INPUT_PULLUP);
  

    // АППАРАТНЫЕ УЛУЧШЕНИЯ ДЛЯ СКОРОСТИ
    analogReadResolution(12);
    analogWrite(FAN_PWM_PIN, 0);        // Инициализировать ШИМ на нужном пине
    analogWriteFrequency(25000);        // Установить частоту (применится к последнему использованному таймеру)

   // Инициализация состояния геркона

    // Прерывания
    attachInterrupt(digitalPinToInterrupt(ENCODER_A_PIN), encoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER_B_PIN), encoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ZERO_CROSS_PIN), zeroCrossISR, RISING);

    // Nextion
    Serial1.begin(9600);
  
    
    // Хранилище
    storageSetup();

    // Инициализация
    digitalWrite(HEATER_PIN, LOW);
    analogWrite(FAN_PWM_PIN, 0);
    digitalWrite(POWER_LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    lastActivity = millis();
    gerkonRaw = digitalRead(GERKON_PIN);
    gerkonDebounced = gerkonRaw;
    gerkonPending = false;
    handToolInHand = gerkonDebounced;
}

void loop() {
    uint32_t now = millis();

    // ВЫСОКОЧАСТОТНОЕ ОБНОВЛЕНИЕ ДАТЧИКА (20 Гц)
    static uint32_t lastTempRead = 0;
    if (now - lastTempRead > 50) {
        updateTemperature();
        lastTempRead = now;
    }

    // БЫСТРОЕ УПРАВЛЕНИЕ КНОПКАМИ (50 Гц)
    static uint32_t lastControlUpdate = 0;
    if (now - lastControlUpdate > 20) {
        handleGercon();
        handlePowerButton();
        handleTempButtons();
        handleFanButtons();
        handleEncoder();
        handleEncoderButton();
        handlePresets();
        lastControlUpdate = now;
    }

    // ПИД РЕГУЛЯТОР (10 Гц)
    static uint32_t lastPidUpdate = 0;
    if (now - lastPidUpdate > 100) {
        updateHeaterLogic();
        lastPidUpdate = now;
    }

    // УПРАВЛЕНИЕ ПЕРИФЕРИЕЙ (20 Гц)
    static uint32_t lastPeripheralUpdate = 0;
    if (now - lastPeripheralUpdate > 50) {
        updateFan();
        updatePowerLED();
        lastPeripheralUpdate = now;
    }

    // ЗАЩИТА ОТ ОБРЫВА ТЕРМОПАРЫ
    if (checkThermocoupleError()) {
        powerOn = false;
        beep(500, 500);
        menu = 0;
        presetEditMode = false;
        activePreset = -1;
        updateHeater(0);
        analogWrite(FAN_PWM_PIN, 0);
        digitalWrite(POWER_LED_PIN, LOW);
    }

    // ДИСПЛЕЙ (5 Гц - чтобы не нагружать)
    static uint32_t lastDisplay = 0;
    if (now - lastDisplay > 200) {
        NextionRefresh();
        lastDisplay = now;
    }

    delay(5); // Минимальная задержка для стабильности
}


