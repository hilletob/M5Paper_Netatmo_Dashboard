#include "widgets.h"
#include "layout.h"
#include "fonts.h"
#include "icons.h"
#include "../config.h"
#include <time.h>

// Helper to draw a card background + border (clears previous ghosting)
void drawCard(M5EPD_Canvas& display, int x, int y, int height, int width = CARD_WIDTH) {
    display.fillRect(x, y, width, height, 0);   // white background
    display.drawRect(x, y, width, height, 15);  // border
}

// Helper to format timestamp as time (HH:MM)
void formatTime(unsigned long timestamp, char* buffer, size_t bufferSize) {
    if (timestamp == 0) {
        snprintf(buffer, bufferSize, "--:--");
        return;
    }
    time_t t = timestamp;
    struct tm* tm = localtime(&t);
    snprintf(buffer, bufferSize, "%02d:%02d", tm->tm_hour, tm->tm_min);
}

// Note: drawTemperature() function removed - TTF fonts support native ° symbol
// Temperature values now use: snprintf(buf, sizeof(buf), "%.1f°C", temp);

// Helper to draw trend arrow using geometric shapes
void drawTrendArrow(M5EPD_Canvas& display, int x, int y, Trend trend) {
    int cx = x + 12;  // Center X
    int cy = y + 12;  // Center Y

    switch (trend) {
        case Trend::UP:
            // Diagonal arrow pointing up-right (↗)
            // Shaft
            display.drawLine(cx - 6, cy + 6, cx + 6, cy - 6, 15);
            display.drawLine(cx - 5, cy + 6, cx + 6, cy - 5, 15);
            display.drawLine(cx - 6, cy + 5, cx + 5, cy - 6, 15);
            // Arrowhead
            display.fillTriangle(cx + 6, cy - 6,
                                cx + 6, cy - 2,
                                cx + 2, cy - 6, 15);
            break;

        case Trend::DOWN:
            // Diagonal arrow pointing down-right (↘)
            // Shaft
            display.drawLine(cx - 6, cy - 6, cx + 6, cy + 6, 15);
            display.drawLine(cx - 5, cy - 6, cx + 6, cy + 5, 15);
            display.drawLine(cx - 6, cy - 5, cx + 5, cy + 6, 15);
            // Arrowhead
            display.fillTriangle(cx + 6, cy + 6,
                                cx + 6, cy + 2,
                                cx + 2, cy + 6, 15);
            break;

        case Trend::STABLE:
            // Horizontal arrow pointing right (→)
            // Shaft
            display.drawLine(cx - 8, cy, cx + 6, cy, 15);
            display.drawLine(cx - 8, cy - 1, cx + 6, cy - 1, 15);
            display.drawLine(cx - 8, cy + 1, cx + 6, cy + 1, 15);
            // Arrowhead
            display.fillTriangle(cx + 6, cy,
                                cx + 2, cy - 4,
                                cx + 2, cy + 4, 15);
            break;

        default:
            break;
    }
}

void drawWeatherIcon(M5EPD_Canvas& display, int x, int y, const char* iconName, int size) {
    // Center point for the icon
    int cx = x + size / 2;
    int cy = y + size / 2;

    if (strcmp(iconName, "sunny") == 0) {
        // Sun icon with rays
        int sunRadius = size / 6;
        int rayLength = size / 5;

        // Draw sun circle (filled)
        display.fillCircle(cx, cy, sunRadius, 15);

        // Draw 8 rays around the sun
        for (int i = 0; i < 8; i++) {
            float angle = i * PI / 4;
            int innerX = cx + cos(angle) * (sunRadius + 2);
            int innerY = cy + sin(angle) * (sunRadius + 2);
            int outerX = cx + cos(angle) * (sunRadius + rayLength);
            int outerY = cy + sin(angle) * (sunRadius + rayLength);

            // Draw thick rays
            display.drawLine(innerX, innerY, outerX, outerY, 15);
            display.drawLine(innerX + 1, innerY, outerX + 1, outerY, 15);
            display.drawLine(innerX, innerY + 1, outerX, outerY + 1, 15);
        }
    }
    else if (strcmp(iconName, "partly_cloudy") == 0) {
        // Sun behind cloud
        int sunRadius = size / 8;
        int sunX = cx - size / 6;
        int sunY = cy - size / 6;

        // Draw partial sun (upper left)
        display.drawCircle(sunX, sunY, sunRadius, 15);
        display.drawCircle(sunX, sunY, sunRadius - 1, 15);

        // Draw 4 sun rays (only visible ones)
        for (int i = 0; i < 4; i++) {
            float angle = (i * PI / 2) + PI;  // Left and top rays
            int innerX = sunX + cos(angle) * (sunRadius + 1);
            int innerY = sunY + sin(angle) * (sunRadius + 1);
            int outerX = sunX + cos(angle) * (sunRadius + size / 8);
            int outerY = sunY + sin(angle) * (sunRadius + size / 8);
            display.drawLine(innerX, innerY, outerX, outerY, 15);
        }

        // Draw cloud (overlapping sun)
        int cloudY = cy + size / 10;
        // Cloud bumps (3 circles)
        display.fillCircle(cx - size / 8, cloudY, size / 8, 15);
        display.fillCircle(cx, cloudY - size / 12, size / 7, 15);
        display.fillCircle(cx + size / 8, cloudY, size / 8, 15);
        // Cloud base
        display.fillRect(cx - size / 6, cloudY, size / 3, size / 8, 15);
    }
    else if (strcmp(iconName, "cloudy") == 0) {
        // Cloud icon - three bumps
        int cloudY = cy;

        // Draw three overlapping circles for cloud bumps
        display.fillCircle(cx - size / 6, cloudY, size / 7, 15);
        display.fillCircle(cx, cloudY - size / 10, size / 6, 15);
        display.fillCircle(cx + size / 6, cloudY, size / 7, 15);

        // Cloud base rectangle
        display.fillRect(cx - size / 5, cloudY, size / 2.5, size / 7, 15);
    }
    else if (strcmp(iconName, "rain") == 0) {
        // Cloud with rain drops
        int cloudY = cy - size / 8;

        // Draw cloud
        display.fillCircle(cx - size / 8, cloudY, size / 9, 15);
        display.fillCircle(cx, cloudY - size / 12, size / 8, 15);
        display.fillCircle(cx + size / 8, cloudY, size / 9, 15);
        display.fillRect(cx - size / 7, cloudY, size / 3.5, size / 9, 15);

        // Draw rain drops (5 drops)
        int dropY = cy + size / 6;
        for (int i = 0; i < 5; i++) {
            int dropX = cx - size / 5 + (i * size / 10);
            // Draw thick rain lines
            display.drawLine(dropX, dropY, dropX, dropY + size / 8, 15);
            display.drawLine(dropX + 1, dropY, dropX + 1, dropY + size / 8, 15);
        }
    }
    else if (strcmp(iconName, "snow") == 0) {
        // Cloud with snowflakes
        int cloudY = cy - size / 8;

        // Draw cloud
        display.fillCircle(cx - size / 8, cloudY, size / 9, 15);
        display.fillCircle(cx, cloudY - size / 12, size / 8, 15);
        display.fillCircle(cx + size / 8, cloudY, size / 9, 15);
        display.fillRect(cx - size / 7, cloudY, size / 3.5, size / 9, 15);

        // Draw snowflakes (asterisks)
        int flakeY = cy + size / 6;
        int flakeSize = size / 12;

        for (int i = 0; i < 3; i++) {
            int flakeX = cx - size / 6 + (i * size / 6);
            // Vertical line
            display.drawLine(flakeX, flakeY - flakeSize, flakeX, flakeY + flakeSize, 15);
            // Horizontal line
            display.drawLine(flakeX - flakeSize, flakeY, flakeX + flakeSize, flakeY, 15);
            // Diagonal lines
            display.drawLine(flakeX - flakeSize/1.5, flakeY - flakeSize/1.5,
                           flakeX + flakeSize/1.5, flakeY + flakeSize/1.5, 15);
            display.drawLine(flakeX - flakeSize/1.5, flakeY + flakeSize/1.5,
                           flakeX + flakeSize/1.5, flakeY - flakeSize/1.5, 15);
        }
    }
}

void drawHeader(M5EPD_Canvas& display, const char* location, unsigned long updateTime, unsigned long nextWakeTime, bool isFallback) {
    // Draw header background
    display.fillRect(0, HEADER_Y, SCREEN_WIDTH, HEADER_HEIGHT, 0);
    display.drawFastHLine(0, HEADER_HEIGHT, SCREEN_WIDTH, 15);

    // Location on left (vertically centered)
    display.setTextColor(15, 0);
    display.setTextDatum(TL_DATUM);
    setBoldFont(display, 28);
    display.drawString(location, MARGIN, HEADER_Y + 12);

    // Right side: two lines with small font
    setRegularFont(display, 24);
    display.setTextDatum(TR_DATUM);

    // Line 1: Last Netatmo update time
    if (updateTime > 0) {
        char updateStr[32];
        time_t t = updateTime;
        struct tm* tm = localtime(&t);
        snprintf(updateStr, sizeof(updateStr), "Aktualisiert: %02d.%02d. %02d:%02d",
                tm->tm_mday, tm->tm_mon + 1, tm->tm_hour, tm->tm_min);
        display.drawString(updateStr, SCREEN_WIDTH - MARGIN, HEADER_Y + 3);
    }

    // Line 2: Next scheduled wake time
    if (nextWakeTime > 0) {
        char nextStr[48];
        time_t t = nextWakeTime;
        struct tm* tm = localtime(&t);
        if (isFallback) {
            snprintf(nextStr, sizeof(nextStr), "Nächstes: %02d:%02d (Fallback)",
                    tm->tm_hour, tm->tm_min);
        } else {
            snprintf(nextStr, sizeof(nextStr), "Nächstes: %02d:%02d",
                    tm->tm_hour, tm->tm_min);
        }
        display.drawString(nextStr, SCREEN_WIDTH - MARGIN, HEADER_Y + 28);
    }
}

void drawIndoorTempWidget(M5EPD_Canvas& display, const IndoorData& data) {
    if (!data.valid) return;

    drawCard(display, INDOOR_TEMP_X, INDOOR_TEMP_Y, TEMP_CARD_HEIGHT);

    display.setTextColor(15, 0);

    // Label
    display.setTextDatum(TL_DATUM);
    setRegularFont(display, 28);
    display.drawString("Innen", INDOOR_TEMP_X + CARD_PADDING, INDOOR_TEMP_Y + CARD_LABEL_Y);

    // Large temperature value (bold) then "°C" inline (small)
    char tempStr[16];
    snprintf(tempStr, sizeof(tempStr), "%.1f", data.temperature);
    setBoldFont(display, 64);
    display.setTextDatum(TL_DATUM);
    int valueX = INDOOR_TEMP_X + CARD_PADDING;
    int valueY = INDOOR_TEMP_Y + CARD_VALUE_Y;
    int textW = display.drawString(tempStr, valueX, valueY);
    setRegularFont(display, 28);
    display.drawString("°C", valueX + textW + 6, valueY + 26);

    // Trend arrow
    drawTrendArrow(display, INDOOR_TEMP_X + CARD_TREND_X_OFFSET, INDOOR_TEMP_Y + CARD_TREND_Y, data.temperatureTrend);

    // Min/max detail (two lines)
    if (data.minTemp != 0 || data.maxTemp != 0) {
        display.setTextDatum(TL_DATUM);
        int y = INDOOR_TEMP_Y + CARD_DETAIL_Y;
        int x = INDOOR_TEMP_X + CARD_PADDING;

        // Line 1: min
        char valStr[16];
        char timeStr[16];
        int w;
        setRegularFont(display, 28);
        w = display.drawString("min ", x, y);
        snprintf(valStr, sizeof(valStr), "%.1f ", data.minTemp);
        setBoldFont(display, 28);
        w += display.drawString(valStr, x + w, y);
        setRegularFont(display, 28);
        w += display.drawString("°C ", x + w, y);
        formatTime(data.dateMinTemp, timeStr, sizeof(timeStr));
        display.drawString(timeStr, x + w, y);

        // Line 2: max
        y += 38;
        setRegularFont(display, 28);
        w = display.drawString("max ", x, y);
        snprintf(valStr, sizeof(valStr), "%.1f ", data.maxTemp);
        setBoldFont(display, 28);
        w += display.drawString(valStr, x + w, y);
        setRegularFont(display, 28);
        w += display.drawString("°C ", x + w, y);
        formatTime(data.dateMaxTemp, timeStr, sizeof(timeStr));
        display.drawString(timeStr, x + w, y);
    }
}

void drawOutdoorTempWidget(M5EPD_Canvas& display, const OutdoorData& data) {
    drawCard(display, OUTDOOR_TEMP_X, OUTDOOR_TEMP_Y, TEMP_CARD_HEIGHT);

    display.setTextColor(15, 0);
    display.setTextDatum(TL_DATUM);
    setRegularFont(display, 28);
    display.drawString("Aussen", OUTDOOR_TEMP_X + CARD_PADDING, OUTDOOR_TEMP_Y + CARD_LABEL_Y);

    if (!data.valid) {
        setBoldFont(display, 64);
        display.drawString("n/a", OUTDOOR_TEMP_X + CARD_PADDING, OUTDOOR_TEMP_Y + CARD_VALUE_Y);
        return;
    }

    char tempStr[16];
    snprintf(tempStr, sizeof(tempStr), "%.1f", data.temperature);
    setBoldFont(display, 64);
    display.setTextDatum(TL_DATUM);
    int valueX = OUTDOOR_TEMP_X + CARD_PADDING;
    int valueY = OUTDOOR_TEMP_Y + CARD_VALUE_Y;
    int textW = display.drawString(tempStr, valueX, valueY);
    setRegularFont(display, 28);
    display.drawString("°C", valueX + textW + 6, valueY + 26);

    drawTrendArrow(display, OUTDOOR_TEMP_X + CARD_TREND_X_OFFSET, OUTDOOR_TEMP_Y + CARD_TREND_Y, data.temperatureTrend);

    // Min/max detail (two lines)
    if (data.minTemp != 0 || data.maxTemp != 0) {
        display.setTextDatum(TL_DATUM);
        int y = OUTDOOR_TEMP_Y + CARD_DETAIL_Y;
        int x = OUTDOOR_TEMP_X + CARD_PADDING;

        // Line 1: min
        char valStr[16];
        char timeStr[16];
        int w;
        setRegularFont(display, 28);
        w = display.drawString("min ", x, y);
        snprintf(valStr, sizeof(valStr), "%.1f ", data.minTemp);
        setBoldFont(display, 28);
        w += display.drawString(valStr, x + w, y);
        setRegularFont(display, 28);
        w += display.drawString("°C ", x + w, y);
        formatTime(data.dateMinTemp, timeStr, sizeof(timeStr));
        display.drawString(timeStr, x + w, y);

        // Line 2: max
        y += 38;
        setRegularFont(display, 28);
        w = display.drawString("max ", x, y);
        snprintf(valStr, sizeof(valStr), "%.1f ", data.maxTemp);
        setBoldFont(display, 28);
        w += display.drawString(valStr, x + w, y);
        setRegularFont(display, 28);
        w += display.drawString("°C ", x + w, y);
        formatTime(data.dateMaxTemp, timeStr, sizeof(timeStr));
        display.drawString(timeStr, x + w, y);
    }
}

void drawIndoorHumidWidget(M5EPD_Canvas& display, const IndoorData& data) {
    if (!data.valid) return;

    drawCard(display, INDOOR_HUMID_X, INDOOR_HUMID_Y, HUMID_CARD_HEIGHT);

    display.setTextColor(15, 0);
    display.setTextDatum(TL_DATUM);
    setRegularFont(display, 28);
    display.drawString("Innen", INDOOR_HUMID_X + CARD_PADDING, INDOOR_HUMID_Y + CARD_LABEL_Y);

    // Draw humidity value (bold) then "%" inline (small)
    char humidStr[16];
    snprintf(humidStr, sizeof(humidStr), "%d", data.humidity);
    display.setTextDatum(TL_DATUM);
    setBoldFont(display, 64);
    int valueX = INDOOR_HUMID_X + CARD_PADDING;
    int valueY = INDOOR_HUMID_Y + CARD_VALUE_Y;
    int textW = display.drawString(humidStr, valueX, valueY);
    setRegularFont(display, 28);
    display.drawString("%", valueX + textW + 6, valueY + 26);

    // Climate status
    const char* status = getHumidityComfort(data.humidity);
    char detailStr[32];
    snprintf(detailStr, sizeof(detailStr), "Klima: %s", status);
    display.setTextDatum(TL_DATUM);
    setRegularFont(display, 28);
    display.drawString(detailStr, INDOOR_HUMID_X + CARD_PADDING, INDOOR_HUMID_Y + CARD_DETAIL_Y);
}

void drawOutdoorHumidWidget(M5EPD_Canvas& display, const OutdoorData& data) {
    drawCard(display, OUTDOOR_HUMID_X, OUTDOOR_HUMID_Y, HUMID_CARD_HEIGHT);

    display.setTextColor(15, 0);
    display.setTextDatum(TL_DATUM);
    setRegularFont(display, 28);
    display.drawString("Aussen", OUTDOOR_HUMID_X + CARD_PADDING, OUTDOOR_HUMID_Y + CARD_LABEL_Y);

    if (!data.valid) {
        setBoldFont(display, 64);
        display.drawString("n/a", OUTDOOR_HUMID_X + CARD_PADDING, OUTDOOR_HUMID_Y + CARD_VALUE_Y);
        return;
    }

    // Draw humidity value (bold) then "%" inline (small)
    char humidStr[16];
    snprintf(humidStr, sizeof(humidStr), "%d", data.humidity);
    display.setTextDatum(TL_DATUM);
    setBoldFont(display, 64);
    int valueX = OUTDOOR_HUMID_X + CARD_PADDING;
    int valueY = OUTDOOR_HUMID_Y + CARD_VALUE_Y;
    int textW = display.drawString(humidStr, valueX, valueY);
    setRegularFont(display, 28);
    display.drawString("%", valueX + textW + 6, valueY + 26);

    // Dew point calculation (Magnus formula)
    float a = 17.27;
    float b = 237.7;
    float alpha = ((a * data.temperature) / (b + data.temperature)) + log(data.humidity / 100.0);
    float dewPoint = (b * alpha) / (a - alpha);

    // Draw dew point: label regular, value bold, unit regular
    display.setTextDatum(TL_DATUM);
    int dewX = OUTDOOR_HUMID_X + CARD_PADDING;
    int dewY = OUTDOOR_HUMID_Y + CARD_DETAIL_Y;
    setRegularFont(display, 28);
    int dewW = display.drawString("Taupunkt: ", dewX, dewY);
    char dewVal[16];
    snprintf(dewVal, sizeof(dewVal), "%.1f ", dewPoint);
    setBoldFont(display, 28);
    dewW += display.drawString(dewVal, dewX + dewW, dewY);
    setRegularFont(display, 28);
    display.drawString("°C", dewX + dewW, dewY);
}

void drawAirQualityWidget(M5EPD_Canvas& display, const IndoorData& data) {
    if (!data.valid) return;

    drawCard(display, AIR_QUALITY_X, AIR_QUALITY_Y, AIR_QUALITY_CARD_HEIGHT);

    display.setTextColor(15, 0);
    display.setTextDatum(TL_DATUM);
    setRegularFont(display, 28);
    display.drawString("Luftqualität innen", AIR_QUALITY_X + CARD_PADDING, AIR_QUALITY_Y + CARD_LABEL_Y);

    // Draw CO2 value (bold) then "ppm" inline
    char co2Str[16];
    snprintf(co2Str, sizeof(co2Str), "%d", data.co2);
    display.setTextDatum(TL_DATUM);
    setBoldFont(display, 64);
    int valueX = AIR_QUALITY_X + CARD_PADDING;
    int valueY = AIR_QUALITY_Y + CARD_VALUE_Y;
    int textW = display.drawString(co2Str, valueX, valueY);

    // Draw "ppm" in smaller font on the same line
    setRegularFont(display, 28);
    display.drawString("ppm", valueX + textW + 6, valueY + 26);

    drawTrendArrow(display, AIR_QUALITY_X + CARD_TREND_X_OFFSET, AIR_QUALITY_Y + CARD_TREND_Y, data.co2Trend);
}

void drawPressureWidget(M5EPD_Canvas& display, const IndoorData& data) {
    if (!data.valid) return;

    drawCard(display, PRESSURE_X, PRESSURE_Y, PRESSURE_CARD_HEIGHT);

    display.setTextColor(15, 0);
    display.setTextDatum(TL_DATUM);
    setRegularFont(display, 28);
    display.drawString("Luftdruck aussen", PRESSURE_X + CARD_PADDING, PRESSURE_Y + CARD_LABEL_Y);

    // Draw pressure value (bold) then "hPa" inline
    char pressStr[16];
    snprintf(pressStr, sizeof(pressStr), "%d", data.pressure);
    display.setTextDatum(TL_DATUM);
    setBoldFont(display, 64);
    int valueX = PRESSURE_X + CARD_PADDING;
    int valueY = PRESSURE_Y + CARD_VALUE_Y;
    int textW = display.drawString(pressStr, valueX, valueY);

    // Draw "hPa" in smaller font on the same line
    setRegularFont(display, 28);
    display.drawString("hPa", valueX + textW + 6, valueY + 26);

    drawTrendArrow(display, PRESSURE_X + CARD_TREND_X_OFFSET, PRESSURE_Y + CARD_TREND_Y, data.pressureTrend);
}

void drawForecastWidget(M5EPD_Canvas& display, const ForecastData& forecast) {
    // Full-width forecast card
    drawCard(display, FORECAST_WIDGET_X, FORECAST_WIDGET_Y, FORECAST_CARD_HEIGHT, FULL_CARD_WIDTH);

    display.setTextColor(15, 0);

    // Label
    display.setTextDatum(TL_DATUM);
    setRegularFont(display, 28);
    display.drawString("Vorhersage", FORECAST_WIDGET_X + CARD_PADDING, FORECAST_WIDGET_Y + CARD_LABEL_Y);

    // Get day names
    time_t now = time(nullptr);
    struct tm* tm = localtime(&now);

    const char* dayNames[] = {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"};

    int baseX = FORECAST_WIDGET_X;

    // Base timestamp for fallback day labels
    time_t todayTs = forecast.days[0].date ? forecast.days[0].date : now;

    // Draw 4 days (today + next 3)
    int dayHeight = 84;
    int startY = FORECAST_WIDGET_Y + 32;

    for (int day = 0; day < 4; day++) {
        const DailyForecast& df = forecast.days[day];

        int rowY = startY + (day * dayHeight);

        // Separator line between rows (not before first)
        if (day > 0) {
            display.drawFastHLine(baseX + 8, rowY - 4, FULL_CARD_WIDTH - 16, 8);  // light gray line
        }

        // Day label
        time_t rowTs = df.date ? df.date : (todayTs + day * 86400);
        struct tm* rowTm = localtime(&rowTs);
        int dayOfWeek = rowTm ? rowTm->tm_wday : ((tm->tm_wday + day) % 7);
        setBoldFont(display, 28);
        display.setTextDatum(TL_DATUM);
        display.drawString(dayNames[dayOfWeek], baseX + FORECAST_DAY_COL_X, rowY + 4);

        if (!df.valid) {
            setRegularFont(display, 24);
            display.drawString("--", baseX + FORECAST_SLOT1_X, rowY + 20);
            continue;
        }

        auto renderSlot = [&](int slotIdx, int xOffset, bool isNight) {
            const DayTimeForecast* dtf = nullptr;
            if (slotIdx < 3 && df.times[slotIdx].hour != 0) {
                dtf = &df.times[slotIdx];
            }

            const char* iconName;
            int8_t tempVal;

            if (isNight) {
                const DayTimeForecast* night = nullptr;
                if (df.times[2].hour != 0) {
                    night = &df.times[2];
                }

                if (night) {
                    iconName = getIconFromCode(night->symbolCode);
                    tempVal = night->temperature;
                } else {
                    iconName = getIconFromCode(df.symbolCode);
                    tempVal = df.tempMin;
                }
            } else if (dtf) {
                iconName = getIconFromCode(dtf->symbolCode);
                tempVal = dtf->temperature;
            } else {
                iconName = getIconFromCode(df.symbolCode);
                if (slotIdx == 0) {
                    tempVal = df.tempMin;
                } else if (slotIdx == 2) {
                    tempVal = df.tempMax;
                } else {
                    tempVal = (int8_t)((df.tempMin + df.tempMax) / 2);
                }
            }

            int iconX = baseX + xOffset;
            int iconY = rowY + 12;
            drawWeatherIcon(display, iconX, iconY, iconName, FORECAST_ICON_SIZE);

            char tempStr[12];
            snprintf(tempStr, sizeof(tempStr), "%d°C", tempVal);
            setBoldFont(display, 24);
            display.setTextDatum(TC_DATUM);
            display.drawString(tempStr, iconX + FORECAST_ICON_SIZE / 2, iconY + FORECAST_ICON_SIZE - 2);
        };

        renderSlot(0, FORECAST_SLOT1_X, false);
        renderSlot(1, FORECAST_SLOT2_X, false);
        renderSlot(2, FORECAST_SLOT3_X, false);
        renderSlot(3, FORECAST_SLOT4_X, true);

        // Precipitation total
        char precipStr[16];
        snprintf(precipStr, sizeof(precipStr), "%.1fmm", df.precipSum / 10.0);
        setRegularFont(display, 24);
        display.setTextDatum(TL_DATUM);
        display.drawString(precipStr, baseX + FORECAST_PRECIP_X, rowY + 18);

        // Min/max temperature range
        char rangeStr[16];
        snprintf(rangeStr, sizeof(rangeStr), "%d/%d°C", df.tempMin, df.tempMax);
        display.drawString(rangeStr, baseX + FORECAST_RANGE_X, rowY + 46);
    }
}

void drawBatteryWidget(M5EPD_Canvas& display, const BatteryStatus& status) {
    // Slim full-width battery bar
    drawCard(display, BATTERY_X, BATTERY_Y, BATTERY_CARD_HEIGHT, FULL_CARD_WIDTH);

    display.setTextColor(15, 0);

    // Battery icon (right side of card)
    int iconX = BATTERY_X + FULL_CARD_WIDTH - 80;
    int iconY = BATTERY_Y + 16;
    display.drawRect(iconX, iconY, 60, 24, 15);
    display.fillRect(iconX + 60, iconY + 6, 4, 12, 15);  // Nub

    // Fill based on percentage (grayscale for low battery)
    int fillW = (56) * status.percent / 100;
    uint8_t fillColor = (status.percent < 20) ? 12 : 15;  // Darker gray for low
    display.fillRect(iconX + 2, iconY + 2, fillW, 20, fillColor);

    // Percentage and voltage on one line
    display.setTextDatum(TL_DATUM);
    setRegularFont(display, 28);

    char statusStr[24];
    snprintf(statusStr, sizeof(statusStr), "%d%% %s", status.percent, status.label);
    display.drawString(statusStr, BATTERY_X + CARD_PADDING, BATTERY_Y + 12);

    char voltStr[16];
    snprintf(voltStr, sizeof(voltStr), "%.2fV", status.millivolts / 1000.0);
    display.drawString(voltStr, BATTERY_X + CARD_PADDING + 200, BATTERY_Y + 12);

    if (status.charging || status.externalPower) {
        setRegularFont(display, 24);
        const char* plug = status.charging ? "Laden" : "Netz";
        display.drawString(plug, BATTERY_X + CARD_PADDING + 330, BATTERY_Y + 14);
    }
}

void drawDashboard(M5EPD_Canvas& display, const DashboardData& data) {
    display.fillCanvas(0);  // White background (M5EPD uses fillCanvas, not fillScreen)

    // Header
    drawHeader(display, LOCATION_NAME, data.updateTime, data.nextWakeTime, data.isFallback);

    // ROW 1: Temperature (col1=indoor, col2=outdoor)
    drawIndoorTempWidget(display, data.weather.indoor);
    drawOutdoorTempWidget(display, data.weather.outdoor);

    // ROW 2: Humidity (col1=indoor, col2=outdoor)
    drawIndoorHumidWidget(display, data.weather.indoor);
    drawOutdoorHumidWidget(display, data.weather.outdoor);

    // ROW 3: CO2 (col1) + Pressure (col2)
    drawAirQualityWidget(display, data.weather.indoor);
    drawPressureWidget(display, data.weather.indoor);

    // ROW 4: Full-width forecast
    drawForecastWidget(display, data.forecast);

    // ROW 5: Full-width battery bar
    drawBatteryWidget(display, evaluateBattery(data.batteryVoltage));
}

// Legacy function names for compatibility
void drawIndoorCO2Widget(M5EPD_Canvas& display, const IndoorData& data) {
    drawAirQualityWidget(display, data);
}

void drawIndoorPressureWidget(M5EPD_Canvas& display, const IndoorData& data) {
    drawPressureWidget(display, data);
}
