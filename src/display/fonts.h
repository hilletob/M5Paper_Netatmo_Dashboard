#ifndef FONTS_H
#define FONTS_H

// TFT_eSPI built-in fonts (available in Seeed_GFX)
// Font numbers correspond to different sizes and styles

// Font 1: 8x8 pixel glyphs (very small, for fine print)
#define FONT_TINY 1

// Font 2: 16x16 pixel glyphs (small, for labels and units)
#define FONT_SMALL 2

// Font 4: 26x26 pixel glyphs (medium, for values and secondary text)
#define FONT_MEDIUM 4

// Font 6: 48x48 pixel glyphs (large, for primary temperature display)
#define FONT_LARGE 6

// Font 7: 7-segment style, 48px high (alternative for temperatures)
#define FONT_SEVEN_SEGMENT 7

// Font 8: 75px high (extra large, if needed for main temperature)
#define FONT_XLARGE 8

// Default font for labels
#define FONT_DEFAULT_LABEL FONT_SMALL

// Default font for values
#define FONT_DEFAULT_VALUE FONT_MEDIUM

// Default font for main temperature display
#define FONT_TEMPERATURE FONT_LARGE

#endif  // FONTS_H
