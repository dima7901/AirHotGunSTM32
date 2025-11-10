#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>

// ПИНЫ
#define ADC_TEMP_PIN        PA0
#define ZERO_CROSS_PIN      PA1
#define HEATER_PIN          PA2
#define POWER_BTN_PIN       PA5
#define GERKON_PIN          PB15
#define ENCODER_A_PIN       PA6
#define ENCODER_B_PIN       PA7
#define FAN_PWM_PIN         PA8


#define ENCODER_SW_PIN      PB0
#define POWER_LED_PIN       PB1
#define BUZZER_PIN          PB2
#define TEMP_UP_PIN         PB3
#define TEMP_DOWN_PIN       PB4
#define FAN_UP_PIN          PB5
#define FAN_DOWN_PIN        PB6
#define PRESET1_PIN         PB7
#define PRESET2_PIN         PB8
#define PRESET3_PIN         PB9

// КОНСТАНТЫ
#define MIN_TEMP            20
#define MAX_TEMP            500
#define MIN_SET_TEMP        100
#define HOT_CUTOFF          530
#define COOLING_TEMP        80
#define MAX_ADC_VAL         4095
#define MAX_SAFE_ADC        4000
#define NUM_PRESETS         3
#define HOLD_TIME_SHORT_MS  500
#define HOLD_TIME_LONG_MS   1000
#define AUTO_SHUTDOWN_MIN   10

// ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ
extern const char* FirmwareVersion;
extern bool powerOn;
extern bool pageFL;
extern bool handToolInHand;
extern uint32_t lastActivity;

extern int16_t currentTemp;
extern int16_t setTemp;
extern int16_t rawAdc;
extern uint8_t fanPercent;
extern uint8_t fanPWMValue;

extern bool gerkonDebounced;      
extern bool gerkonRaw;            
extern bool gerkonPending;        
extern uint32_t gerkonDebounceTime;
extern bool heaterHasBeenActiveSinceShutdown;
extern bool coolingCompleted;
extern bool hasCooledBelowThreshold;
extern int16_t animationInterval;
extern bool animatFL;
struct Preset {
    int16_t temp;
    uint8_t fan;
};
extern Preset presets[NUM_PRESETS];

extern float Kp, Ki, Kd;
extern float pidLastError;
extern float pidIntegral;
extern uint8_t heaterOutput;

extern volatile int8_t encoderDelta;
extern volatile uint8_t encoderState;

extern uint8_t menu;
extern uint8_t editParam;
extern int8_t activePreset;
extern bool presetEditMode;

extern volatile uint8_t triacValue;
extern volatile uint8_t zeroCrossCount;

extern uint32_t encoderPressStart;
extern uint32_t presetPressStart[NUM_PRESETS];
extern uint32_t tempUpPressStart;
extern uint32_t tempDownPressStart;
extern uint32_t fanUpPressStart;
extern uint32_t fanDownPressStart;

#endif
