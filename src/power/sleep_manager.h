#ifndef SLEEP_MANAGER_H
#define SLEEP_MANAGER_H

#include <Arduino.h>
#include "esp_sleep.h"
#include "esp_task_wdt.h"

class SleepManager {
private:
    // Persisted variables (stored in LittleFS)
    static uint8_t wakeCount;
    static bool lastUpdateSuccess;

    // Load/save state from/to LittleFS
    static void loadState();
    static void saveState();

public:
    // Initialize sleep manager
    static void init();

    // Read current epoch from BM8563 hardware RTC
    static time_t readHardwareRtc();

    // Write NTP-synced time to BM8563 hardware RTC
    static void writeHardwareRtc(time_t epoch);

    // Enter deep sleep until an absolute target time
    // Uses RTC alarm for sleeps > RTC_ALARM_THRESHOLD_SEC, duration otherwise
    static void deepSleepUntil(time_t targetEpoch);

    // Enter deep sleep for specified seconds (wrapper around deepSleepUntil)
    static void deepSleep(uint32_t seconds);

    // Get current epoch from hardware RTC
    static time_t getEpoch();

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
