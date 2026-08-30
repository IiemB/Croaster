#pragma once
#include <Arduino.h>
#include <CroasterPinConfig.h>

// ============================================================================
// ESP8266 (NodeMCU / ESP-12E) implementation configuration — edit this file.
// Pin, dummy-mode, LED and display configuration lives HERE (the implementation
// side), never in the Croaster library.
// ============================================================================

// Use dummy sensor data (no real thermocouples required).
const bool dummyMode = false;

// Thermocouple (MAX6675) pin layout for the NodeMCU / ESP-12E.
// SCK=D5, SO=D7, CS_BT=D8, CS_ET=D6
const CroasterPinConfig pins = {D5, D7, D8, D6};

// Built-in LED: pin + active level (used by the "blink" command).
// Set ledPin = -1 to disable the LED entirely.
const int8_t ledPin = LED_BUILTIN; // NodeMCU onboard LED is on GPIO2 (D4)
const uint8_t ledOnLevel = LOW;    // LOW = active-low (most boards), HIGH = active-high
