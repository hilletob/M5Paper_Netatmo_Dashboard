#ifndef WIDGETS_H
#define WIDGETS_H

#include "TFT_eSPI.h"
#include "../data/weather_data.h"

// Weather icon renderer
void drawWeatherIcon(TFT_eSprite& display, int x, int y, const char* iconName, int size = 40);

// Widget drawing functions
void drawIndoorTempWidget(TFT_eSprite& display, const IndoorData& data);
void drawIndoorHumidWidget(TFT_eSprite& display, const IndoorData& data);
void drawIndoorPressureWidget(TFT_eSprite& display, const IndoorData& data);
void drawIndoorCO2Widget(TFT_eSprite& display, const IndoorData& data);

void drawOutdoorTempWidget(TFT_eSprite& display, const OutdoorData& data);

void drawForecast3hWidget(TFT_eSprite& display, const ForecastPoint& forecast);
void drawForecast6hWidget(TFT_eSprite& display, const ForecastPoint& forecast);

void drawStatusBar(TFT_eSprite& display, uint32_t batteryMv, uint8_t batteryPercent,
                   unsigned long updateTime);

// Complete dashboard renderer
void drawDashboard(TFT_eSprite& display, const DashboardData& data);

#endif  // WIDGETS_H
