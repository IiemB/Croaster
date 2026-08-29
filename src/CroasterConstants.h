#pragma once

// ============================================================================
// Board capability detection (compile time)
// ----------------------------------------------------------------------------
// The library builds for any board. These macros tell the consuming project
// which optional subsystems the current board supports so it can wire them
// conditionally (e.g. only create a CroasterBleManager when BLE is available).
// ============================================================================

#if defined(ESP32)
#define CROASTER_HAS_BLE 1
#else
#define CROASTER_HAS_BLE 0
#endif

#if defined(ESP32) || defined(ESP8266)
#define CROASTER_HAS_WIFI 1
#else
#define CROASTER_HAS_WIFI 0
#endif

// BLE Service and Characteristic UUIDs
#define SERVICE_UUID "1cc9b045-a6e9-4bd5-b874-07d4f2d57843"
#define DATA_UUID "d56d0059-ad65-43f3-b971-431d48f89a69"

// Debug macro
#define debugln(x) Serial.println(x)

// Firmware version
constexpr double version = 0.62;

// Smoothing factor of a temperature value
#define SMOOTHING_FACTOR 5

// NOTE: display and LED hardware specifics are NOT defined here. The
// implementation owns them: display size/reset (e.g. in CroasterDisplaySSD1306.h)
// and built-in LED pin/polarity (e.g. in the implementation's config.h).
