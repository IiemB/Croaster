#pragma once
#include <WebSocketsServer.h>
#include "CroasterCore.h"
#include "CommandHandler.h"

extern WebSocketsServer webSocket;

/**
 * Initializes WebSocket server and its event handler.
 */
void setupWebSocket(CommandHandler &commandHandler);

/**
 * @brief Continuously handles WebSocket communication.
 *
 * This function is responsible for managing the WebSocket connection,
 * processing incoming messages, and sending outgoing messages in a loop.
 * It ensures that the WebSocket remains active and responsive.
 */
void loopWebSocket();

/**
 * Sends sensor data over WebSocket and updates OLED.
 */
void broadcastData(CroasterCore &croaster);
