#pragma once
#include <Arduino.h>

/**
 * @brief Retrieves the unique chip ID of the device.
 * @return A string representing the unique chip ID.
 */
String getUniqueChipId();

/**
 * @brief Retrieves a shortened version of the chip ID.
 * @param length The desired length of the shortened ID (default is 4).
 * @return A string representing the shortened chip ID.
 */
String getShortChipId(uint8_t length = 4);

/**
 * @brief Generates a device name using a prefix, suffix, and chip ID.
 * @param prefix The prefix for the device name.
 * @param suffix The suffix for the device name.
 * @param length The length of the chip ID to include (default is 4).
 * @return A string representing the device name.
 */
String getDeviceName(String prefix = "", String suffix = "", uint8_t length = 4);

/**
 * @brief Retrieves the device's IP address as a string.
 * @return The IP address in dot-decimal notation.
 */
String getIpAddress();

/**
 * @brief Retrieves the SSID name of the WiFi network.
 *
 * @return A String containing the SSID name.
 */
String getSsidName();