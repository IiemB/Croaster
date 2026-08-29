#include "CroasterWebSocketManager.h"
#include <ArduinoJson.h>
#include "CroasterWiFiManager.h"
#include "CroasterConstants.h"

CroasterWebSocketManager::CroasterWebSocketManager(CroasterCore &croaster, CroasterCommandHandler &commandHandler, CroasterDisplay *display, uint16_t port)
    : server(port), croaster(&croaster), commandHandler(&commandHandler), display(display)
{
}

void CroasterWebSocketManager::handleEvent(const String &cmd, uint8_t num)
{
    if (cmd.startsWith("OTA_BEGIN:"))
    {
        uint32_t size = cmd.substring(10).toInt();
        otaHandler.begin(size);

        debugln(cmd);

        return;
    }

    String response;

    if (commandHandler->handle(cmd, response))
    {
        if (!response.isEmpty())
            server.sendTXT(num, response);

        debugln("# [CMD-SOCKET] " + cmd);
        if (!response.isEmpty())
            debugln("# [CMD-SOCKET-RESP] " + response);
    }
}

void CroasterWebSocketManager::begin()
{
    server.begin();
    server.onEvent([this](uint8_t num, WStype_t type, uint8_t *payload, size_t length)
                   {
        switch (type) {
            case WStype_DISCONNECTED:
                clientConnected--;

                debugln("# WebSocket Client Disconnected " + String(clientConnected));

                if ((display && display->isFirmwareUpdating()) || otaHandler.isReceiving())
                {
                    debugln("# [OTA] WebSocket disconnected during OTA - restarting...");
                    if (display)
                        display->updatingStatusToggle(false);
                    CroasterWiFiManager::restart();
                    return;
                }

                break;
            case WStype_CONNECTED:
                clientConnected++;

                debugln("# WebSocket Client Connected " + String(clientConnected));

                break;
            case WStype_TEXT:
                this->handleEvent(String((char *)payload), num);

                break;
            case WStype_BIN:
                if (otaHandler.isReceiving())
                {
                    String result = otaHandler.handleBinary(payload, length);

                    server.sendTXT(num, result);

                    int progress = int((double(otaHandler.getWritten()) / double(otaHandler.getTotal())) * 100.0);

                    if (display)
                    {
                        display->updatingStatusToggle(true);
                        display->updateFirmwareUpdateProgress(progress);
                    }
                }

                break;

            default:
                break;
        } });

    debugln("# WebSocket started");
}

void CroasterWebSocketManager::loop()
{
    server.loop();

    broadcastData();

    otaHandler.handleState();
}

bool CroasterWebSocketManager::isClientConnected() const
{
    return clientConnected > 0;
}

void CroasterWebSocketManager::broadcastData()
{
    if (!isClientConnected() || otaHandler.isReceiving())
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
