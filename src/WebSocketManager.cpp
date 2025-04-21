#include "CommandHandler.h"
#include "WebSocketManager.h"
#include <ArduinoJson.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include "WiFiManagerUtil.h"

WebSocketsServer webSocket(81);
String socketEventMessage = "";
unsigned long lastWebSocketSend = 0;

void handleWebSocketEvent(const String &cmd, uint8_t num, CroasterCore &croaster, DisplayManager &displayManager)
{
    bool restart = false, erase = false;
    String response;

    if (handleCommand(cmd, croaster, displayManager, response, restart, erase))
    {
        if (!response.isEmpty())
        {
            webSocket.sendTXT(num, response);
        }
        if (erase)
            eraseESP();
        if (restart)
            ESP.restart();
    }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
        debugln("# WebSocket disconnected");
        break;
    case WStype_CONNECTED:
        debugln("# WebSocket connected");
        break;
    case WStype_TEXT:
        handleWebSocketEvent(String((char *)payload), num, *(CroasterCore *)nullptr, *(DisplayManager *)nullptr); // Patched later
        break;
    default:
        break;
    }
}

void setupWebSocket(CroasterCore &croaster, DisplayManager &displayManager)
{
    webSocket.begin();
    webSocket.onEvent([&croaster, &displayManager](uint8_t num, WStype_t type, uint8_t *payload, size_t length)
                      { handleWebSocketEvent(String((char *)payload), num, croaster, displayManager); });
    debugln("# WebSocket started");
}

void broadcastData(CroasterCore &croaster)
{
    if (millis() - lastWebSocketSend < croaster.intervalSendData)
        return;

    lastWebSocketSend = millis();

    String jsonData = croaster.getJsonData(socketEventMessage);
    webSocket.broadcastTXT(jsonData);

    String ip = WiFi.localIP().toString();
    debugln("# IP: " + ip);

    debugln("# JSON: " + jsonData);

    socketEventMessage = "";
}
