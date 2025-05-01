#pragma once
#include <WebSocketsServer.h>
#include "CroasterCore.h"
#include "CommandHandler.h"
#include "OtaHandler.h"

/**
 * @class WebSocketManager
 * @brief Manages WebSocket communication for the Croaster device.
 */
class WebSocketManager
{
public:
    /**
     * @brief Constructs a WebSocketManager instance.
     * @param core Reference to the CroasterCore instance.
     * @param handler Reference to the CommandHandler instance.
     * @param port The WebSocket server port (default is 81).
     */
    WebSocketManager(CroasterCore &core, CommandHandler &handler, uint16_t port = 81);

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

    CroasterCore *croaster = nullptr;         ///< Pointer to the CroasterCore instance.
    CommandHandler *commandHandler = nullptr; ///< Pointer to the CommandHandler instance.

    OtaHandler otaHandler; ///< Pointer to the OtaHandler instance.

    unsigned long lastSend = 0; ///< Timestamp of the last data broadcast.

    /**
     * @brief Handles WebSocket events such as messages from clients.
     * @param cmd The command received from the client.
     * @param num The client number.
     */
    void handleEvent(const String &cmd, uint8_t num);

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
