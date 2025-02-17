#include <Arduino.h>
#include <SPI.h>
#include <WiFiManager.h>
#include <WebSocketsServer.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <Croaster.h>

// Globals
Croaster croaster(true, 2.42);

WiFiManager wifiManager;
WebSocketsServer webSocket(81);
LiquidCrystal_I2C display(0x27, 16, 2);

unsigned long lastWebSocketSend = 0;
unsigned long lastDisplayUpdate = 0;

bool showIp = false;
String socketEventMessage = "";

// Function Prototypes
void updateDisplay();
void broadcastData();
void handleWebSocketEvent(const String &cmd, uint8_t num);
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
void configModeCallback(WiFiManager *myWiFiManager);

// Function Implementations
void setup()
{
    Serial.begin(115200);
    display.init();
    display.backlight();
    croaster.init();

    wifiManager.setDebugOutput(croaster.useDummyData);
    wifiManager.setConfigPortalBlocking(false);
    wifiManager.setAPCallback(configModeCallback);

    wifiManager.setClass("invert");

    // set custom ip for portal
    wifiManager.setAPStaticIPConfig(IPAddress(10, 0, 1, 1), IPAddress(10, 0, 1, 1), IPAddress(255, 255, 255, 0));

    if (wifiManager.autoConnect(croaster.ssidName.c_str()))
        debugln("# WiFi Connected");

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    debugln("# WebSocket started");
}

void loop()
{
    wifiManager.process();
    webSocket.loop();
    croaster.loop();
    updateDisplay();
    broadcastData();
}

void updateDisplay()
{
    if (millis() - lastDisplayUpdate < 1500)
        return;
    lastDisplayUpdate = millis();

    display.clear();
    display.setCursor(0, 0);
    display.print("ET: " + String(croaster.tempET) + " BT: " + String(croaster.tempBT));
    display.setCursor(0, 1);
    if (showIp)
    {
        display.print(WiFi.localIP().toString());
        showIp = false;
    }
    else
    {
        display.print("T: " + String((int)croaster.tempR) + String(croaster.temperatureUnit) + "H: " + String((int)croaster.humidity) + "%");
        showIp = true;
    }
}

void broadcastData()
{
    if (millis() - lastWebSocketSend < croaster.intervalSendData)
        return;
    lastWebSocketSend = millis();

    String jsonData = croaster.getJsonData(socketEventMessage);

    webSocket.broadcastTXT(jsonData);

    debugln("");
    debugln("# " + WiFi.localIP().toString());
    debugln("# Json Data: " + jsonData);

    socketEventMessage = "";
}

void handleWebSocketEvent(const String &cmd, uint8_t num)
{
    StaticJsonDocument<96> request;
    if (deserializeJson(request, cmd))
    {
        debugln("# Invalid JSON command");
        return;
    }

    if (request["command"].is<String>())
    {
        String command = request["command"];

        socketEventMessage = command;

        // Send data to Artisan
        if (command == "getData")
        {
            croaster.idJsonData = request["id"];

            socketEventMessage = "";

            String jsonData = croaster.getJsonData(socketEventMessage, true);

            webSocket.broadcastTXT(jsonData);

            debugln("");
            debugln("# Json Data Artisan: " + jsonData);

            return;
        }

        if (command == "restartesp")
        {
            String jsonData = croaster.getJsonData(socketEventMessage);

            webSocket.sendTXT(num, jsonData);

            ESP.restart();

            return;
        }

        if (command == "erase")
        {
            String jsonData = croaster.getJsonData(socketEventMessage);

            webSocket.sendTXT(num, jsonData);

            wifiManager.erase();

            ESP.restart();

            return;
        }
    }

    if (request["command"].is<JsonObject>())
    {
        JsonObject command = request["command"];

        // Handle interval update
        if (command.containsKey("interval") && command["interval"].is<int>())
        {
            int intervalSeconds = command["interval"]; // Read interval value

            croaster.intervalSendData = intervalSeconds * 1000; // Convert seconds to milliseconds

            debugln("# Interval updated to " + String(intervalSeconds) + " seconds");

            socketEventMessage = String(intervalSeconds) + "seconds";
        }

        if (command.containsKey("tempUnit") && command["tempUnit"].is<String>())
        {
            String tempUnit = command["tempUnit"]; // Read tempUnit value

            croaster.changeTemperatureUnit(tempUnit);

            debugln("# Temperature Unit updated to " + tempUnit);

            socketEventMessage = tempUnit;
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
        handleWebSocketEvent(String((char *)payload), num);
        break;

    default:
        break;
    }
}

void configModeCallback(WiFiManager *myWiFiManager)
{
    debugln("# Config mode: " + WiFi.softAPIP().toString());
}
