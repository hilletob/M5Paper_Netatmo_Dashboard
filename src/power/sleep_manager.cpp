#include "sleep_manager.h"
#include <time.h>
#include <M5EPD.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

// File path for persistent state
static const char* STATE_FILE = "/sleep_state.json";

// Static variables (loaded from LittleFS on init)
time_t SleepManager::rtcEpoch = 0;
uint8_t SleepManager::wakeCount = 0;
bool SleepManager::lastUpdateSuccess = false;

void SleepManager::loadState() {
    if (!LittleFS.exists(STATE_FILE)) {
        ESP_LOGI("sleep", "No state file found - this is first boot");
        wakeCount = 0;  // Explicitly set to 0 for first boot
        return;
    }

    File file = LittleFS.open(STATE_FILE, "r");
    if (!file) {
        ESP_LOGW("sleep", "Failed to open state file");
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        ESP_LOGW("sleep", "Failed to parse state file: %s", error.c_str());
        return;
    }

    rtcEpoch = doc["epoch"] | 0L;
    wakeCount = doc["wakeCount"] | 0;
    lastUpdateSuccess = doc["lastSuccess"] | false;

    ESP_LOGI("sleep", "State loaded: epoch=%ld, wakeCount=%d, lastSuccess=%d",
             rtcEpoch, wakeCount, lastUpdateSuccess);
}

void SleepManager::saveState() {
    File file = LittleFS.open(STATE_FILE, "w");
    if (!file) {
        ESP_LOGE("sleep", "Failed to create state file");
        return;
    }

    JsonDocument doc;
    doc["epoch"] = rtcEpoch;
    doc["wakeCount"] = wakeCount;
    doc["lastSuccess"] = lastUpdateSuccess;

    serializeJson(doc, file);
    file.close();

    ESP_LOGI("sleep", "State saved to LittleFS");
}

void SleepManager::init() {
    // Disable watchdog timer (ePaper display update takes 15-20 seconds)
    esp_task_wdt_deinit();

    // Initialize LittleFS if not already mounted
    if (!LittleFS.begin(true)) {  // true = format if mount fails
        ESP_LOGE("sleep", "Failed to mount LittleFS");
    }

    // Load persistent state from LittleFS
    loadState();

    // Read current time from M5Paper's hardware RTC (BM8563)
    // This RTC keeps time even when the device is powered off
    rtc_time_t rtcTime;
    rtc_date_t rtcDate;
    M5.RTC.getTime(&rtcTime);
    M5.RTC.getDate(&rtcDate);

    // Convert RTC time to epoch
    struct tm timeinfo = {};
    timeinfo.tm_year = rtcDate.year - 1900;
    timeinfo.tm_mon = rtcDate.mon - 1;
    timeinfo.tm_mday = rtcDate.day;
    timeinfo.tm_hour = rtcTime.hour;
    timeinfo.tm_min = rtcTime.min;
    timeinfo.tm_sec = rtcTime.sec;

    time_t hwRtcEpoch = mktime(&timeinfo);

    // Use hardware RTC time if it looks valid (year >= 2024)
    if (rtcDate.year >= 2024) {
        rtcEpoch = hwRtcEpoch;
        ESP_LOGI("sleep", "Using hardware RTC time: %04d-%02d-%02d %02d:%02d:%02d",
                 rtcDate.year, rtcDate.mon, rtcDate.day,
                 rtcTime.hour, rtcTime.min, rtcTime.sec);
    } else {
        ESP_LOGW("sleep", "Hardware RTC time invalid (year=%d), keeping saved epoch", rtcDate.year);
    }

    ESP_LOGI("sleep", "Sleep manager initialized");
    ESP_LOGI("sleep", "Wake count: %d", wakeCount);
    ESP_LOGI("sleep", "RTC epoch: %ld", rtcEpoch);
}

void SleepManager::deepSleep(uint32_t seconds) {
    // Update RTC time before sleeping
    rtcEpoch += seconds;

    ESP_LOGI("sleep", "Entering deep sleep for %u seconds", seconds);
    ESP_LOGI("sleep", "Next RTC epoch: %ld", rtcEpoch);

    // Save state to LittleFS before shutdown
    saveState();

    // Shut down WiFi completely
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(100);

    // Flush serial output before sleeping
    Serial.flush();
    delay(100);

    // Use M5Paper's hardware shutdown with RTC wake-up
    // This properly cuts power and uses the hardware RTC (BM8563) for wake-up
    ESP_LOGI("sleep", "Calling M5.shutdown(%u)...", seconds);
    M5.shutdown(seconds);

    // If M5.shutdown() returns (shouldn't happen), try ESP32 deep sleep as fallback
    ESP_LOGW("sleep", "M5.shutdown returned unexpectedly, using ESP32 deep sleep");
    M5.EPD.Sleep();
    delay(500);

    uint64_t sleep_us = (uint64_t)seconds * 1000000ULL;
    esp_sleep_enable_timer_wakeup(sleep_us);
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
