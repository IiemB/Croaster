#pragma once
#include <WiFiManager.h>

/**
 * Initializes and configures WiFiManager in non-blocking mode.
 */
void setupWiFiManager(const String &apName);

/**
 * @brief Handles the WiFi manager process, including initialization,
 *        configuration, and connection management.
 *
 * This function is responsible for managing the WiFi connection
 * process. It may include tasks such as starting the WiFi manager,
 * handling user input for network selection, and ensuring a stable
 * connection to the desired network.
 *
 * @note Ensure that this function is called during the appropriate
 *       phase of your application lifecycle to manage WiFi connectivity.
 */
void processWiFiManager();

/**
 * Reboots the ESP device via WiFiManager.
 */
void restartESP();

/**
 * Erases saved credentials and restarts ESP.
 */
void eraseESP();

/**
 * Called when config portal is triggered.
 */
void configModeCallback(WiFiManager *myWiFiManager);

/**
 * Retrieves the IP address of the device as a string.
 *
 * @return String The IP address of the device in standard dot-decimal notation.
 */
String getIpAddress();
