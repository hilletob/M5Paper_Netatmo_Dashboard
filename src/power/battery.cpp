#include "battery.h"

// M5Paper has built-in battery management via M5.getBatteryVoltage()
// These functions are stubs for compatibility with XIAO code

void Battery::init() {
    // NOT USED on M5Paper (uses M5.getBatteryVoltage() instead)
}

uint32_t Battery::readVoltage() {
    // NOT USED on M5Paper (uses M5.getBatteryVoltage() instead)
    return 0;
}

uint8_t Battery::readPercentage() {
    // NOT USED on M5Paper (uses M5.getBatteryVoltage() instead)
    return 0;
}

void Battery::read(uint32_t& voltageMv, uint8_t& percent) {
    // NOT USED on M5Paper (uses M5.getBatteryVoltage() instead)
    voltageMv = 0;
    percent = 0;
}
