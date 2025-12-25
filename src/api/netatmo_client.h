#ifndef NETATMO_CLIENT_H
#define NETATMO_CLIENT_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "../data/weather_data.h"
#include "../config.h"

class NetatmoClient {
private:
    String accessToken;
    unsigned long tokenExpiry;  // millis() when token expires

    // Refresh the OAuth2 access token using the refresh token
    bool refreshAccessToken();

    // Ensure we have a valid access token (refresh if needed)
    bool ensureValidToken();

    // Parse indoor module data from JSON
    void parseIndoorData(JsonObject device, IndoorData& indoor);

    // Parse outdoor module data from JSON
    void parseOutdoorData(JsonObject module, OutdoorData& outdoor);

    // Parse wind module data from JSON
    void parseWindData(JsonObject module, WindData& wind);

    // Parse rain module data from JSON
    void parseRainData(JsonObject module, RainData& rain);

    // Find a module by type in the modules array
    JsonObject findModuleByType(JsonArray modules, const char* type);

public:
    NetatmoClient();

    // Get current weather data from Netatmo station
    bool getWeatherData(WeatherData& data);

    // Get the last update timestamp from Netatmo station
    unsigned long getLastUpdateTime();

    // Calculate CO2 trend from historical data (past 3 hours)
    Trend calculateCO2Trend(const char* deviceId, const char* moduleId);
};

#endif  // NETATMO_CLIENT_H
