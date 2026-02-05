#include "battery.h"

static uint8_t mapRange(uint32_t value, uint32_t inMin, uint32_t inMax, uint8_t outMin, uint8_t outMax) {
    value = constrain(value, inMin, inMax);
    return map(value, inMin, inMax, outMin, outMax);
}

static uint8_t estimatePercent(uint32_t millivolts) {
    if (millivolts >= 3900) return 100;                     // Resting full voltage after unplugging
    if (millivolts >= 3800) return mapRange(millivolts, 3800, 3900, 70, 99);
    if (millivolts >= 3700) return mapRange(millivolts, 3700, 3800, 50, 70);
    if (millivolts >= 3600) return mapRange(millivolts, 3600, 3700, 30, 50);
    if (millivolts >= 3500) return mapRange(millivolts, 3500, 3600, 15, 30);
    if (millivolts >= 3300) return mapRange(millivolts, 3300, 3500, 5, 15);
    return 0;
}

BatteryStatus evaluateBattery(uint32_t millivolts) {
    BatteryStatus status;
    status.millivolts = millivolts;

    status.charging = (millivolts >= 4340);       // ~4.35V while actively charging
    status.externalPower = (millivolts >= 4170);  // ~4.17V when topped off on USB

    if (status.externalPower || status.charging) {
        status.percent = 100;
    } else {
        status.percent = estimatePercent(millivolts);
    }

    if (status.charging) {
        status.label = "Laden";
    } else if (status.externalPower) {
        status.label = "Voll (Netz)";
    } else if (status.percent >= 98) {
        status.label = "Voll";
    } else if (status.percent <= 20) {
        status.label = "Niedrig";
    } else {
        status.label = "Akkubetrieb";
    }

    return status;
}
