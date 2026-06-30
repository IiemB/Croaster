#pragma once
#include <WiFiManager.h>

/**
 * @brief Sets up the WiFiManager with the specified access point name.
 * @param apName The name of the access point.
 */
void setupWiFiManager(const String &apName);

/**
 * @brief Processes WiFiManager tasks, including connection management.
 */
void processWiFiManager();

/**
 * @brief Restarts the ESP device.
 */
void restartESP();

/**
 * @brief Erases saved WiFi credentials and restarts the ESP device.
 */
void eraseESP();

/**
 * @brief Callback triggered when the WiFiManager enters configuration mode.
 * @param myWiFiManager Pointer to the WiFiManager instance.
 */
void configModeCallback(WiFiManager *myWiFiManager);
