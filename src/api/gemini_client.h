#ifndef GEMINI_CLIENT_H
#define GEMINI_CLIENT_H

#include <Arduino.h>
#include "../data/weather_data.h"
#include "../config.h"

class GeminiClient {
private:
    // Build the prompt from sensor data and timestamp
    String buildPrompt(const WeatherData& weather, unsigned long timestamp);

    // Detect current season from timestamp (winter/spring/summer/fall)
    const char* getSeason(unsigned long timestamp);

    // Detect time of day (morning/afternoon/evening/night)
    const char* getTimeOfDay(unsigned long timestamp);

    // Check for Swiss holidays (Christmas, Easter, New Year, etc.)
    const char* getHoliday(unsigned long timestamp);

public:
    GeminiClient();

    // Generate funny weather commentary in German/Schweizerdeutsch
    // Returns empty string on failure
    String generateCommentary(const WeatherData& weather, unsigned long timestamp);
};

#endif  // GEMINI_CLIENT_H
