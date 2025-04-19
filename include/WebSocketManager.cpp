#include "WebSocketManager.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include "WiFiManagerUtil.h"

WebSocketsServer webSocket(81);
String socketEventMessage = "";
unsigned long lastWebSocketSend = 0;

void handleWebSocketEvent(const String &cmd, uint8_t num, TempsManager &croaster)
{
    StaticJsonDocument<96> doc;

    if (deserializeJson(doc, cmd))
    {
        debugln("# WebSocket: Invalid JSON");
        return;
    }

    if (doc["command"].is<String>())
    {
        String command = doc["command"];
        socketEventMessage = command;

        if (command == "getData")
        {
            croaster.idJsonData = doc["id"];
            String res = croaster.getJsonData("", true);
            webSocket.broadcastTXT(res);
            return;
        }

        if (command == "restartesp")
        {
            String data = croaster.getJsonData(command);
            webSocket.sendTXT(num, data);
            ESP.restart();
        }
        else if (command == "erase")
        {
            eraseESP();
        }
        else if (command == "dummyOn")
        {
            croaster.useDummyData = true;
            croaster.resetHistory();
        }
        else if (command == "dummyOff")
        {
            croaster.useDummyData = false;
            croaster.resetHistory();
        }
    }

    if (doc["command"].is<JsonObject>())
    {
        JsonObject cmd = doc["command"];
        if (cmd.containsKey("interval"))
        {
            croaster.intervalSendData = cmd["interval"].as<int>() * 1000;
            socketEventMessage = String(cmd["interval"].as<int>()) + "seconds";
        }
        if (cmd.containsKey("tempUnit"))
        {
            String unit = cmd["tempUnit"];
            croaster.changeTemperatureUnit(unit);
            socketEventMessage = unit;
        }
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
        handleWebSocketEvent(String((char *)payload), num, *(TempsManager *)nullptr); // Patched later
        break;
    default:
        break;
    }
}

void setupWebSocket(TempsManager &croaster)
{
    webSocket.begin();
    webSocket.onEvent([&croaster](uint8_t num, WStype_t type, uint8_t *payload, size_t length)
                      { handleWebSocketEvent(String((char *)payload), num, croaster); });
    debugln("# WebSocket started");
}

void broadcastData(TempsManager &croaster)
{
    if (millis() - lastWebSocketSend < croaster.intervalSendData)
        return;
    lastWebSocketSend = millis();

    String jsonData = croaster.getJsonData(socketEventMessage);
    webSocket.broadcastTXT(jsonData);

    String ip = WiFi.localIP().toString();
    debugln("# JSON: " + jsonData);

    socketEventMessage = "";
}
