#pragma once
#include <CroasterPinConfig.h>

// ============================================================================
// Implementation configuration — edit this file for your board.
// Pin, display and dummy-mode configuration lives HERE (the implementation
// side), never in the Croaster library.
// ============================================================================

// Use dummy sensor data (no real thermocouples required).
const bool dummyMode = false;

// Thermocouple (MAX6675) pin layout for this board.
#if defined(ESP8266)
// NodeMCU / ESP-12E: SCK=D5, SO=D7, CS_BT=D8, CS_ET=D6
const CroasterPinConfig pins = {D5, D7, D8, D6};
#elif defined(ESP32)
// ESP32-C3 Super Mini: SCK=4, SO=5, CS_BT=7, CS_ET=6
const CroasterPinConfig pins = {4, 5, 7, 6};
#else
const CroasterPinConfig pins = {4, 5, 7, 6};
#endif
