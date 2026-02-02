#include "widgets.h"
#include "layout.h"
#include "fonts.h"
#include "icons.h"
#include "../config.h"
#include <time.h>

// Helper to draw a card border
void drawCard(M5EPD_Canvas& display, int x, int y, int height) {
    display.drawRect(x, y, CARD_WIDTH, height, 15);
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

void drawHeader(M5EPD_Canvas& display, const char* location, unsigned long timestamp) {
    // Draw header background
    display.fillRect(0, HEADER_Y, SCREEN_WIDTH, HEADER_HEIGHT, 0);
    display.drawFastHLine(0, HEADER_HEIGHT, SCREEN_WIDTH, 15);

    // Location on left
    display.setTextColor(15, 0);
    display.setTextDatum(TL_DATUM);
    setRegularFont(display, 28);
    display.drawString(location, MARGIN, HEADER_Y + 15);  // Vertically centered

    // Date/time on right (same size as location)
    if (timestamp > 0) {
        time_t t = timestamp;
        struct tm* tm = localtime(&t);
        char dateStr[32];
        snprintf(dateStr, sizeof(dateStr), "%02d.%02d.%04d %02d:%02d",
                tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900,
                tm->tm_hour, tm->tm_min);
        display.setTextDatum(TR_DATUM);
        setRegularFont(display, 28);  // Match location font size
        display.drawString(dateStr, SCREEN_WIDTH - MARGIN, HEADER_Y + 15);  // Vertically centered
    }
}

void drawIndoorTempWidget(M5EPD_Canvas& display, const IndoorData& data) {
    if (!data.valid) return;

    display.drawRect(INDOOR_TEMP_X, INDOOR_TEMP_Y, CARD_WIDTH, TEMP_CARD_HEIGHT, 15);

    display.setTextColor(15, 0);

    // Label
    display.setTextDatum(TL_DATUM);
    setRegularFont(display, 28);
    display.drawString("Innen", INDOOR_TEMP_X + CARD_PADDING, INDOOR_TEMP_Y + CARD_LABEL_Y);

    // Large temperature value with degree symbol
    char tempStr[16];
    snprintf(tempStr, sizeof(tempStr), "%.1f°C", data.temperature);
    setBoldFont(display, 48);
    display.setTextDatum(TL_DATUM);
    display.drawString(tempStr, INDOOR_TEMP_X + CARD_PADDING, INDOOR_TEMP_Y + CARD_VALUE_Y);

    // Trend arrow
    drawTrendArrow(display, INDOOR_TEMP_X + CARD_TREND_X_OFFSET, INDOOR_TEMP_Y + CARD_TREND_Y, data.temperatureTrend);

    // Min/max detail with custom degree symbols (two lines)
    if (data.minTemp != 0 || data.maxTemp != 0) {
        display.setTextDatum(TL_DATUM);
        setRegularFont(display, 28);
        int y = INDOOR_TEMP_Y + CARD_DETAIL_Y;
        int pipeX = INDOOR_TEMP_X + 120;  // Fixed position for pipe alignment

        // Line 1: min
        int x = INDOOR_TEMP_X + CARD_PADDING;
        setRegularFont(display, 28);
        display.drawString("min  ", x, y);
        x += display.textWidth("min  ");

        // Temperature value in bold
        setBoldFont(display, 48);
        char tempStr[10];
        snprintf(tempStr, sizeof(tempStr), "%.1f", data.minTemp);
        display.drawString(tempStr, x, y);
        x += display.textWidth(tempStr) + 5;

        // Degree symbol
        display.drawCircle(x, y + 3, 2, 15);
        display.drawCircle(x, y + 3, 3, 15);
        x += 5;

        setRegularFont(display, 28);
        display.drawString("C  ", x, y);
        x += display.textWidth("C  ");

        // Min time
        char timeStr[16];
        formatTime(data.dateMinTemp, timeStr, sizeof(timeStr));
        display.drawString(timeStr, x, y);
        display.drawString(" Uhr", x + display.textWidth("00:00"), y);

        // Line 2: max
        y += 55;  // Move to next line (larger fonts)
        x = INDOOR_TEMP_X + CARD_PADDING;
        setRegularFont(display, 28);
        display.drawString("max  ", x, y);
        x += display.textWidth("max  ");

        // Temperature value in bold
        setBoldFont(display, 48);
        snprintf(tempStr, sizeof(tempStr), "%.1f", data.maxTemp);
        display.drawString(tempStr, x, y);
        x += display.textWidth(tempStr) + 5;

        // Degree symbol
        display.drawCircle(x, y + 3, 2, 15);
        display.drawCircle(x, y + 3, 3, 15);
        x += 5;

        setRegularFont(display, 28);
        display.drawString("C  ", x, y);
        x += display.textWidth("C  ");

        // Max time
        formatTime(data.dateMaxTemp, timeStr, sizeof(timeStr));
        display.drawString(timeStr, x, y);
        display.drawString(" Uhr", x + display.textWidth("00:00"), y);
    }
}

void drawOutdoorTempWidget(M5EPD_Canvas& display, const OutdoorData& data) {
    display.drawRect(OUTDOOR_TEMP_X, OUTDOOR_TEMP_Y, CARD_WIDTH, TEMP_CARD_HEIGHT, 15);

    display.setTextColor(15, 0);
    display.setTextDatum(TL_DATUM);
    setRegularFont(display, 28);
    display.drawString("Aussen", OUTDOOR_TEMP_X + CARD_PADDING, OUTDOOR_TEMP_Y + CARD_LABEL_Y);

    if (!data.valid) {
        setBoldFont(display, 48);
        display.drawString("n/a", OUTDOOR_TEMP_X + CARD_PADDING, OUTDOOR_TEMP_Y + CARD_VALUE_Y);
        return;
    }

    char tempStr[16];
    snprintf(tempStr, sizeof(tempStr), "%.1f°C", data.temperature);
    setBoldFont(display, 48);
    display.setTextDatum(TL_DATUM);
    display.drawString(tempStr, OUTDOOR_TEMP_X + CARD_PADDING, OUTDOOR_TEMP_Y + CARD_VALUE_Y);

    drawTrendArrow(display, OUTDOOR_TEMP_X + CARD_TREND_X_OFFSET, OUTDOOR_TEMP_Y + CARD_TREND_Y, data.temperatureTrend);

    // Min/max detail with custom degree symbols (two lines)
    if (data.minTemp != 0 || data.maxTemp != 0) {
        display.setTextDatum(TL_DATUM);
        setRegularFont(display, 28);
        int y = OUTDOOR_TEMP_Y + CARD_DETAIL_Y;
        int pipeX = OUTDOOR_TEMP_X + 120;  // Fixed position for pipe alignment

        // Line 1: min
        int x = OUTDOOR_TEMP_X + CARD_PADDING;
        setRegularFont(display, 28);
        display.drawString("min  ", x, y);
        x += display.textWidth("min  ");

        // Temperature value in bold
        setBoldFont(display, 48);
        char tempStr[10];
        snprintf(tempStr, sizeof(tempStr), "%.1f", data.minTemp);
        display.drawString(tempStr, x, y);
        x += display.textWidth(tempStr) + 5;

        // Degree symbol
        display.drawCircle(x, y + 3, 2, 15);
        display.drawCircle(x, y + 3, 3, 15);
        x += 5;

        setRegularFont(display, 28);
        display.drawString("C  ", x, y);
        x += display.textWidth("C  ");

        // Min time
        char timeStr[16];
        formatTime(data.dateMinTemp, timeStr, sizeof(timeStr));
        display.drawString(timeStr, x, y);
        display.drawString(" Uhr", x + display.textWidth("00:00"), y);

        // Line 2: max
        y += 55;  // Move to next line (larger fonts)
        x = OUTDOOR_TEMP_X + CARD_PADDING;
        setRegularFont(display, 28);
        display.drawString("max  ", x, y);
        x += display.textWidth("max  ");

        // Temperature value in bold
        setBoldFont(display, 48);
        snprintf(tempStr, sizeof(tempStr), "%.1f", data.maxTemp);
        display.drawString(tempStr, x, y);
        x += display.textWidth(tempStr) + 5;

        // Degree symbol
        display.drawCircle(x, y + 3, 2, 15);
        display.drawCircle(x, y + 3, 3, 15);
        x += 5;

        setRegularFont(display, 28);
        display.drawString("C  ", x, y);
        x += display.textWidth("C  ");

        // Max time
        formatTime(data.dateMaxTemp, timeStr, sizeof(timeStr));
        display.drawString(timeStr, x, y);
        display.drawString(" Uhr", x + display.textWidth("00:00"), y);
    }
}

void drawIndoorHumidWidget(M5EPD_Canvas& display, const IndoorData& data) {
    if (!data.valid) return;

    drawCard(display, INDOOR_HUMID_X, INDOOR_HUMID_Y, HUMID_CARD_HEIGHT);

    display.setTextColor(15, 0);
    display.setTextDatum(TL_DATUM);
    setRegularFont(display, 28);
    display.drawString("Innen", INDOOR_HUMID_X + CARD_PADDING, INDOOR_HUMID_Y + CARD_LABEL_Y);

    // Draw humidity value (bold) and unit (non-bold) separately
    char humidStr[16];
    snprintf(humidStr, sizeof(humidStr), "%d", data.humidity);
    display.setTextDatum(TL_DATUM);
    setBoldFont(display, 48);
    int valueX = INDOOR_HUMID_X + CARD_PADDING;
    int valueY = INDOOR_HUMID_Y + CARD_VALUE_Y;
    display.drawString(humidStr, valueX, valueY);

    // Draw unit in non-bold (measure width before font switch)
    int valueWidth = display.textWidth(humidStr);
    setRegularFont(display, 28);  // Use FSS18 to match size of FSSB18
    int unitX = valueX + valueWidth + 15;  // Increased spacing for larger fonts
    display.drawString("%", unitX, valueY);

    // Climate status
    const char* status = getHumidityComfort(data.humidity);
    char detailStr[32];
    snprintf(detailStr, sizeof(detailStr), "Raumklima: %s", status);
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
        setBoldFont(display, 48);
        display.drawString("n/a", OUTDOOR_HUMID_X + CARD_PADDING, OUTDOOR_HUMID_Y + CARD_VALUE_Y);
        return;
    }

    // Draw humidity value (bold) and unit (non-bold) separately
    char humidStr[16];
    snprintf(humidStr, sizeof(humidStr), "%d", data.humidity);
    display.setTextDatum(TL_DATUM);
    setBoldFont(display, 48);
    int valueX = OUTDOOR_HUMID_X + CARD_PADDING;
    int valueY = OUTDOOR_HUMID_Y + CARD_VALUE_Y;
    display.drawString(humidStr, valueX, valueY);

    // Draw unit in non-bold (measure width before font switch)
    int valueWidth = display.textWidth(humidStr);
    setRegularFont(display, 28);  // Use FSS18 to match size of FSSB18
    int unitX = valueX + valueWidth + 15;  // Increased spacing for larger fonts
    display.drawString("%", unitX, valueY);

    // Dew point calculation (Magnus formula)
    float a = 17.27;
    float b = 237.7;
    float alpha = ((a * data.temperature) / (b + data.temperature)) + log(data.humidity / 100.0);
    float dewPoint = (b * alpha) / (a - alpha);

    // Draw dew point with custom degree symbol (value bold, unit non-bold)
    display.setTextDatum(TL_DATUM);
    setRegularFont(display, 28);
    int dewX = OUTDOOR_HUMID_X + CARD_PADDING;
    int dewY = OUTDOOR_HUMID_Y + CARD_DETAIL_Y;

    // Draw "Taupunkt: " in non-bold
    display.drawString("Taupunkt: ", dewX, dewY);
    dewX += display.textWidth("Taupunkt: ");

    // Draw temperature value in bold
    char dewTempStr[10];
    snprintf(dewTempStr, sizeof(dewTempStr), "%.1f", dewPoint);
    setBoldFont(display, 48);
    display.drawString(dewTempStr, dewX, dewY);
    dewX += display.textWidth(dewTempStr) + 5;

    // Draw degree symbol (circle)
    display.drawCircle(dewX, dewY + 3, 2, 15);
    display.drawCircle(dewX, dewY + 3, 3, 15);
    dewX += 5;

    // Draw "C" in non-bold
    setRegularFont(display, 28);
    display.drawString("C", dewX, dewY);
}

void drawAirQualityWidget(M5EPD_Canvas& display, const IndoorData& data) {
    if (!data.valid) return;

    display.drawRect(AIR_QUALITY_X, AIR_QUALITY_Y, CARD_WIDTH, AIR_QUALITY_CARD_HEIGHT, 15);

    display.setTextColor(15, 0);
    display.setTextDatum(TL_DATUM);
    setRegularFont(display, 28);
    display.drawString("Luftqualitaet innen", AIR_QUALITY_X + CARD_PADDING, AIR_QUALITY_Y + CARD_LABEL_Y);

    // Draw CO2 value (bold) and unit (non-bold) separately
    char co2Str[16];
    snprintf(co2Str, sizeof(co2Str), "%d", data.co2);
    display.setTextDatum(TL_DATUM);
    setBoldFont(display, 48);
    int valueX = AIR_QUALITY_X + CARD_PADDING;
    int valueY = AIR_QUALITY_Y + CARD_VALUE_Y;
    display.drawString(co2Str, valueX, valueY);

    // Draw unit in non-bold (measure width before font switch)
    int valueWidth = display.textWidth(co2Str);
    setRegularFont(display, 28);  // Use FSS18 to match size of FSSB18
    int unitX = valueX + valueWidth + 15;  // Increased spacing for larger fonts
    display.drawString("ppm", unitX, valueY);

    drawTrendArrow(display, AIR_QUALITY_X + CARD_TREND_X_OFFSET, AIR_QUALITY_Y + CARD_TREND_Y, data.co2Trend);
}

void drawPressureWidget(M5EPD_Canvas& display, const IndoorData& data) {
    if (!data.valid) return;

    display.drawRect(PRESSURE_X, PRESSURE_Y, CARD_WIDTH, PRESSURE_CARD_HEIGHT, 15);

    display.setTextColor(15, 0);
    display.setTextDatum(TL_DATUM);
    setRegularFont(display, 28);
    display.drawString("Luftdruck aussen", PRESSURE_X + CARD_PADDING, PRESSURE_Y + CARD_LABEL_Y);

    // Draw pressure value (bold) and unit (non-bold) separately
    char pressStr[16];
    snprintf(pressStr, sizeof(pressStr), "%d", data.pressure);
    display.setTextDatum(TL_DATUM);
    setBoldFont(display, 48);
    int valueX = PRESSURE_X + CARD_PADDING;
    int valueY = PRESSURE_Y + CARD_VALUE_Y;
    display.drawString(pressStr, valueX, valueY);

    // Draw unit in non-bold (measure width before font switch)
    int valueWidth = display.textWidth(pressStr);
    setRegularFont(display, 28);  // Use FSS18 to match size of FSSB18
    int unitX = valueX + valueWidth + 15;  // Increased spacing for larger fonts
    display.drawString("hPa", unitX, valueY);

    drawTrendArrow(display, PRESSURE_X + CARD_TREND_X_OFFSET, PRESSURE_Y + CARD_TREND_Y, data.pressureTrend);
}

// Legacy forecast widgets (deprecated - now using 3-day forecast column)
// Kept for backward compatibility but not used in 3-column layout
void drawForecast3hWidget(M5EPD_Canvas& display, const ForecastPoint& forecast) {
    // No longer used in 3-column layout
}

void drawForecast6hWidget(M5EPD_Canvas& display, const ForecastPoint& forecast) {
    // No longer used in 3-column layout
}

void drawForecastWidget(M5EPD_Canvas& display, const ForecastData& forecast) {
    // Position: FORECAST_WIDGET_X, FORECAST_WIDGET_Y
    // Size: CARD_WIDTH × FORECAST_CARD_HEIGHT (288px)
    display.drawRect(FORECAST_WIDGET_X, FORECAST_WIDGET_Y, CARD_WIDTH, FORECAST_CARD_HEIGHT, 15);

    display.setTextColor(15, 0);

    // Label
    display.setTextDatum(TL_DATUM);
    setRegularFont(display, 28);
    display.drawString("3-Tage Vorhersage", FORECAST_WIDGET_X + CARD_PADDING, FORECAST_WIDGET_Y + CARD_LABEL_Y);

    // Get day names
    time_t now = time(nullptr);
    struct tm* tm = localtime(&now);

    const char* dayNames[] = {"So", "Mo", "Di", "Mi", "Do", "Fr", "Sa"};

    // Draw 3 days in a vertical layout
    int dayHeight = 80;  // Height per day row
    int startY = FORECAST_WIDGET_Y + 45;  // After label

    for (int day = 0; day < 3; day++) {
        const DailyForecast& df = forecast.days[day];

        int rowY = startY + (day * dayHeight);
        int rowX = FORECAST_WIDGET_X + CARD_PADDING;

        // Day label (Mo, Di, Mi, etc.)
        int dayOfWeek = (tm->tm_wday + day) % 7;
        setRegularFont(display, 28);
        display.setTextDatum(TL_DATUM);
        display.drawString(dayNames[dayOfWeek], rowX, rowY);

        if (!df.valid) {
            display.drawString("--", rowX + 40, rowY);
            continue;
        }

        // Weather icon (32x32)
        const char* icon = getIconFromCode(df.symbolCode);
        drawWeatherIcon(display, rowX + 35, rowY - 4, icon, 32);

        // Temperature range (min/max)
        char tempStr[20];
        snprintf(tempStr, sizeof(tempStr), "%d/%d", df.tempMin, df.tempMax);
        setBoldFont(display, 48);
        display.setTextDatum(TL_DATUM);
        int tempX = rowX + 75;
        display.drawString(tempStr, tempX, rowY);

        // Degree symbol
        int tempWidth = display.textWidth(tempStr);
        display.drawCircle(tempX + tempWidth + 4, rowY + 3, 2, 15);
        display.drawCircle(tempX + tempWidth + 4, rowY + 3, 3, 15);
        setRegularFont(display, 28);
        display.drawString("C", tempX + tempWidth + 9, rowY);

        // Precipitation (if any)
        if (df.precipSum > 0) {
            char precipStr[10];
            snprintf(precipStr, sizeof(precipStr), "%.1f", df.precipSum / 10.0);
            setRegularFont(display, 28);
            display.setTextDatum(TR_DATUM);
            display.drawString("mm", FORECAST_WIDGET_X + CARD_WIDTH - CARD_PADDING, rowY);

            int unitWidth = display.textWidth("mm");
            setBoldFont(display, 48);
            display.drawString(precipStr, FORECAST_WIDGET_X + CARD_WIDTH - CARD_PADDING - unitWidth - 5, rowY);
        }
    }
}

// Helper: Draw 3-time grid (06h, 12h, 18h)
void drawDayTimeGrid(M5EPD_Canvas& display, const DayTimeForecast times[3],
                     int x, int y, int width) {
    int cellWidth = width / 3;

    for (int i = 0; i < 3; i++) {
        int cellX = x + (i * cellWidth);
        const DayTimeForecast& dt = times[i];
        bool hasData = (dt.hour != 0);  // hour == 0 means no data

        setRegularFont(display, 28);
        display.setTextDatum(TC_DATUM);

        if (hasData) {
            // Weather icon (24×24px)
            const char* icon = getIconFromCode(dt.symbolCode);
            drawWeatherIcon(display, cellX + cellWidth/2 - 12, y, icon, 24);

            // Temperature with custom degree symbol (matching other widgets)
            char tempStr[6];
            snprintf(tempStr, sizeof(tempStr), "%d", dt.temperature);
            int tempTextY = y + 27;

            // Draw temperature number in bold
            setBoldFont(display, 48);
            int tempWidth = display.textWidth(tempStr);
            int startX = cellX + cellWidth/2 - (tempWidth + 12) / 2;  // Center: text + spacing + degree + C
            display.setTextDatum(TL_DATUM);
            display.drawString(tempStr, startX, tempTextY);

            // Draw degree symbol (two circles) with spacing
            int degX = startX + tempWidth + 4;  // Increased spacing from 2 to 4
            display.drawCircle(degX, tempTextY + 3, 2, 15);
            display.drawCircle(degX, tempTextY + 3, 3, 15);

            // Draw "C" in non-bold
            setRegularFont(display, 28);
            display.drawString("C", degX + 5, tempTextY);

            // Precipitation with unit (value bold, unit non-bold)
            char precipStr[10];
            snprintf(precipStr, sizeof(precipStr), "%.1f", dt.precipitationMm / 10.0);

            // Calculate centered position for value + space + unit
            setBoldFont(display, 48);
            int precipValueWidth = display.textWidth(precipStr);
            setRegularFont(display, 28);
            int spaceWidth = display.textWidth(" ");
            int precipUnitWidth = display.textWidth("mm");
            int totalWidth = precipValueWidth + spaceWidth + precipUnitWidth;
            int precipStartX = cellX + cellWidth/2 - totalWidth/2;

            // Draw value in bold
            setBoldFont(display, 48);
            display.setTextDatum(TL_DATUM);
            display.drawString(precipStr, precipStartX, y + 42);

            // Draw unit in non-bold with space
            setRegularFont(display, 28);
            display.drawString(" mm", precipStartX + precipValueWidth, y + 42);
        } else {
            // Show placeholder for missing data
            display.drawString("-", cellX + cellWidth/2, y + 15);
        }
    }
}

// Helper: Draw single daily forecast section
void drawDailyForecastSection(M5EPD_Canvas& display, const DailyForecast& day,
                               int x, int y, const char* dayLabel, bool isToday) {
    // NOT USED on M5Paper (no forecast widgets in portrait layout)
    (void)display;
    (void)day;
    (void)x;
    (void)y;
    (void)dayLabel;
    (void)isToday;
}

// Main 3-day forecast column
void draw3DayForecastColumn(M5EPD_Canvas& display, const ForecastData& forecast) {
    // NOT USED on M5Paper (no forecast widgets in portrait layout)
    (void)display;  // Suppress unused parameter warning
    (void)forecast;
}

void drawStatusBar(M5EPD_Canvas& display) {
    // Just draw separator line at bottom
    display.drawFastHLine(0, STATUS_BAR_Y, SCREEN_WIDTH, 15);
}

void drawDashboard(M5EPD_Canvas& display, const DashboardData& data) {
    display.fillCanvas(0);  // White background (M5EPD uses fillCanvas, not fillScreen)

    // Header
    drawHeader(display, LOCATION_NAME, data.updateTime);

    // COLUMN 1: INDOOR SENSORS
    drawIndoorTempWidget(display, data.weather.indoor);
    drawIndoorHumidWidget(display, data.weather.indoor);
    drawAirQualityWidget(display, data.weather.indoor);
    drawPressureWidget(display, data.weather.indoor);
    drawStatusInfo(display);

    // COLUMN 2: OUTDOOR + FORECAST + BATTERY
    drawOutdoorTempWidget(display, data.weather.outdoor);
    drawOutdoorHumidWidget(display, data.weather.outdoor);
    drawForecastWidget(display, data.forecast);
    drawBatteryWidget(display, data.batteryVoltage, data.batteryPercent);

    // Status bar (separator line only)
    drawStatusBar(display);
}

void drawBatteryWidget(M5EPD_Canvas& display, uint32_t voltage, uint8_t percent) {
    display.drawRect(BATTERY_X, BATTERY_Y, CARD_WIDTH, BATTERY_CARD_HEIGHT, 15);

    display.setTextColor(15, 0);
    display.setTextDatum(TL_DATUM);

    // Label
    setRegularFont(display, 28);
    display.drawString("Batterie", BATTERY_X + CARD_PADDING, BATTERY_Y + CARD_LABEL_Y);

    // Battery icon
    int iconX = BATTERY_X + CARD_WIDTH - 80;
    int iconY = BATTERY_Y + CARD_VALUE_Y;
    display.drawRect(iconX, iconY, 60, 24, 15);
    display.fillRect(iconX + 60, iconY + 6, 4, 12, 15);  // Nub

    // Fill based on percentage (grayscale for low battery)
    int fillW = (56) * percent / 100;
    uint8_t fillColor = (percent < 20) ? 12 : 15;  // Darker gray for low
    display.fillRect(iconX + 2, iconY + 2, fillW, 20, fillColor);

    // Percentage text
    char percentStr[8];
    snprintf(percentStr, sizeof(percentStr), "%d%%", percent);
    setBoldFont(display, 48);
    display.drawString(percentStr, BATTERY_X + CARD_PADDING, BATTERY_Y + CARD_VALUE_Y);

    // Voltage
    char voltStr[16];
    snprintf(voltStr, sizeof(voltStr), "%.2fV", voltage / 1000.0);
    setRegularFont(display, 28);
    display.drawString(voltStr, BATTERY_X + CARD_PADDING + 80, BATTERY_Y + CARD_VALUE_Y + 15);
}

void drawStatusInfo(M5EPD_Canvas& display) {
    // Optional: Status/info card in column 1 bottom
    // For now, leave empty (or draw a minimal card)
    display.drawRect(STATUS_INFO_X, STATUS_INFO_Y, CARD_WIDTH, STATUS_INFO_HEIGHT, 15);
}

// Legacy function names for compatibility
void drawIndoorCO2Widget(M5EPD_Canvas& display, const IndoorData& data) {
    drawAirQualityWidget(display, data);
}

void drawIndoorPressureWidget(M5EPD_Canvas& display, const IndoorData& data) {
    drawPressureWidget(display, data);
}
