#pragma once

#include <Arduino.h>
#include <WebSocketsServer.h>
#include "Constants.h"
#include "WiFiManagerUtil.h"
#if defined(ESP32)
#include <Update.h>
#elif defined(ESP8266)
#include <Updater.h>
#endif

/**
 * @class OtaHandler
 * @brief Handles Over-The-Air (OTA) updates via WebSockets for ESP32 devices.
 */
class OtaHandler
{
public:
    /**
     * @brief Constructor for OtaHandler.
     */
    OtaHandler();

    /**
     * @brief Initializes the OTA process with the total size of the update.
     * @param totalSize The total size of the OTA update in bytes.
     */
    void begin(uint32_t totalSize);

    /**
     * @brief Processes incoming binary data for the OTA update.
     * @param data Pointer to the binary data.
     * @param len Length of the binary data.
     * @param server Reference to the WebSocketsServer handling the connection.
     * @param clientId ID of the client sending the data.
     * @return True if the data was handled successfully, false otherwise.
     */
    bool handleBinary(uint8_t *data, size_t len, WebSocketsServer &server, uint8_t clientId);

    /**
     * @brief Checks if the OTA process is currently receiving data.
     * @return True if data is being received, false otherwise.
     */
    bool isReceiving() const;

    /**
     * @brief Gets the total size of the OTA update.
     * @return The total size in bytes.
     */
    uint32_t getTotal() const;

    /**
     * @brief Gets the number of bytes written so far during the OTA process.
     * @return The number of bytes written.
     */
    uint32_t getWritten() const;

private:
    /**
     * @enum State
     * @brief Represents the current state of the OTA process.
     */
    enum class State
    {
        Idle,      ///< No OTA process is active.
        Receiving, ///< OTA packets are being received.
        Done       ///< OTA process is complete.
    };

    State state = State::Idle; ///< Current state of the OTA process.
    uint32_t totalSize = 0;    ///< Total size of the OTA update in bytes.
    uint32_t written = 0;      ///< Number of bytes written so far.

    /**
     * @brief Finalizes the OTA process upon successful completion.
     * @param server Reference to the WebSocketsServer handling the connection.
     * @param clientId ID of the client that initiated the OTA process.
     */
    void finalize(bool hasError = false);

    /**
     * @brief Resets the internal state of the handler to its initial values.
     */
    void resetState();
};
