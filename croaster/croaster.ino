#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <WebSocketsServer.h>
#include <WiFiManager.h>
#include <Thermocouple.h>
#include <MAX6675_Thermocouple.h>
#include <SPI.h>
#include "DHT.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ezButton.h>

#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)

// Artisan setup
#define maxLenght 30
String inString = String(maxLenght);
bool isArtisan = true;

//  Setup MAX6675
#define SCK_PIN D8
#define SO_PIN D7
#define CS_PIN_BT D5
#define CS_PIN_ET D6

Thermocouple *thermocouple_bt;
Thermocouple *thermocouple_et;
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

//  Setup time and date
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "0.id.pool.ntp.org", 25200, 60000);
char Time[] = "TIME:00:00:00";
char Date[] = "DATE:00/00/2000";
byte last_second, second_, minute_, hour_, day_, month_;
int year_;

// Setup button
ezButton button(D3);

// Setup LCD
LiquidCrystal_I2C display(0x27, 16, 2);
uint8_t selectedAdditonalDisplay;

// Setup Croaster
float fwVersion = 2.0;

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
    webSocket.sendTXT(num, "connected");
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

    if (cmd == "swapetbt")
    {
      isEtBtSwapped = true;
    }

    if (cmd == "swapbtet")
    {
      isEtBtSwapped = false;
    }

    webSocket.sendTXT(num, cmd);
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

void timeUpdater()
{
  timeClient.update();
  unsigned long unix_epoch = timeClient.getEpochTime();

  second_ = second(unix_epoch);
  if (last_second != second_)
  {
    minute_ = minute(unix_epoch);
    hour_ = hour(unix_epoch);
    day_ = day(unix_epoch);
    month_ = month(unix_epoch);
    year_ = year(unix_epoch);

    Time[12] = second_ % 10 + 48;
    Time[11] = second_ / 10 + 48;
    Time[9] = minute_ % 10 + 48;
    Time[8] = minute_ / 10 + 48;
    Time[6] = hour_ % 10 + 48;
    Time[5] = hour_ / 10 + 48;

    Date[5] = day_ / 10 + 48;
    Date[6] = day_ % 10 + 48;
    Date[8] = month_ / 10 + 48;
    Date[9] = month_ % 10 + 48;
    Date[13] = (year_ / 10) % 10 + 48;
    Date[14] = year_ % 10 % 10 + 48;

    last_second = second_;
  }
}

void updateDisplay()
{
  display.clear();
  switch (selectedAdditonalDisplay)
  {
  case 0:
    display.setCursor(0, 0);
    display.print("Drum Temp: " + String(temp_et) + " C");
    display.setCursor(0, 1);
    display.print("Bean Temp: " + String(temp_bt) + " C");
    break;
  case 1:
    display.setCursor(0, 0);
    display.print(Time);
    display.setCursor(0, 1);
    display.print(Date);
    break;
  case 2:
    display.setCursor(0, 0);
    display.print("Croaster IP Addr");
    display.setCursor(0, 1);
    display.print(WiFi.localIP().toString());
    break;
  case 3:
    display.setCursor(0, 0);
    display.print("R Temp: " + String(temp) + " C");
    display.setCursor(0, 1);
    display.print("R Humd: " + String(humd) + " %");
    break;
  default:
    break;
  }
  display.display();
}

void setup()
{
  Serial.begin(115200);
  button.setDebounceTime(50);

  display.init();
  display.backlight();

  splash();

  thermocouple_bt = new MAX6675_Thermocouple(SCK_PIN, CS_PIN_BT, SO_PIN);
  thermocouple_et = new MAX6675_Thermocouple(SCK_PIN, CS_PIN_ET, SO_PIN);

  wifiManager.setConfigPortalBlocking(false);
  wifiManager.setAPCallback(configModeCallback);

  display.clear();
  display.setCursor(0, 0);
  display.print("Firmware Version");
  display.setCursor(0, 1);
  display.print(String(fwVersion));
  display.display();
  delay(2000);

  if (wifiManager.autoConnect("Croaster"))
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
  timeClient.begin();
  webSocket.begin();                 // websocket Begin
  webSocket.onEvent(webSocketEvent); // set Event for websocket
  debugln("# Websocket is started");
}

void loop()
{
  button.loop();
  wifiManager.process();
  webSocket.loop();
  handleArtisan();
  timeUpdater();

  isWifiConnected = WiFi.status() == WL_CONNECTED;

  if (millis() - millisReadTemp >= 250)
  {
    millisReadTemp = millis();
    if (isEtBtSwapped)
    {
      temp_bt = thermocouple_et->readCelsius();
      temp_et = thermocouple_bt->readCelsius();
    }
    else
    {
      temp_et = thermocouple_et->readCelsius();
      temp_bt = thermocouple_bt->readCelsius();
    }

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

    humd = dht.readHumidity();
    temp = dht.readTemperature();

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
    millisUpdateDisplay = millis();

    getRor(temp_et, temp_bt, timer);

    updateDisplay();
  }

  if (millis() - millisWebSocket >= intervalSendData)
  {
    millisWebSocket = millis();

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
    debugln("# isEtBtSwapped: " + String(isEtBtSwapped));
    debugln("# intervalSendData: " + String(intervalSendData));
    debugln("# Json Data: " + jsonData);
  }

  if (button.isPressed())
  {
    if (++selectedAdditonalDisplay == 4)
      selectedAdditonalDisplay = 0;
  }
}