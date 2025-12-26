# ESP32 Weather Dashboard - Claude Context

## Project Overview

Weather dashboard for XIAO ESP32-C3 with 7.5" ePaper display showing Netatmo indoor/outdoor data, met.no weather forecasts, and Gemini AI commentary.

**Hardware**: XIAO ESP32-C3 (320KB RAM, 4MB Flash) + 7.5" ePaper (800×480, 1-bit monochrome)

**Key Libraries**:
- `Seeed_GFX@2.0.3` - ePaper display driver
- `ArduinoJson@7.4.2` - API response parsing
- `HTTPClient` - Netatmo/met.no/Gemini API calls
- `LittleFS` - Persistent cache storage

## Architecture

### Power Management
- **Deep sleep cycle**: 11 minutes between updates
- **Smart scheduling**: Wakes 11 minutes after Netatmo's last update timestamp
- **WiFi management**: Only enabled during data fetch, disabled before display update
- **Watchdog disabled**: ePaper refresh takes 15-20 seconds

### Data Flow
1. Wake from deep sleep
2. Load previous data from cache (for CO2 trend calculation)
3. Connect WiFi (3-5s)
4. Sync NTP time (if needed)
5. Fetch Netatmo data (indoor/outdoor sensors)
6. Fetch met.no forecast (3-day forecast)
7. **Generate Gemini AI commentary** (3-5s)
8. Calculate CO2 trend (compare with cached value)
9. Disconnect WiFi **before display update**
10. Update ePaper display (15-20s)
11. Enter deep sleep

### Critical Implementation Details

#### Display Re-initialization Bug
**Issue**: Seeed_GFX has a bug where `display.update()` fails if `display.begin()` wasn't called immediately before.

**Solution**: Always call `display.begin()` + `display.setRotation(0)` before `display.update()`.

**Location**: `src/main.cpp:293-294`

#### WiFi Interference
**Issue**: WiFi radio interferes with ePaper SPI communication, causing corruption.

**Solution**: Call `disconnectWiFi()` (with full radio power-off) before any display operations.

**Location**: `src/main.cpp:166`

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

**Solution**: Custom degree symbol rendering using concentric circles:
1. Renders numeric value using FreeFonts
2. Measures text width with `textWidth()`
3. Draws degree symbol as two concentric circles (radius 2 and 3)
4. Positions "C" after the symbol

**Implementation**:
- `drawTemperature()` helper: `src/display/widgets.cpp:14-37`
- Inline rendering for min/max: `src/display/widgets.cpp:248-286` (indoor), `310-348` (outdoor)
- Inline rendering for forecast: `src/display/widgets.cpp:583-615`

**Spacing**: 2 pixels before degree symbol, degree circle at y+3 offset

**Used in**:
- Indoor/Outdoor current temperature
- Indoor/Outdoor min/max temperatures
- 3-day forecast temperatures

### Umlaut Replacement

**Problem**: Gemini AI generates German text with umlauts.

**Solution**: Automatic replacement in `gemini_client.cpp:188-195`:
- ä → ae, ö → oe, ü → ue
- Ä → Ae, Ö → Oe, Ü → Ue
- ß → ss

## Display Layout

### Widget System

**Grid**: 3-column layout (left: indoor, middle: outdoor+AI, right: 3-day forecast)

**Card dimensions**:
- Width: 256px
- Height: 95px
- Padding: 6px
- Spacing: 8px

**Layout constants**: `src/display/layout.h`

### Widgets

| Widget | Location | Data Source |
|--------|----------|-------------|
| Indoor Temp | Col 1, Row 1 | Netatmo indoor sensor |
| Indoor Humidity | Col 1, Row 2 | Netatmo indoor sensor |
| Indoor CO2 | Col 1, Row 3 | Netatmo indoor sensor |
| Indoor Pressure | Col 1, Row 4 | Netatmo indoor sensor |
| Outdoor Temp | Col 2, Row 1 | Netatmo outdoor module |
| Outdoor Humidity | Col 2, Row 2 | Netatmo outdoor module |
| **AI Commentary** | Col 2, Rows 3-4 | **Gemini 2.0 Flash-Exp** |
| 3-Day Forecast | Col 3 (full height) | met.no API |

**Removed**: Wind and Rain widgets (replaced by AI widget)

## API Integration

### Netatmo API

**Authentication**: OAuth2 refresh token flow
**Endpoint**: `https://api.netatmo.com/api/getstationsdata`
**Update frequency**: Every 10 minutes
**Data returned**: Indoor/outdoor sensors, trends

**Client**: `src/api/netatmo_client.cpp`

**Trends**:
- Temperature (indoor/outdoor): From Netatmo `temp_trend` field
- Pressure (indoor): From Netatmo `pressure_trend` field
- **CO2 (indoor)**: Calculated by comparing with cached value (10 ppm threshold)

**Trend calculation**: `src/main.cpp:135-151`

### met.no (yr.no) API

**Endpoint**: `https://api.met.no/weatherapi/locationforecast/2.0/compact`
**Authentication**: None (requires User-Agent header with contact info)
**Update frequency**: Hourly
**Data returned**: Temperature, precipitation, weather symbols

**Client**: `src/api/meteo_client.cpp`

**Format**: 3-day daily forecast with min/max temperatures, precipitation, wind, and symbol codes

**HTTP Caching**: Uses `Last-Modified` and `Expires` headers to reduce API calls

### Gemini AI API

**Model**: `gemini-2.0-flash-exp`
**Endpoint**: `https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash-exp:generateContent`
**Authentication**: API key in `x-goog-api-key` header
**Rate limit**: 15 req/min, 1,500 req/day (free tier)

**Client**: `src/api/gemini_client.cpp`

**Prompt Structure**:
1. System instruction: Friendly, dry-humorous German weather assistant
2. Tone & style guidelines: Professional, trocken-humorig, never albern
3. Rules: Max 140 chars, 1 sentence, no emojis, no numbers repetition
4. Context: Unix timestamp, location (Davos)
5. Weather data: Indoor/outdoor temps, humidity, CO2, pressure with trends

**Response processing**:
- Trim whitespace
- Replace umlauts (ä→ae, ö→oe, ü→ue, ß→ss)
- Display in AI widget with word wrapping

**Line height**: 20px (increased from 16px for better readability)

## Data Caching

**Storage**: LittleFS filesystem
**File**: `/weather_cache.json`
**Format**: JSON serialization of `DashboardData`

**Purpose**:
- Offline operation when WiFi/API unavailable
- **CO2 trend calculation** (compare current with cached value)
- Faster startup (show cached data while fetching fresh)

**Cache age**: Displayed in status bar when using cached data

**Implementation**: `src/data/cache.cpp`

**CO2 Trend Logic**:
```cpp
int co2Diff = current_co2 - cached_co2;
if (abs(co2Diff) < 10) → STABLE
else if (co2Diff > 0) → UP
else → DOWN
```

## Recent Changes (2025-12-26)

### Gemini AI Widget Implementation

**Changes**:
1. Replaced Wind and Rain widgets with single AI widget (256×198px)
2. Added `gemini_client.cpp` for AI API integration
3. Added `aiCommentary` field to `DashboardData`
4. Implemented professional German prompt system
5. Added automatic umlaut replacement for ASCII display
6. Increased line height to 20px for better readability

**Files modified**:
- `src/api/gemini_client.{h,cpp}` - NEW
- `src/api/http_utils.h` - Added `httpPostJSON()` for Gemini
- `src/data/weather_data.h` - Added `aiCommentary` field
- `src/config.h` - Added Gemini API constants
- `src/display/widgets.{h,cpp}` - Replaced Wind/Rain with AI widget
- `src/main.cpp` - Integrated Gemini client

### CO2 Trend Calculation

**Implementation**: `src/main.cpp:113-151`

**Logic**:
1. Load previous CO2 value from cache
2. Fetch new CO2 value from Netatmo
3. Calculate difference
4. Apply 10 ppm threshold for STABLE
5. Set trend: UP, DOWN, or STABLE

**Reason**: Netatmo API doesn't provide CO2 trend, only temp/pressure trends

### Custom Degree Symbols Everywhere

**Implementation**: `src/display/widgets.cpp`

**Locations**:
- Current temperatures: `drawTemperature()` helper
- Min/max temperatures: Inline rendering with circles
- Forecast temperatures: Inline rendering with circles

**Reason**: FreeFonts are ASCII-only, no ° character support

## Known Issues & Limitations

### Font Limitations
- **No German umlauts**: FreeFonts are ASCII-only (automatically replaced)
- **No special characters**: €, °, µ require custom drawing or ASCII substitutes
- **U8g2 incompatible**: 1-bit sprite buffers don't work with U8g2 rendering

### Memory Constraints
- **ESP32-C3 RAM**: 320KB total, ~200KB free at runtime
- **JSON buffers**: Sized for typical API responses (~16KB)
- **Font data**: ~40KB Flash for FreeFonts
- **Gemini response**: ~2.2KB additional (prompt + response)

### Display Quirks
- **Refresh time**: 15-20 seconds (normal for ePaper)
- **Re-init required**: Must call `begin()` before every `update()`
- **WiFi interference**: Disable WiFi before display operations

### API Limitations
- **Gemini rate limits**: 15/min, 1500/day (our usage: ~130/day = OK)
- **Gemini response time**: 3-5 seconds per request
- **met.no caching**: Respects HTTP cache headers to reduce load

## Development Tips

### Building & Uploading
```bash
# Build only
pio run

# Build + upload
pio run --target upload

# Monitor serial output
pio device monitor -b 115200
```

### Serial Debugging

Look for these log tags:
- `[main]` - Boot sequence, sleep scheduling, CO2 trend calculation
- `[wifi]` - Connection status, RSSI
- `[netatmo]` / `[meteo]` / `[gemini]` - API calls, parsing
- `[cache]` - Cache save/load operations
- `[display]` - Display updates, timing
- `[sleep]` - Wake count, sleep calculations

### Testing Gemini Integration

**Monitor for**:
- `[gemini] Generating weather commentary`
- `[gemini] Generated: <commentary text>`
- `[gemini] API request failed` (check rate limits)
- `[main] Gemini: <commentary>` (logged before display)

**Common issues**:
- HTTP 429: Rate limit exceeded (wait 1 minute)
- Empty commentary: API error, widget shows border only
- Umlauts in log: Normal, replaced before display

## File Reference

| File | Purpose | Key Contents |
|------|---------|--------------|
| `src/main.cpp` | Main program | Setup, fetch, CO2 trend, display, sleep |
| `src/config.h` | Default config | WiFi, Netatmo, Gemini, met.no constants |
| `src/config.local.h` | User credentials | Gitignored, API keys |
| `src/display/widgets.cpp` | UI rendering | 8 widgets + AI widget + degree symbols |
| `src/display/layout.h` | Layout constants | Coordinates, sizes, spacing |
| `src/display/fonts.h` | Font aliases | FreeFonts definitions |
| `src/api/netatmo_client.cpp` | Netatmo API | OAuth2, station data, trends |
| `src/api/meteo_client.cpp` | met.no API | Forecast, HTTP caching |
| `src/api/gemini_client.cpp` | Gemini AI | Prompt building, umlaut replacement |
| `src/api/http_utils.h` | HTTP helpers | GET, POST JSON |
| `src/data/weather_data.h` | Data structures | Indoor, Outdoor, Forecast, AI |
| `src/data/cache.cpp` | Persistent storage | LittleFS JSON cache |
| `src/power/sleep_manager.cpp` | Sleep/wake | RTC memory, timestamps |

## Future Considerations

### Potential Improvements
- **Font alternatives**: Custom bitmap fonts with umlauts (high effort)
- **Color ePaper**: Requires different Seeed_GFX driver
- **More stations**: Multi-station support with switching
- **Gemini model upgrade**: Switch to newer models as available

### Not Recommended
- **U8g2 migration**: Incompatible with current display driver
- **TFT_eSPI fork**: Seeed_GFX is customized for XIAO ePaper board
- **Smaller update interval**: Netatmo only updates every 10 minutes
- **More API calls**: Stay within free tier limits
