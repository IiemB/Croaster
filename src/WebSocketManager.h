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
 * Sends sensor data over WebSocket and updates OLED.
 */
void broadcastData(CroasterCore &croaster);
