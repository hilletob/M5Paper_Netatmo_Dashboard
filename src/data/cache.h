#ifndef CACHE_H
#define CACHE_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "weather_data.h"
#include "../config.h"

class DataCache {
public:
    // Initialize LittleFS
    static bool init();

    // Save dashboard data to cache
    static bool save(const DashboardData& data);

    // Load dashboard data from cache
    static bool load(DashboardData& data);

    // Check if cache exists and is valid
    static bool isValid();

    // Get cache age in seconds
    static unsigned long getAgeSeconds();

    // Clear cache
    static void clear();
};

#endif  // CACHE_H
