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

// OLED Display Size (used by display implementations, not by the core)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// BLE Service and Characteristic UUIDs
#define SERVICE_UUID "1cc9b045-a6e9-4bd5-b874-07d4f2d57843"
#define DATA_UUID "d56d0059-ad65-43f3-b971-431d48f89a69"

// OLED Reset Pin (not used)
#define OLED_RESET -1

// Debug macro
#define debugln(x) Serial.println(x)

// Built-in LED polarity (active level).
// Override via build flags if the board's LED is active-high, e.g. -DLED_ON=HIGH.
#ifndef LED_ON
#define LED_ON LOW
#endif
#ifndef LED_OFF
#define LED_OFF HIGH
#endif

// Firmware version
constexpr double version = 0.52;

// Smoothing factor of a temperature value
#define SMOOTHING_FACTOR 5
