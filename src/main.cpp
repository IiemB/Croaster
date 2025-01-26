#include <Arduino.h>
#include <SPI.h>
#include <DHT.h>
#include <MAX6675_Thermocouple.h>
#include <WiFiManager.h>
#include <WebSocketsServer.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

// Debug Macros
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)

// Pin Definitions
#define SCK_PIN D8
#define SO_PIN D7
#define CS_PIN_BT D5
#define CS_PIN_ET D6
#define DHT_PIN D4
#define DHTTYPE DHT11

// Croaster Class Definition
class Croaster
{
private:
  Thermocouple *thermocoupleBT;
  Thermocouple *thermocoupleET;
  DHT dht;

  float etHistory[60] = {}, btHistory[60] = {}, timeHistory[60] = {};
  bool historyInitialized = false;

  unsigned long lastSensorRead = 0;
  unsigned long lastRORUpdate = 0;

  bool useDummyData;

  float convertTemperature(float tempCelsius)
  {
    if (temperatureUnit == "F") // Fahrenheit
    {
      return (tempCelsius * 9.0 / 5.0) + 32.0;
    }
    else if (temperatureUnit == "K") // Kelvin
    {
      return tempCelsius + 273.15;
    }
    else // Celsius
    {
      return tempCelsius;
    }
  }

  // Reset history
  void resetHistory()
  {
    for (int i = 0; i < 60; i++)
    {
      etHistory[i] = tempET;
      btHistory[i] = tempBT;
      timeHistory[i] = timer;
    }

    historyInitialized = false;

    debugln("# Temperature histories reset due to unit change.");
  }

  void readSensors()
  {
    if (millis() - lastSensorRead < 250)
      return;
    lastSensorRead = millis();
    timer = millis() / 1000.0;

    if (useDummyData)
    {
      tempBT = random(30, 40);
      tempET = random(30, 40);
      humidity = random(30, 50);
      tempR = random(20, 30);
    }
    else
    {
      tempBT = thermocoupleBT->readCelsius();
      tempET = thermocoupleET->readCelsius();
      humidity = dht.readHumidity();
      tempR = dht.readTemperature();

      if (isnan(tempBT))
      {
        debugln("# Error: Failed to read BT!");
        tempBT = 0;
      }

      if (isnan(tempET))
      {
        debugln("# Error: Failed to read ET!");
        tempET = 0;
      }

      if (isnan(humidity) || isnan(tempR))
      {
        debugln("# Error: Failed to read DHT sensor!");
        humidity = 0;
        tempR = 0;
      }

      tempBT = convertTemperature(tempBT);
      tempET = convertTemperature(tempET);
      tempR = convertTemperature(tempR);
    }
  }

  void updateROR()
  {
    if (millis() - lastRORUpdate < 1000)
      return;

    lastRORUpdate = millis();

    if (!historyInitialized)
    {
      for (int i = 0; i < 60; i++)
      {
        etHistory[i] = tempET;
        btHistory[i] = tempBT;
        timeHistory[i] = timer;
      }

      historyInitialized = true;

      return;
    }

    for (int i = 0; i < 59; i++)
    {
      etHistory[i] = etHistory[i + 1];
      btHistory[i] = btHistory[i + 1];
      timeHistory[i] = timeHistory[i + 1];
    }

    etHistory[59] = tempET;
    btHistory[59] = tempBT;
    timeHistory[59] = timer;

    float deltaET = etHistory[59] - etHistory[0];
    float deltaBT = btHistory[59] - btHistory[0];
    float deltaTimer = timeHistory[59] - timeHistory[0];

    rorET = (deltaET / deltaTimer) * 60;
    rorBT = (deltaBT / deltaTimer) * 60;
  }

public:
  Croaster(bool dummyMode, const String &version) : dht(DHT_PIN, DHTTYPE), useDummyData(dummyMode), versionName(version)
  {
    thermocoupleBT = new MAX6675_Thermocouple(SCK_PIN, CS_PIN_BT, SO_PIN);
    thermocoupleET = new MAX6675_Thermocouple(SCK_PIN, CS_PIN_ET, SO_PIN);
  }

  ~Croaster()
  {
    delete thermocoupleBT;
    delete thermocoupleET;
  }

  String temperatureUnit = "C";

  String versionName;

  float timer = 0, rorET = 0, rorBT = 0, tempET = 0, tempBT = 0, humidity = 0, tempR = 0;
  unsigned long intervalSendData = 3000;
  int idJsonData = 0;

  const char *ssidName = "Croaster";

  void init()
  {
    dht.begin();
    debugln("# Croaster Initialized.");
  }

  void loop()
  {
    readSensors();
    updateROR();
  }

  void changeTemperatureUnit(String unit)
  {
    if (unit == "C" || unit == "F" || unit == "K")
    {
      temperatureUnit = unit;

      resetHistory();

      debugln("# Temperature unit set to " + unit);
    }
    else
    {
      debugln("# Invalid temperature unit. Use C, F, or K.");
    }
  }

  String getJsonData(const String &message = "")

  {
    StaticJsonDocument<384> doc;

    doc["id"] = idJsonData;
    doc["roasterID"] = ssidName;

    if (!message.isEmpty())
      doc["message"] = message;

    JsonObject data = doc.createNestedObject("data");
    data["BT"] = tempBT;
    data["ET"] = tempET;

    JsonObject croaster = doc.createNestedObject("croaster");

    croaster["version"] = versionName;
    croaster["interval"] = intervalSendData;
    croaster["timer"] = timer;
    croaster["tempET"] = tempET;
    croaster["tempBT"] = tempBT;
    croaster["rorET"] = rorET;
    croaster["rorBT"] = rorBT;
    croaster["humidity"] = humidity;
    croaster["tempR"] = tempR;
    croaster["tempUnit"] = String(temperatureUnit);

    String jsonOutput;

    serializeJson(doc, jsonOutput);

    return jsonOutput;
  }
};

// Globals
Croaster croaster(true, "V2.4");

WiFiManager wifiManager;
WebSocketsServer webSocket(81);
LiquidCrystal_I2C display(0x27, 16, 2);

unsigned long lastWebSocketSend = 0;
unsigned long lastDisplayUpdate = 0;

bool isWifiConnected = false;
bool isCroasterConnected = false;
bool showIp = false;

String socketEventMessage = "";

// Function Prototypes
void updateDisplay();
void sendDataToCroaster();
void handleWebSocketCommand(const String &cmd, uint8_t num);
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
void configModeCallback(WiFiManager *myWiFiManager);

// Function Implementations
void setup()
{
  Serial.begin(115200);
  display.init();
  display.backlight();
  croaster.init();

  wifiManager.setConfigPortalBlocking(false);
  wifiManager.setAPCallback(configModeCallback);
  if (wifiManager.autoConnect(croaster.ssidName))
  {
    isWifiConnected = true;
    debugln("# WiFi Connected");
  }

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
  sendDataToCroaster();
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
  if (showIp && isWifiConnected && !isCroasterConnected)
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

void sendDataToCroaster()
{
  if (millis() - lastWebSocketSend < croaster.intervalSendData)
    return;
  lastWebSocketSend = millis();

  String jsonData = croaster.getJsonData(socketEventMessage);

  if (isWifiConnected && isCroasterConnected)
    webSocket.broadcastTXT(jsonData);

  debugln("");
  if (isWifiConnected)
    debugln("# " + WiFi.localIP().toString());
  debugln("# Json Data: " + jsonData);

  socketEventMessage = "";
}

void handleWebSocketCommand(const String &cmd, uint8_t num)
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

    if (command == "getData")
    {
      croaster.idJsonData = request["id"];

      socketEventMessage = "";
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
    isCroasterConnected = false;
    debugln("# WebSocket disconnected");
    break;
  case WStype_CONNECTED:
    isCroasterConnected = true;
    debugln("# WebSocket connected");
    break;
  case WStype_TEXT:
    handleWebSocketCommand(String((char *)payload), num);
    break;
  default:
    break;
  }
}

void configModeCallback(WiFiManager *myWiFiManager)
{
  debugln("# Config mode: " + WiFi.softAPIP().toString());
}
