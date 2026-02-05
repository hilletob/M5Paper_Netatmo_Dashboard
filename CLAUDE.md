# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
pio run                    # Build firmware
pio run --target upload    # Flash to device
pio device monitor         # Serial console (115200 baud)
pio run -t clean           # Clear build artifacts
```

## Architecture Overview

This is an ESP32 weather dashboard that fetches Netatmo sensor data and met.no forecasts, renders them on an ePaper display, then enters deep sleep. The device runs in single-shot mode (setup() does everything, loop() never executes).

### Execution Flow

```
setup() → Hardware init → WiFi → NTP sync → Fetch APIs → Cache to LittleFS → Render → Deep sleep
```

The device calculates its next wake time based on Netatmo's last update timestamp (11 minutes after) to ensure fresh data.

### Component Responsibilities

| Directory | Purpose |
|-----------|---------|
| `src/api/` | API clients: `netatmo_client` (OAuth2 + weather), `meteo_client` (forecasts), `http_utils.h` (shared HTTP/retry logic) |
| `src/display/` | `widgets.cpp` (card rendering), `layout.h` (coordinates/sizing), `icons.cpp` (weather icons), fonts |
| `src/data/` | `weather_data.h` (all data structures), `cache.cpp` (LittleFS JSON persistence) |
| `src/power/` | `sleep_manager` (deep sleep scheduling), `battery` (voltage → percentage) |
| `src/main.cpp` | Orchestration: WiFi, NTP, API calls, display updates, sleep |

### Data Flow

```
Netatmo API → WeatherData  ─┐
                            ├→ DashboardData → widgets.cpp → ePaper
met.no API  → ForecastData ─┘
                  ↓
              LittleFS cache (fallback if WiFi fails)
```

### Key Data Structures (weather_data.h)

- `IndoorData` / `OutdoorData`: temperature, humidity, trends, min/max
- `WeatherData`: combines indoor + outdoor + wind + rain + timestamp
- `DailyForecast`: min/max temps + 3 time slots (06h, 12h, 18h)
- `DashboardData`: unified struct for rendering (weather + forecast + battery)

### Configuration

- `src/config.h`: defaults (empty credentials, timeouts, NTP servers)
- `src/config.local.h`: user credentials (WiFi, API keys) — gitignored
- Uses `#if __has_include()` pattern for optional local overrides

### Display System

- 540x960 portrait, M5EPD_Canvas
- FreeFonts only (ASCII, no umlauts) — degree symbols custom-drawn
- UPDATE_MODE_GC16 (16-level grayscale, ~2s refresh)
- Hard refresh on every wake to prevent ghosting

### Power Strategy

- Wake count persisted in LittleFS (detects first boot vs subsequent)
- Sleep duration: 1-15 minutes, calculated from Netatmo update time
- Watchdog disabled during ePaper rendering (can take >15s)

## Coding Style

- Arduino C++, 4-space indentation, braces on same line
- Functions: `camelCase`, Types: `PascalCase`, Constants: `ALL_CAPS_SNAKE`
- Logging: `ESP_LOGI/W/E` preferred over `Serial.print`

## Serial Log Prefixes

`[wifi]` `[netatmo]` `[meteo]` `[cache]` `[display]` `[sleep]` `[battery]`

## Key Files for Common Changes

- **UI layout**: `src/display/layout.h` (coordinates), `src/display/widgets.cpp` (rendering)
- **API changes**: `src/api/netatmo_client.cpp`, `src/api/meteo_client.cpp`
- **Data structures**: `src/data/weather_data.h`
- **Flow control**: `src/main.cpp`
- **Sleep timing**: `src/power/sleep_manager.cpp`
