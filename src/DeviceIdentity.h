#pragma once
#include <Arduino.h>

/**
 * Returns a unique identifier string for the current ESP chip.
 * - ESP32: 12-char HEX string (from efuse MAC)
 * - ESP8266: 6-char HEX string (from chip ID)
 */
String getUniqueChipId();

/**
 * Returns a device name based on prefix + chip ID, e.g., "Croaster_1234ABCD"
 */
String getDeviceName(String prefix = "", String suffix = "");
