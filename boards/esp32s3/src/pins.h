#pragma once

// ============================================================================
// ES3C28P (ESP32-S3 2.8" ILI9341 LCD) display pin layout — edit this file.
// Used by the LVGL UI (LvglUi.cpp) and the FT6336G touch driver (LvglTouch.cpp).
// The thermocouple pins and LED live in config.h.
// ============================================================================

// Adafruit ILI9341 (SPI) — backlight via LEDC PWM on IO45.
#define LCD_CS_PIN 10
#define LCD_DC_PIN 46
#define LCD_SCLK_PIN 12
#define LCD_MOSI_PIN 11
#define LCD_MISO_PIN 13
#define LCD_RST_PIN (-1) // tied to the board's reset
#define LCD_BL_PIN 45

// FT6336G capacitive touch (I2C, address 0x38).
#define TOUCH_SDA_PIN 16
#define TOUCH_SCL_PIN 15
#define TOUCH_RST_PIN 18
#define TOUCH_INT_PIN 17
