#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WebSocketsServer.h>
#include <WiFiManager.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <Croaster.h>

Croaster croaster;

unsigned long millisWebSocket = 0;
unsigned long millisUpdateDisplay = 0;

//  Setup WiFiManager
bool isWifiConnected = false;
WiFiManager wifiManager;

// Setup WebSocket
WebSocketsServer webSocket = WebSocketsServer(81);

// Setup LCD
LiquidCrystal_I2C display(0x27, 16, 2);

// Croaster Connection
bool isCroasterConnected = false;
bool showIp = false;

void splash()
{
  for (int i = 0; i <= 7; i++)
  {
    display.clear();
    display.setCursor(3, 0);
    display.print("WELCOME TO");
    display.setCursor(4, 1);
    display.print("CROASTER");
    display.display();
    delay(250);
    display.clear();
    delay(250);
  }

  delay(3000);
}

void configModeCallback(WiFiManager *myWiFiManager)
{
  debugln("# Entered config mode");
  debug("# ");
  debugln(WiFi.softAPIP());
  debugln("# " + myWiFiManager->getConfigPortalSSID());
}

void handleSocketEventCommand(String cmd, uint8_t num)
{

  StaticJsonDocument<96> request;

  DeserializationError error = deserializeJson(request, cmd);

  if (error)
  {
    debug(F("# deserializeJson() failed: "));
    debugln(error.f_str());
    return;
  }

  String command = request["command"];

  if (command == "getData")
  {
    croaster.idJsonData = request["id"];
  }

  if (command == "restartesp")
  {
    ESP.restart();
  }

  if (command == "satudetik")
  {
    croaster.intervalSendData = 1000;
  }

  if (command == "duadetik")
  {
    croaster.intervalSendData = 2000;
  }

  if (command == "tigadetik")
  {
    croaster.intervalSendData = 3000;
  }

  if (command == "empatdetik")
  {
    croaster.intervalSendData = 4000;
  }

  if (command == "limadetik")
  {
    croaster.intervalSendData = 5000;
  }

  if (command == "erase")
  {
    wifiManager.erase();
  }

  croaster.updateJsonData(command);

  webSocket.sendTXT(num, croaster.jsonData);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  // webscket event method
  String command = "";
  switch (type)
  {
  case WStype_DISCONNECTED:
    debugln("# Websocket is disconnected");

    isCroasterConnected = false;

    break;
  case WStype_CONNECTED:

    debugln("# Websocket is connected");
    debugln("# " + webSocket.remoteIP(num).toString());
    // webSocket.sendTXT(num, "connected");

    isCroasterConnected = true;

    break;
  case WStype_TEXT:
    command = "";

    for (size_t i = 0; i < length; i++)
    {
      command = command + (char)payload[i];
    }

    handleSocketEventCommand(command, num);

    break;
  case WStype_FRAGMENT_TEXT_START:
    break;
  case WStype_FRAGMENT_BIN_START:
    break;
  case WStype_BIN:
    hexdump(payload, length);
    break;
  default:
    break;
  }
}

void updateDisplay()
{
  if (!(millis() - millisUpdateDisplay >= 1500))
    return;

  millisUpdateDisplay = millis();

  display.clear();
  display.setCursor(0, 0);
  display.print("ET: " + String(croaster.temp_et) + " " + "BT: " + String(croaster.temp_bt));

  if (showIp && isWifiConnected && !isCroasterConnected)
  {
    display.setCursor(0, 1);
    display.print(WiFi.localIP().toString());

    showIp = false;
  }
  else
  {
    display.setCursor(0, 1);
    display.print("RT: " + String(static_cast<int>(croaster.temp)) + "C" + " " + "RH: " + String(static_cast<int>(croaster.humd)) + "%");

    showIp = true;
  }

  display.display();
}

void sendDataToCroaster()
{
  if (!(millis() - millisWebSocket >= croaster.intervalSendData))
    return;

  millisWebSocket = millis();

  debugln("");
  if (isWifiConnected)
  {
    debugln("# " + WiFi.localIP().toString());
  }

  if (isWifiConnected && isCroasterConnected)
  {
    webSocket.broadcastTXT(croaster.jsonData);
  }

  debugln("# Json Data: " + croaster.jsonData);
  debugln("");
}

void setup()
{
  Serial.begin(115200);

  display.init();
  display.backlight();

  splash();

  croaster.init();

  wifiManager.setConfigPortalBlocking(false);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setClass("invert");    // dark theme
  wifiManager.setScanDispPerc(true); // display percentages instead of graphs for RSSI

  display.clear();
  display.setCursor(0, 0);
  display.print("Firmware Version");
  display.setCursor(0, 1);
  display.print(String(croaster.fwVersion));
  display.display();
  delay(2000);

  if (wifiManager.autoConnect(croaster.ssidName))
  {
    String connectedSSID = wifiManager.getWiFiSSID(true);
    debugln("# WiFi Connected... yeey :)");

    display.clear();
    display.setCursor(0, 0);
    display.print("WiFi Connected");
    display.setCursor(0, 1);
    display.print(connectedSSID);
    display.display();
    delay(2000);
  }
  else
  {
    debugln("# Configportal running");

    display.clear();
    display.setCursor(4, 0);
    display.print("WiFi Not");
    display.setCursor(4, 1);
    display.print("Connected");
    display.display();
    delay(2000);
  }

  dht.begin();
  webSocket.begin();                 // websocket Begin
  webSocket.onEvent(webSocketEvent); // set Event for websocket

  debugln("# Websocket is started");
}

void loop()
{
  wifiManager.process();
  webSocket.loop();
  croaster.loop();

  isWifiConnected = WiFi.status() == WL_CONNECTED;

  updateDisplay();

  sendDataToCroaster();
}