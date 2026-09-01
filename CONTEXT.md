# CONTEXT.md — Croaster firmware (current state)

## What this repo is
The **Croaster** coffee-roaster firmware, structured as a reusable
display/pin-agnostic library (`croaster/`) plus standalone per-board PlatformIO
projects (`boards/<board>/`). It is a git submodule of the ICRM
Flutter app (remote `git@github.com:IiemB/Croaster.git`) and is consumed via
`lib_deps = https://github.com/IiemB/Croaster.git`.

## Repo layout
| Path | Purpose |
|:---|:---|
| `croaster/src/` | Library core: `CroasterCore`, `CroasterDisplay` (interface), `CroasterApp` (begin()/loop()), `CroasterCommandHandler`, `CroasterBleManager`, `CroasterWebSocketManager`, `CroasterOtaHandler`, `CroasterWiFiManager`, `CroasterDeviceIdentity`, `CroasterPinConfig`, `CroasterConstants` |
| `boards/common/` | Shared SSD1306 display + animation (small lib, added via `../common`) |
| `boards/esp32c3/` | ESP32-C3 Super Mini — SSD1306 OLED; custom OTA partition (`custom32c3sm.csv` lives in this folder) |
| `boards/esp8266/` | NodeMCU / ESP-12E — SSD1306 OLED |
| `boards/esp32s3/` | ES3C28P 2.8" ILI9341 LCD — LVGL 9 UI + FT6336G touch (`src/ui/`) |
| `builds/` | gitignored firmware output of `./build_all.sh` |

## Boards
| Board | MCU | Display | Env | Flash (build_all) |
|:---|:---|:---|:---|:---|
| `esp32c3` | ESP32-C3 | 128×64 SSD1306 | `esp32c3` | ~85.7% |
| `esp8266` | ESP8266 | 128×64 SSD1306 | `esp8266` | ~39.8% |
| `esp32s3` (ES3C28P) | ESP32-S3 | 320×240 ILI9341 + LVGL 9 | `esp32s3` | ~29.0% |

## How a board is wired
- `main.cpp` creates `CroasterCore core(dummyMode, pins)`, a display subclass
  (e.g. `CroasterDisplaySSD1306` / `CroasterDisplayS3`), and
  `CroasterApp app(core, &display, ledPin, ledOnLevel)`, then calls
  `app.begin()` / `app.loop()`.
- Board id is hardcoded per board via
  `CroasterDeviceIdentity::setBoardName("esp32s3")` in its `main.cpp`, read
  by `CroasterDeviceIdentity::boardName()` (no compile-time board guards;
  defaults to `unknown` until set).
- Builds: `build_all.sh` builds only the **release** env of each board
  (`build_type = release`; `pio run -e <board>`); `./upload_monitor.sh <board>`
  builds + uploads the **debug** variant (`<board>-debug`, `extends` the
  release env) to the connected board, then opens the serial monitor.
- Library features added in 0.52: roast timer (`CroasterCore::roastTimerStart/
  Pause/Reset`), `getDeviceInfo` now reports `board`/`darkMode`/`brightness`
  (display state via `CroasterDisplay::isDarkMode()/getBrightness()`), and a
  BLE-guarded `CroasterApp::ble()` accessor.

## Git state (2026-08-30)
- Branch `dev` (HEAD `3f52109`, tracks `origin/dev`); the `boards/` + `croaster/`
  restructure, per-board `setBoardName()`, and the 0.62 bump are committed
  (merged from `decoupling` via PR #31). Working tree **clean** after this batch.
- The ESP32-S3 LVGL implementation was ported from the old single-project
  `ES3C28P` branch into `boards/esp32s3/`
  (`git show ES3C28P:<path>` is the source of the UI).
- `build_all.sh` emits `builds/Croaster_<board>_<version>.bin` (version read
  from `croaster/src/CroasterConstants.h`).
- This batch adds `format.sh` + `.clang-format` (all sources reformatted) and
  the esp32s3 `darkMode` / `brightness` custom commands.
 - Recent esp32s3 UI changes (2026-09-01): added an `About` page (version,
   board ID, short chip ID), `darkMode` switch and `brightness` slider in the
   About page, vertical-swipe navigation for the chart (vertical stack), and
   horizontal-swipe toggling between Main <-> About. LVGL animations were
   shortened for snappier transitions; `LvglTouch` now exposes a vertical
   swipe callback. Built firmware: `builds/Croaster_esp32s3_0.62.bin`.
- ⚠️ **Downstream:** the ICRM app consumes this repo via
  `lib_deps = https://github.com/IiemB/Croaster.git`, which needs `library.json`
  at the repo root — update it to the `croaster/` subfolder (or add a root
  `library.json`).

## Next steps
- Update the ICRM app's `lib_deps` for the `croaster/` subfolder.
- Flash + verify the esp32s3 build on real hardware (touch, chart swipe,
 - Flash + verify the esp32s3 build on real hardware (touch, chart vertical
   swipe, horizontal About toggle, dark mode, brightness, OTA over BLE/WebSocket).
- Confirm `boardName()` strings against the ICRM app's `CroasterBoardTypes`.
 - Consider persisting `darkMode`/`brightness` to NVS so settings survive reboots.

## Gotchas
- PlatformIO not on PATH → `~/.platformio/penv/bin/pio` or `PIO=`.
- Libs are **live-linked** via `symlink://` lib_deps
  (`Croaster=symlink://../../croaster`, `CroasterDisplaySSD1306=symlink://../common`)
  — editing `croaster/src/` needs no `.pio/libdeps` cleanup.
- ESP32-C3: WiFiManager AP can fail to broadcast under the PIO core (2.0.17) —
  fixed via DIO flash mode + `WiFi.setTxPower(WIFI_POWER_18_5dBm)` in the
  configMode callback (see `boards/esp32c3/` notes).
- ESP32-S3 (LVGL): heavy UI work must run on its FreeRTOS task (Core 0), never
  the Arduino loop task; the theme switch is deferred via
  `themeRebuildPending` + `LvglUi::rebuildTheme()` on the LVGL task.
- ESP32-S3: WSS connect needs the mbedTLS PSRAM/buffer Kconfig overrides in
  `boards/esp32s3/esp32s3-sdkconfig.defaults` (otherwise
  `MBEDTLS_ERR_SSL_ALLOC_FAILED`).
