# Croaster — ESP32-S3 Implementation (board: ES3C28P)

This folder is a standalone **PlatformIO** project for the **ES3C28P** board
(ESP32-S3 2.8" ILI9341 LCD). It consumes the Croaster library from
`croaster/` (`../../croaster`) and implements the **LVGL** UI on the built-in
IPS LCD, wired
into the library's display-agnostic `CroasterApp` (`src/main.cpp`).

---

## 📋 Board

- **MCU:** ESP32-S3 (dual-core Xtensa LX7), **WiFi + BLE**
- **Flash:** 16 MB · **PSRAM:** 8 MB (OPI PSRAM with DIO flash)
- **Display:** ILI9341 240×320 SPI TFT — landscape 320×240 (LVGL 9)
- **Touch:** FT6336G capacitive I2C
- **Partition scheme:** `default_16MB.csv`

---

## 🔧 Build & Upload

```bash
cd boards/esp32s3
pio run -e esp32s3 -t upload
```

Or build every board and collect the firmware with `./build_all.sh` (outputs
land in the gitignored `builds/` folder).

> BLE is compiled in automatically — `CroasterBleManager` is only built when the
> board has BLE (`CROASTER_HAS_BLE`).
>
> ESP-IDF Kconfig overrides (`esp32s3-sdkconfig.defaults`) fix
> `MBEDTLS_ERR_SSL_ALLOC_FAILED` on WSS connect and tune the NimBLE stack — see
> the file for full explanations.

---

## 📺 UI

Instrument-style LVGL dashboard (landscape 320×240):

- top bar with brand/IP and WiFi/BLE status chips
- BT/ET temperature cards with RoR
- bottom bar: roast timer + a single **START → STOP → RESET** control button
- swipe left for a rolling BT/ET roast-profile chart, swipe right to return
- double-tap toggles the backlight (LEDC PWM on IO45); light/dark theme

The roast timer lives in the Croaster library core (`CroasterCore`:
`roastTimerStart/Pause/Reset`) and is surfaced by the UI and the
`getDeviceInfo` JSON command. Custom WebSocket/BLE commands registered in
`src/main.cpp` control the backlight (`brightness`) and theme (`darkMode`).

---

## 🔌 Wiring

| Component | GPIO |
|:---|:---:|
| **ILI9341** CS | 10 |
| **ILI9341** DC | 46 |
| **ILI9341** SCK | 12 |
| **ILI9341** MOSI | 11 |
| **ILI9341** MISO | 13 |
| **ILI9341** RST | −1 (tied to board reset) |
| **Backlight** BL | 45 (LEDC PWM) |
| **FT6336G** SDA | 16 |
| **FT6336G** SCL | 15 |
| **FT6336G** RST | 18 |
| **FT6336G** INT | 17 |
| **ET sensor** SCK | 2 |
| **ET sensor** SO | 3 |
| **ET sensor** CS | 21 |
| **BT sensor** SCK | 2 |
| **BT sensor** SO | 3 |
| **BT sensor** CS | 14 |

Both thermocouples share the **SCK** and **SO** lines (SPI bus); they are
distinguished by their individual **CS** pins. All components run on **3.3V**.

---

## ⚙️ Configuration

| File | Purpose |
|:---|:---|
| `src/config.h` | Dummy mode, thermocouple pins (`pins = {2, 3, 14, 21}`), LED |
| `src/pins.h` | LCD + touch pin layout (used by `LvglUi` / `LvglTouch`) |
| `src/ui/` | LVGL UI (`LvglUi.*`), touch (`LvglTouch.*`) and `lv_conf.h` |
| `platformio.ini` | Env `esp32s3`; board id `CroasterDeviceIdentity::setBoardName("esp32s3")` in `src/main.cpp` |
| `esp32s3-sdkconfig.defaults` | ESP-IDF Kconfig overrides (mbedTLS, NimBLE) |

---
