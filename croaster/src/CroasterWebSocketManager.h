#pragma once
#include <WebSocketsServer.h>

#include "CroasterCommandHandler.h"
#include "CroasterCore.h"
#include "CroasterDisplay.h"
#include "CroasterOtaHandler.h"

/**
 * @class CroasterWebSocketManager
 * @brief Manages WebSocket communication for the Croaster device.
 */
class CroasterWebSocketManager {
public:
    /**
     * @brief Constructs a CroasterWebSocketManager instance.
     * @param core Reference to the CroasterCore instance.
     * @param handler Reference to the CroasterCommandHandler instance.
     * @param display Optional display for OTA progress (may be nullptr).
     * @param port The WebSocket server port (default is 81).
     */
    CroasterWebSocketManager(CroasterCore& core, CroasterCommandHandler& handler, CroasterDisplay* display,
                             uint16_t port = 81);

    /**
     * @brief Initializes the WebSocket server.
     */
    void begin();

    /**
     * @brief Handles WebSocket-related tasks in the main loop.
     */
    void loop();

private:
    WebSocketsServer server; ///< WebSocket server instance.

    CroasterCore* croaster = nullptr;                 ///< Pointer to the CroasterCore instance.
    CroasterCommandHandler* commandHandler = nullptr; ///< Pointer to the CroasterCommandHandler instance.
    CroasterDisplay* display = nullptr;               ///< Optional display (may be nullptr).

    CroasterOtaHandler otaHandler; ///< Handles OTA firmware updates over WebSocket.

    unsigned long lastSend = 0; ///< Timestamp of the last data broadcast.

    /**
     * @brief Handles WebSocket events such as messages from clients.
     * @param cmd The command received from the client.
     * @param num The client number.
     */
    void handleEvent(const String& cmd, uint8_t num);

    int clientConnected = 0; ///< Tracks the number of connected clients.

    /**
     * @brief Checks if a WebSocket client is connected.
     * @return True if a client is connected, false otherwise.
     */
    bool isClientConnected() const;

    /**
     * @brief Broadcasts data to all connected WebSocket clients.
     */
    void broadcastData();
};
