#pragma once
#include <Arduino.h>

/**
 * Returns a unique identifier string for the current ESP chip.
 * - ESP32: 12-char HEX string (from efuse MAC)
 * - ESP8266: 6-char HEX string (from chip ID)
 */
String getUniqueChipId();

/**
 * Retrieves a shortened version of the chip ID.
 *
 * length The number of bytes to include in the shortened chip ID. Default is 4.
 * A String containing the shortened chip ID.
 */
String getShortChipId(uint8_t length = 4);

/**
 * Returns a device name based on prefix + chip ID, e.g., "Croaster_1234ABCD"
 */
String getDeviceName(String prefix = "", String suffix = "", uint8_t length = 4);
