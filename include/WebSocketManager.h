#pragma once
#include <WebSocketsServer.h>
#include "TempsManager.h"
#include "DisplayManager.h"

extern WebSocketsServer webSocket;

/**
 * Initializes WebSocket server and its event handler.
 */
void setupWebSocket(TempsManager &croaster);

/**
 * Sends sensor data over WebSocket and updates OLED.
 */
void broadcastData(TempsManager &croaster);
