#ifndef METEO_CLIENT_H
#define METEO_CLIENT_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "../data/weather_data.h"
#include "../config.h"

class MeteoClient {
private:
    // HTTP caching (persistent across deep sleep via RTC_DATA_ATTR in .cpp)
    static char lastModified[32];
    static unsigned long expiresTimestamp;

    // Parse ISO8601 timestamp to Unix epoch (UTC → local time)
    time_t parseISO8601(const char* timeStr);

    // Parse HTTP date header (RFC 2822) to Unix epoch
    time_t parseHTTPDate(const char* dateStr);

    // Parse met.no symbol code to enum index
    uint8_t parseSymbolCode(const char* symbol);

    // Legacy: Parse a forecast point from JSON (deprecated, kept for compatibility)
    void parseForecastPoint(JsonObject hourly, int index, ForecastPoint& point);

public:
    MeteoClient();

    // Get 3-day forecast from met.no API
    bool getForecast(ForecastData& data);

    // Get weather description from WMO code (legacy)
    static const char* getWeatherDescription(uint8_t code);

    // Get simple weather icon name from WMO code (legacy)
    static const char* getWeatherIcon(uint8_t code);
};

#endif  // METEO_CLIENT_H
