#include "cache.h"

bool DataCache::init() {
    if (!LittleFS.begin(true)) {  // Format if mount fails
        ESP_LOGE("cache", "LittleFS mount failed");
        return false;
    }

    ESP_LOGI("cache", "LittleFS initialized");
    ESP_LOGI("cache", "Total: %u bytes, Used: %u bytes",
            LittleFS.totalBytes(), LittleFS.usedBytes());

    return true;
}

bool DataCache::save(const DashboardData& data) {
    ESP_LOGI("cache", "Saving dashboard data to cache");

    JsonDocument doc;

    // Metadata
    doc["cacheTime"] = millis();
    doc["updateTime"] = data.updateTime;

    // Indoor data
    JsonObject indoor = doc["indoor"].to<JsonObject>();
    indoor["temp"] = data.weather.indoor.temperature;
    indoor["humidity"] = data.weather.indoor.humidity;
    indoor["co2"] = data.weather.indoor.co2;
    indoor["noise"] = data.weather.indoor.noise;
    indoor["pressure"] = data.weather.indoor.pressure;
    indoor["tempTrend"] = (int)data.weather.indoor.temperatureTrend;
    indoor["pressTrend"] = (int)data.weather.indoor.pressureTrend;
    indoor["co2Trend"] = (int)data.weather.indoor.co2Trend;
    indoor["minTemp"] = data.weather.indoor.minTemp;
    indoor["maxTemp"] = data.weather.indoor.maxTemp;
    indoor["valid"] = data.weather.indoor.valid;

    // Outdoor data
    JsonObject outdoor = doc["outdoor"].to<JsonObject>();
    outdoor["temp"] = data.weather.outdoor.temperature;
    outdoor["humidity"] = data.weather.outdoor.humidity;
    outdoor["tempTrend"] = (int)data.weather.outdoor.temperatureTrend;
    outdoor["minTemp"] = data.weather.outdoor.minTemp;
    outdoor["maxTemp"] = data.weather.outdoor.maxTemp;
    outdoor["valid"] = data.weather.outdoor.valid;

    // Forecast current
    JsonObject current = doc["current"].to<JsonObject>();
    current["temp"] = data.forecast.current.temperature;
    current["code"] = data.forecast.current.weatherCode;
    current["humidity"] = data.forecast.current.humidity;
    current["valid"] = data.forecast.current.valid;

    // Forecast 3h
    JsonObject f3h = doc["forecast3h"].to<JsonObject>();
    f3h["temp"] = data.forecast.forecast3h.temperature;
    f3h["code"] = data.forecast.forecast3h.weatherCode;
    f3h["precip"] = data.forecast.forecast3h.precipitation;
    f3h["valid"] = data.forecast.forecast3h.valid;

    // Forecast 6h
    JsonObject f6h = doc["forecast6h"].to<JsonObject>();
    f6h["temp"] = data.forecast.forecast6h.temperature;
    f6h["code"] = data.forecast.forecast6h.weatherCode;
    f6h["precip"] = data.forecast.forecast6h.precipitation;
    f6h["valid"] = data.forecast.forecast6h.valid;

    // Battery
    doc["batteryMv"] = data.batteryVoltage;
    doc["batteryPct"] = data.batteryPercent;

    // Write to file
    File file = LittleFS.open(CACHE_FILE, "w");
    if (!file) {
        ESP_LOGE("cache", "Failed to open cache file for writing");
        return false;
    }

    size_t written = serializeJson(doc, file);
    file.close();

    if (written == 0) {
        ESP_LOGE("cache", "Failed to write cache file");
        return false;
    }

    ESP_LOGI("cache", "Cache saved: %u bytes", written);
    return true;
}

bool DataCache::load(DashboardData& data) {
    if (!LittleFS.exists(CACHE_FILE)) {
        ESP_LOGW("cache", "Cache file does not exist");
        return false;
    }

    File file = LittleFS.open(CACHE_FILE, "r");
    if (!file) {
        ESP_LOGE("cache", "Failed to open cache file for reading");
        return false;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        ESP_LOGE("cache", "Failed to parse cache file: %s", error.c_str());
        return false;
    }

    // Check cache age
    unsigned long cacheTime = doc["cacheTime"] | 0;
    unsigned long ageMs = millis() - cacheTime;
    unsigned long ageSec = ageMs / 1000;

    ESP_LOGI("cache", "Cache age: %lu seconds", ageSec);

    if (ageSec > CACHE_MAX_AGE_SEC) {
        ESP_LOGW("cache", "Cache is too old (> %d seconds)", CACHE_MAX_AGE_SEC);
        return false;
    }

    // Load data
    data.updateTime = doc["updateTime"] | 0;

    // Indoor data
    JsonObject indoor = doc["indoor"];
    data.weather.indoor.temperature = indoor["temp"] | 0.0f;
    data.weather.indoor.humidity = indoor["humidity"] | 0;
    data.weather.indoor.co2 = indoor["co2"] | 0;
    data.weather.indoor.noise = indoor["noise"] | 0;
    data.weather.indoor.pressure = indoor["pressure"] | 0;
    data.weather.indoor.temperatureTrend = (Trend)(indoor["tempTrend"] | 0);
    data.weather.indoor.pressureTrend = (Trend)(indoor["pressTrend"] | 0);
    data.weather.indoor.co2Trend = (Trend)(indoor["co2Trend"] | 0);
    data.weather.indoor.minTemp = indoor["minTemp"] | 0.0f;
    data.weather.indoor.maxTemp = indoor["maxTemp"] | 0.0f;
    data.weather.indoor.valid = indoor["valid"] | false;

    // Outdoor data
    JsonObject outdoor = doc["outdoor"];
    data.weather.outdoor.temperature = outdoor["temp"] | 0.0f;
    data.weather.outdoor.humidity = outdoor["humidity"] | 0;
    data.weather.outdoor.temperatureTrend = (Trend)(outdoor["tempTrend"] | 0);
    data.weather.outdoor.minTemp = outdoor["minTemp"] | 0.0f;
    data.weather.outdoor.maxTemp = outdoor["maxTemp"] | 0.0f;
    data.weather.outdoor.valid = outdoor["valid"] | false;

    // Forecast current
    JsonObject current = doc["current"];
    data.forecast.current.temperature = current["temp"] | 0.0f;
    data.forecast.current.weatherCode = current["code"] | 0;
    data.forecast.current.humidity = current["humidity"] | 0;
    data.forecast.current.valid = current["valid"] | false;

    // Forecast 3h
    JsonObject f3h = doc["forecast3h"];
    data.forecast.forecast3h.temperature = f3h["temp"] | 0.0f;
    data.forecast.forecast3h.weatherCode = f3h["code"] | 0;
    data.forecast.forecast3h.precipitation = f3h["precip"] | 0.0f;
    data.forecast.forecast3h.valid = f3h["valid"] | false;

    // Forecast 6h
    JsonObject f6h = doc["forecast6h"];
    data.forecast.forecast6h.temperature = f6h["temp"] | 0.0f;
    data.forecast.forecast6h.weatherCode = f6h["code"] | 0;
    data.forecast.forecast6h.precipitation = f6h["precip"] | 0.0f;
    data.forecast.forecast6h.valid = f6h["valid"] | false;

    // Battery
    data.batteryVoltage = doc["batteryMv"] | 0;
    data.batteryPercent = doc["batteryPct"] | 0;

    ESP_LOGI("cache", "Cache loaded successfully");
    return true;
}

bool DataCache::isValid() {
    if (!LittleFS.exists(CACHE_FILE)) {
        return false;
    }

    return (getAgeSeconds() <= CACHE_MAX_AGE_SEC);
}

unsigned long DataCache::getAgeSeconds() {
    if (!LittleFS.exists(CACHE_FILE)) {
        return UINT32_MAX;  // Very old
    }

    File file = LittleFS.open(CACHE_FILE, "r");
    if (!file) {
        return UINT32_MAX;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        return UINT32_MAX;
    }

    unsigned long cacheTime = doc["cacheTime"] | 0;
    unsigned long ageMs = millis() - cacheTime;
    return ageMs / 1000;
}

void DataCache::clear() {
    if (LittleFS.exists(CACHE_FILE)) {
        LittleFS.remove(CACHE_FILE);
        ESP_LOGI("cache", "Cache cleared");
    }
}
