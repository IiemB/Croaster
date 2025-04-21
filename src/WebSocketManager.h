#pragma once
#include <WebSocketsServer.h>
#include "CroasterCore.h"
#include "DisplayManager.h"

extern WebSocketsServer webSocket;

/**
 * Initializes WebSocket server and its event handler.
 */
void setupWebSocket(CroasterCore &croaster);

/**
 * Sends sensor data over WebSocket and updates OLED.
 */
void broadcastData(CroasterCore &croaster);
