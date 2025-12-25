#ifndef SLEEP_MANAGER_H
#define SLEEP_MANAGER_H

#include <Arduino.h>
#include "esp_sleep.h"
#include "esp_task_wdt.h"

class SleepManager {
private:
    // RTC persisted variables (survive deep sleep)
    static RTC_DATA_ATTR time_t rtcEpoch;
    static RTC_DATA_ATTR uint8_t wakeCount;
    static RTC_DATA_ATTR bool lastUpdateSuccess;

public:
    // Initialize sleep manager
    static void init();

    // Enter deep sleep for specified seconds
    static void deepSleep(uint32_t seconds);

    // Get current epoch from RTC persistent memory
    static time_t getEpoch();

    // Set current epoch (call after NTP sync)
    static void setEpoch(time_t epoch);

    // Get wake counter
    static uint8_t getWakeCount();

    // Increment wake counter
    static void incrementWakeCount();

    // Get last update success status
    static bool getLastUpdateSuccess();

    // Set last update success status
    static void setLastUpdateSuccess(bool success);

    // Check if NTP sync is needed
    static bool shouldSyncTime();
};

#endif  // SLEEP_MANAGER_H
