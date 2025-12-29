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

// Column positions
#define COL1_X MARGIN                              // 6
#define COL2_X (COL1_X + CARD_WIDTH + COLUMN_GAP)  // 273
// Math check: 6 + 261 + 6 + 261 + 6 = 540 ✓

// Card heights - BALANCED for readability
#define TEMP_CARD_HEIGHT 150       // Temperature cards (original: 130)
#define HUMID_CARD_HEIGHT 130      // Humidity cards (original: 100)
#define AIR_QUALITY_CARD_HEIGHT 140 // CO2 card (original: 90)
#define PRESSURE_CARD_HEIGHT 140    // Pressure card (original: 90)
#define AI_CARD_HEIGHT 288         // AI commentary - spans rows 3+4 (original: 310)
#define BATTERY_CARD_HEIGHT 90     // Battery card (original: 60)
#define STATUS_INFO_HEIGHT 90      // Status/info card (original: 60)

// Row positions (calculated)
#define ROW1_Y (HEADER_HEIGHT + MARGIN)                             // 58
#define ROW2_Y (ROW1_Y + TEMP_CARD_HEIGHT + CARD_SPACING)           // 196
#define ROW3_Y (ROW2_Y + HUMID_CARD_HEIGHT + CARD_SPACING)          // 304
#define ROW4_Y (ROW3_Y + AIR_QUALITY_CARD_HEIGHT + CARD_SPACING)    // 402
#define ROW5_Y (ROW4_Y + PRESSURE_CARD_HEIGHT + CARD_SPACING)       // 500

// Card internal layout - BALANCED spacing
#define CARD_PADDING 8           // Moderate padding
#define CARD_LABEL_Y 10          // Label position from card top
#define CARD_VALUE_Y 36          // Value position
#define CARD_TREND_X_OFFSET 210  // Trend icon X offset
#define CARD_TREND_Y 40          // Trend icon Y position
#define CARD_DETAIL_Y 95         // Detail text position

// ============================================================================
// COLUMN 1: INDOOR SENSORS (Netatmo Base Station)
// ============================================================================
#define INDOOR_TEMP_X COL1_X
#define INDOOR_TEMP_Y ROW1_Y

#define INDOOR_HUMID_X COL1_X
#define INDOOR_HUMID_Y ROW2_Y

#define AIR_QUALITY_X COL1_X
#define AIR_QUALITY_Y ROW3_Y

#define PRESSURE_X COL1_X
#define PRESSURE_Y ROW4_Y

#define STATUS_INFO_X COL1_X
#define STATUS_INFO_Y ROW5_Y

// ============================================================================
// COLUMN 2: OUTDOOR SENSORS + AI + BATTERY
// ============================================================================
#define OUTDOOR_TEMP_X COL2_X
#define OUTDOOR_TEMP_Y ROW1_Y

#define OUTDOOR_HUMID_X COL2_X
#define OUTDOOR_HUMID_Y ROW2_Y

#define AI_WIDGET_X COL2_X
#define AI_WIDGET_Y ROW3_Y

#define BATTERY_X COL2_X
#define BATTERY_Y ROW5_Y

// Status bar at bottom
#define STATUS_BAR_Y (ROW5_Y + BATTERY_CARD_HEIGHT + CARD_SPACING)  // ~568
#define STATUS_BAR_HEIGHT 15

#endif  // LAYOUT_H
