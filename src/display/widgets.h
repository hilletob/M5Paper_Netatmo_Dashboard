#ifndef WIDGETS_H
#define WIDGETS_H

#include <M5EPD.h>
#include "../data/weather_data.h"
#include "../power/battery.h"

// Weather icon renderer
void drawWeatherIcon(M5EPD_Canvas& display, int x, int y, const char* iconName, int size = 40);

// Widget drawing functions
void drawIndoorTempWidget(M5EPD_Canvas& display, const IndoorData& data);
void drawIndoorHumidWidget(M5EPD_Canvas& display, const IndoorData& data);
void drawAirQualityWidget(M5EPD_Canvas& display, const IndoorData& data);  // Renamed from drawIndoorCO2Widget
void drawPressureWidget(M5EPD_Canvas& display, const IndoorData& data);     // Renamed from drawIndoorPressureWidget

void drawOutdoorTempWidget(M5EPD_Canvas& display, const OutdoorData& data);
void drawOutdoorHumidWidget(M5EPD_Canvas& display, const OutdoorData& data);

void drawForecastWidget(M5EPD_Canvas& display, const ForecastData& forecast);
void drawBatteryWidget(M5EPD_Canvas& display, const BatteryStatus& status);

// Header with update times
void drawHeader(M5EPD_Canvas& display, const char* location, unsigned long updateTime, unsigned long nextWakeTime, bool isFallback = false);

// Complete dashboard renderer
void drawDashboard(M5EPD_Canvas& display, const DashboardData& data);

#endif  // WIDGETS_H
