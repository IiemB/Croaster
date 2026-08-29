#pragma once
#include <Arduino.h>
#include <CroasterPinConfig.h>

// ============================================================================
// ES3C28P (ESP32-S3 2.8" ILI9341 LCD) implementation configuration — edit.
// Thermocouple pins, dummy-mode and LED live HERE (implementation side),
// never in the Croaster library. The LCD/touch pins live in pins.h.
// ============================================================================

// Use dummy sensor data (no real thermocouples required).
const bool dummyMode = false;

// Thermocouple (MAX6675) pin layout for the ES3C28P.
// SCK=2, SO=3, CS_BT=14, CS_ET=21
const CroasterPinConfig pins = {2, 3, 14, 21};

// Built-in LED: pin + active level (used by the "blink" command).
// The ES3C28P board exposes no user LED on a known pin, so it's disabled.
// Set ledPin to a real GPIO (e.g. the onboard WS2812B via NeoPixel) to use it.
const int8_t ledPin = -1;
const uint8_t ledOnLevel = LOW; // irrelevant while ledPin == -1
