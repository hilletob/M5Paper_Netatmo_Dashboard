#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <M5EPD.h>
#include "config.h"

// API clients
#include "api/netatmo_client.h"
#include "api/meteo_client.h"

// Display
#include "display/layout.h"
#include "display/widgets.h"
#include "display/fonts.h"

// Data
#include "data/weather_data.h"
#include "data/cache.h"

// Power management
#include "power/sleep_manager.h"
#include "power/battery.h"

// Global objects
M5EPD_Canvas canvas(&M5.EPD);

NetatmoClient netatmoClient;
MeteoClient meteoClient;

// Function prototypes
bool connectWiFi();
void disconnectWiFi();
bool syncTime();
bool fetchWeatherData(DashboardData& data);
void updateDisplay(const DashboardData& data);
unsigned long calculateNextWakeTime(unsigned long netatmoLastUpdate, bool& isFallback);
void enterSleep(unsigned long nextWakeTime);

void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    delay(1000);  // Wait for serial to stabilize

    ESP_LOGI("main", "=== ESP32 Weather Dashboard Starting ===");
    ESP_LOGI("main", "Build date: %s %s", __DATE__, __TIME__);

    // Suppress noisy font_render debug messages (font_cache_init spam)
    esp_log_level_set("font_render", ESP_LOG_WARN);

    // M5Paper hardware initialization
    M5.begin();
    M5.EPD.SetRotation(90);  // Portrait mode
    M5.RTC.begin();
    M5.RTC.setAlarmIRQ(-1);  // Clear any stale RTC alarm from previous wake

    // Create canvas
    canvas.createCanvas(540, 960);
    canvas.fillCanvas(0);

    // Initialize TTF fonts
    initTTFFonts(canvas);
    ESP_LOGI("main", "TTF fonts initialized");

    ESP_LOGI("main", "M5Paper initialized (540×960 portrait)");

    // Initialize sleep manager (disables watchdog)
    SleepManager::init();
    SleepManager::incrementWakeCount();

    // Initialize LittleFS for caching
    if (!DataCache::init()) {
        ESP_LOGE("main", "Failed to initialize cache");
    }

    // Configure timezone for Switzerland (must be done on every boot as it doesn't persist)
    // POSIX TZ string: CET-1CEST,M3.5.0,M10.5.0/3
    // - CET-1: Central European Time, UTC+1
    // - CEST: Central European Summer Time
    // - M3.5.0: DST starts last Sunday in March at 02:00
    // - M10.5.0/3: DST ends last Sunday in October at 03:00
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();
    ESP_LOGI("main", "Timezone configured: CET/CEST");

    // Show loading screen only on first boot; keep last display on subsequent wakes
    if (SleepManager::getWakeCount() == 1) {
        ESP_LOGI("main", "Wake #%d - showing initial loading screen", SleepManager::getWakeCount());
        M5.EPD.Clear(true);

        canvas.fillCanvas(0);
        canvas.setTextColor(15, 0);
        canvas.setTextDatum(MC_DATUM);  // Middle-center for centered text

        setBoldFont(canvas, 48);
        canvas.drawString("Wetter Dashboard", SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 60);

        setRegularFont(canvas, 28);
        canvas.drawString(LOCATION_NAME, SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 10);

        setRegularFont(canvas, 28);
        canvas.drawString("Wettergötter werden konsultiert...", SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 40);

        setRegularFont(canvas, 28);
        canvas.drawString("(Erstmalige Initialisierung)", SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 80);

        canvas.pushCanvas(0, 0, UPDATE_MODE_GC16);
        delay(500);  // Brief pause to ensure display update completes
    } else {
        ESP_LOGI("main", "Wake #%d - keeping previous display during data refresh", SleepManager::getWakeCount());
    }

    // Prepare dashboard data
    DashboardData dashboardData;
    bool dataAvailable = false;

    // Connect WiFi
    if (connectWiFi()) {
        // Sync time if needed
        if (SleepManager::shouldSyncTime()) {
            if (syncTime()) {
                ESP_LOGI("main", "Time synchronized successfully");
            } else {
                ESP_LOGW("main", "Time sync failed, using RTC time");
            }
        }

        // Fetch fresh weather data
        if (fetchWeatherData(dashboardData)) {
            ESP_LOGI("main", "Weather data fetched successfully");

            // Save to cache for offline use
            DataCache::save(dashboardData);
            dataAvailable = true;
            SleepManager::setLastUpdateSuccess(true);
        } else {
            ESP_LOGE("main", "Failed to fetch weather data");
            SleepManager::setLastUpdateSuccess(false);
        }
    } else {
        ESP_LOGE("main", "WiFi connection failed");
    }

    // CRITICAL: Disconnect WiFi before display operations
    disconnectWiFi();

    // If fresh data not available, try to load from cache
    if (!dataAvailable) {
        ESP_LOGI("main", "Attempting to load cached data");
        if (DataCache::load(dashboardData)) {
            ESP_LOGI("main", "Loaded data from cache (age: %lu sec)",
                    DataCache::getAgeSeconds());
            dataAvailable = true;
        }
    }

    // Read battery (M5Paper) with status mapping
    BatteryStatus battery = evaluateBattery(M5.getBatteryVoltage());

    dashboardData.batteryVoltage = battery.millivolts;
    dashboardData.batteryPercent = battery.percent;
    dashboardData.updateTime = SleepManager::getEpoch();

    ESP_LOGI("main", "Battery: %d mV (%d%%, %s)%s", battery.millivolts,
            battery.percent, battery.label,
            battery.charging ? " [charging]" : (battery.externalPower ? " [usb]" : ""));

    // Check for critically low battery (below 3.3V)
    if (battery.millivolts > 0 && battery.millivolts < 3300) {
        ESP_LOGW("main", "CRITICAL: Battery voltage too low for reliable deep sleep!");
        ESP_LOGW("main", "Device may not wake up. Please charge battery.");
    }

    // Calculate next wake time before display update (so header can show it)
    dashboardData.nextWakeTime = calculateNextWakeTime(dashboardData.weather.timestamp, dashboardData.isFallback);

    // Update display
    if (dataAvailable) {
        updateDisplay(dashboardData);
    } else {
        // Show error screen
        ESP_LOGE("main", "No data available (fresh or cached)");
        canvas.fillCanvas(0);
        canvas.setTextColor(15, 0);
        canvas.setTextDatum(MC_DATUM);

        // Error message with TTF fonts
        setBoldFont(canvas, 48);  // Large bold
        canvas.drawString("FEHLER", SCREEN_WIDTH/2, SCREEN_HEIGHT/2 - 50);

        setRegularFont(canvas, 28);  // Medium
        canvas.drawString("Keine Daten verfügbar", SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 10);

        setRegularFont(canvas, 28);  // Medium
        canvas.drawString("WiFi und API prüfen", SCREEN_WIDTH/2, SCREEN_HEIGHT/2 + 50);

        canvas.pushCanvas(0, 0, UPDATE_MODE_GC16);
        M5.EPD.Sleep();
    }

    // Enter deep sleep until nextWakeTime
    enterSleep(dashboardData.nextWakeTime);
}

void loop() {
    // Should never reach here (deep sleep in setup)
    ESP_LOGE("main", "ERROR: Reached loop() - this should not happen");
    delay(60000);
}

bool connectWiFi() {
    ESP_LOGI("wifi", "Connecting to WiFi: %s", WIFI_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime > WIFI_TIMEOUT_MS) {
            ESP_LOGE("wifi", "Connection timeout");
            return false;
        }
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    ESP_LOGI("wifi", "Connected! IP: %s", WiFi.localIP().toString().c_str());
    ESP_LOGI("wifi", "RSSI: %d dBm", WiFi.RSSI());

    return true;
}

void disconnectWiFi() {
    ESP_LOGI("wifi", "Disconnecting WiFi");
    WiFi.disconnect(true, true);  // Disconnect and power off radio
    WiFi.mode(WIFI_OFF);
    delay(500);  // Wait for full shutdown
}

bool syncTime() {
    ESP_LOGI("time", "Syncing time via NTP");

    // Configure NTP with timezone (ESP32-specific function that handles timezone correctly)
    // POSIX TZ string: CET-1CEST,M3.5.0,M10.5.0/3
    configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", NTP_SERVER_1, NTP_SERVER_2);

    // Wait for time sync (max 15 seconds)
    int retries = 30;
    while (retries > 0) {
        time_t now = time(nullptr);
        if (now > 1700000000) {  // Year > 2023
            struct tm* timeinfo = gmtime(&now);
            ESP_LOGI("time", "Time synced: %04d-%02d-%02d %02d:%02d:%02d UTC",
                    timeinfo->tm_year + 1900,
                    timeinfo->tm_mon + 1,
                    timeinfo->tm_mday,
                    timeinfo->tm_hour,
                    timeinfo->tm_min,
                    timeinfo->tm_sec);

            SleepManager::writeHardwareRtc(now);
            return true;
        }

        delay(500);
        retries--;
    }

    ESP_LOGE("time", "NTP sync timeout");
    return false;
}

bool fetchWeatherData(DashboardData& data) {
    ESP_LOGI("main", "Fetching weather data from APIs");

    bool success = true;

    // Fetch Netatmo data
    if (!netatmoClient.getWeatherData(data.weather)) {
        ESP_LOGE("main", "Failed to fetch Netatmo data");
        success = false;
    }

    // Fetch met.no forecast data
    if (!meteoClient.getForecast(data.forecast)) {
        ESP_LOGW("main", "Failed to fetch forecast data");
    }

    // Even if one API fails, we can still show partial data
    return (data.weather.indoor.valid || data.weather.outdoor.valid);
}

void updateDisplay(const DashboardData& data) {
    ESP_LOGI("display", "Updating ePaper display");

    // Hard-refresh on every wake to clear ghosting artifacts (graue Linien)
    M5.EPD.Clear(true);

    // Clear canvas and draw dashboard
    canvas.fillCanvas(0);
    drawDashboard(canvas, data);

    // Push canvas to display (takes ~2 seconds)
    ESP_LOGI("display", "Pushing canvas to display...");
    unsigned long startTime = millis();
    canvas.pushCanvas(0, 0, UPDATE_MODE_GC16);
    unsigned long duration = millis() - startTime;
    ESP_LOGI("display", "Display updated in %lu ms", duration);

    // Put display to sleep
    M5.EPD.Sleep();
}

unsigned long calculateNextWakeTime(unsigned long netatmoLastUpdate, bool& isFallback) {
    time_t now = time(nullptr);
    isFallback = false;

    if (netatmoLastUpdate == 0) {
        // No data at all — full fallback
        ESP_LOGW("sleep", "No Netatmo timestamp, fallback %d sec", FALLBACK_SLEEP_SEC);
        isFallback = true;
        return now + FALLBACK_SLEEP_SEC;
    }

    time_t candidate = netatmoLastUpdate + UPDATE_INTERVAL_SEC;

    if (candidate > now) {
        // Normal case: wake 11 min after last Netatmo update
        ESP_LOGI("sleep", "Next wake in %ld sec (11 min after Netatmo update)", (long)(candidate - now));
        return candidate;
    }

    // Data is stale (candidate is in the past) — short retry
    ESP_LOGW("sleep", "Data stale by %ld sec, retrying in %d sec", (long)(now - candidate), MINIMUM_SLEEP_SEC);
    isFallback = true;
    return now + MINIMUM_SLEEP_SEC;
}

void enterSleep(unsigned long nextWakeTime) {
    time_t now = time(nullptr);
    long sleepSeconds = (long)nextWakeTime - (long)now;
    if (sleepSeconds < MINIMUM_SLEEP_SEC) sleepSeconds = MINIMUM_SLEEP_SEC;

    ESP_LOGI("main", "=== Sleeping %ld sec (%ld min) ===", sleepSeconds, sleepSeconds / 60);
    SleepManager::deepSleepUntil((time_t)nextWakeTime);
}
