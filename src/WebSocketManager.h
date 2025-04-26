#pragma once
#include <WebSocketsServer.h>
#include "CroasterCore.h"
#include "CommandHandler.h"

class WebSocketManager
{
public:
    WebSocketManager(CroasterCore &core, CommandHandler &handler, uint16_t port = 81);

    void begin();

    void loop();

private:
    WebSocketsServer server;

    CroasterCore *croaster = nullptr;
    CommandHandler *commandHandler = nullptr;

    unsigned long lastSend = 0;

    void handleEvent(const String &cmd, uint8_t num);

    int clientConnected = 0;

    bool isClientConnected() const;

    void broadcastData();
};
