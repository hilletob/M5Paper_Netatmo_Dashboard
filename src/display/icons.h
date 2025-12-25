#ifndef ICONS_H
#define ICONS_H

#include <Arduino.h>

// Icon dimensions
#define WEATHER_ICON_WIDTH 48
#define WEATHER_ICON_HEIGHT 48
#define ARROW_ICON_WIDTH 24
#define ARROW_ICON_HEIGHT 24

// Weather icons from Material Design Icons (MDI)
// Format: 1 bit per pixel, MSB first
// Generated from: https://pictogrammers.com/library/mdi/

// weather-sunny (F0599)
extern const unsigned char mdi_weather_sunny[];

// weather-partly-cloudy (F0595)
extern const unsigned char mdi_weather_partly_cloudy[];

// weather-cloudy (F0590)
extern const unsigned char mdi_weather_cloudy[];

// weather-rainy (F0597)
extern const unsigned char mdi_weather_rainy[];

// weather-snowy (F0598)
extern const unsigned char mdi_weather_snowy[];

// Trend arrow icons
// arrow-top-right (F005D)
extern const unsigned char mdi_arrow_top_right[];

// arrow-right (F0054)
extern const unsigned char mdi_arrow_right[];

// arrow-bottom-right (F0045)
extern const unsigned char mdi_arrow_bottom_right[];

#endif // ICONS_H
