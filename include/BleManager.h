#pragma once
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "TempsManager.h"
#include "Constants.h"

/**
 * Initializes BLE server, characteristics, and advertising.
 * @param croaster Reference to the TempsManager instance for handling commands.
 * @param bleDeviceConnected Flag to be updated on client connection/disconnection.
 */
void setupBLE(TempsManager &croaster, bool &bleDeviceConnected);
