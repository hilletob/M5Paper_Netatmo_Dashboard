# Repository Guidelines

## Project Structure & Module Organization
- **Firmware:** `src/` with APIs in `src/api`, display widgets/layout in `src/display`, cached data in `src/data`, and power control in `src/power`; entry point `src/main.cpp`.
- **Configuration:** Shared defaults in `src/config.h`; developer secrets in `src/config.local.h` (gitignored). Hardware helpers live in `include/` (see `driver.h`).
- **Docs & assets:** Reference screenshots and notes in `docs/`. PlatformIO metadata in `platformio.ini`; build outputs land in `.pio/` (do not commit).

## Build, Test, and Development Commands
- `pio run`: Compile the M5Paper firmware using the default `m5paper` environment.
- `pio run --target upload`: Flash the board over the configured port.
- `pio device monitor`: Open the serial monitor at `115200` baud for runtime logs.
- `pio run -t clean`: Clear cached build outputs if you hit odd build/link errors.

## Coding Style & Naming Conventions
- **Language:** Arduino/C++ with 4-space indentation and brace-on-same-line style (match `src/main.cpp`).
- **Naming:** Functions camelCase (e.g., `connectWiFi`), classes/types PascalCase, constants/macros ALL_CAPS_SNAKE.
- **Logging:** Prefer `ESP_LOGI/W/E`; reserve `Serial.print` for quick bring-up only. Keep headers light and include only what you need.

## Testing Guidelines
- No automated unit tests are present. Use `pio run` to confirm builds before pushing.
- For networking, caching, or sleep changes, validate manually via `pio device monitor`; check Netatmo/Gemini responses and wake timing in logs.
- If adding tests, place them under `test/` with clear feature-based names (e.g., `test_cache_state`) using PlatformIO’s Unity harness.

## Commit & Pull Request Guidelines
- **Commits:** Imperative, concise subjects under 72 chars (e.g., “Fix timezone on first boot”). Avoid committing secrets.
- **PRs:** Include a short summary of the feature/fix, commands run (`pio run`, monitor steps), and screenshots/photos for UI/layout changes. Link issues when relevant.

## Security & Configuration Tips
- Keep WiFi, Netatmo, and Gemini credentials only in `src/config.local.h`; never log or commit them.
- Regenerate tokens/keys if exposed. Adjust fonts/assets mindful of PSRAM/heap limits on M5Paper.
- Set `UPDATE_INTERVAL_MIN` and timezone strings carefully—wrong values can lead to stale data or missed updates.
