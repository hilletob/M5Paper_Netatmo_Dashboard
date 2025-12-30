#ifndef TTF_HELPERS_H
#define TTF_HELPERS_H

#include <M5EPD.h>

// Font types
enum LoadedFont {
    FONT_REGULAR,
    FONT_BOLD
};

// Initialize TTF fonts (call once in setup)
void initTTFFonts(M5EPD_Canvas& canvas);

// Font switching helpers
void setRegularFont(M5EPD_Canvas& canvas, uint16_t size);
void setBoldFont(M5EPD_Canvas& canvas, uint16_t size);

#endif  // TTF_HELPERS_H
