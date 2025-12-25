#ifndef LAYOUT_H
#define LAYOUT_H

// Screen dimensions (800x480 landscape)
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 480

// General layout constants
#define MARGIN 8
#define CARD_SPACING 8
#define COLUMN_GAP 8

// Header section
#define HEADER_HEIGHT 45
#define HEADER_Y 0

// Two-column card grid (like netatmo-dashboard)
#define CARD_WIDTH 392
#define CARD_HEIGHT 95

// Left column positions
#define LEFT_COLUMN_X MARGIN
#define LEFT_COLUMN_WIDTH CARD_WIDTH

// Right column positions
#define RIGHT_COLUMN_X (LEFT_COLUMN_X + CARD_WIDTH + COLUMN_GAP)
#define RIGHT_COLUMN_WIDTH CARD_WIDTH

// Card vertical positions (4 rows of cards)
#define ROW1_Y (HEADER_HEIGHT + MARGIN)
#define ROW2_Y (ROW1_Y + CARD_HEIGHT + CARD_SPACING)
#define ROW3_Y (ROW2_Y + CARD_HEIGHT + CARD_SPACING)
#define ROW4_Y (ROW3_Y + CARD_HEIGHT + CARD_SPACING)

// Card internal layout
#define CARD_PADDING 6
#define CARD_LABEL_Y 8           // Label position from card top
#define CARD_VALUE_Y 28          // Large value position
#define CARD_TREND_X_OFFSET 340  // Trend icon X offset from card left
#define CARD_TREND_Y 30          // Trend icon Y position
#define CARD_DETAIL_Y 75         // Detail text position (min/max, etc)

// Row 1: Temperature cards
#define INDOOR_TEMP_X LEFT_COLUMN_X
#define INDOOR_TEMP_Y ROW1_Y
#define OUTDOOR_TEMP_X RIGHT_COLUMN_X
#define OUTDOOR_TEMP_Y ROW1_Y

// Row 2: Humidity cards
#define INDOOR_HUMID_X LEFT_COLUMN_X
#define INDOOR_HUMID_Y ROW2_Y
#define OUTDOOR_HUMID_X RIGHT_COLUMN_X
#define OUTDOOR_HUMID_Y ROW2_Y

// Row 3: Air Quality (CO2) + Pressure
#define AIR_QUALITY_X LEFT_COLUMN_X
#define AIR_QUALITY_Y ROW3_Y
#define PRESSURE_X RIGHT_COLUMN_X
#define PRESSURE_Y ROW3_Y

// Row 4: Forecast cards
#define FORECAST_3H_X LEFT_COLUMN_X
#define FORECAST_3H_Y ROW4_Y
#define FORECAST_6H_X RIGHT_COLUMN_X
#define FORECAST_6H_Y ROW4_Y

// Status bar at bottom (thin strip)
#define STATUS_BAR_Y 465
#define STATUS_BAR_HEIGHT 15

#endif  // LAYOUT_H
