#pragma once

#include "CroasterConstants.h"

#if CROASTER_HAS_BLE

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "CroasterCore.h"
#include "CroasterDisplay.h"
#include "CroasterCommandHandler.h"
#include "CroasterOtaHandler.h"

/**
 * @class CroasterBleManager
 * @brief Manages BLE (Bluetooth Low Energy) communication for the Croaster device.
 *
 * Only compiled on boards that provide BLE (see CROASTER_HAS_BLE in
 * CroasterConstants.h). The display is optional and may be nullptr.
 */
class CroasterBleManager
{
public:
    /**
     * @brief Constructs a CroasterBleManager instance.
     * @param croaster Reference to the CroasterCore instance.
     * @param commandHandler Reference to the CroasterCommandHandler instance.
     * @param display Optional display for OTA progress (may be nullptr).
     */
    CroasterBleManager(CroasterCore &croaster, CroasterCommandHandler &commandHandler, CroasterDisplay *display);

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

    CroasterCommandHandler *commandHandler = nullptr; ///< Pointer to the CroasterCommandHandler instance.
    CroasterCore *croaster = nullptr;                 ///< Pointer to the CroasterCore instance.
    CroasterDisplay *display = nullptr;               ///< Optional display (may be nullptr).

    CroasterOtaHandler otaHandler; ///< Handles OTA firmware updates over BLE.

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
