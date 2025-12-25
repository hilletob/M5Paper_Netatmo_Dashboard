#include "sleep_manager.h"
#include <time.h>

// Initialize static RTC variables
RTC_DATA_ATTR time_t SleepManager::rtcEpoch = 0;
RTC_DATA_ATTR uint8_t SleepManager::wakeCount = 0;
RTC_DATA_ATTR bool SleepManager::lastUpdateSuccess = false;

void SleepManager::init() {
    // Disable watchdog timer (ePaper display update takes 15-20 seconds)
    esp_task_wdt_deinit();

    ESP_LOGI("sleep", "Sleep manager initialized");
    ESP_LOGI("sleep", "Wake count: %d", wakeCount);
    ESP_LOGI("sleep", "RTC epoch: %ld", rtcEpoch);
}

void SleepManager::deepSleep(uint32_t seconds) {
    // Update RTC time before sleeping
    rtcEpoch += seconds;

    ESP_LOGI("sleep", "Entering deep sleep for %u seconds", seconds);
    ESP_LOGI("sleep", "Next RTC epoch: %ld", rtcEpoch);

    // Configure timer wake-up
    esp_sleep_enable_timer_wakeup(seconds * 1000000ULL);  // Convert to microseconds

    // Give time for serial output and display to finish
    delay(1000);

    // Enter deep sleep (never returns)
    esp_deep_sleep_start();
}

time_t SleepManager::getEpoch() {
    return rtcEpoch;
}

void SleepManager::setEpoch(time_t epoch) {
    rtcEpoch = epoch;
    ESP_LOGI("sleep", "RTC epoch set to: %ld", rtcEpoch);
}

uint8_t SleepManager::getWakeCount() {
    return wakeCount;
}

void SleepManager::incrementWakeCount() {
    wakeCount++;
    ESP_LOGD("sleep", "Wake count incremented to: %d", wakeCount);
}

bool SleepManager::getLastUpdateSuccess() {
    return lastUpdateSuccess;
}

void SleepManager::setLastUpdateSuccess(bool success) {
    lastUpdateSuccess = success;
    ESP_LOGD("sleep", "Last update success: %s", success ? "true" : "false");
}

bool SleepManager::shouldSyncTime() {
    // Sync time if:
    // 1. Never synced (rtcEpoch == 0)
    // 2. Year is unrealistic (< 2024)
    // 3. It's around midnight (23:59:30 - 00:00:30 UTC)

    if (rtcEpoch == 0) {
        ESP_LOGI("sleep", "Time sync needed: Never synced");
        return true;
    }

    struct tm* tm = gmtime(&rtcEpoch);
    if (tm->tm_year + 1900 < 2024) {
        ESP_LOGI("sleep", "Time sync needed: Year is %d", tm->tm_year + 1900);
        return true;
    }

    // Check if around midnight (daily sync)
    if (tm->tm_hour == 0 && tm->tm_min == 0) {
        ESP_LOGI("sleep", "Time sync needed: Daily sync at midnight");
        return true;
    }

    return false;
}
