#include "meteo_client.h"
#include "http_utils.h"

MeteoClient::MeteoClient() {
}

void MeteoClient::parseForecastPoint(JsonObject hourly, int index, ForecastPoint& point) {
    JsonArray times = hourly["time"];
    JsonArray temps = hourly["temperature_2m"];
    JsonArray codes = hourly["weather_code"];
    JsonArray precip = hourly["precipitation"];

    if (index >= times.size()) {
        ESP_LOGW("meteo", "Index %d out of range", index);
        return;
    }

    // Parse timestamp (ISO8601 string to Unix timestamp)
    const char* timeStr = times[index];
    // Simple ISO8601 parsing (assumes format: "2024-12-25T14:00")
    // For production, use a proper datetime library
    struct tm tm;
    memset(&tm, 0, sizeof(tm));
    if (sscanf(timeStr, "%d-%d-%dT%d:%d",
              &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min) == 5) {
        tm.tm_year -= 1900;  // Years since 1900
        tm.tm_mon -= 1;      // Months since January
        point.timestamp = mktime(&tm);
    }

    point.temperature = temps[index] | 0.0f;
    point.weatherCode = codes[index] | 0;
    point.precipitation = precip[index] | 0.0f;
    point.valid = true;

    ESP_LOGD("meteo", "Forecast[%d]: %.1f°C, code %d, precip %.1fmm",
            index, point.temperature, point.weatherCode, point.precipitation);
}

bool MeteoClient::getForecast(ForecastData& data) {
    ESP_LOGI("meteo", "Fetching forecast from Open-Meteo");

    // Build URL with parameters
    String url = METEO_FORECAST_URL;
    url += "?latitude=";
    url += String(LOCATION_LAT, 4);
    url += "&longitude=";
    url += String(LOCATION_LON, 4);
    url += "&current=temperature_2m,relative_humidity_2m,weather_code,wind_speed_10m,wind_direction_10m";
    url += "&hourly=temperature_2m,weather_code,precipitation";
    url += "&timezone=Europe/Zurich";
    url += "&forecast_hours=12";  // Get more hours for 3h and 6h forecasts

    ESP_LOGD("meteo", "URL: %s", url.c_str());

    // Make API request
    JsonDocument doc;
    if (!HTTPUtils::httpGetJSONWithRetry(url.c_str(), doc)) {
        ESP_LOGE("meteo", "Failed to fetch forecast");
        return false;
    }

    // Parse current conditions
    JsonObject current = doc["current"];
    if (current) {
        data.current.temperature = current["temperature_2m"] | 0.0f;
        data.current.humidity = current["relative_humidity_2m"] | 0;
        data.current.weatherCode = current["weather_code"] | 0;
        data.current.windSpeed = current["wind_speed_10m"] | 0;
        data.current.windDirection = current["wind_direction_10m"] | 0;

        const char* timeStr = current["time"];
        if (timeStr) {
            struct tm tm;
            memset(&tm, 0, sizeof(tm));
            if (sscanf(timeStr, "%d-%d-%dT%d:%d",
                      &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min) == 5) {
                tm.tm_year -= 1900;
                tm.tm_mon -= 1;
                data.current.timestamp = mktime(&tm);
            }
        }

        data.current.valid = true;

        ESP_LOGI("meteo", "Current: %.1f°C, code %d, %d%% RH",
                data.current.temperature, data.current.weatherCode, data.current.humidity);
    }

    // Parse hourly forecast
    JsonObject hourly = doc["hourly"];
    if (hourly) {
        // Index 3 = +3 hours, Index 6 = +6 hours
        parseForecastPoint(hourly, 3, data.forecast3h);
        parseForecastPoint(hourly, 6, data.forecast6h);
    }

    ESP_LOGI("meteo", "Forecast fetch complete");
    return true;
}

const char* MeteoClient::getWeatherDescription(uint8_t code) {
    // WMO Weather interpretation codes
    switch (code) {
        case 0: return "Clear sky";
        case 1: return "Mainly clear";
        case 2: return "Partly cloudy";
        case 3: return "Overcast";
        case 45:
        case 48: return "Fog";
        case 51:
        case 53:
        case 55: return "Drizzle";
        case 56:
        case 57: return "Freezing drizzle";
        case 61: return "Slight rain";
        case 63: return "Moderate rain";
        case 65: return "Heavy rain";
        case 66:
        case 67: return "Freezing rain";
        case 71: return "Slight snow";
        case 73: return "Moderate snow";
        case 75: return "Heavy snow";
        case 77: return "Snow grains";
        case 80:
        case 81:
        case 82: return "Rain showers";
        case 85:
        case 86: return "Snow showers";
        case 95: return "Thunderstorm";
        case 96:
        case 99: return "Thunderstorm with hail";
        default: return "Unknown";
    }
}

const char* MeteoClient::getWeatherIcon(uint8_t code) {
    // Return simple icon names for rendering
    if (code == 0 || code == 1) return "sunny";
    if (code == 2) return "partly_cloudy";
    if (code == 3) return "cloudy";
    if (code >= 45 && code <= 48) return "fog";
    if (code >= 51 && code <= 57) return "drizzle";
    if (code >= 61 && code <= 67) return "rain";
    if (code >= 71 && code <= 77) return "snow";
    if (code >= 80 && code <= 82) return "showers";
    if (code >= 85 && code <= 86) return "snow_showers";
    if (code >= 95) return "thunderstorm";
    return "unknown";
}
