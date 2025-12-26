#include "netatmo_client.h"
#include "http_utils.h"

NetatmoClient::NetatmoClient() : accessToken(""), tokenExpiry(0) {
}

bool NetatmoClient::refreshAccessToken() {
    ESP_LOGI("netatmo", "Refreshing OAuth2 access token");

    // Build form data for token refresh
    String formData = "grant_type=refresh_token";
    formData += "&refresh_token=";
    formData += NETATMO_REFRESH_TOKEN;
    formData += "&client_id=";
    formData += NETATMO_CLIENT_ID;
    formData += "&client_secret=";
    formData += NETATMO_CLIENT_SECRET;

    // Make POST request to token endpoint
    JsonDocument doc;
    if (!HTTPUtils::httpPostForm(NETATMO_TOKEN_URL, formData.c_str(), doc)) {
        ESP_LOGE("netatmo", "Token refresh failed");
        return false;
    }

    // Parse response
    if (!doc["access_token"].is<const char*>()) {
        ESP_LOGE("netatmo", "No access_token in response");
        return false;
    }

    accessToken = doc["access_token"].as<String>();
    int expiresIn = doc["expires_in"] | 10800;  // Default 3 hours
    tokenExpiry = millis() + (expiresIn * 1000);

    ESP_LOGI("netatmo", "Token refreshed, expires in %d seconds", expiresIn);
    return true;
}

bool NetatmoClient::ensureValidToken() {
    // Check if token is still valid (with 60s buffer)
    if (accessToken.length() > 0 && millis() < (tokenExpiry - 60000)) {
        return true;
    }

    return refreshAccessToken();
}

void NetatmoClient::parseIndoorData(JsonObject device, IndoorData& indoor) {
    JsonObject dashboard = device["dashboard_data"];
    if (!dashboard) {
        ESP_LOGW("netatmo", "No dashboard_data in device");
        return;
    }

    indoor.temperature = dashboard["Temperature"] | 0.0f;
    indoor.humidity = dashboard["Humidity"] | 0;
    indoor.co2 = dashboard["CO2"] | 0;
    indoor.noise = dashboard["Noise"] | 0;

    // Try multiple pressure field names
    if (dashboard.containsKey("Pressure")) {
        indoor.pressure = dashboard["Pressure"];
        ESP_LOGD("netatmo", "Got pressure from 'Pressure' field: %d", indoor.pressure);
    } else if (dashboard.containsKey("AbsolutePressure")) {
        indoor.pressure = dashboard["AbsolutePressure"];
        ESP_LOGD("netatmo", "Got pressure from 'AbsolutePressure' field: %d", indoor.pressure);
    } else if (dashboard.containsKey("pressure")) {
        indoor.pressure = dashboard["pressure"];
        ESP_LOGD("netatmo", "Got pressure from 'pressure' field: %d", indoor.pressure);
    } else {
        indoor.pressure = 0;
        ESP_LOGW("netatmo", "No pressure field found in dashboard_data!");
        // Log all available keys for debugging
        for (JsonPair kv : dashboard) {
            ESP_LOGD("netatmo", "Available key: %s", kv.key().c_str());
        }
    }

    // Parse trends
    const char* tempTrend = dashboard["temp_trend"] | "unknown";
    const char* pressTrend = dashboard["pressure_trend"] | "unknown";
    const char* co2Trend = dashboard["co2_trend"];

    indoor.temperatureTrend = stringToTrend(tempTrend);
    indoor.pressureTrend = stringToTrend(pressTrend);

    // CO2 trend is not provided by Netatmo API, default to STABLE
    if (co2Trend && strlen(co2Trend) > 0) {
        indoor.co2Trend = stringToTrend(co2Trend);
        ESP_LOGD("netatmo", "Trends - Temp: %s, Pressure: %s, CO2: %s", tempTrend, pressTrend, co2Trend);
    } else {
        indoor.co2Trend = Trend::STABLE;
        ESP_LOGD("netatmo", "Trends - Temp: %s, Pressure: %s, CO2: stable (not provided by API)", tempTrend, pressTrend);
    }

    // Parse min/max temperatures
    indoor.minTemp = dashboard["min_temp"] | 0.0f;
    indoor.maxTemp = dashboard["max_temp"] | 0.0f;
    indoor.dateMinTemp = dashboard["date_min_temp"] | 0;
    indoor.dateMaxTemp = dashboard["date_max_temp"] | 0;

    indoor.valid = true;

    ESP_LOGI("netatmo", "Indoor: %.1f°C, %d%% RH, %d ppm CO2, %d mbar",
            indoor.temperature, indoor.humidity, indoor.co2, indoor.pressure);
}

void NetatmoClient::parseOutdoorData(JsonObject module, OutdoorData& outdoor) {
    JsonObject dashboard = module["dashboard_data"];
    if (!dashboard) {
        ESP_LOGW("netatmo", "No dashboard_data in outdoor module");
        return;
    }

    outdoor.temperature = dashboard["Temperature"] | 0.0f;
    outdoor.humidity = dashboard["Humidity"] | 0;

    const char* tempTrend = dashboard["temp_trend"] | "unknown";
    outdoor.temperatureTrend = stringToTrend(tempTrend);

    outdoor.minTemp = dashboard["min_temp"] | 0.0f;
    outdoor.maxTemp = dashboard["max_temp"] | 0.0f;
    outdoor.dateMinTemp = dashboard["date_min_temp"] | 0;
    outdoor.dateMaxTemp = dashboard["date_max_temp"] | 0;

    outdoor.valid = true;

    ESP_LOGI("netatmo", "Outdoor: %.1f°C, %d%% RH", outdoor.temperature, outdoor.humidity);
}

void NetatmoClient::parseWindData(JsonObject module, WindData& wind) {
    JsonObject dashboard = module["dashboard_data"];
    if (!dashboard) {
        ESP_LOGW("netatmo", "No dashboard_data in wind module");
        return;
    }

    wind.strength = dashboard["WindStrength"] | 0;
    wind.angle = dashboard["WindAngle"] | 0;
    wind.gustStrength = dashboard["GustStrength"] | 0;
    wind.gustAngle = dashboard["GustAngle"] | 0;
    wind.maxWindStrength = dashboard["max_wind_str"] | 0;
    wind.dateMaxWind = dashboard["date_max_wind_str"] | 0;

    wind.valid = true;

    ESP_LOGI("netatmo", "Wind: %d km/h from %d°, gust %d km/h",
            wind.strength, wind.angle, wind.gustStrength);
}

void NetatmoClient::parseRainData(JsonObject module, RainData& rain) {
    JsonObject dashboard = module["dashboard_data"];
    if (!dashboard) {
        ESP_LOGW("netatmo", "No dashboard_data in rain module");
        return;
    }

    rain.current = dashboard["Rain"] | 0.0f;
    rain.sum1h = dashboard["sum_rain_1"] | 0.0f;
    rain.sum24h = dashboard["sum_rain_24"] | 0.0f;

    rain.valid = true;

    ESP_LOGI("netatmo", "Rain: %.1fmm, 1h: %.1fmm, 24h: %.1fmm",
            rain.current, rain.sum1h, rain.sum24h);
}

JsonObject NetatmoClient::findModuleByType(JsonArray modules, const char* type) {
    for (JsonObject module : modules) {
        const char* moduleType = module["type"];
        if (moduleType && strcmp(moduleType, type) == 0) {
            return module;
        }
    }
    return JsonObject();
}

bool NetatmoClient::getWeatherData(WeatherData& data) {
    ESP_LOGI("netatmo", "Fetching weather data");

    // Ensure valid token
    if (!ensureValidToken()) {
        ESP_LOGE("netatmo", "Failed to get valid token");
        return false;
    }

    // Build URL with device ID parameter
    String url = NETATMO_WEATHER_URL;
    url += "?device_id=";
    url += NETATMO_DEVICE_ID;

    // Make API request
    JsonDocument doc;
    if (!HTTPUtils::httpGetJSONWithRetry(url.c_str(), doc, accessToken.c_str())) {
        ESP_LOGE("netatmo", "Failed to fetch weather data");
        return false;
    }

    // Parse response
    JsonObject body = doc["body"];
    if (!body) {
        ESP_LOGE("netatmo", "No body in response");
        return false;
    }

    JsonArray devices = body["devices"];
    if (!devices || devices.size() == 0) {
        ESP_LOGE("netatmo", "No devices in response");
        return false;
    }

    JsonObject device = devices[0];

    // Get station name
    data.stationName = device["station_name"] | "Unknown";

    // Get last measurement timestamp
    JsonObject dashboard = device["dashboard_data"];
    if (dashboard) {
        data.timestamp = dashboard["time_utc"] | 0;
    }

    // Parse indoor data (main device)
    parseIndoorData(device, data.indoor);

    // Get modules array
    JsonArray modules = device["modules"];
    if (modules) {
        // Find and parse outdoor module (NAModule1)
        JsonObject outdoorModule = findModuleByType(modules, "NAModule1");
        if (!outdoorModule.isNull()) {
            parseOutdoorData(outdoorModule, data.outdoor);
        }

        // Find and parse wind module (NAModule2)
        JsonObject windModule = findModuleByType(modules, "NAModule2");
        if (!windModule.isNull()) {
            parseWindData(windModule, data.wind);
        }

        // Find and parse rain module (NAModule3)
        JsonObject rainModule = findModuleByType(modules, "NAModule3");
        if (!rainModule.isNull()) {
            parseRainData(rainModule, data.rain);
        }
    }

    ESP_LOGI("netatmo", "Weather data fetch complete");
    return true;
}

unsigned long NetatmoClient::getLastUpdateTime() {
    ESP_LOGI("netatmo", "Fetching last update time");

    // Ensure valid token
    if (!ensureValidToken()) {
        ESP_LOGE("netatmo", "Failed to get valid token");
        return 0;
    }

    // Build URL
    String url = NETATMO_WEATHER_URL;
    url += "?device_id=";
    url += NETATMO_DEVICE_ID;

    // Make API request
    JsonDocument doc;
    if (!HTTPUtils::httpGetJSON(url.c_str(), doc, accessToken.c_str())) {
        ESP_LOGE("netatmo", "Failed to fetch last update time");
        return 0;
    }

    // Parse timestamp
    JsonObject body = doc["body"];
    if (!body) {
        return 0;
    }

    JsonArray devices = body["devices"];
    if (!devices || devices.size() == 0) {
        return 0;
    }

    JsonObject device = devices[0];
    JsonObject dashboard = device["dashboard_data"];
    if (!dashboard) {
        return 0;
    }

    unsigned long timestamp = dashboard["time_utc"] | 0;
    ESP_LOGI("netatmo", "Last update: %lu", timestamp);

    return timestamp;
}

Trend NetatmoClient::calculateCO2Trend(const char* deviceId, const char* moduleId) {
    // This would require calling the getmeasure API with historical data
    // For simplicity in this implementation, we return STABLE
    // A full implementation would fetch last 3 hours of CO2 data and calculate slope
    return Trend::STABLE;
}
