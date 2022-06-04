#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <WiFiManager.h>
#include "DHT.h"

#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)

// Artisan setup
#define maxLenght 30
String inString = String(maxLenght);
bool isArtisan = true;

float timer = 0;
int temp_et = 0;
int temp_bt = 0;
float ror_et = 0;
float ror_bt = 0;
int arrEt[61] = {};
int arrBt[61] = {};
float arrTimer[61] = {};
bool isArrInitialized = false;

#define DHT_PIN D4
#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);
float humd = 0;
float temp = 0;
float hic = 0;

//  Setup millis and timer
unsigned long millisReadTemp = 0;
unsigned long millisWebSocket = 0;
unsigned long millisIp = 0;
unsigned long millisInvertDisplay = 0;
unsigned long millisUpdateDisplay = 0;
unsigned long intervalSendData = 3000;

//  Setup WiFiManager
bool isWifiConnected = false;
WiFiManager wifiManager;

// Setup WebSocket
WebSocketsServer webSocket = WebSocketsServer(81);

float fwVersion = 0.99;

void getRor(int et, int bt, float timer)
{
  if (isArrInitialized)
  {
    for (int i = 0; i <= 59; i++)
    {
      arrEt[i] = arrEt[i + 1];
      arrBt[i] = arrBt[i + 1];
      arrTimer[i] = arrTimer[i + 1];
    }

    arrEt[59] = et;
    arrBt[59] = bt;
    arrTimer[59] = timer;

    int32_t dEt = arrEt[59] - arrEt[0];
    int32_t dBt = arrBt[59] - arrBt[0];
    float dT = arrTimer[59] - arrTimer[0];

    ror_et = (dEt / dT) * 60;
    ror_bt = (dBt / dT) * 60;
  }
  else
  {
    for (int i = 0; i <= 59; i++)
    {
      arrEt[i] = et;
      arrBt[i] = bt;
      arrTimer[i] = timer;
    }
    isArrInitialized = true;
  }
}

void configModeCallback(WiFiManager *myWiFiManager)
{
  debugln("# Entered config mode");
  debug("# ");
  debugln(WiFi.softAPIP());
  debugln("# " + myWiFiManager->getConfigPortalSSID());
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  // webscket event method
  String cmd = "";
  switch (type)
  {
  case WStype_DISCONNECTED:
    debugln("# Websocket is disconnected");
    break;
  case WStype_CONNECTED:
  {
    debugln("# Websocket is connected");
    debugln("# " + webSocket.remoteIP(num).toString());
    // webSocket.sendTXT(num, "connected");
  }
  break;
  case WStype_TEXT:
    cmd = "";

    for (size_t i = 0; i < length; i++)
    {
      cmd = cmd + (char)payload[i];
    }

    if (cmd == "readdata")
    {
      // when command from app is "poweron"
      // recieved command from mobile app
      // we can do task according to command from mobile using if-else-else if
    }

    if (cmd == "restartesp")
    {
      ESP.restart();
    }

    if (cmd == "satudetik")
    {
      intervalSendData = 1000;
    }

    if (cmd == "duadetik")
    {
      intervalSendData = 2000;
    }

    if (cmd == "tigadetik")
    {
      intervalSendData = 3000;
    }

    if (cmd == "empatdetik")
    {
      intervalSendData = 4000;
    }

    if (cmd == "limadetik")
    {
      intervalSendData = 5000;
    }

    // webSocket.sendTXT(num, cmd);
    // send response to mobile, if command is "poweron" then response will be "poweron:success"
    // this response can be used to track down the success of command in mobile app.
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

void handleArtisan()
{
  if (Serial.available() > 0)
  {
    String command = Serial.readStringUntil('\n');

    if (command == "READ")
    {
      String x = String(hic) + "," + String(temp_et) + "," + String(temp_bt) + ",0.00";
      debugln(x);
      Serial.flush();
    }
    else if (command == "CHAN;1200")
    {
      debugln("# CHAN;1200");
      Serial.flush();
    }
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  wifiManager.setConfigPortalBlocking(false);
  wifiManager.setAPCallback(configModeCallback);

  if (wifiManager.autoConnect("Craoster Dummy Data"))
  {
    String connectedSSID = wifiManager.getWiFiSSID(true);
    debugln("# WiFi Connected... yeey :)");
    delay(2000);
  }
  else
  {
    debugln("# Configportal running");
    delay(2000);
  }

  webSocket.begin();                 // websocket Begin
  webSocket.onEvent(webSocketEvent); // set Event for websocket
  debugln("# Websocket is started");
}

void loop()
{
  wifiManager.process();
  webSocket.loop();
  handleArtisan();

  if (millis() - millisReadTemp >= 250)
  {
    if (isWifiConnected)
    {
      digitalWrite(LED_BUILTIN, LOW);
    }

    millisReadTemp = millis();
    temp_bt = random(25, 71);
    temp_et = random(25, 71);

    timer = millis() * 0.001;

    if (isnan(temp_bt) || temp_bt > 9000)
    {
      debugln("# Failed to read BT!");
      temp_bt = 0;
    }

    if (isnan(temp_et) || temp_et > 9000)
    {
      debugln("# Failed to read ET!");
      temp_et = 0;
    }

    humd = random(19, 50);
    temp = random(19, 50);

    if (isnan(humd) || isnan(temp))
    {
      debugln("# Failed to read from DHT sensor!");
      humd = 0;
      temp = 0;
      hic = 0;
      return;
    }
    else
    {
      hic = dht.computeHeatIndex(temp, humd, false);
    }
  }

  if (millis() - millisUpdateDisplay >= 1500)
  {
    if (isWifiConnected)
    {
      digitalWrite(LED_BUILTIN, HIGH);
    }

    millisUpdateDisplay = millis();

    getRor(temp_et, temp_bt, timer);
  }

  if (millis() - millisWebSocket >= intervalSendData)
  {
    if (isWifiConnected)
    {
      digitalWrite(LED_BUILTIN, LOW);
    }

    millisWebSocket = millis();

    isWifiConnected = WiFi.status() == WL_CONNECTED;    

    String arrJsonData[11] = {
        "{",
        "\"fv\":" + String(fwVersion),
        ",\"timer\":" + String(timer),
        ",\"et\":" + String(temp_et),
        ",\"bt\":" + String(temp_bt),
        ",\"rorEt\":" + String(ror_et),
        ",\"rorBt\":" + String(ror_bt),
        ",\"humd\":" + String(humd),
        ",\"temp\":" + String(temp),
        ",\"hic\":" + String(hic),
        "}",
    };

    String jsonData;

    for (int i = 0; i <= 10; i++)
    {
      jsonData = jsonData + arrJsonData[i];
    }

    webSocket.broadcastTXT(jsonData);
    debugln("# isWifiConnected: " + String(isWifiConnected));
    debugln("# intervalSendData: " + String(intervalSendData));
    debugln("# Json Data: " + jsonData);
  } 
}