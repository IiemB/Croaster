#pragma once

// OLED Display Size
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// BLE Service and Characteristic UUIDs
#define SERVICE_UUID "1cc9b045-a6e9-4bd5-b874-07d4f2d57843"
#define DATA_UUID "d56d0059-ad65-43f3-b971-431d48f89a69"

// OLED Reset Pin (not used)
#define OLED_RESET -1

// Debug macro
#define debugln(x) Serial.println(x)

// Firmware version
const double version = 0.44;

// Dummy mode
const bool dummyMode = false;

// Smoothing factor of a temperature value
#define SMOOTHING_FACTOR 5
