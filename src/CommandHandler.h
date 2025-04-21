#pragma once
#include <ArduinoJson.h>
#include "CroasterCore.h"

/**
 * Parses and handles incoming BLE or WebSocket JSON commands.
 *
 * @param json         Raw JSON string from client
 * @param croaster     Reference to the sensor/logic manager
 * @param responseOut  Filled with response (if any)
 * @param restart      Set to true if restart is requested
 * @param erase        Set to true if erase is requested
 * @return true if command is valid and handled, false otherwise
 */
bool handleCommand(const String &json, CroasterCore &croaster, String &responseOut, bool &restart, bool &erase);
