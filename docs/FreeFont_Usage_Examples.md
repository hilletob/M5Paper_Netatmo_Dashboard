# FreeFont Usage Examples - German Umlauts Support

## Overview
The ESP32 weather dashboard now supports FreeFonts which include full German umlaut support (ä, ö, ü, Ä, Ö, Ü, ß).

## How to Use FreeFonts

### Old Way (Built-in Fonts - NO umlauts)
```cpp
display.setTextDatum(TL_DATUM);
display.drawString("Aussen", x, y, FONT_SMALL);  // Wrong: "ue" instead of "ü"
```

### New Way (FreeFonts - WITH umlauts)
```cpp
display.setTextDatum(TL_DATUM);
display.setFreeFont(FSS9);  // Use FreeSans 9pt
display.drawString("Außen", x, y);  // Correct: ß is supported!
```

## Key Differences

| Feature | Built-in Fonts | FreeFonts |
|---------|---------------|-----------|
| **Set Font** | `drawString(text, x, y, FONT_NUMBER)` | `setFreeFont(FONT_NAME)` then `drawString(text, x, y)` |
| **Umlauts** | ❌ Not supported | ✅ Fully supported |
| **Font Type** | Fixed bitmap | Vector-based |
| **Memory** | Less memory | More memory |

## Available Font Shortcuts

### Recommended for Weather Dashboard
```cpp
FONT_DEFAULT_LABEL   // FSS9  - Small labels (9pt)
FONT_DEFAULT_VALUE   // FSSB12 - Bold values (12pt)
FONT_TEMPERATURE     // FSSB18 - Large bold temps (18pt)
FONT_STATUS          // FSS9  - Status text (9pt)
FONT_TITLE           // FSSB12 - Section titles (12pt)
```

### All Available Fonts

#### FreeSans (Clean, Modern)
```cpp
FSS9, FSS12, FSS18, FSS24           // Regular
FSSB9, FSSB12, FSSB18, FSSB24       // Bold (recommended for values)
FSSI9, FSSI12, FSSI18, FSSI24       // Italic/Oblique
```

#### FreeSerif (Classic, Elegant)
```cpp
FS9, FS12, FS18, FS24               // Regular
FSB9, FSB12, FSB18, FSB24           // Bold
```

#### FreeMono (Monospace, Technical)
```cpp
FM9, FM12, FM18, FM24               // Regular
FMB9, FMB12, FMB18, FMB24          // Bold
```

## Code Examples

### Example 1: Simple Label with Umlaut
```cpp
// Before (no umlaut support)
display.drawString("Temperatur", x, y, FONT_SMALL);

// After (with umlaut support)
display.setFreeFont(FONT_DEFAULT_LABEL);  // or FSS9
display.drawString("Außentemperatur", x, y);
```

### Example 2: Temperature Display
```cpp
// Large temperature value with FreeFont
display.setTextColor(TFT_BLACK, TFT_WHITE);
display.setTextDatum(TL_DATUM);

// Temperature label with ß
display.setFreeFont(FONT_DEFAULT_LABEL);  // Small
display.drawString("Außen", x, y);

// Temperature value - bold and larger
display.setFreeFont(FONT_TEMPERATURE);  // Large bold
display.drawString("23.5°C", x, y + 30);
```

### Example 3: German Status Messages
```cpp
// CO2 quality indicator with proper German
display.setFreeFont(FONT_STATUS);
display.drawString("Luftqualität: Ausgezeichnet", x, y);

// Humidity comfort with umlauts
display.setFreeFont(FSS9);
display.drawString("Feuchtigkeit: Zu feucht", x, y + 20);

// Pressure trend
display.setFreeFont(FSS9);
display.drawString("Luftdruck: Stabil", x, y + 40);
```

### Example 4: Complete Widget Update
```cpp
void drawOutdoorTempWidget(TFT_eSprite& display, const OutdoorData& data) {
    drawCard(display, x, y);

    display.setTextColor(TFT_BLACK, TFT_WHITE);
    display.setTextDatum(TL_DATUM);

    // Label with proper German ß
    display.setFreeFont(FONT_DEFAULT_LABEL);
    display.drawString("Außen", x + CARD_PADDING, y + CARD_LABEL_Y);

    // Temperature value - larger and bold
    char tempStr[16];
    snprintf(tempStr, sizeof(tempStr), "%.1f°C", data.temperature);
    display.setFreeFont(FONT_TEMPERATURE);
    display.drawString(tempStr, x + CARD_PADDING, y + CARD_VALUE_Y);

    // Min/Max details
    char detailStr[64];
    snprintf(detailStr, sizeof(detailStr), "Min %.1f°  Max %.1f°", data.minTemp, data.maxTemp);
    display.setFreeFont(FSS9);
    display.drawString(detailStr, x + CARD_PADDING, y + CARD_DETAIL_Y);
}
```

### Example 5: Mixing Fonts for Visual Hierarchy
```cpp
// Title: Bold, larger
display.setFreeFont(FONT_TITLE);  // FSSB12
display.drawString("Wetterdaten", 10, 10);

// Labels: Regular, small
display.setFreeFont(FONT_DEFAULT_LABEL);  // FSS9
display.drawString("Außentemperatur:", 10, 40);

// Values: Bold, medium
display.setFreeFont(FONT_DEFAULT_VALUE);  // FSSB12
display.drawString("23.5°C", 150, 40);
```

## German Text Examples with Umlauts

Here are common German weather terms that now work correctly:

```cpp
// Temperature
"Außentemperatur"
"Innentemperatur"
"Höchsttemperatur"
"Tiefsttemperatur"

// Air quality
"Luftqualität"
"Ausgezeichnet"
"Außergewöhnlich"

// Humidity
"Luftfeuchtigkeit"
"Zu feucht"
"Zu trocken"
"Gemäßigt"

// Weather conditions
"Bewölkt"
"Regnerisch"
"Größtenteils sonnig"
"Außerordentlich kalt"

// Status messages
"Aktualisierung läuft..."
"Daten können nicht geladen werden"
"Außenmodul nicht verfügbar"
```

## Important Notes

1. **Memory Usage**: FreeFonts use more memory than built-in fonts. Monitor heap with `ESP.getFreeHeap()`

2. **Font Metrics**: FreeFont text positioning may differ slightly from built-in fonts. You may need to adjust Y coordinates.

3. **Mixing Fonts**: You can use both built-in and FreeFonts in the same sketch:
   ```cpp
   // Built-in font (fast, but no umlauts)
   display.drawString("Basic", x, y, FONT_SMALL);

   // FreeFont (slower, but with umlauts)
   display.setFreeFont(FSS9);
   display.drawString("Außen", x, y + 20);
   ```

4. **Performance**: FreeFonts are slightly slower to render than built-in fonts, but the difference is negligible for our use case.

## Testing Your Fonts

To verify umlaut support works:

```cpp
void testUmlauts() {
    display.fillScreen(TFT_WHITE);
    display.setTextColor(TFT_BLACK, TFT_WHITE);
    display.setTextDatum(TL_DATUM);

    int y = 20;

    // Test all umlauts
    display.setFreeFont(FSSB12);
    display.drawString("Umlauts Test:", 10, y); y += 30;

    display.setFreeFont(FSS12);
    display.drawString("ä ö ü Ä Ö Ü ß", 10, y); y += 30;

    display.drawString("Außentemperatur", 10, y); y += 30;
    display.drawString("Luftqualität", 10, y); y += 30;
    display.drawString("Höchstwert", 10, y); y += 30;

    display.update();
}
```

## Troubleshooting

### Problem: Umlauts show as boxes or garbage
**Solution**: Make sure you're using `setFreeFont()` not `setTextFont(number)`

### Problem: Font looks too small/large
**Solution**: Try different sizes:
- 9pt = Small labels
- 12pt = Normal text
- 18pt = Large values
- 24pt = Extra large headings

### Problem: Text is cut off
**Solution**: FreeFont metrics differ from built-in fonts. Adjust Y position or use `setTextDatum()` for proper alignment.

### Problem: Compilation error "font not found"
**Solution**: Make sure `Free_Fonts.h` is in `src/display/` folder and fonts.h includes it.
