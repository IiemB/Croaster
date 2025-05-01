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
     */
    BleManager(CroasterCore &croaster, CommandHandler &commandHandler);

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
