#include "battery.h"

static uint8_t mapRange(uint32_t value, uint32_t inMin, uint32_t inMax, uint8_t outMin, uint8_t outMax) {
    value = constrain(value, inMin, inMax);
    return map(value, inMin, inMax, outMin, outMax);
}

static uint8_t estimatePercent(uint32_t millivolts) {
    if (millivolts >= 3950) return 100;
    if (millivolts >= 3900) return 99;
    if (millivolts >= 3850) return mapRange(millivolts, 3850, 3900, 90, 99);
    if (millivolts >= 3800) return mapRange(millivolts, 3800, 3850, 80, 90);
    if (millivolts >= 3750) return mapRange(millivolts, 3750, 3800, 70, 80);
    if (millivolts >= 3700) return mapRange(millivolts, 3700, 3750, 60, 70);
    if (millivolts >= 3660) return mapRange(millivolts, 3660, 3700, 50, 60);
    if (millivolts >= 3620) return mapRange(millivolts, 3620, 3660, 40, 50);
    if (millivolts >= 3580) return mapRange(millivolts, 3580, 3620, 30, 40);
    if (millivolts >= 3500) return mapRange(millivolts, 3500, 3580, 20, 30);
    if (millivolts >= 3400) return mapRange(millivolts, 3400, 3500, 10, 20);
    if (millivolts >= 3300) return mapRange(millivolts, 3300, 3400, 0, 10);
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
