#pragma once
#include <Arduino.h>

/**
 * @class CroasterDeviceIdentity
 * @brief Chip ID, device name and network identity helpers.
 */
class CroasterDeviceIdentity
{
public:
    /**
     * @brief Retrieves the unique chip ID of the device.
     * @return A string representing the unique chip ID.
     */
    static String uniqueChipId();

    /**
     * @brief Retrieves a shortened version of the chip ID.
     * @param length The desired length of the shortened ID (default is 4).
     * @return A string representing the shortened chip ID.
     */
    static String shortChipId(uint8_t length = 4);

    /**
     * @brief Generates a device name using a prefix, suffix, and chip ID.
     * @param prefix The prefix for the device name.
     * @param suffix The suffix for the device name.
     * @param length The length of the chip ID to include (default is 4).
     * @return A string representing the device name.
     */
    static String deviceName(String prefix = "", String suffix = "", uint8_t length = 4);

    /**
     * @brief Retrieves the device's IP address as a string.
     * @return The IP address in dot-decimal notation.
     */
    static String ipAddress();

    /**
     * @brief Retrieves the SSID name of the WiFi network.
     * @return A String containing the SSID name.
     */
    static String ssidName();

    /**
     * @brief Identifies the board this firmware is running on.
     * @return A machine-friendly board id (e.g. "esp32c3", "esp32s3",
     *         "esp8266") matching the app's CroasterBoardTypes enum. Set by
     *         the implementation at startup via setBoardName(); defaults to
     *         "unknown" until then.
     */
    static String boardName();

    /**
     * @brief Sets the board id reported by boardName().
     * @param name A machine-friendly board id (e.g. "esp32s3"), hardcoded by
     *             each implementation in its own main.cpp (no build flag).
     */
    static void setBoardName(const String &name);
};
