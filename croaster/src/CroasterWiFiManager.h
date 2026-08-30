#pragma once
#include <WiFiManager.h>

/**
 * @class CroasterWiFiManager
 * @brief Encapsulates the WiFiManager captive-portal setup and lifecycle.
 */
class CroasterWiFiManager {
public:
    /**
     * @brief Sets up WiFiManager with the specified access point name.
     * @param apName The name of the access point.
     */
    static void setup(const String& apName);

    /**
     * @brief Processes WiFiManager tasks, including connection management.
     */
    static void process();

    /**
     * @brief Restarts the ESP device.
     */
    static void restart();

    /**
     * @brief Erases saved WiFi credentials and restarts the ESP device.
     */
    static void erase();

private:
    /**
     * @brief Callback triggered when the WiFiManager enters configuration mode.
     * @param myWiFiManager Pointer to the WiFiManager instance.
     */
    static void configModeCallback(WiFiManager* myWiFiManager);
};
