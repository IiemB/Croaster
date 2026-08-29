#pragma once
#include <Arduino.h>
#include <CroasterPinConfig.h>

// ============================================================================
// ESP32-C3 Super Mini implementation configuration — edit this file.
// Pin, dummy-mode, LED and display configuration lives HERE (the implementation
// side), never in the Croaster library.
// ============================================================================

// Use dummy sensor data (no real thermocouples required).
const bool dummyMode = false;

// Thermocouple (MAX6675) pin layout for the ESP32-C3 Super Mini.
// SCK=4, SO=5, CS_BT=7, CS_ET=6
const CroasterPinConfig pins = {4, 5, 7, 6};

// Built-in LED: pin + active level (used by the "blink" command).
// Set ledPin = -1 to disable the LED entirely.
const int8_t ledPin = LED_BUILTIN;
const uint8_t ledOnLevel = LOW; // LOW = active-low (most boards), HIGH = active-high
