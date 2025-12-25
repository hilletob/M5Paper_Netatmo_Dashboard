#ifndef WEATHER_DATA_H
#define WEATHER_DATA_H

#include <Arduino.h>

// Trend indicators
enum class Trend {
    STABLE,
    UP,
    DOWN,
    UNKNOWN
};

// Indoor climate data (from main Netatmo device)
struct IndoorData {
    float temperature;          // °C
    uint8_t humidity;           // %
    uint16_t co2;               // ppm
    uint8_t noise;              // dB
    uint16_t pressure;          // mbar
    Trend temperatureTrend;
    Trend pressureTrend;
    Trend co2Trend;
    float minTemp;              // Daily minimum
    float maxTemp;              // Daily maximum
    unsigned long dateMinTemp;  // Unix timestamp
    unsigned long dateMaxTemp;  // Unix timestamp
    bool valid;                 // Data validity flag

    IndoorData() : temperature(0), humidity(0), co2(0), noise(0), pressure(0),
                   temperatureTrend(Trend::UNKNOWN), pressureTrend(Trend::UNKNOWN),
                   co2Trend(Trend::UNKNOWN), minTemp(0), maxTemp(0),
                   dateMinTemp(0), dateMaxTemp(0), valid(false) {}
};

// Outdoor climate data (from outdoor Netatmo module)
struct OutdoorData {
    float temperature;          // °C
    uint8_t humidity;           // %
    Trend temperatureTrend;
    float minTemp;              // Daily minimum
    float maxTemp;              // Daily maximum
    unsigned long dateMinTemp;  // Unix timestamp
    unsigned long dateMaxTemp;  // Unix timestamp
    bool valid;                 // Data validity flag

    OutdoorData() : temperature(0), humidity(0), temperatureTrend(Trend::UNKNOWN),
                    minTemp(0), maxTemp(0), dateMinTemp(0), dateMaxTemp(0), valid(false) {}
};

// Wind data (from wind Netatmo module)
struct WindData {
    uint16_t strength;          // km/h
    uint16_t angle;             // degrees (0-360)
    uint16_t gustStrength;      // km/h
    uint16_t gustAngle;         // degrees
    uint16_t maxWindStrength;   // Daily max km/h
    unsigned long dateMaxWind;  // Unix timestamp
    bool valid;                 // Data validity flag

    WindData() : strength(0), angle(0), gustStrength(0), gustAngle(0),
                 maxWindStrength(0), dateMaxWind(0), valid(false) {}
};

// Rain data (from rain Netatmo module)
struct RainData {
    float current;              // mm
    float sum1h;                // mm (last hour)
    float sum24h;               // mm (last 24 hours)
    bool valid;                 // Data validity flag

    RainData() : current(0), sum1h(0), sum24h(0), valid(false) {}
};

// Complete Netatmo weather data
struct WeatherData {
    IndoorData indoor;
    OutdoorData outdoor;
    WindData wind;
    RainData rain;
    unsigned long timestamp;    // Unix timestamp of last measurement
    String stationName;

    WeatherData() : timestamp(0), stationName("") {}
};

// Forecast data point (from MeteoSwiss/Open-Meteo)
struct ForecastPoint {
    unsigned long timestamp;    // Unix timestamp
    float temperature;          // °C
    uint8_t weatherCode;        // WMO weather code
    float precipitation;        // mm
    uint8_t humidity;           // %
    uint16_t windSpeed;         // km/h
    uint16_t windDirection;     // degrees
    bool valid;                 // Data validity flag

    ForecastPoint() : timestamp(0), temperature(0), weatherCode(0),
                     precipitation(0), humidity(0), windSpeed(0),
                     windDirection(0), valid(false) {}
};

// MeteoSwiss forecast data
struct ForecastData {
    ForecastPoint current;      // Current conditions
    ForecastPoint forecast3h;   // +3 hours forecast
    ForecastPoint forecast6h;   // +6 hours forecast

    ForecastData() {}
};

// Complete dashboard data (combination of Netatmo + MeteoSwiss)
struct DashboardData {
    WeatherData weather;
    ForecastData forecast;
    uint32_t batteryVoltage;    // mV
    uint8_t batteryPercent;     // %
    unsigned long updateTime;   // Unix timestamp of dashboard update

    DashboardData() : batteryVoltage(0), batteryPercent(0), updateTime(0) {}
};

// Helper function to convert trend string to enum
inline Trend stringToTrend(const char* trendStr) {
    if (strcmp(trendStr, "up") == 0) return Trend::UP;
    if (strcmp(trendStr, "down") == 0) return Trend::DOWN;
    if (strcmp(trendStr, "stable") == 0) return Trend::STABLE;
    return Trend::UNKNOWN;
}

// Helper function to convert trend enum to symbol
inline const char* trendToSymbol(Trend trend) {
    switch (trend) {
        case Trend::UP: return "↑";
        case Trend::DOWN: return "↓";
        case Trend::STABLE: return "→";
        default: return "?";
    }
}

// Helper function to get CO2 quality level (German)
inline const char* getCO2Quality(uint16_t co2) {
    if (co2 > 1500) return "De Vater isch am Lufte";
    if (co2 < 800) return "Ausgezeichnet";
    if (co2 < 1000) return "Gut";
    if (co2 < 1500) return "Mittel";
    if (co2 < 2000) return "Schlecht";
    return "Sehr schlecht";
}

// Helper function to get humidity comfort level (German)
inline const char* getHumidityComfort(uint8_t humidity) {
    if (humidity < 30) return "Zu trocken";
    if (humidity <= 60) return "Optimal";
    if (humidity <= 70) return "Feucht";
    return "Zu feucht";
}

#endif  // WEATHER_DATA_H
