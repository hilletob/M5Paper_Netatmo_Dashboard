#ifndef LAYOUT_H
#define LAYOUT_H

// Screen dimensions (540×960 portrait) - M5Paper
#define SCREEN_WIDTH 540
#define SCREEN_HEIGHT 960

// General layout constants - BALANCED
#define MARGIN 6               // Moderate margins
#define CARD_SPACING 6         // Moderate spacing between cards
#define COLUMN_GAP 6           // Moderate gap between columns

// Header section
#define HEADER_HEIGHT 55       // Header height
#define HEADER_Y 0

// Two-column card grid
#define CARD_WIDTH 261         // Card width
#define FULL_CARD_WIDTH 528    // Full-width card (540 - 2*MARGIN)

// Column positions
#define COL1_X MARGIN                              // 6
#define COL2_X (COL1_X + CARD_WIDTH + COLUMN_GAP)  // 273
// Math check: 6 + 261 + 6 + 261 + 6 = 540 ✓

// Card heights - BALANCED for readability
#define TEMP_CARD_HEIGHT 200        // Temperature cards – more vertical room for min/max
#define HUMID_CARD_HEIGHT 130       // Humidity cards
#define AIR_QUALITY_CARD_HEIGHT 100 // CO2 card (inline "ppm" saves vertical space)
#define PRESSURE_CARD_HEIGHT 100    // Pressure card (inline "hPa")
#define FORECAST_CARD_HEIGHT 280    // 3-day forecast
#define BATTERY_CARD_HEIGHT 55      // Battery card (slim bar)

// Row positions (calculated)
#define ROW1_Y (HEADER_HEIGHT + MARGIN)                             // 61
#define ROW2_Y (ROW1_Y + TEMP_CARD_HEIGHT + CARD_SPACING)           // 267
#define ROW3_Y (ROW2_Y + HUMID_CARD_HEIGHT + CARD_SPACING)          // 403
#define ROW4_Y (ROW3_Y + AIR_QUALITY_CARD_HEIGHT + CARD_SPACING)    // 509
#define ROW5_Y (ROW4_Y + FORECAST_CARD_HEIGHT + CARD_SPACING)       // 795

// Card internal layout - BALANCED spacing
#define CARD_PADDING 8           // Moderate padding
#define CARD_LABEL_Y 6           // Label position from card top
#define CARD_VALUE_Y 36          // Value position
#define CARD_TREND_X_OFFSET 210  // Trend icon X offset
#define CARD_TREND_Y 40          // Trend icon Y position
#define CARD_DETAIL_Y 96         // Detail text position (higher to fit larger cards)

// ============================================================================
// COLUMN 1: INDOOR SENSORS (Netatmo Base Station)
// ============================================================================
#define INDOOR_TEMP_X COL1_X
#define INDOOR_TEMP_Y ROW1_Y

#define INDOOR_HUMID_X COL1_X
#define INDOOR_HUMID_Y ROW2_Y

#define AIR_QUALITY_X COL1_X
#define AIR_QUALITY_Y ROW3_Y

// ============================================================================
// COLUMN 2: OUTDOOR SENSORS
// ============================================================================
#define OUTDOOR_TEMP_X COL2_X
#define OUTDOOR_TEMP_Y ROW1_Y

#define OUTDOOR_HUMID_X COL2_X
#define OUTDOOR_HUMID_Y ROW2_Y

#define PRESSURE_X COL2_X
#define PRESSURE_Y ROW3_Y

// ============================================================================
// FULL-WIDTH: FORECAST + BATTERY
// ============================================================================
#define FORECAST_WIDGET_X MARGIN
#define FORECAST_WIDGET_Y ROW4_Y

#define BATTERY_X MARGIN
#define BATTERY_Y ROW5_Y

// Forecast internal layout constants
#define FORECAST_DAY_COL_X 8         // Day name column offset
#define FORECAST_MORN_ICON_X 50      // Morning icon column offset
#define FORECAST_MORN_TEMP_X 92      // Morning temp column offset
#define FORECAST_AFT_ICON_X 180      // Afternoon icon column offset
#define FORECAST_AFT_TEMP_X 222      // Afternoon temp column offset
#define FORECAST_PRECIP_X 330        // Precipitation column offset
#define FORECAST_RANGE_X 430         // Temp range column offset

#endif  // LAYOUT_H
