#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WebSocketsServer.h>
#include <WiFiManager.h>
// #include <Thermocouple.h>
// #include <MAX6675_Thermocouple.h>
// #include <SPI.h>
#include "DHT.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)

//  Setup MAX6675
#define SCK_PIN D8
#define SO_PIN D7
#define CS_PIN_BT D5
#define CS_PIN_ET D6

// Thermocouple *thermocouple_bt;
// Thermocouple *thermocouple_et;
float timer = 0;
int temp_et = 0;
int temp_bt = 0;
float ror_et = 0;
float ror_bt = 0;
int arrEt[61] = {};
int arrBt[61] = {};
float arrTimer[61] = {};
bool isEtBtSwapped = false;
bool isArrInitialized = false;

//  Setup DHT11
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

// Setup LCD
LiquidCrystal_I2C display(0x27, 16, 2);

// Setup Croaster
float fwVersion = 2.2;
const char *ssidName = "Croaster";

String jsonData;
int idJsonData = 0;

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

void updateJsonData()
{
  StaticJsonDocument<256> doc;

  doc["id"] = idJsonData;

  JsonObject data = doc.createNestedObject("data");
  data["BT"] = temp_bt;
  data["ET"] = temp_et;

  JsonObject croaster = doc.createNestedObject("croaster");
  croaster["fv"] = fwVersion;
  croaster["timer"] = timer;
  croaster["et"] = temp_et;
  croaster["bt"] = temp_bt;
  croaster["rorEt"] = ror_et;
  croaster["rorBt"] = ror_bt;
  croaster["humd"] = humd;
  croaster["temp"] = temp;
  croaster["hic"] = hic;

  String tempJsonData;

  serializeJson(doc, tempJsonData);

  jsonData = tempJsonData;
}

void handleArtisanCommand(String cmd, uint8_t num)
{

  StaticJsonDocument<96> request;

  DeserializationError error = deserializeJson(request, cmd);

  if (error)
  {
    debug(F("deserializeJson() failed: "));
    debugln(error.f_str());
    return;
  }

  String command = request["command"];

  idJsonData = request["id"];

  if (command == "getData")
  {
    updateJsonData();

    webSocket.sendTXT(num, jsonData);
  }
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

    handleArtisanCommand(cmd, num);

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

    if (cmd == "swapetbt")
    {
      isEtBtSwapped = true;
    }

    if (cmd == "swapbtet")
    {
      isEtBtSwapped = false;
    }

    if (cmd == "croaster connect")
    {
      isCroasterConnected = true;
    }

    if (cmd == "croaster disconnect")
    {
      isCroasterConnected = false;
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

void updateDisplay()
{
  display.clear();
  display.setCursor(0, 0);
  display.print("ET: " + String(temp_et) + " " + "BT: " + String(temp_bt));

  if (showIp && isWifiConnected && !isCroasterConnected)
  {
    display.setCursor(0, 1);
    display.print(WiFi.localIP().toString());

    showIp = false;
  }
  else
  {
    display.setCursor(0, 1);
    display.print("RT: " + String(static_cast<int>(temp)) + "C" + " " + "RH: " + String(static_cast<int>(humd)) + "%");

    showIp = true;
  }

  display.display();
}

void setup()
{
  Serial.begin(115200);

  display.init();
  display.backlight();

  splash();

  // thermocouple_bt = new MAX6675_Thermocouple(SCK_PIN, CS_PIN_BT, SO_PIN);
  // thermocouple_et = new MAX6675_Thermocouple(SCK_PIN, CS_PIN_ET, SO_PIN);

  wifiManager.setConfigPortalBlocking(false);
  wifiManager.setAPCallback(configModeCallback);

  display.clear();
  display.setCursor(0, 0);
  display.print("Firmware Version");
  display.setCursor(0, 1);
  display.print(String(fwVersion));
  display.display();
  delay(2000);

  if (wifiManager.autoConnect(ssidName))
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

  isWifiConnected = WiFi.status() == WL_CONNECTED;

  if (millis() - millisReadTemp >= 250)
  {
    millisReadTemp = millis();
    // if (isEtBtSwapped)
    // {
    //   temp_bt = thermocouple_et->readCelsius();
    //   temp_et = thermocouple_bt->readCelsius();
    // }
    // else
    // {
    //   temp_et = thermocouple_et->readCelsius();
    //   temp_bt = thermocouple_bt->readCelsius();
    // }

    // NOTE DUMMY
    temp_et = random(30, 36);
    temp_bt = random(30, 36);

    timer = millis() * 0.001;

    // if (isnan(temp_bt) || temp_bt > 9000)
    // {
    //   debugln("# Failed to read BT!");
    //   temp_bt = 0;
    // }

    // if (isnan(temp_et) || temp_et > 9000)
    // {
    //   debugln("# Failed to read ET!");
    //   temp_et = 0;
    // }

    // humd = dht.readHumidity();
    // temp = dht.readTemperature();

    // if (isnan(humd) || isnan(temp))
    // {
    //   debugln("# Failed to read from DHT sensor!");
    //   humd = 0;
    //   temp = 0;
    //   hic = 0;
    // }
    // else
    // {
    //   hic = dht.computeHeatIndex(temp, humd, false);
    // }

    // NOTE DUMMY
    humd = random(30, 40);
    temp = random(30, 40);
    hic = dht.computeHeatIndex(temp, humd, false);
  }

  if (millis() - millisUpdateDisplay >= 1500)
  {
    millisUpdateDisplay = millis();

    getRor(temp_et, temp_bt, timer);

    updateDisplay();
  }

  if (millis() - millisWebSocket >= intervalSendData)
  {
    millisWebSocket = millis();

    updateJsonData();

    webSocket.broadcastTXT(jsonData);

    debugln("");
    if (isWifiConnected)
    {
      debugln("# " + WiFi.localIP().toString());
    }

    debugln("# isEtBtSwapped: " + String(isEtBtSwapped));
    debugln("# intervalSendData: " + String(intervalSendData));
    debugln("# Json Data: " + jsonData);
    debugln("");
  }
}