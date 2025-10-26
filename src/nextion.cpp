#include "nextion.h"

void NextionSendNum(const char* var, int value) {
    Serial1.print(var);
    Serial1.print(".val=");
    Serial1.print(value);
    Serial1.write(0xFF); Serial1.write(0xFF); Serial1.write(0xFF);
}

void NextionSendCommand(const char* cmd) {
    Serial1.print(cmd);
    Serial1.write(0xFF); Serial1.write(0xFF); Serial1.write(0xFF);
}

void NextionSendString(const char* var, const char* str) {
    Serial1.print(var);
    Serial1.print(".txt=\"");
    Serial1.print(str);
    Serial1.print("\"");
    Serial1.write(0xFF); Serial1.write(0xFF); Serial1.write(0xFF);
}

void NextionRefresh() {
    NextionSendNum("currentTemp", currentTemp);
    NextionSendNum("setTemp", setTemp);
    NextionSendNum("fanSpeed", fanPercent);
    NextionSendNum("menu", menu);
    NextionSendNum("editparam", editParam);
    NextionSendNum("preset_mode", presetEditMode ? activePreset + 1 : 0);

    NextionSendNum("p1temp", presets[0].temp);
    NextionSendNum("p1fan", presets[0].fan);
    
    NextionSendNum("p2temp", presets[1].temp);
    NextionSendNum("p2fan", presets[1].fan);
    
    NextionSendNum("p3temp", presets[2].temp);
    NextionSendNum("p3fan", presets[2].fan);
    
    if (presetEditMode) {
        NextionSendString("status", "EDIT PRESET");
    } else if (!powerOn) {
        NextionSendString("status", "STANDBY");
    } else if (!handToolInHand) {
        NextionSendString("status", "READY");
    } else {
        NextionSendString("status", "WORK");
    }
}
