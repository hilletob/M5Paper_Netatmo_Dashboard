#include "sleep_manager.h"
#include "../config.h"
#include <time.h>
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
        ESP_LOGW("sleep", "Hardware RTC time invalid (year=%d), falling back to system time",
                 rtcDate.year);
        time_t sysTime = time(nullptr);
        if (sysTime > 1700000000) {
            return sysTime;
        }
        return 0;
    }

    struct tm timeinfo = {};
    timeinfo.tm_year = rtcDate.year - 1900;
    timeinfo.tm_mon = rtcDate.mon - 1;
    timeinfo.tm_mday = rtcDate.day;
    timeinfo.tm_hour = rtcTime.hour;
    timeinfo.tm_min = rtcTime.min;
    timeinfo.tm_sec = rtcTime.sec;

    time_t epoch = mktime(&timeinfo);

    ESP_LOGD("sleep", "Hardware RTC read: %04d-%02d-%02d %02d:%02d:%02d (epoch=%ld)",
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

    // Read current time from hardware RTC
    time_t hwTime = readHardwareRtc();
    if (hwTime > 0) {
        struct tm* tm = gmtime(&hwTime);
        ESP_LOGI("sleep", "Using hardware RTC time: %04d-%02d-%02d %02d:%02d:%02d UTC",
                 tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                 tm->tm_hour, tm->tm_min, tm->tm_sec);
    } else {
        ESP_LOGW("sleep", "No valid time source available yet");
    }

    ESP_LOGI("sleep", "Sleep manager initialized");
    ESP_LOGI("sleep", "Wake count: %d", wakeCount);
}

void SleepManager::deepSleepUntil(time_t targetEpoch) {
    time_t now = readHardwareRtc();
    uint32_t seconds = 0;

    if (targetEpoch > now) {
        seconds = targetEpoch - now;
    } else {
        seconds = MINIMUM_SLEEP_SEC;
    }

    // Clamp
    if (seconds < MINIMUM_SLEEP_SEC) seconds = MINIMUM_SLEEP_SEC;
    if (seconds > MAXIMUM_SLEEP_SEC) seconds = MAXIMUM_SLEEP_SEC;

    ESP_LOGI("sleep", "Entering deep sleep for %u seconds", seconds);

    // Save state to LittleFS before shutdown
    saveState();

    // Shut down WiFi completely
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(100);

    // Flush serial output before sleeping
    Serial.flush();
    delay(100);

    // For longer sleeps, use absolute RTC alarm to avoid BM8563 rounding errors
    if (seconds > RTC_ALARM_THRESHOLD_SEC && targetEpoch > now) {
        struct tm* tm = gmtime(&targetEpoch);
        rtc_time_t alarmTime;
        alarmTime.hour = tm->tm_hour;
        alarmTime.min = tm->tm_min;
        alarmTime.sec = tm->tm_sec;

        rtc_date_t alarmDate;
        alarmDate.year = -1;
        alarmDate.mon = -1;
        alarmDate.day = tm->tm_mday;
        alarmDate.week = -1;

        ESP_LOGI("sleep", "Using RTC alarm: wake at %02d:%02d:%02d UTC (day %d)",
                 alarmTime.hour, alarmTime.min, alarmTime.sec, alarmDate.day);

        M5.shutdown(alarmDate, alarmTime);
    } else {
        ESP_LOGI("sleep", "Using duration-based wake: %u seconds", seconds);
        M5.shutdown(seconds);
    }

    // If M5.shutdown() returns (shouldn't happen), try ESP32 deep sleep as fallback
    ESP_LOGW("sleep", "M5.shutdown returned unexpectedly, using ESP32 deep sleep");
    M5.EPD.Sleep();
    delay(500);

    uint64_t sleep_us = (uint64_t)seconds * 1000000ULL;
    esp_sleep_enable_timer_wakeup(sleep_us);
    esp_deep_sleep_start();
}

void SleepManager::deepSleep(uint32_t seconds) {
    time_t now = readHardwareRtc();
    deepSleepUntil(now + seconds);
}

time_t SleepManager::getEpoch() {
    return readHardwareRtc();
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
    time_t now = readHardwareRtc();

    if (now == 0) {
        ESP_LOGI("sleep", "Time sync needed: No valid time");
        return true;
    }

    struct tm* tm = gmtime(&now);
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
