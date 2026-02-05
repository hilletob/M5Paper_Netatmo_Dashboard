#ifndef BATTERY_H
#define BATTERY_H

#include <Arduino.h>

struct BatteryStatus {
    uint32_t millivolts;
    uint8_t percent;
    const char* label;
    bool charging;
    bool externalPower;

    BatteryStatus()
        : millivolts(0), percent(0), label(""), charging(false), externalPower(false) {}
};

// Estimate battery percentage and state based on voltage readings.
BatteryStatus evaluateBattery(uint32_t millivolts);

#endif  // BATTERY_H
