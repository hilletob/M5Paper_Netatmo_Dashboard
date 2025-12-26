# ESP32 Weather Dashboard - Claude Context

## Project Overview

Weather dashboard for XIAO ESP32-C3 with 7.5" ePaper display showing Netatmo indoor/outdoor data and MeteoSwiss forecasts.

**Hardware**: XIAO ESP32-C3 (320KB RAM, 4MB Flash) + 7.5" ePaper (800×480, 1-bit monochrome)

**Key Libraries**:
- `Seeed_GFX@2.0.3` - ePaper display driver
- `ArduinoJson@7.4.2` - API response parsing
- `HTTPClient` - Netatmo/MeteoSwiss API calls
- `LittleFS` - Persistent cache storage

## Architecture

### Power Management
- **Deep sleep cycle**: 11 minutes between updates
- **Smart scheduling**: Wakes 11 minutes after Netatmo's last update timestamp
- **WiFi management**: Only enabled during data fetch, disabled before display update
- **Watchdog disabled**: ePaper refresh takes 15-20 seconds

### Data Flow
1. Wake from deep sleep
2. Connect WiFi (3-5s)
3. Sync NTP time (if needed)
4. Fetch Netatmo data (indoor/outdoor sensors)
5. Fetch MeteoSwiss forecast (met.no API)
6. Disconnect WiFi **before display update**
7. Update ePaper display (15-20s)
8. Enter deep sleep

### Critical Implementation Details

#### Display Re-initialization Bug
**Issue**: Seeed_GFX has a bug where `display.update()` fails if `display.begin()` wasn't called immediately before.

**Solution**: Always call `display.begin()` + `display.setRotation(0)` before `display.update()`.

**Location**: `src/main.cpp:277-278`

#### WiFi Interference
**Issue**: WiFi radio interferes with ePaper SPI communication, causing corruption.

**Solution**: Call `disconnectWiFi()` (with full radio power-off) before any display operations.

**Location**: `src/main.cpp:139`

## Font System

### FreeFonts (Current Implementation)

**Library**: FreeFonts from Seeed_GFX
**Character support**: ASCII only (0x20-0x7E)
**Limitation**: No German umlauts (ä, ö, ü), no degree symbol (°)

**Fonts in use**:
- `FSS9`, `FSS12`, `FSS18`, `FSS24` - FreeSans (labels)
- `FSSB9`, `FSSB12`, `FSSB18`, `FSSB24` - FreeSansBold (values, titles)

**Defined in**: `src/display/fonts.h`

### Custom Degree Symbol Solution

**Problem**: FreeFonts don't support the ° character (U+00B0).

**Solution**: `drawTemperature()` helper function that:
1. Renders numeric value using FreeFonts
2. Measures text width with `textWidth()`
3. Draws degree symbol as two concentric circles (radius 2 and 3)
4. Positions "C" after the symbol

**Implementation**: `src/display/widgets.cpp:14-37`

**Spacing**: 6 pixels between number and degree symbol

**Used by**:
- Indoor temperature widget (line 240)
- Outdoor temperature widget (line 272)

### U8g2 Migration Attempt (Failed)

**Goal**: Enable UTF-8 support for German umlauts.

**Approaches tried**:
1. **U8g2_FOR_ADAFRUIT_GFX**: Adapter library incompatible with 1-bit sprite buffers (no text rendered)
2. **Native U8g2**: Custom tile callbacks had coordinate space mismatches (partial text rendering)

**Decision**: Reverted to FreeFonts. German text uses ASCII approximations:
- "Außen" → "Aussen"
- "Luftqualität" → "Luftqualitaet"
- "Böen" → "Boen"
- "ÜBERMORGEN" → "UEBERMORGEN"

**Commit**: Reverted in commit `10e3036`

## Display Layout

### Widget System

**Grid**: 2×4 layout (400px indoor section + 400px outdoor/forecast section)

**Card dimensions**:
- Width: 380px
- Height: 100px
- Padding: 10px

**Layout constants**: `src/display/layout.h`

### Widgets

| Widget | Location | Data Source |
|--------|----------|-------------|
| Indoor Temp | Top-left | Netatmo indoor sensor |
| Indoor Humidity | 2nd-left | Netatmo indoor sensor |
| Air Quality (CO2) | 3rd-left | Netatmo indoor sensor |
| Pressure | Bottom-left | Netatmo indoor sensor |
| Outdoor Temp | Top-right | Netatmo outdoor module |
| Outdoor Humidity | 2nd-right | Netatmo outdoor module |
| Wind | 3rd-right | Netatmo wind gauge |
| Rain | Bottom-right | Netatmo rain gauge |
| Forecast Grid | Right section | met.no API (3 daily slots) |

### Text Rendering

**Old approach** (before latest update):
```cpp
display.setFreeFont(FSSB18);
display.drawString("18.3", x, y);
display.setFreeFont(FSS12);
display.drawString("°C", x + 160, y + 10);  // Separate, spaced out
```

**Current approach** (combined units):
```cpp
// For non-temperature values
snprintf(str, sizeof(str), "%d%%", humidity);  // "65%"
display.setFreeFont(FSSB18);
display.drawString(str, x, y);

// For temperature
drawTemperature(display, temp, x, y, FSSB18, FSS12, TL_DATUM);  // "18.3 °C"
```

## API Integration

### Netatmo API

**Authentication**: OAuth2 refresh token flow
**Endpoint**: `https://api.netatmo.com/api/getstationsdata`
**Update frequency**: Every 10 minutes
**Data returned**: Indoor/outdoor sensors, modules (wind, rain)

**Client**: `src/api/netatmo_client.cpp`

**Key fields**:
- `time_utc` - Last update timestamp (used for smart scheduling)
- `dashboard_data` - Current readings
- `co2_calibrating` - CO2 sensor status

### MeteoSwiss (met.no API)

**Endpoint**: `https://api.met.no/weatherapi/locationforecast/2.0/compact`
**Authentication**: None (requires User-Agent header)
**Update frequency**: Hourly
**Data returned**: Temperature, precipitation, weather symbols

**Client**: `src/api/meteo_client.cpp`

**Time slots**: 3 daily forecasts (morning, afternoon, evening) at 06:00, 12:00, 18:00

## Data Caching

**Storage**: LittleFS filesystem
**File**: `/cache.json`
**Format**: JSON serialization of `DashboardData`

**Purpose**:
- Offline operation when WiFi/API unavailable
- Faster startup (show cached data while fetching fresh)

**Cache age**: Displayed in status bar when using cached data

**Implementation**: `src/data/cache.cpp`

## Recent Changes (Latest)

### Custom Degree Symbol (2025-12-26)

**Commit**: `2e0fae5` - "Draw custom degree symbol and combine units with values"

**Changes**:
1. Added `drawTemperature()` helper to draw ° as circle
2. Increased spacing to 6px before degree symbol
3. Combined all units with values (%, ppm, hPa, km/h, mm/24h)
4. Fixed forecast temps to show °C for both min/max

**Reason**: FreeFonts don't support ° symbol (ASCII limitation)

## Known Issues & Limitations

### Font Limitations
- **No German umlauts**: FreeFonts are ASCII-only
- **No special characters**: €, °, µ require custom drawing or ASCII substitutes
- **U8g2 incompatible**: 1-bit sprite buffers don't work with U8g2 rendering

### Memory Constraints
- **ESP32-C3 RAM**: 320KB total, ~200KB free at runtime
- **JSON buffers**: Sized for typical API responses (~16KB)
- **Font data**: ~40KB Flash for FreeFonts

### Display Quirks
- **Refresh time**: 15-20 seconds (normal for ePaper)
- **Re-init required**: Must call `begin()` before every `update()`
- **WiFi interference**: Disable WiFi before display operations

## Development Tips

### Building & Uploading
```bash
# Build only
pio run

# Build + upload
pio run --target upload --upload-port /dev/cu.usbmodem101

# Monitor serial output
pio device monitor -b 115200
```

### Serial Debugging

Look for these log tags:
- `[main]` - Boot sequence, sleep scheduling
- `[wifi]` - Connection status, RSSI
- `[netatmo]` / `[meteo]` - API calls, parsing
- `[cache]` - Cache save/load operations
- `[display]` - Display updates, timing
- `[sleep]` - Wake count, sleep calculations
- `[battery]` - Voltage and percentage readings

### Testing Without Hardware

**Challenge**: No ePaper simulator exists for Seeed_GFX.

**Approach**:
1. Test API clients separately (print JSON to serial)
2. Verify widget rendering logic with debug prints
3. Use cached data for offline testing

## File Reference

| File | Purpose | Key Contents |
|------|---------|--------------|
| `src/main.cpp` | Main program | Setup, fetch, display, sleep cycle |
| `src/config.h` | Default config | WiFi, Netatmo, constants |
| `src/config.local.h` | User credentials | Gitignored, user-specific settings |
| `src/display/widgets.cpp` | UI rendering | 10 widgets + drawTemperature() helper |
| `src/display/layout.h` | Layout constants | Coordinates, sizes, spacing |
| `src/display/fonts.h` | Font aliases | FreeFonts definitions |
| `src/api/netatmo_client.cpp` | Netatmo API | OAuth2, station data parsing |
| `src/api/meteo_client.cpp` | met.no API | Forecast fetching |
| `src/data/weather_data.h` | Data structures | IndoorData, OutdoorData, ForecastData |
| `src/data/cache.cpp` | Persistent storage | LittleFS JSON cache |
| `src/power/sleep_manager.cpp` | Sleep/wake | RTC memory, wake count, timestamps |
| `src/power/battery.cpp` | Battery monitor | ADC reading, voltage divider |

## Future Considerations

### Potential Improvements
- **Font alternatives**: Custom bitmap fonts with umlauts (high effort)
- **Color ePaper**: Requires different Seeed_GFX driver
- **More stations**: Multi-station support with switching
- **Touch input**: 7.5" display has no touch, would need external buttons

### Not Recommended
- **U8g2 migration**: Incompatible with current display driver
- **TFT_eSPI fork**: Seeed_GFX is customized for XIAO ePaper board
- **Smaller update interval**: Netatmo only updates every 10 minutes
