#include "ttf_helpers.h"
#include "liberation_fonts.h"

static LoadedFont currentFont = FONT_REGULAR;

void initTTFFonts(M5EPD_Canvas& canvas) {
    // Load Regular font and create renders for sizes 9, 12, 18
    canvas.loadFont(LiberationSans_Regular_ttf, LiberationSans_Regular_ttf_len);
    canvas.createRender(9, 256);
    canvas.createRender(12, 256);
    canvas.createRender(18, 256);

    // Switch to Bold and create renders
    canvas.unloadFont();
    canvas.loadFont(LiberationSans_Bold_ttf, LiberationSans_Bold_ttf_len);
    canvas.createRender(9, 256);
    canvas.createRender(12, 256);
    canvas.createRender(18, 256);
    currentFont = FONT_BOLD;

    ESP_LOGI("ttf", "Liberation Sans fonts initialized (regular + bold, 9/12/18pt)");
}

void setRegularFont(M5EPD_Canvas& canvas, uint16_t size) {
    if (currentFont != FONT_REGULAR) {
        canvas.unloadFont();
        canvas.loadFont(LiberationSans_Regular_ttf, LiberationSans_Regular_ttf_len);
        // Recreate renders after loading font
        canvas.createRender(9, 256);
        canvas.createRender(12, 256);
        canvas.createRender(18, 256);
        currentFont = FONT_REGULAR;
    }
    canvas.setTextSize(size);
}

void setBoldFont(M5EPD_Canvas& canvas, uint16_t size) {
    if (currentFont != FONT_BOLD) {
        canvas.unloadFont();
        canvas.loadFont(LiberationSans_Bold_ttf, LiberationSans_Bold_ttf_len);
        // Recreate renders after loading font
        canvas.createRender(9, 256);
        canvas.createRender(12, 256);
        canvas.createRender(18, 256);
        currentFont = FONT_BOLD;
    }
    canvas.setTextSize(size);
}
