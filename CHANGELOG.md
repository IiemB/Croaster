# Changelog — Croaster Firmware

All notable changes to the Croaster firmware are documented here.  
Format follows [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

---

## [0.62] — 2026-08-29 (Current)

### Changed
- **`build_all.sh` output naming** — firmware is now collected as
  `builds/Croaster_<board>_<version>.bin` (e.g. `Croaster_esp32s3_0.62.bin`);
  the version is read from `src/CroasterConstants.h` so the filename stays in
  sync with the firmware version
- **Firmware version bumped to `0.62`** — `src/CroasterConstants.h`,
  `library.json`, `implementation/common/library.json`

---

## [0.52] — 2026-08-29

### Added
- **Repository is now a reusable library** (`library.json` at the repo root) — another project can consume it via `lib_deps = https://github.com/IiemB/Croaster.git`
- **`examples/` renamed to `implementation/<board>/`** — per-board implementations (`implementation/esp32c3/`, `implementation/esp8266/`), each consuming the library from `../..` exactly like an external project
- **`implementation/esp32s3/`** — ESP32-S3 (ES3C28P) 2.8" ILI9341 LCD board
  (LVGL 9 UI + FT6336G touch), ported from the `ES3C28P` branch into the
  per-board structure (`CroasterDisplayS3` wrapping the LVGL UI in `src/ui/`;
  `esp32s3-sdkconfig.defaults` for mbedTLS/NimBLE Kconfig fixes)
- **Roast timer** in the library core — `CroasterCore::roastTimerStart()/roastTimerPause()/roastTimerReset()` + `roastTimer` (seconds) & `roastTimerIsRunning()`; surfaced by the S3 UI's single START/STOP/RESET button
- **Board + display-state reporting** — `CroasterDeviceIdentity::boardName()` now returns a per-implementation id set via the `-DCROASTER_BOARD_NAME=` build flag (`esp32s3`/`esp32c3`/`esp8266`/`unknown`, no compile-time board guards); `getDeviceInfo` now includes `board`, `darkMode` and `brightness` (via new `CroasterDisplay::isDarkMode()/getBrightness()`); `CroasterApp::ble()` accessor
- **`build_all.sh`** — builds every board implementation and collects the
  firmware into the gitignored `builds/` folder; board-specific
  `CROASTER_ES3C28P` compile-time guards removed from the implementation
  sources (each board builds its own folder unconditionally); `AGENTS.md` and
  `CONTEXT.md` added for the firmware repo
- **Shared SSD1306 display in `implementation/common/`** — both the `esp32c3` and `esp8266` implementations reference the same display/ animation (no duplicated data); the ESP8266 (NodeMCU / ESP-12E) implementation was restored as `implementation/esp8266/`
- **`CroasterDisplay` abstract interface** — the library core is display-agnostic; consuming projects implement their own display (the SSD1306 implementation lives in the reference example as `CroasterDisplaySSD1306`). A `nullptr` display is fully supported (headless boards)
- **`CroasterPinConfig` struct** — pin layout is decoupled from the core and passed to `CroasterCore`; each implementation provides its own pins in `config.h`
- **`CroasterApp` single `begin()`/`loop()` entry point** — lives in the library (`src/CroasterApp.h/.cpp`); the sketch is reduced to `app.begin()` / `app.loop()`
- **`CroasterApp` is display-agnostic** — it takes `CroasterCore&` + `CroasterDisplay*` (+ LED pin/level); the display type is defined in the implementation (`main.cpp`)
- **Custom command registration** — `CroasterCommandHandler::onCommand()` / `onJsonCommand()` let an implementation add commands without touching the library
- **Compile-time BLE detection** via `CROASTER_HAS_BLE` (1 on ESP32, 0 elsewhere) in `CroasterConstants.h` — `CroasterBleManager` is only compiled when the board has BLE
- `SmoothThermocoupleFix.cpp` added to the library (fixes the MAX6675_Thermocouple v2.0.2 link error for all build types)

### Changed
- All classes renamed with the `Croaster` prefix: `BleManager` → `CroasterBleManager`, `CommandHandler` → `CroasterCommandHandler`, `WebSocketManager` → `CroasterWebSocketManager`, `OtaHandler` → `CroasterOtaHandler`, `WiFiManagerUtil` → `CroasterWiFiManager` (class with static methods), `DeviceIdentity` → `CroasterDeviceIdentity`, `DisplayManager` → `CroasterDisplay` (interface) / `CroasterDisplaySSD1306` (reference impl), `DisplayAnimation` → `CroasterDisplayAnimation`
- `Constants.h` → `CroasterConstants.h`, `PinConfig.h` → `CroasterPinConfig.h`
- `LED_ON`/`LED_OFF` macros removed from `CroasterConstants.h`; `CroasterCommandHandler` now takes `ledPin`/`ledOnLevel` per-instance (supplied by the implementation); removed the redundant `-D LED_ON` build flag
- `CroasterApp` moved from the implementation into the library core (`src/CroasterApp.h/.cpp`); registered in `library.json`
- **Arduino IDE support removed** — PlatformIO is the only workflow; deleted `copy_to_ino.sh`, `library.properties`, `setup_custom_partition_macos.sh` and `references.md`
- Removed `CroasterPinConfig::defaults()` — `CroasterCore` now requires an explicit `CroasterPinConfig` (no library pin defaults)
- **Board-specific docs moved to per-board READMEs** (`implementation/<board>/README.md`); the root `README.md`/`README_ID.md` are now board-agnostic (wiring, partitions, board names live in each implementation's README); removed `lib_extra_dirs` from the implementation `platformio.ini` files
- Pin and dummy-mode configuration moved to the implementation (`implementation/esp32c3/src/config.h`); the `dummyMode` global was removed from `CroasterConstants.h`
- Display constants (`SCREEN_WIDTH`/`SCREEN_HEIGHT`/`OLED_RESET`) moved to `CroasterDisplaySSD1306.h`; LED pin/polarity moved to the implementation's `config.h` (`ledPin`/`ledOnLevel`) — `CroasterConstants.h` no longer defines display/LED config
- `CroasterWiFiManager::restart()/erase()` replace the old `restartESP()/eraseESP()` helpers

---

## [0.51] — 2026-06-01

### Changed
- Refactored OTA handling to return a JSON progress string directly while simplifying internal state management
- Updated `BleManager` and `WebSocketManager` to use the streamlined OTA progress flow
- Simplified command and core OTA integration paths for the new handler contract

### Fixed
- Increased the display inversion duration from 5 seconds to 60 seconds

---

## [0.50] — 2026-05-30

### Added
- **OTA over BLE** (ESP32 only): firmware updates can now be performed via BLE in addition to WebSocket/WiFi
- Timeout checks during BLE OTA to handle stalled transfers

### Changed
- Improved OTA status reporting: `OtaHandler` now returns an `OtaResult` struct containing a JSON progress payload and an error flag
- Simplified `OtaHandler` internals — reduced code size while improving clarity
- `BleManager` updated to relay OTA binary data to `OtaHandler` and forward progress JSON back to the BLE client
- `WebSocketManager` updated to forward OTA progress JSON back to the WebSocket client consistently

---

## [0.46] — 2026-05-28

### Added
- `getExtra` command with corresponding data generation methods (`genRandomString`, random number/boolean)

### Changed
- Refactored command handling and improved JSON response generation
- Simplified command handling by removing `restart` and `erase` parameters from `handleJsonCommand`

### Fixed
- Removed `.DS_Store` files and added to `.gitignore`

---

## [0.45] — 2026-02-28 to 2026-05-13

### Added
- `displayToggle` command to toggle OLED display on/off
- BLE MTU set to 517 for improved throughput
- `genRandomString` function for generating random strings
- Indonesian FAQ (`FAQ_ID.md`) and Indonesian README (`README_ID.md`) translations
- Issue and pull request templates for contribution guidelines

### Changed
- Updated firmware progress display during OTA updates
- Updated library dependencies (`platformio.ini`)
- Set `dummyMode` to `false` (production default)

### Fixed
- Corrected spelling of `genRandomString`
- Updated JSON handling comments
- Updated WiFiManager library version and improved `platformio.ini` formatting

---

## [0.44] — 2025-05-15

### Added
- SSID name included in JSON data response

### Fixed
- Updated firmware version to `0.44`

---

## [0.42 – 0.43] — 2025-05-01 to 2025-05-06

### Added
- Custom partition configuration (`custom32c3sm.csv`) for ESP32C3 Super Mini to support OTA
- OTA update handling via `OtaHandler` class
- `resetState` method in `OtaHandler`; updated `begin` method to call it

### Changed
- Refactored `OtaHandler`: replaced `debugf` with `debugln`, updated `finalize` method
- Refactored `DisplayManager` to handle OLED presence checks and simplify `begin` method
- Streamlined WiFi setup by removing redundant mode setting
- Standardized temperature variable naming across `CroasterCore` and `DisplayManager`
- Updated temperature handling and command processing for improved clarity

### Fixed
- Initialized `Wire` in `isOledPresent` to ensure proper I2C communication
- Updated `dummyMode` to `false` and set WiFi mode in setup

---

## [0.41] — 2025-05-01

### Added
- OTA update handling; firmware version bumped to `0.41`

### Changed
- Removed OTA handling from `BleManager` (moved to dedicated `OtaHandler`)

---

## [0.40] — 2025-05-01

### Changed
- Refactored firmware to version `0.40`
- Added partitions configuration and reference link

---

## [0.31] — 2025-04-25 to 2025-04-28

### Changed
- Updated GPIO pin assignments in README
- Enhanced device identity functions for IP and SSID retrieval
- Improved JSON command handling in `CommandHandler`
- Enhanced sensor reading logic in `CroasterCore`
- Optimized display management in `DisplayManager`
- Updated blink logic and delay in `CommandHandler`
- Added `blinkIndicator` method in `DisplayManager`
- Adjusted `intervalSendData` handling for consistent timing

---

## [0.30 – Refactor Period] — 2025-04-22 to 2025-04-26

### Added
- `DeviceIdentity` class for chip ID, device name, IP address helpers
- Loop functions for `WebSocket` and `WiFiManager` for better code organization
- Screen rotation functionality in `DisplayManager`
- `testdrawline` function in `DisplayManager` for visual debugging
- IP address display toggle in `DisplayManager`
- `CommandHandler` class for centralized JSON command parsing and dispatching (BLE & WebSocket)
- ESP32 and ESP8266 conditional compilation (`#if defined(ESP32)` / `#if defined(ESP8266)`)
- Detailed class and method documentation (Doxygen-style headers)

### Changed
- Integrated `DisplayManager` into command handling and BLE setup
- Refactored BLE setup to use device name and `CommandHandler`
- Simplified BLE and WebSocket handling by integrating `CommandHandler`
- Updated `getDeviceName` and `getShortChipId` functions for improved prefix/suffix handling
- Optimized setup sequence — `Serial` initialization before manager setup
- Modularized command handling and improved temperature reading logic
- Updated JSON command handling to use proper JSON serialization and logging
- Changed `croasterInterval` type to `unsigned long` in `DisplayManager`
- Reordered constructor initialization in `WebSocketManager`
- Removed inheritance from `BLEServerCallbacks` and `BLECharacteristicCallbacks` in `BleManager`

### Fixed
- Changed `croasterInterval` type to `unsigned long` for consistency in `broadcastData`
- Updated git clone URL to use SSH format in README files

---

## [2.48] — 2025-02-24

### Changed
- Updated pin definitions
- Refactored display output and WiFi handling
- Renamed target directory and file for clarity
- Removed unused library

---

## [2.47] — 2025-02-24

### Changed
- Updated pin definitions and refactored code

---

## [2.46] — 2025-02-21

### Changed
- Minor bump; version consistency update

---

## [2.45] — 2025-02-21

### Changed
- Added Croaster Arduino sketch (`.ino`) and updated conversion script
- Updated `copy_to_ino.sh` to generate the Arduino sketch; implemented BLE server functionality
- Added conversion script for `main.cpp` and `Croaster.h`; refactored BLE characteristic handling

---

## [Early Refactor — ESP32 Core] — 2025-02-20 to 2025-02-24

### Added
- `Croaster` class with sensor integration, temperature conversion, and JSON data handling
- LED blinking functionality based on WiFi and BLE connection status
- BLE server functionality (`BleManager`)

### Changed
- Updated serial communication speeds and flash size in `platformio.ini`
- Refactored BLE data handling to use `String` type
- Updated Croaster class to use new MAX6675 library
- Adjusted pin definitions for new hardware
- Enhanced dummy data handling

---

## [Initial Modular Refactor] — 2025-04-19 to 2025-04-21

### Added
- File conversion script (`copy_to_ino.sh`) using Bash for Arduino sketch generation
- Dedicated `src/` directory structure for modular C++ source files

### Changed
- Replaced `TempsManager` with `CroasterCore` across the codebase
- Modularized codebase: `BleManager`, `WiFiManagerUtil`, `WebSocketManager`, `DisplayManager`, `CroasterCore`
- Updated `DisplayManager` methods for improved data handling and display updates
- Replaced Dart file copying logic with a Bash script

---

## [Pre-Modular] — 2021-02-07 to 2025-02-19

### Added
- Initial project commit with ESP32-based BLE coffee roaster monitor (2021-02-07)
- Arduino `.ino` file support
- PlatformIO configuration (`platformio.ini`) for ESP8266 and ESP32
- Basic Croaster class with sensor integration and JSON data output
- README and documentation (English)
- Artisan Roaster Scope connection instructions

### Changed
- Updated project from Arduino-only to PlatformIO build system
- Updated to production dummy data settings
- Progressively enhanced display output, WiFi handling, and BLE connectivity
- Updated README multiple times for wiring diagrams, hardware components, and connection guides

---

*For full commit history, run:*
```bash
git log --oneline
```
