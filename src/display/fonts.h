#ifndef FONTS_H
#define FONTS_H

// Include FreeFont support
#include "Free_Fonts.h"

// =============================================================================
// FREEFONTS (ASCII only - no umlauts due to font limitations)
// =============================================================================
// Usage: display.setFreeFont(FONT_NAME);
// Note: FreeFonts in Seeed_GFX only support ASCII (0x20-0x7E)
// For umlauts, text uses ASCII approximations (ae, oe, ue, ss)

// --- FreeSans (Clean sans-serif, recommended for labels) ---
#define FSS9   &FreeSans9pt7b             // Small labels
#define FSS12  &FreeSans12pt7b            // Medium labels
#define FSS18  &FreeSans18pt7b            // Large labels
#define FSS24  &FreeSans24pt7b            // Extra large labels

#define FSSB9  &FreeSansBold9pt7b         // Small bold
#define FSSB12 &FreeSansBold12pt7b        // Medium bold (recommended for values)
#define FSSB18 &FreeSansBold18pt7b        // Large bold (recommended for temperatures)
#define FSSB24 &FreeSansBold24pt7b        // Extra large bold

// =============================================================================
// RECOMMENDED FONTS FOR WEATHER DASHBOARD
// =============================================================================
#define FONT_DEFAULT_LABEL  FSS9          // Labels
#define FONT_DEFAULT_VALUE  FSSB12        // Values: "23.5°C", "850 ppm"
#define FONT_TEMPERATURE    FSSB18        // Main temperature display
#define FONT_STATUS         FSS9          // Status text
#define FONT_TITLE          FSSB12        // Section titles

#endif  // FONTS_H
