#include "sleep_manager.h"
#include "../config.h"
#include <time.h>
#include <sys/time.h>
#include <M5EPD.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

// File path for persistent state
static const char* STATE_FILE = "/sleep_state.json";

// Static variables (loaded from LittleFS on init)
uint8_t SleepManager::wakeCount = 0;
bool SleepManager::lastUpdateSuccess = false;

void SleepManager::loadState() {
    if (!LittleFS.exists(STATE_FILE)) {
        ESP_LOGI("sleep", "No state file found - this is first boot");
        wakeCount = 0;
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

    wakeCount = doc["wakeCount"] | 0;
    lastUpdateSuccess = doc["lastSuccess"] | false;

    ESP_LOGI("sleep", "State loaded: wakeCount=%d, lastSuccess=%d",
             wakeCount, lastUpdateSuccess);
}

void SleepManager::saveState() {
    File file = LittleFS.open(STATE_FILE, "w");
    if (!file) {
        ESP_LOGE("sleep", "Failed to create state file");
        return;
    }

    JsonDocument doc;
    doc["wakeCount"] = wakeCount;
    doc["lastSuccess"] = lastUpdateSuccess;

    serializeJson(doc, file);
    file.close();

    ESP_LOGI("sleep", "State saved to LittleFS");
}

time_t SleepManager::readHardwareRtc() {
    rtc_time_t rtcTime;
    rtc_date_t rtcDate;
    M5.RTC.getTime(&rtcTime);
    M5.RTC.getDate(&rtcDate);

    // Validate year
    if (rtcDate.year < 2024) {
        ESP_LOGW("sleep", "Hardware RTC time invalid (year=%d)", rtcDate.year);
        return 0;
    }

    struct tm timeinfo = {};
    timeinfo.tm_year = rtcDate.year - 1900;
    timeinfo.tm_mon = rtcDate.mon - 1;
    timeinfo.tm_mday = rtcDate.day;
    timeinfo.tm_hour = rtcTime.hour;
    timeinfo.tm_min = rtcTime.min;
    timeinfo.tm_sec = rtcTime.sec;
    timeinfo.tm_isdst = 0;

    // NOTE: This function must only be called BEFORE setenv("TZ",...) in setup(),
    // so that mktime() interprets the hardware RTC values as UTC (the default).
    // After TZ is set, use time(nullptr) instead — the system clock is seeded
    // from this value in init().
    time_t epoch = mktime(&timeinfo);

    ESP_LOGI("sleep", "Hardware RTC: %04d-%02d-%02d %02d:%02d:%02d UTC (epoch=%ld)",
             rtcDate.year, rtcDate.mon, rtcDate.day,
             rtcTime.hour, rtcTime.min, rtcTime.sec, epoch);

    return epoch;
}

void SleepManager::writeHardwareRtc(time_t epoch) {
    struct tm* tm = gmtime(&epoch);

    rtc_time_t rtcTime;
    rtcTime.hour = tm->tm_hour;
    rtcTime.min = tm->tm_min;
    rtcTime.sec = tm->tm_sec;

    rtc_date_t rtcDate;
    rtcDate.year = tm->tm_year + 1900;
    rtcDate.mon = tm->tm_mon + 1;
    rtcDate.day = tm->tm_mday;
    rtcDate.week = tm->tm_wday;

    M5.RTC.setTime(&rtcTime);
    M5.RTC.setDate(&rtcDate);

    ESP_LOGI("sleep", "Hardware RTC set to: %04d-%02d-%02d %02d:%02d:%02d UTC",
             rtcDate.year, rtcDate.mon, rtcDate.day,
             rtcTime.hour, rtcTime.min, rtcTime.sec);
}

void SleepManager::init() {
    // Disable watchdog timer (ePaper display update takes 15-20 seconds)
    esp_task_wdt_deinit();

    // Initialize LittleFS if not already mounted
    if (!LittleFS.begin(true)) {  // true = format if mount fails
        ESP_LOGE("sleep", "Failed to mount LittleFS");
    }

    // Load persistent state from LittleFS (wakeCount + lastSuccess only)
    loadState();

    // Read hardware RTC and seed the system clock.
    // IMPORTANT: This runs BEFORE TZ is configured in setup(), so mktime()
    // correctly treats the RTC values as UTC.
    time_t hwTime = readHardwareRtc();
    if (hwTime > 0) {
        struct timeval tv = { hwTime, 0 };
        settimeofday(&tv, NULL);
        ESP_LOGI("sleep", "System clock seeded from hardware RTC (epoch=%ld)", hwTime);
    } else {
        ESP_LOGW("sleep", "No valid time source available yet");
    }

    ESP_LOGI("sleep", "Sleep manager initialized, wake count: %d", wakeCount);
}

void SleepManager::deepSleepUntil(time_t targetEpoch) {
    time_t now = time(nullptr);
    uint32_t seconds = 0;

    if (targetEpoch > now) {
        seconds = targetEpoch - now;
    } else {
        seconds = MINIMUM_SLEEP_SEC;
    }

    // Clamp
    if (seconds < MINIMUM_SLEEP_SEC) seconds = MINIMUM_SLEEP_SEC;
    if (seconds > MAXIMUM_SLEEP_SEC) seconds = MAXIMUM_SLEEP_SEC;

    ESP_LOGI("sleep", "Entering deep sleep for %u seconds (%u min)", seconds, seconds / 60);

    // Save state to LittleFS before shutdown
    saveState();

    // Shut down WiFi completely
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(100);

    // Flush serial output before sleeping
    Serial.flush();
    delay(100);

    // Simple duration-based shutdown via BM8563 timer
    M5.shutdown(seconds);

    // If M5.shutdown() returns (shouldn't happen), try ESP32 deep sleep as fallback
    ESP_LOGW("sleep", "M5.shutdown returned unexpectedly, using ESP32 deep sleep");
    M5.EPD.Sleep();
    delay(500);

    uint64_t sleep_us = (uint64_t)seconds * 1000000ULL;
    esp_sleep_enable_timer_wakeup(sleep_us);
    esp_deep_sleep_start();
}

void SleepManager::deepSleep(uint32_t seconds) {
    deepSleepUntil(time(nullptr) + seconds);
}

time_t SleepManager::getEpoch() {
    return time(nullptr);
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
    time_t now = time(nullptr);

    if (now < 1700000000) {
        ESP_LOGI("sleep", "Time sync needed: System clock not set");
        return true;
    }

    struct tm* tm = gmtime(&now);

    // Daily sync around midnight UTC
    if (tm->tm_hour == 0 && tm->tm_min == 0) {
        ESP_LOGI("sleep", "Time sync needed: Daily sync at midnight");
        return true;
    }

    return false;
}
