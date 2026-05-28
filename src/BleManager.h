#pragma once

#if defined(ESP32)

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "CroasterCore.h"
#include "DisplayManager.h"
#include "Constants.h"
#include "CommandHandler.h"
#include "OtaHandler.h"

/**
 * @class BleManager
 * @brief Manages BLE (Bluetooth Low Energy) communication for the Croaster device.
 */
class BleManager
{
public:
    /**
     * @brief Constructs a BleManager instance.
     * @param croaster Reference to the CroasterCore instance.
     * @param commandHandler Reference to the CommandHandler instance.
     * @param displayManager Reference to the DisplayManager instance.
     */
    BleManager(CroasterCore &croaster, CommandHandler &commandHandler, DisplayManager &displayManager);

    /**
     * @brief Initializes the BLE server and characteristics.
     */
    void begin();

    /**
     * @brief Handles BLE-related tasks in the main loop.
     */
    void loop();

    /**
     * @brief Checks if a BLE client is connected.
     * @return True if a client is connected, false otherwise.
     */
    bool isClientConnected() const;

private:
    BLEServer *pServer = nullptr;                     ///< Pointer to the BLE server instance.
    BLECharacteristic *pDataCharacteristic = nullptr; ///< Pointer to the BLE data characteristic.

    CommandHandler *commandHandler = nullptr; ///< Pointer to the CommandHandler instance.
    CroasterCore *croaster = nullptr;         ///< Pointer to the CroasterCore instance.
    DisplayManager *displayManager = nullptr; ///< Pointer to the DisplayManager instance.

    OtaHandler otaHandler; ///< Handles OTA firmware updates over BLE.

    unsigned long lastSend = 0; ///< Timestamp of the last data broadcast.

    bool clientConnected = false; ///< Indicates if a BLE client is connected.

    /**
     * @brief Broadcasts data to connected BLE clients.
     */
    void broadcastData();

    /**
     * @brief Sends data to the BLE client.
     * @param data The data to send as a string.
     */
    void sendData(const String &data);

    /**
     * @class ServerCallbacks
     * @brief Handles BLE server connection and disconnection events.
     */
    class ServerCallbacks;

    /**
     * @class CharacteristicCallbacks
     * @brief Handles BLE characteristic read/write events.
     */
    class CharacteristicCallbacks;
};

#endif
