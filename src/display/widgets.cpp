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

// Helper to draw trend arrow using MDI bitmap icons
void drawTrendArrow(TFT_eSprite& display, int x, int y, Trend trend) {
    const unsigned char* icon = nullptr;

    switch (trend) {
        case Trend::UP:
            icon = mdi_arrow_top_right;
            break;
        case Trend::DOWN:
            icon = mdi_arrow_bottom_right;
            break;
        case Trend::STABLE:
            icon = mdi_arrow_right;
            break;
        default:
            return;
    }

    if (icon) {
        display.drawBitmap(x, y, icon, ARROW_ICON_WIDTH, ARROW_ICON_HEIGHT, TFT_BLACK);
    }
}

void drawWeatherIcon(TFT_eSprite& display, int x, int y, const char* iconName, int size) {
    // Use MDI bitmap weather icons
    const unsigned char* icon = nullptr;

    if (strcmp(iconName, "sunny") == 0) {
        icon = mdi_weather_sunny;
    }
    else if (strcmp(iconName, "partly_cloudy") == 0) {
        icon = mdi_weather_partly_cloudy;
    }
    else if (strcmp(iconName, "cloudy") == 0) {
        icon = mdi_weather_cloudy;
    }
    else if (strcmp(iconName, "rain") == 0) {
        icon = mdi_weather_rainy;
    }
    else if (strcmp(iconName, "snow") == 0) {
        icon = mdi_weather_snowy;
    }

    if (icon) {
        // Center the 48x48 icon in the requested size area
        int offsetX = (size - WEATHER_ICON_WIDTH) / 2;
        int offsetY = (size - WEATHER_ICON_HEIGHT) / 2;
        display.drawBitmap(x + offsetX, y + offsetY, icon, WEATHER_ICON_WIDTH, WEATHER_ICON_HEIGHT, TFT_BLACK);
    }
}

void drawHeader(TFT_eSprite& display, const char* location, unsigned long timestamp) {
    // Draw header background
    display.fillRect(0, HEADER_Y, SCREEN_WIDTH, HEADER_HEIGHT, TFT_WHITE);
    display.drawFastHLine(0, HEADER_HEIGHT, SCREEN_WIDTH, TFT_BLACK);

    // Location on left
    display.setTextColor(TFT_BLACK, TFT_WHITE);
    display.setTextDatum(TL_DATUM);
    display.drawString(location, MARGIN, HEADER_Y + 8, FONT_MEDIUM);

    // Date/time on right
    if (timestamp > 0) {
        time_t t = timestamp;
        struct tm* tm = localtime(&t);
        char dateStr[32];
        snprintf(dateStr, sizeof(dateStr), "%02d.%02d.%04d %02d:%02d",
                tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900,
                tm->tm_hour, tm->tm_min);
        display.setTextDatum(TR_DATUM);
        display.drawString(dateStr, SCREEN_WIDTH - MARGIN, HEADER_Y + 8, FONT_SMALL);
    }
}

void drawIndoorTempWidget(TFT_eSprite& display, const IndoorData& data) {
    if (!data.valid) return;

    drawCard(display, INDOOR_TEMP_X, INDOOR_TEMP_Y);

    display.setTextColor(TFT_BLACK, TFT_WHITE);

    // Label
    display.setTextDatum(TL_DATUM);
    display.drawString("Innen", INDOOR_TEMP_X + CARD_PADDING, INDOOR_TEMP_Y + CARD_LABEL_Y, FONT_SMALL);

    // Large temperature value
    char tempStr[16];
    snprintf(tempStr, sizeof(tempStr), "%.1f", data.temperature);
    display.setTextDatum(TL_DATUM);
    display.drawString(tempStr, INDOOR_TEMP_X + CARD_PADDING, INDOOR_TEMP_Y + CARD_VALUE_Y, FONT_LARGE);

    // Unit (positioned further right)
    display.drawString("C", INDOOR_TEMP_X + 160, INDOOR_TEMP_Y + CARD_VALUE_Y + 10, FONT_MEDIUM);

    // Trend arrow
    drawTrendArrow(display, INDOOR_TEMP_X + CARD_TREND_X_OFFSET, INDOOR_TEMP_Y + CARD_TREND_Y, data.temperatureTrend);

    // Min/max detail
    if (data.minTemp != 0 || data.maxTemp != 0) {
        char detailStr[64];
        snprintf(detailStr, sizeof(detailStr), "min %.1fC  max %.1fC", data.minTemp, data.maxTemp);
        display.setTextDatum(TL_DATUM);
        display.drawString(detailStr, INDOOR_TEMP_X + CARD_PADDING, INDOOR_TEMP_Y + CARD_DETAIL_Y, FONT_TINY);
    }
}

void drawOutdoorTempWidget(TFT_eSprite& display, const OutdoorData& data) {
    drawCard(display, OUTDOOR_TEMP_X, OUTDOOR_TEMP_Y);

    display.setTextColor(TFT_BLACK, TFT_WHITE);
    display.setTextDatum(TL_DATUM);
    display.drawString("Aussen", OUTDOOR_TEMP_X + CARD_PADDING, OUTDOOR_TEMP_Y + CARD_LABEL_Y, FONT_SMALL);

    if (!data.valid) {
        display.drawString("n/a", OUTDOOR_TEMP_X + CARD_PADDING, OUTDOOR_TEMP_Y + CARD_VALUE_Y, FONT_LARGE);
        return;
    }

    char tempStr[16];
    snprintf(tempStr, sizeof(tempStr), "%.1f", data.temperature);
    display.setTextDatum(TL_DATUM);
    display.drawString(tempStr, OUTDOOR_TEMP_X + CARD_PADDING, OUTDOOR_TEMP_Y + CARD_VALUE_Y, FONT_LARGE);
    display.drawString("C", OUTDOOR_TEMP_X + 160, OUTDOOR_TEMP_Y + CARD_VALUE_Y + 10, FONT_MEDIUM);

    drawTrendArrow(display, OUTDOOR_TEMP_X + CARD_TREND_X_OFFSET, OUTDOOR_TEMP_Y + CARD_TREND_Y, data.temperatureTrend);

    if (data.minTemp != 0 || data.maxTemp != 0) {
        char detailStr[64];
        snprintf(detailStr, sizeof(detailStr), "min %.1fC  max %.1fC", data.minTemp, data.maxTemp);
        display.setTextDatum(TL_DATUM);
        display.drawString(detailStr, OUTDOOR_TEMP_X + CARD_PADDING, OUTDOOR_TEMP_Y + CARD_DETAIL_Y, FONT_TINY);
    }
}

void drawIndoorHumidWidget(TFT_eSprite& display, const IndoorData& data) {
    if (!data.valid) return;

    drawCard(display, INDOOR_HUMID_X, INDOOR_HUMID_Y);

    display.setTextColor(TFT_BLACK, TFT_WHITE);
    display.setTextDatum(TL_DATUM);
    display.drawString("Innen", INDOOR_HUMID_X + CARD_PADDING, INDOOR_HUMID_Y + CARD_LABEL_Y, FONT_SMALL);

    char humidStr[16];
    snprintf(humidStr, sizeof(humidStr), "%d", data.humidity);
    display.setTextDatum(TL_DATUM);
    display.drawString(humidStr, INDOOR_HUMID_X + CARD_PADDING, INDOOR_HUMID_Y + CARD_VALUE_Y, FONT_LARGE);
    display.drawString("%", INDOOR_HUMID_X + 140, INDOOR_HUMID_Y + CARD_VALUE_Y + 10, FONT_MEDIUM);

    // Climate status
    const char* status = getHumidityComfort(data.humidity);
    char detailStr[32];
    snprintf(detailStr, sizeof(detailStr), "Raumklima: %s", status);
    display.setTextDatum(TL_DATUM);
    display.drawString(detailStr, INDOOR_HUMID_X + CARD_PADDING, INDOOR_HUMID_Y + CARD_DETAIL_Y, FONT_TINY);
}

void drawOutdoorHumidWidget(TFT_eSprite& display, const OutdoorData& data) {
    drawCard(display, OUTDOOR_HUMID_X, OUTDOOR_HUMID_Y);

    display.setTextColor(TFT_BLACK, TFT_WHITE);
    display.setTextDatum(TL_DATUM);
    display.drawString("Aussen", OUTDOOR_HUMID_X + CARD_PADDING, OUTDOOR_HUMID_Y + CARD_LABEL_Y, FONT_SMALL);

    if (!data.valid) {
        display.drawString("n/a", OUTDOOR_HUMID_X + CARD_PADDING, OUTDOOR_HUMID_Y + CARD_VALUE_Y, FONT_LARGE);
        return;
    }

    char humidStr[16];
    snprintf(humidStr, sizeof(humidStr), "%d", data.humidity);
    display.setTextDatum(TL_DATUM);
    display.drawString(humidStr, OUTDOOR_HUMID_X + CARD_PADDING, OUTDOOR_HUMID_Y + CARD_VALUE_Y, FONT_LARGE);
    display.drawString("%", OUTDOOR_HUMID_X + 140, OUTDOOR_HUMID_Y + CARD_VALUE_Y + 10, FONT_MEDIUM);

    // Dew point calculation (Magnus formula)
    float a = 17.27;
    float b = 237.7;
    float alpha = ((a * data.temperature) / (b + data.temperature)) + log(data.humidity / 100.0);
    float dewPoint = (b * alpha) / (a - alpha);

    char detailStr[32];
    snprintf(detailStr, sizeof(detailStr), "Taupunkt: %.1fC", dewPoint);
    display.setTextDatum(TL_DATUM);
    display.drawString(detailStr, OUTDOOR_HUMID_X + CARD_PADDING, OUTDOOR_HUMID_Y + CARD_DETAIL_Y, FONT_TINY);
}

void drawAirQualityWidget(TFT_eSprite& display, const IndoorData& data) {
    if (!data.valid) return;

    drawCard(display, AIR_QUALITY_X, AIR_QUALITY_Y);

    display.setTextColor(TFT_BLACK, TFT_WHITE);
    display.setTextDatum(TL_DATUM);
    display.drawString("Luftqualitat innen", AIR_QUALITY_X + CARD_PADDING, AIR_QUALITY_Y + CARD_LABEL_Y, FONT_SMALL);

    char co2Str[16];
    snprintf(co2Str, sizeof(co2Str), "%d", data.co2);
    display.setTextDatum(TL_DATUM);
    display.drawString(co2Str, AIR_QUALITY_X + CARD_PADDING, AIR_QUALITY_Y + CARD_VALUE_Y, FONT_LARGE);
    display.drawString("ppm", AIR_QUALITY_X + 180, AIR_QUALITY_Y + CARD_VALUE_Y + 10, FONT_SMALL);

    drawTrendArrow(display, AIR_QUALITY_X + CARD_TREND_X_OFFSET, AIR_QUALITY_Y + CARD_TREND_Y, data.co2Trend);

    // CO2 quality status
    const char* quality = getCO2Quality(data.co2);
    display.setTextDatum(TL_DATUM);
    display.drawString(quality, AIR_QUALITY_X + CARD_PADDING, AIR_QUALITY_Y + CARD_DETAIL_Y, FONT_TINY);
}

void drawPressureWidget(TFT_eSprite& display, const IndoorData& data) {
    if (!data.valid) return;

    drawCard(display, PRESSURE_X, PRESSURE_Y);

    display.setTextColor(TFT_BLACK, TFT_WHITE);
    display.setTextDatum(TL_DATUM);
    display.drawString("Luftdruck aussen", PRESSURE_X + CARD_PADDING, PRESSURE_Y + CARD_LABEL_Y, FONT_SMALL);

    char pressStr[16];
    snprintf(pressStr, sizeof(pressStr), "%d", data.pressure);
    display.setTextDatum(TL_DATUM);
    display.drawString(pressStr, PRESSURE_X + CARD_PADDING, PRESSURE_Y + CARD_VALUE_Y, FONT_LARGE);
    display.drawString("hPa", PRESSURE_X + 180, PRESSURE_Y + CARD_VALUE_Y + 10, FONT_SMALL);

    drawTrendArrow(display, PRESSURE_X + CARD_TREND_X_OFFSET, PRESSURE_Y + CARD_TREND_Y, data.pressureTrend);

    // Trend label
    const char* trendLabel = (data.pressureTrend == Trend::UP) ? "Trend: Steigend" :
                             (data.pressureTrend == Trend::DOWN) ? "Trend: Fallend" : "Trend: Stabil";
    display.setTextDatum(TL_DATUM);
    display.drawString(trendLabel, PRESSURE_X + CARD_PADDING, PRESSURE_Y + CARD_DETAIL_Y, FONT_TINY);
}

void drawForecast3hWidget(TFT_eSprite& display, const ForecastPoint& forecast) {
    drawCard(display, FORECAST_3H_X, FORECAST_3H_Y);

    display.setTextColor(TFT_BLACK, TFT_WHITE);
    display.setTextDatum(TL_DATUM);
    display.drawString("Vorhersage 3h", FORECAST_3H_X + CARD_PADDING, FORECAST_3H_Y + CARD_LABEL_Y, FONT_SMALL);

    if (!forecast.valid) {
        display.drawString("n/a", FORECAST_3H_X + CARD_PADDING, FORECAST_3H_Y + CARD_VALUE_Y, FONT_LARGE);
        return;
    }

    // Weather icon
    const char* iconName = (forecast.weatherCode >= 61 && forecast.weatherCode <= 67) ? "rain" :
                           (forecast.weatherCode >= 71 && forecast.weatherCode <= 77) ? "snow" :
                           (forecast.weatherCode == 3) ? "cloudy" :
                           (forecast.weatherCode == 2) ? "partly_cloudy" : "sunny";
    drawWeatherIcon(display, FORECAST_3H_X + 10, FORECAST_3H_Y + 25, iconName, 45);

    // Temperature
    char tempStr[16];
    snprintf(tempStr, sizeof(tempStr), "%.0fC", forecast.temperature);
    display.setTextDatum(TL_DATUM);
    display.drawString(tempStr, FORECAST_3H_X + 80, FORECAST_3H_Y + CARD_VALUE_Y + 5, FONT_MEDIUM);

    // Precipitation
    if (forecast.precipitation > 0.1) {
        char precipStr[16];
        snprintf(precipStr, sizeof(precipStr), "%.1fmm", forecast.precipitation);
        display.setTextDatum(TL_DATUM);
        display.drawString(precipStr, FORECAST_3H_X + CARD_PADDING, FORECAST_3H_Y + CARD_DETAIL_Y, FONT_TINY);
    }
}

void drawForecast6hWidget(TFT_eSprite& display, const ForecastPoint& forecast) {
    drawCard(display, FORECAST_6H_X, FORECAST_6H_Y);

    display.setTextColor(TFT_BLACK, TFT_WHITE);
    display.setTextDatum(TL_DATUM);
    display.drawString("Vorhersage 6h", FORECAST_6H_X + CARD_PADDING, FORECAST_6H_Y + CARD_LABEL_Y, FONT_SMALL);

    if (!forecast.valid) {
        display.drawString("n/a", FORECAST_6H_X + CARD_PADDING, FORECAST_6H_Y + CARD_VALUE_Y, FONT_LARGE);
        return;
    }

    const char* iconName = (forecast.weatherCode >= 61 && forecast.weatherCode <= 67) ? "rain" :
                           (forecast.weatherCode >= 71 && forecast.weatherCode <= 77) ? "snow" :
                           (forecast.weatherCode == 3) ? "cloudy" :
                           (forecast.weatherCode == 2) ? "partly_cloudy" : "sunny";
    drawWeatherIcon(display, FORECAST_6H_X + 10, FORECAST_6H_Y + 25, iconName, 45);

    char tempStr[16];
    snprintf(tempStr, sizeof(tempStr), "%.0fC", forecast.temperature);
    display.setTextDatum(TL_DATUM);
    display.drawString(tempStr, FORECAST_6H_X + 80, FORECAST_6H_Y + CARD_VALUE_Y + 5, FONT_MEDIUM);

    if (forecast.precipitation > 0.1) {
        char precipStr[16];
        snprintf(precipStr, sizeof(precipStr), "%.1fmm", forecast.precipitation);
        display.setTextDatum(TL_DATUM);
        display.drawString(precipStr, FORECAST_6H_X + CARD_PADDING, FORECAST_6H_Y + CARD_DETAIL_Y, FONT_TINY);
    }
}

void drawStatusBar(TFT_eSprite& display, uint32_t batteryMv, uint8_t batteryPercent, unsigned long updateTime) {
    display.drawFastHLine(0, STATUS_BAR_Y, SCREEN_WIDTH, TFT_BLACK);

    display.setTextColor(TFT_BLACK, TFT_WHITE);
    display.setTextDatum(TL_DATUM);

    char statusStr[64];
    snprintf(statusStr, sizeof(statusStr), "Bat: %dmV (%d%%)", batteryMv, batteryPercent);
    display.drawString(statusStr, MARGIN, STATUS_BAR_Y + 2, FONT_TINY);
}

void drawDashboard(TFT_eSprite& display, const DashboardData& data) {
    display.fillScreen(TFT_WHITE);

    // Header
    drawHeader(display, LOCATION_NAME, data.updateTime);

    // Row 1: Temperature cards
    drawIndoorTempWidget(display, data.weather.indoor);
    drawOutdoorTempWidget(display, data.weather.outdoor);

    // Row 2: Humidity cards
    drawIndoorHumidWidget(display, data.weather.indoor);
    drawOutdoorHumidWidget(display, data.weather.outdoor);

    // Row 3: Air Quality + Pressure
    drawAirQualityWidget(display, data.weather.indoor);
    drawPressureWidget(display, data.weather.indoor);

    // Row 4: Forecast cards
    drawForecast3hWidget(display, data.forecast.forecast3h);
    drawForecast6hWidget(display, data.forecast.forecast6h);

    // Status bar
    drawStatusBar(display, data.batteryVoltage, data.batteryPercent, data.updateTime);
}

// Legacy function names for compatibility
void drawIndoorCO2Widget(TFT_eSprite& display, const IndoorData& data) {
    drawAirQualityWidget(display, data);
}

void drawIndoorPressureWidget(TFT_eSprite& display, const IndoorData& data) {
    drawPressureWidget(display, data);
}
