#ifndef METEO_CLIENT_H
#define METEO_CLIENT_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "../data/weather_data.h"
#include "../config.h"

class MeteoClient {
private:
    // Parse a forecast point from JSON
    void parseForecastPoint(JsonObject hourly, int index, ForecastPoint& point);

public:
    MeteoClient();

    // Get current weather and forecast from Open-Meteo API
    bool getForecast(ForecastData& data);

    // Get weather description from WMO code
    static const char* getWeatherDescription(uint8_t code);

    // Get simple weather icon name from WMO code
    static const char* getWeatherIcon(uint8_t code);
};

#endif  // METEO_CLIENT_H
