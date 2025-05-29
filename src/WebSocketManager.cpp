#include "WebSocketManager.h"
#include <ArduinoJson.h>
#include "WiFiManagerUtil.h"
#include "Constants.h"

WebSocketManager::WebSocketManager(CroasterCore &croaster, CommandHandler &commandHandler, uint16_t port) : server(port), croaster(&croaster), commandHandler(&commandHandler)
{
}

void WebSocketManager::handleEvent(const String &cmd, uint8_t num)
{
    if (cmd.startsWith("OTA_BEGIN:"))
    {

        uint32_t size = cmd.substring(10).toInt();
        otaHandler.begin(size);

        debugln(cmd);

        return;
    }

    bool restart = false, erase = false;

    String response;

    if (commandHandler->handle(cmd, response, restart, erase))
    {

        if (!response.isEmpty())
        {
            server.sendTXT(num, response);
        }

        if (erase)
            eraseESP();

        if (restart)
            ESP.restart();

        debugln("# [CMD-SOCKET] " + cmd);
    }
}

void WebSocketManager::begin()
{
    server.begin();
    server.onEvent([this](uint8_t num, WStype_t type, uint8_t *payload, size_t length)
                   {
        switch (type) {
            case WStype_DISCONNECTED:
                clientConnected--;

                debugln("# WebSocket Client Disconnected " + String(clientConnected));

                break;
            case WStype_CONNECTED:
                clientConnected++;

                debugln("# WebSocket Client Connected " + String(clientConnected));

                break;
            case WStype_TEXT:
                this->handleEvent(String((char *)payload), num);
                
                break;
            case WStype_BIN :
                if (otaHandler.isReceiving())
                {
                    otaHandler.handleBinary(payload, length, server, num);
                }

                break;

            default:
                break;
        } });

    debugln("# WebSocket started");
}

void WebSocketManager::loop()
{
    server.loop();

    broadcastData();
}

bool WebSocketManager::isClientConnected() const
{
    return clientConnected > 0;
}

void WebSocketManager::broadcastData()
{

    if (!isClientConnected())
        return;

    unsigned long now = millis();

    unsigned long interval = croaster->intervalSendData() * 1000;

    if (now - lastSend >= interval)
    {
        lastSend = now;

        String jsonData = croaster->getJsonData();
        server.broadcastTXT(jsonData);

        debugln("# [SOCKET-JSON] " + jsonData);
        debugln("");
    }
}
