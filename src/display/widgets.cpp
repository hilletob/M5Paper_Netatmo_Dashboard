#include "widgets.h"
#include "layout.h"
#include "fonts.h"
#include "icons.h"
#include "../config.h"
#include <time.h>

// Helper to draw a card border
void drawCard(TFT_eSprite& display, int x, int y) {
    display.drawRect(x, y, CARD_WIDTH, CARD_HEIGHT, TFT_BLACK);
}

// Helper to draw temperature with degree symbol (drawn as circle)
void drawTemperature(TFT_eSprite& display, float temp, int16_t x, int16_t y,
                     const GFXfont* valueFont, const GFXfont* unitFont, uint8_t datum) {
    char tempStr[16];
    snprintf(tempStr, sizeof(tempStr), "%.1f", temp);

    display.setTextDatum(datum);
    display.setFreeFont(valueFont);

    // Draw temperature value
    display.drawString(tempStr, x, y);

    // Measure text width to position degree symbol
    int16_t textWidth = display.textWidth(tempStr);

    // Draw degree symbol as small circle (positioned at top-right of number)
    int16_t degX = x + textWidth + 6;  // 6px spacing
    int16_t degY = y + 3;              // Offset from top
    display.drawCircle(degX, degY, 3, TFT_BLACK);
    display.drawCircle(degX, degY, 2, TFT_BLACK);  // Thicker circle

    // Draw "C" after degree symbol
    display.setFreeFont(unitFont);
    display.drawString("C", degX + 7, y);
}

// Helper to draw trend arrow using geometric shapes
void drawTrendArrow(TFT_eSprite& display, int x, int y, Trend trend) {
    int cx = x + 12;  // Center X
    int cy = y + 12;  // Center Y

    switch (trend) {
        case Trend::UP:
            // Diagonal arrow pointing up-right (↗)
            // Shaft
            display.drawLine(cx - 6, cy + 6, cx + 6, cy - 6, TFT_BLACK);
            display.drawLine(cx - 5, cy + 6, cx + 6, cy - 5, TFT_BLACK);
            display.drawLine(cx - 6, cy + 5, cx + 5, cy - 6, TFT_BLACK);
            // Arrowhead
            display.fillTriangle(cx + 6, cy - 6,
                                cx + 6, cy - 2,
                                cx + 2, cy - 6, TFT_BLACK);
            break;

        case Trend::DOWN:
            // Diagonal arrow pointing down-right (↘)
            // Shaft
            display.drawLine(cx - 6, cy - 6, cx + 6, cy + 6, TFT_BLACK);
            display.drawLine(cx - 5, cy - 6, cx + 6, cy + 5, TFT_BLACK);
            display.drawLine(cx - 6, cy - 5, cx + 5, cy + 6, TFT_BLACK);
            // Arrowhead
            display.fillTriangle(cx + 6, cy + 6,
                                cx + 6, cy + 2,
                                cx + 2, cy + 6, TFT_BLACK);
            break;

        case Trend::STABLE:
            // Horizontal arrow pointing right (→)
            // Shaft
            display.drawLine(cx - 8, cy, cx + 6, cy, TFT_BLACK);
            display.drawLine(cx - 8, cy - 1, cx + 6, cy - 1, TFT_BLACK);
            display.drawLine(cx - 8, cy + 1, cx + 6, cy + 1, TFT_BLACK);
            // Arrowhead
            display.fillTriangle(cx + 6, cy,
                                cx + 2, cy - 4,
                                cx + 2, cy + 4, TFT_BLACK);
            break;

        default:
            break;
    }
}

void drawWeatherIcon(TFT_eSprite& display, int x, int y, const char* iconName, int size) {
    // Center point for the icon
    int cx = x + size / 2;
    int cy = y + size / 2;

    if (strcmp(iconName, "sunny") == 0) {
        // Sun icon with rays
        int sunRadius = size / 6;
        int rayLength = size / 5;

        // Draw sun circle (filled)
        display.fillCircle(cx, cy, sunRadius, TFT_BLACK);

        // Draw 8 rays around the sun
        for (int i = 0; i < 8; i++) {
            float angle = i * PI / 4;
            int innerX = cx + cos(angle) * (sunRadius + 2);
            int innerY = cy + sin(angle) * (sunRadius + 2);
            int outerX = cx + cos(angle) * (sunRadius + rayLength);
            int outerY = cy + sin(angle) * (sunRadius + rayLength);

            // Draw thick rays
            display.drawLine(innerX, innerY, outerX, outerY, TFT_BLACK);
            display.drawLine(innerX + 1, innerY, outerX + 1, outerY, TFT_BLACK);
            display.drawLine(innerX, innerY + 1, outerX, outerY + 1, TFT_BLACK);
        }
    }
    else if (strcmp(iconName, "partly_cloudy") == 0) {
        // Sun behind cloud
        int sunRadius = size / 8;
        int sunX = cx - size / 6;
        int sunY = cy - size / 6;

        // Draw partial sun (upper left)
        display.drawCircle(sunX, sunY, sunRadius, TFT_BLACK);
        display.drawCircle(sunX, sunY, sunRadius - 1, TFT_BLACK);

        // Draw 4 sun rays (only visible ones)
        for (int i = 0; i < 4; i++) {
            float angle = (i * PI / 2) + PI;  // Left and top rays
            int innerX = sunX + cos(angle) * (sunRadius + 1);
            int innerY = sunY + sin(angle) * (sunRadius + 1);
            int outerX = sunX + cos(angle) * (sunRadius + size / 8);
            int outerY = sunY + sin(angle) * (sunRadius + size / 8);
            display.drawLine(innerX, innerY, outerX, outerY, TFT_BLACK);
        }

        // Draw cloud (overlapping sun)
        int cloudY = cy + size / 10;
        // Cloud bumps (3 circles)
        display.fillCircle(cx - size / 8, cloudY, size / 8, TFT_BLACK);
        display.fillCircle(cx, cloudY - size / 12, size / 7, TFT_BLACK);
        display.fillCircle(cx + size / 8, cloudY, size / 8, TFT_BLACK);
        // Cloud base
        display.fillRect(cx - size / 6, cloudY, size / 3, size / 8, TFT_BLACK);
    }
    else if (strcmp(iconName, "cloudy") == 0) {
        // Cloud icon - three bumps
        int cloudY = cy;

        // Draw three overlapping circles for cloud bumps
        display.fillCircle(cx - size / 6, cloudY, size / 7, TFT_BLACK);
        display.fillCircle(cx, cloudY - size / 10, size / 6, TFT_BLACK);
        display.fillCircle(cx + size / 6, cloudY, size / 7, TFT_BLACK);

        // Cloud base rectangle
        display.fillRect(cx - size / 5, cloudY, size / 2.5, size / 7, TFT_BLACK);
    }
    else if (strcmp(iconName, "rain") == 0) {
        // Cloud with rain drops
        int cloudY = cy - size / 8;

        // Draw cloud
        display.fillCircle(cx - size / 8, cloudY, size / 9, TFT_BLACK);
        display.fillCircle(cx, cloudY - size / 12, size / 8, TFT_BLACK);
        display.fillCircle(cx + size / 8, cloudY, size / 9, TFT_BLACK);
        display.fillRect(cx - size / 7, cloudY, size / 3.5, size / 9, TFT_BLACK);

        // Draw rain drops (5 drops)
        int dropY = cy + size / 6;
        for (int i = 0; i < 5; i++) {
            int dropX = cx - size / 5 + (i * size / 10);
            // Draw thick rain lines
            display.drawLine(dropX, dropY, dropX, dropY + size / 8, TFT_BLACK);
            display.drawLine(dropX + 1, dropY, dropX + 1, dropY + size / 8, TFT_BLACK);
        }
    }
    else if (strcmp(iconName, "snow") == 0) {
        // Cloud with snowflakes
        int cloudY = cy - size / 8;

        // Draw cloud
        display.fillCircle(cx - size / 8, cloudY, size / 9, TFT_BLACK);
        display.fillCircle(cx, cloudY - size / 12, size / 8, TFT_BLACK);
        display.fillCircle(cx + size / 8, cloudY, size / 9, TFT_BLACK);
        display.fillRect(cx - size / 7, cloudY, size / 3.5, size / 9, TFT_BLACK);

        // Draw snowflakes (asterisks)
        int flakeY = cy + size / 6;
        int flakeSize = size / 12;

        for (int i = 0; i < 3; i++) {
            int flakeX = cx - size / 6 + (i * size / 6);
            // Vertical line
            display.drawLine(flakeX, flakeY - flakeSize, flakeX, flakeY + flakeSize, TFT_BLACK);
            // Horizontal line
            display.drawLine(flakeX - flakeSize, flakeY, flakeX + flakeSize, flakeY, TFT_BLACK);
            // Diagonal lines
            display.drawLine(flakeX - flakeSize/1.5, flakeY - flakeSize/1.5,
                           flakeX + flakeSize/1.5, flakeY + flakeSize/1.5, TFT_BLACK);
            display.drawLine(flakeX - flakeSize/1.5, flakeY + flakeSize/1.5,
                           flakeX + flakeSize/1.5, flakeY - flakeSize/1.5, TFT_BLACK);
        }
    }
}

void drawHeader(TFT_eSprite& display, const char* location, unsigned long timestamp) {
    // Draw header background
    display.fillRect(0, HEADER_Y, SCREEN_WIDTH, HEADER_HEIGHT, TFT_WHITE);
    display.drawFastHLine(0, HEADER_HEIGHT, SCREEN_WIDTH, TFT_BLACK);

    // Location on left
    display.setTextColor(TFT_BLACK, TFT_WHITE);
    display.setTextDatum(TL_DATUM);
    display.setFreeFont(FSS12);
    display.drawString(location, MARGIN, HEADER_Y + 8);

    // Date/time on right
    if (timestamp > 0) {
        time_t t = timestamp;
        struct tm* tm = localtime(&t);
        char dateStr[32];
        snprintf(dateStr, sizeof(dateStr), "%02d.%02d.%04d %02d:%02d",
                tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900,
                tm->tm_hour, tm->tm_min);
        display.setTextDatum(TR_DATUM);
        display.setFreeFont(FSS9);
        display.drawString(dateStr, SCREEN_WIDTH - MARGIN, HEADER_Y + 8);
    }
}

void drawIndoorTempWidget(TFT_eSprite& display, const IndoorData& data) {
    if (!data.valid) return;

    drawCard(display, INDOOR_TEMP_X, INDOOR_TEMP_Y);

    display.setTextColor(TFT_BLACK, TFT_WHITE);

    // Label
    display.setTextDatum(TL_DATUM);
    display.setFreeFont(FSS9);
    display.drawString("Innen", INDOOR_TEMP_X + CARD_PADDING, INDOOR_TEMP_Y + CARD_LABEL_Y);

    // Large temperature value with degree symbol
    drawTemperature(display, data.temperature,
                    INDOOR_TEMP_X + CARD_PADDING,
                    INDOOR_TEMP_Y + CARD_VALUE_Y,
                    FSSB18, FSS12, TL_DATUM);

    // Trend arrow
    drawTrendArrow(display, INDOOR_TEMP_X + CARD_TREND_X_OFFSET, INDOOR_TEMP_Y + CARD_TREND_Y, data.temperatureTrend);

    // Min/max detail
    if (data.minTemp != 0 || data.maxTemp != 0) {
        char detailStr[64];
        snprintf(detailStr, sizeof(detailStr), "min %.1f°C  max %.1f°C", data.minTemp, data.maxTemp);
        display.setTextDatum(TL_DATUM);
        display.setFreeFont(FSS9);
        display.drawString(detailStr, INDOOR_TEMP_X + CARD_PADDING, INDOOR_TEMP_Y + CARD_DETAIL_Y);
    }
}

void drawOutdoorTempWidget(TFT_eSprite& display, const OutdoorData& data) {
    drawCard(display, OUTDOOR_TEMP_X, OUTDOOR_TEMP_Y);

    display.setTextColor(TFT_BLACK, TFT_WHITE);
    display.setTextDatum(TL_DATUM);
    display.setFreeFont(FSS9);
    display.drawString("Aussen", OUTDOOR_TEMP_X + CARD_PADDING, OUTDOOR_TEMP_Y + CARD_LABEL_Y);

    if (!data.valid) {
        display.setFreeFont(FSSB18);
        display.drawString("n/a", OUTDOOR_TEMP_X + CARD_PADDING, OUTDOOR_TEMP_Y + CARD_VALUE_Y);
        return;
    }

    drawTemperature(display, data.temperature,
                    OUTDOOR_TEMP_X + CARD_PADDING,
                    OUTDOOR_TEMP_Y + CARD_VALUE_Y,
                    FSSB18, FSS12, TL_DATUM);

    drawTrendArrow(display, OUTDOOR_TEMP_X + CARD_TREND_X_OFFSET, OUTDOOR_TEMP_Y + CARD_TREND_Y, data.temperatureTrend);

    if (data.minTemp != 0 || data.maxTemp != 0) {
        char detailStr[64];
        snprintf(detailStr, sizeof(detailStr), "min %.1f°C  max %.1f°C", data.minTemp, data.maxTemp);
        display.setTextDatum(TL_DATUM);
        display.setFreeFont(FSS9);
        display.drawString(detailStr, OUTDOOR_TEMP_X + CARD_PADDING, OUTDOOR_TEMP_Y + CARD_DETAIL_Y);
    }
}

void drawIndoorHumidWidget(TFT_eSprite& display, const IndoorData& data) {
    if (!data.valid) return;

    drawCard(display, INDOOR_HUMID_X, INDOOR_HUMID_Y);

    display.setTextColor(TFT_BLACK, TFT_WHITE);
    display.setTextDatum(TL_DATUM);
    display.setFreeFont(FSS9);
    display.drawString("Innen", INDOOR_HUMID_X + CARD_PADDING, INDOOR_HUMID_Y + CARD_LABEL_Y);

    char humidStr[16];
    snprintf(humidStr, sizeof(humidStr), "%d%%", data.humidity);
    display.setTextDatum(TL_DATUM);
    display.setFreeFont(FSSB18);
    display.drawString(humidStr, INDOOR_HUMID_X + CARD_PADDING, INDOOR_HUMID_Y + CARD_VALUE_Y);

    // Climate status
    const char* status = getHumidityComfort(data.humidity);
    char detailStr[32];
    snprintf(detailStr, sizeof(detailStr), "Raumklima: %s", status);
    display.setTextDatum(TL_DATUM);
    display.setFreeFont(FSS9);
    display.drawString(detailStr, INDOOR_HUMID_X + CARD_PADDING, INDOOR_HUMID_Y + CARD_DETAIL_Y);
}

void drawOutdoorHumidWidget(TFT_eSprite& display, const OutdoorData& data) {
    drawCard(display, OUTDOOR_HUMID_X, OUTDOOR_HUMID_Y);

    display.setTextColor(TFT_BLACK, TFT_WHITE);
    display.setTextDatum(TL_DATUM);
    display.setFreeFont(FSS9);
    display.drawString("Aussen", OUTDOOR_HUMID_X + CARD_PADDING, OUTDOOR_HUMID_Y + CARD_LABEL_Y);

    if (!data.valid) {
        display.setFreeFont(FSSB18);
        display.drawString("n/a", OUTDOOR_HUMID_X + CARD_PADDING, OUTDOOR_HUMID_Y + CARD_VALUE_Y);
        return;
    }

    char humidStr[16];
    snprintf(humidStr, sizeof(humidStr), "%d%%", data.humidity);
    display.setTextDatum(TL_DATUM);
    display.setFreeFont(FSSB18);
    display.drawString(humidStr, OUTDOOR_HUMID_X + CARD_PADDING, OUTDOOR_HUMID_Y + CARD_VALUE_Y);

    // Dew point calculation (Magnus formula)
    float a = 17.27;
    float b = 237.7;
    float alpha = ((a * data.temperature) / (b + data.temperature)) + log(data.humidity / 100.0);
    float dewPoint = (b * alpha) / (a - alpha);

    char detailStr[32];
    snprintf(detailStr, sizeof(detailStr), "Taupunkt: %.1f°C", dewPoint);
    display.setTextDatum(TL_DATUM);
    display.setFreeFont(FSS9);
    display.drawString(detailStr, OUTDOOR_HUMID_X + CARD_PADDING, OUTDOOR_HUMID_Y + CARD_DETAIL_Y);
}

void drawAirQualityWidget(TFT_eSprite& display, const IndoorData& data) {
    if (!data.valid) return;

    drawCard(display, AIR_QUALITY_X, AIR_QUALITY_Y);

    display.setTextColor(TFT_BLACK, TFT_WHITE);
    display.setTextDatum(TL_DATUM);
    display.setFreeFont(FSS9);
    display.drawString("Luftqualitaet innen", AIR_QUALITY_X + CARD_PADDING, AIR_QUALITY_Y + CARD_LABEL_Y);

    char co2Str[16];
    snprintf(co2Str, sizeof(co2Str), "%dppm", data.co2);
    display.setTextDatum(TL_DATUM);
    display.setFreeFont(FSSB18);
    display.drawString(co2Str, AIR_QUALITY_X + CARD_PADDING, AIR_QUALITY_Y + CARD_VALUE_Y);

    drawTrendArrow(display, AIR_QUALITY_X + CARD_TREND_X_OFFSET, AIR_QUALITY_Y + CARD_TREND_Y, data.co2Trend);

    // CO2 quality status
    const char* quality = getCO2Quality(data.co2);
    display.setTextDatum(TL_DATUM);
    display.setFreeFont(FSS9);
    display.drawString(quality, AIR_QUALITY_X + CARD_PADDING, AIR_QUALITY_Y + CARD_DETAIL_Y);
}

void drawPressureWidget(TFT_eSprite& display, const IndoorData& data) {
    if (!data.valid) return;

    drawCard(display, PRESSURE_X, PRESSURE_Y);

    display.setTextColor(TFT_BLACK, TFT_WHITE);
    display.setTextDatum(TL_DATUM);
    display.setFreeFont(FSS9);
    display.drawString("Luftdruck außen", PRESSURE_X + CARD_PADDING, PRESSURE_Y + CARD_LABEL_Y);

    char pressStr[16];
    snprintf(pressStr, sizeof(pressStr), "%dhPa", data.pressure);
    display.setTextDatum(TL_DATUM);
    display.setFreeFont(FSSB18);
    display.drawString(pressStr, PRESSURE_X + CARD_PADDING, PRESSURE_Y + CARD_VALUE_Y);

    drawTrendArrow(display, PRESSURE_X + CARD_TREND_X_OFFSET, PRESSURE_Y + CARD_TREND_Y, data.pressureTrend);

    // Trend label
    const char* trendLabel = (data.pressureTrend == Trend::UP) ? "Trend: Steigend" :
                             (data.pressureTrend == Trend::DOWN) ? "Trend: Fallend" : "Trend: Stabil";
    display.setTextDatum(TL_DATUM);
    display.setFreeFont(FSS9);
    display.drawString(trendLabel, PRESSURE_X + CARD_PADDING, PRESSURE_Y + CARD_DETAIL_Y);
}

// Legacy forecast widgets (deprecated - now using 3-day forecast column)
// Kept for backward compatibility but not used in 3-column layout
void drawForecast3hWidget(TFT_eSprite& display, const ForecastPoint& forecast) {
    // No longer used in 3-column layout
}

void drawForecast6hWidget(TFT_eSprite& display, const ForecastPoint& forecast) {
    // No longer used in 3-column layout
}

void drawWindWidget(TFT_eSprite& display, const WindData& data) {
    drawCard(display, WIND_X, WIND_Y);

    display.setTextColor(TFT_BLACK, TFT_WHITE);
    display.setTextDatum(TL_DATUM);
    display.setFreeFont(FSS9);
    display.drawString("Wind", WIND_X + CARD_PADDING, WIND_Y + CARD_LABEL_Y);

    if (!data.valid) {
        display.setFreeFont(FSSB18);
        display.drawString("n/a", WIND_X + CARD_PADDING, WIND_Y + CARD_VALUE_Y);
        return;
    }

    // Wind speed
    char speedStr[16];
    snprintf(speedStr, sizeof(speedStr), "%dkm/h", data.strength);
    display.setTextDatum(TL_DATUM);
    display.setFreeFont(FSSB18);
    display.drawString(speedStr, WIND_X + CARD_PADDING, WIND_Y + CARD_VALUE_Y);

    // Wind direction as compass text (N, NO, O, SO, S, SW, W, NW)
    const char* directions[] = {"N", "NO", "O", "SO", "S", "SW", "W", "NW"};
    int dirIdx = ((data.angle + 22) / 45) % 8;
    display.setTextDatum(TR_DATUM);
    display.setFreeFont(FSS12);
    display.drawString(directions[dirIdx], WIND_X + CARD_WIDTH - 10, WIND_Y + CARD_VALUE_Y + 10);

    // Details: Gusts and max wind
    char detailStr[48];
    snprintf(detailStr, sizeof(detailStr), "Boen: %dkm/h  Max: %dkm/h",
             data.gustStrength, data.maxWindStrength);
    display.setTextDatum(TL_DATUM);
    display.setFreeFont(FSS9);
    display.drawString(detailStr, WIND_X + CARD_PADDING, WIND_Y + CARD_DETAIL_Y);
}

void drawRainWidget(TFT_eSprite& display, const RainData& data) {
    drawCard(display, RAIN_X, RAIN_Y);

    display.setTextColor(TFT_BLACK, TFT_WHITE);
    display.setTextDatum(TL_DATUM);
    display.setFreeFont(FSS9);
    display.drawString("Niederschlag", RAIN_X + CARD_PADDING, RAIN_Y + CARD_LABEL_Y);

    if (!data.valid) {
        display.setFreeFont(FSSB18);
        display.drawString("n/a", RAIN_X + CARD_PADDING, RAIN_Y + CARD_VALUE_Y);
        return;
    }

    // Current rain (24h sum displayed as main value)
    char rainStr[24];
    snprintf(rainStr, sizeof(rainStr), "%.1fmm/24h", data.sum24h);
    display.setTextDatum(TL_DATUM);
    display.setFreeFont(FSSB18);
    display.drawString(rainStr, RAIN_X + CARD_PADDING, RAIN_Y + CARD_VALUE_Y);

    // Details: 1h sum
    char detailStr[32];
    snprintf(detailStr, sizeof(detailStr), "Letzte Stunde: %.1fmm", data.sum1h);
    display.setTextDatum(TL_DATUM);
    display.setFreeFont(FSS9);
    display.drawString(detailStr, RAIN_X + CARD_PADDING, RAIN_Y + CARD_DETAIL_Y);
}

// Helper: Draw 3-time grid (06h, 12h, 18h)
void drawDayTimeGrid(TFT_eSprite& display, const DayTimeForecast times[3],
                     int x, int y, int width) {
    int cellWidth = width / 3;

    for (int i = 0; i < 3; i++) {
        int cellX = x + (i * cellWidth);
        const DayTimeForecast& dt = times[i];

        // Time label
        char timeStr[6];
        snprintf(timeStr, sizeof(timeStr), "%02dh", dt.hour);
        display.setFreeFont(FSS9);
        display.setTextDatum(TC_DATUM);
        display.drawString(timeStr, cellX + cellWidth/2, y);

        // Weather icon (24×24px)
        const char* icon = getIconFromCode(dt.symbolCode);
        drawWeatherIcon(display, cellX + cellWidth/2 - 12, y + 15, icon, 24);

        // Temperature
        char tempStr[6];
        snprintf(tempStr, sizeof(tempStr), "%d°", dt.temperature);
        display.setFreeFont(FSS9);
        display.drawString(tempStr, cellX + cellWidth/2, y + 42);

        // Precipitation (if > 0)
        if (dt.precipitationMm > 0) {
            char precipStr[8];
            snprintf(precipStr, sizeof(precipStr), "%.1fmm", dt.precipitationMm / 10.0);
            display.drawString(precipStr, cellX + cellWidth/2, y + 60);
        }
    }
}

// Helper: Draw single daily forecast section
void drawDailyForecastSection(TFT_eSprite& display, const DailyForecast& day,
                               int x, int y, const char* dayLabel, bool isToday) {
    display.setTextColor(TFT_BLACK, TFT_WHITE);
    display.setTextDatum(TL_DATUM);

    // Day header (HEUTE = bold, others = normal)
    display.setFreeFont(isToday ? FSSB12 : FSS12);
    display.drawString(dayLabel, x + 6, y);

    if (!day.valid) {
        display.setFreeFont(FSS9);
        display.drawString("n/a", x + 6, y + 20);
        return;
    }

    // Icon size (HEUTE = 30×30, others = 24×24)
    int iconSize = isToday ? 30 : 24;
    int iconY = y + (isToday ? 22 : 18);

    // Daily weather icon
    const char* iconName = getIconFromCode(day.symbolCode);
    drawWeatherIcon(display, x + 6, iconY, iconName, iconSize);

    // Temperature range
    char tempStr[20];
    snprintf(tempStr, sizeof(tempStr), "%d°C/%d°C", day.tempMin, day.tempMax);
    display.setFreeFont(isToday ? FSSB12 : FSS12);
    display.setTextDatum(TL_DATUM);
    display.drawString(tempStr, x + 6 + iconSize + 8, iconY + 5);

    // Precipitation (if > 0)
    if (day.precipSum > 0) {
        char precipStr[16];
        snprintf(precipStr, sizeof(precipStr), "%.1fmm", day.precipSum / 10.0);
        display.setFreeFont(FSS9);
        display.setTextDatum(TR_DATUM);
        display.drawString(precipStr, x + FORECAST_COL_WIDTH - 6, iconY + 5);
    }

    // 3-time grid (06h, 12h, 18h)
    int gridY = y + (isToday ? 70 : 55);
    drawDayTimeGrid(display, day.times, x + 6, gridY, FORECAST_COL_WIDTH - 12);
}

// Main 3-day forecast column
void draw3DayForecastColumn(TFT_eSprite& display, const ForecastData& forecast) {
    // Draw tall card border
    display.drawRect(FORECAST_COL_X, FORECAST_COL_Y, FORECAST_COL_WIDTH, FORECAST_COL_HEIGHT, TFT_BLACK);

    // Three daily sections (no header, start at top)
    const char* dayLabels[] = {"HEUTE", "MORGEN", "UEBERMORGEN"};
    int yPositions[] = {
        FORECAST_COL_Y + 6,       // HEUTE: starts at top
        FORECAST_COL_Y + 146,     // MORGEN
        FORECAST_COL_Y + 271      // UEBERMORGEN
    };

    for (int i = 0; i < 3; i++) {
        // Draw separator between days
        if (i > 0) {
            display.drawFastHLine(FORECAST_COL_X, yPositions[i] - 1, FORECAST_COL_WIDTH, TFT_BLACK);
        }

        drawDailyForecastSection(display, forecast.days[i],
                                 FORECAST_COL_X, yPositions[i],
                                 dayLabels[i], i == 0);  // isToday = (i == 0)
    }
}

void drawStatusBar(TFT_eSprite& display) {
    // Just draw separator line at bottom
    display.drawFastHLine(0, STATUS_BAR_Y, SCREEN_WIDTH, TFT_BLACK);
}

void drawDashboard(TFT_eSprite& display, const DashboardData& data) {
    display.fillScreen(TFT_WHITE);

    // Header
    drawHeader(display, LOCATION_NAME, data.updateTime);

    // COLUMN 1: INDOOR SENSORS
    drawIndoorTempWidget(display, data.weather.indoor);
    drawIndoorHumidWidget(display, data.weather.indoor);
    drawAirQualityWidget(display, data.weather.indoor);
    drawPressureWidget(display, data.weather.indoor);

    // COLUMN 2: OUTDOOR SENSORS
    drawOutdoorTempWidget(display, data.weather.outdoor);
    drawOutdoorHumidWidget(display, data.weather.outdoor);
    drawWindWidget(display, data.weather.wind);
    drawRainWidget(display, data.weather.rain);

    // COLUMN 3: 3-DAY FORECAST
    draw3DayForecastColumn(display, data.forecast);

    // Status bar (separator line only)
    drawStatusBar(display);
}

// Legacy function names for compatibility
void drawIndoorCO2Widget(TFT_eSprite& display, const IndoorData& data) {
    drawAirQualityWidget(display, data);
}

void drawIndoorPressureWidget(TFT_eSprite& display, const IndoorData& data) {
    drawPressureWidget(display, data);
}
