#pragma once

#if defined(ESP32) // Only include for ESP32 boards

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "CroasterCore.h"
#include "DisplayManager.h"
#include "Constants.h"
#include "CommandHandler.h"

/**
 * Initializes BLE server, characteristics, and advertising.
 * @param croaster Reference to the CroasterCore instance for handling commands.
 * @param bleDeviceConnected Flag to be updated on client connection/disconnection.
 */
void setupBLE(String name, CommandHandler &commandHandler);

#endif
