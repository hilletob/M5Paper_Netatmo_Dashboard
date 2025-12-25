#ifndef BATTERY_H
#define BATTERY_H

#include <Arduino.h>
#include "../config.h"

class Battery {
public:
    // Initialize battery monitoring
    static void init();

    // Read battery voltage in millivolts
    static uint32_t readVoltage();

    // Read battery percentage (0-100%)
    static uint8_t readPercentage();

    // Read both voltage and percentage
    static void read(uint32_t& voltageMv, uint8_t& percent);
};

#endif  // BATTERY_H
