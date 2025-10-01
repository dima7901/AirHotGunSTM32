#include <Arduino.h>
#include "globals.h"
#include "display.h"

void initDisplay() {
    // Инициализация Nextion
    Serial.begin(9600); // Для Nextion дисплея
    // Дополнительная инициализация если нужно
}

void updateDisplayData() {
    // Основные параметры
    NextionSendNum("currenttemp", currentTemperature);
    NextionSendNum("settemp", targetTemperature);
    NextionSendNum("setflow", targetAirFlow);
    NextionSendNum("powerbar", pidPowerOutput);
    
    // Статусы
    if (coolingProcess) {
        NextionSendStr("status", "COOLING");
    } else if (gunInHand && heatingActive) {
        NextionSendStr("status", "WORKING");
    } else if (systemPowered) {
        NextionSendStr("status", "STANDBY");
    } else {
        NextionSendStr("status", "OFF");
    }
}

// Заглушки - вы замените на свои рабочие функции
void NextionSendNum(String field, int value) {
    String command = field + ".val=" + String(value);
    Serial.print(command);
    Serial.write(0xFF);
    Serial.write(0xFF);
    Serial.write(0xFF);
}

void NextionSendStr(String field, String value) {
    String command = field + ".txt=\"" + value + "\"";
    Serial.print(command);
    Serial.write(0xFF);
    Serial.write(0xFF);
    Serial.write(0xFF);
}
