#include "WebSocketManager.h"
#include <ArduinoJson.h>
#include "WiFiManagerUtil.h"
#include "Constants.h"

WebSocketManager::WebSocketManager(CroasterCore &croaster, CommandHandler &commandHandler, DisplayManager &displayManager, uint16_t port) : server(port), croaster(&croaster), commandHandler(&commandHandler), displayManager(&displayManager)
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

void WebSocketManager::begin()
{
    server.begin();
    server.onEvent([this](uint8_t num, WStype_t type, uint8_t *payload, size_t length)
                   {
        switch (type) {
            case WStype_DISCONNECTED:
                clientConnected--;

                debugln("# WebSocket Client Disconnected " + String(clientConnected));

                if (displayManager->isFirmwareUpdating() || otaHandler.isReceiving())
                {
                    debugln("# [OTA] WebSocket disconnected during OTA - restarting...");
                    displayManager->updatingStatusToggle(false);
                    restartESP();
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
            case WStype_BIN :
                if (otaHandler.isReceiving())
                {
                    String result = otaHandler.handleBinary(payload, length);

                    server.sendTXT(num, result);
                    
                    int progress = int((double(otaHandler.getWritten()) / double(otaHandler.getTotal())) * 100.0);

                    displayManager->updatingStatusToggle(true);

                    displayManager->updateFirmwareUpdateProgress(progress);
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

    otaHandler.handleState();
}

bool WebSocketManager::isClientConnected() const
{
    return clientConnected > 0;
}

void WebSocketManager::broadcastData()
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
