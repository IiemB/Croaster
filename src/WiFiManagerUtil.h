#pragma once
#include <WiFiManager.h>

/**
 * Initializes and configures WiFiManager in non-blocking mode.
 */
void setupWiFiManager(const String &apName);

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

extern WiFiManager wifiManager;
