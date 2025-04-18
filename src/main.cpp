#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <WiFiManager.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <Croaster.h>
#include <DisplayManager.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// BLE UUIDs
#define SERVICE_UUID "1cc9b045-a6e9-4bd5-b874-07d4f2d57843"
#define DATA_UUID "d56d0059-ad65-43f3-b971-431d48f89a69"

double version = 2.52;

DisplayManager displayManager(SCREEN_WIDTH, SCREEN_HEIGHT, version);

Croaster croaster(version, true);

// Global Variables
BLEServer *pServer = nullptr;
BLECharacteristic *pDataCharacteristic = nullptr;

bool bleDeviceConnected = false;

WiFiManager wifiManager;
WebSocketsServer webSocket(81);

uint32_t passkey = 123456; // Set your PIN here

unsigned long lastWebSocketSend = 0;
unsigned long lastDisplayUpdate = 0;

unsigned long lastBlinkLed = 0;
bool isLedOn = false;

String socketEventMessage = "";

// Function Prototypes
void broadcastData();
void handleWebSocketEvent(const String &cmd, uint8_t num);
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
void configModeCallback(WiFiManager *myWiFiManager);
void restartESP();
void eraseESP();
void blinkLED();

// Callback for Client Connection
class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer) override
  {
    bleDeviceConnected = true;
    debugln("# Client Connected");
  }

  void onDisconnect(BLEServer *pServer) override
  {
    bleDeviceConnected = false;
    debugln("# Client Disconnected");
    BLEDevice::startAdvertising(); // Restart advertising
  }
};

// Callback for Data Written from Client
class MyCharacteristicCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic) override
  {
    String receivedData = (pCharacteristic->getValue()).c_str();

    if (receivedData.length() > 0)
    {
      // Convert HEX to string
      String cmd = "";

      for (char c : receivedData)
      {
        cmd += (char)c;
      }

      StaticJsonDocument<96> request;

      if (deserializeJson(request, cmd))
      {
        debugln("# Invalid JSON command");

        return;
      }

      debugln("# [BLE] [cmd] " + cmd);

      // Handle Commands and Send Responses

      if (request["command"].is<String>())
      {
        String command = request["command"];

        if (command == "getData")
        {
          String jsonData = croaster.getJsonData(command);

          pCharacteristic->setValue(jsonData.c_str());

          pCharacteristic->notify();

          return;
        }

        if (command == "restartesp")
        {
          String jsonData = croaster.getJsonData(command);

          pCharacteristic->setValue(jsonData.c_str());

          pCharacteristic->notify();

          restartESP();

          return;
        }

        if (command == "erase")
        {
          String jsonData = croaster.getJsonData(command);

          pCharacteristic->setValue(jsonData.c_str());

          pCharacteristic->notify();

          eraseESP();

          return;
        }

        if (command == "dummyOn")
        {
          croaster.useDummyData = true;
          croaster.resetHistory();

          return;
        }

        if (command == "dummyOff")
        {
          croaster.useDummyData = false;
          croaster.resetHistory();

          return;
        }
      }

      if (request["command"].is<JsonObject>())
      {
        JsonObject command = request["command"];

        if (command.containsKey("tempUnit") && command["tempUnit"].is<String>())
        {
          String tempUnit = command["tempUnit"]; // Read tempUnit value

          croaster.changeTemperatureUnit(tempUnit);

          String jsonData = croaster.getJsonData(tempUnit);

          pCharacteristic->setValue(jsonData.c_str());

          pCharacteristic->notify();

          return;
        }
      }
    }
  }
};

class MySecurityCallbacks : public BLESecurityCallbacks
{
  uint32_t onPassKeyRequest()
  {
    debugln("# Passkey Request");
    return passkey;
  }
  void onPassKeyNotify(uint32_t pass_key)
  {
    debugln("# Passkey: %06d\n" + pass_key);
  }
  bool onConfirmPIN(uint32_t pin)
  {
    debugln("# PIN Confirmed");
    return pin == passkey;
  }
  bool onSecurityRequest()
  {
    debugln("# Security Request");
    return true;
  }
  void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl)
  {
    if (cmpl.success)
    {
      debugln("# Authentication Success");
    }
    else
    {
      debugln("# Authentication Failed");
    }
  }
};

void setup()
{
  Serial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(LED_BUILTIN, LOW);

  delay(1000);

  debugln("# Setting up WiFi Manager");

  wifiManager.setDebugOutput(true);
  wifiManager.setConfigPortalBlocking(false);
  wifiManager.setAPCallback(configModeCallback);

  wifiManager.setClass("invert");

  wifiManager.setConnectTimeout(10);

  // set custom ip for portal
  wifiManager.setAPStaticIPConfig(IPAddress(10, 0, 1, 1), IPAddress(10, 0, 1, 1), IPAddress(255, 255, 255, 0));

  if (wifiManager.autoConnect(croaster.ssidName.c_str()))
    debugln("# WiFi Connected");

  debugln("# Starting BLE Server");

  // Initialize BLE
  BLEDevice::init(croaster.ssidName.c_str());
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create BLE Service and Characteristics
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pDataCharacteristic = pService->createCharacteristic(
      DATA_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_NOTIFY |
          BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_WRITE_NR);

  // Enable Security
  BLESecurity *pSecurity = new BLESecurity();
  pSecurity->setStaticPIN(passkey);
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);

  // Add Descriptor for Notifications
  pDataCharacteristic->addDescriptor(new BLE2902());
  pDataCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

  // Start Service and Advertising
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  BLEDevice::startAdvertising();

  debugln("# Bluetooth Logger ready and waiting for commands");

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  debugln("# WebSocket started");

  // Initialize OLED display
  if (!displayManager.begin())
  {
    for (;;)
      ; // Halt if display fails
  }
}

void loop()
{
  wifiManager.process();
  webSocket.loop();
  croaster.loop();
  broadcastData();
  blinkLED();
}

// Function Implementations

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

  displayManager.updateDisplay(croaster.tempET, croaster.tempBT, croaster.temperatureUnit);

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

      eraseESP();

      return;
    }

    if (command == "dummyOn")
    {
      croaster.useDummyData = true;
      croaster.resetHistory();

      return;
    }

    if (command == "dummyOff")
    {
      croaster.useDummyData = false;
      croaster.resetHistory();

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

    croaster.ipAddress = WiFi.localIP().toString();

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

void restartESP()
{
  wifiManager.reboot();
}

void eraseESP()
{
  wifiManager.erase();

  restartESP();
}

void blinkLED()
{
  unsigned long intervalLED = 500;

  if (WiFi.isConnected() && bleDeviceConnected)
  {
    intervalLED = 1000;
  }
  else
  {
    if (WiFi.isConnected())
      intervalLED = 3000;

    if (bleDeviceConnected)
      intervalLED = 2000;
  }

  if (millis() - lastBlinkLed < intervalLED)
    return;

  lastBlinkLed = millis();

  if (isLedOn)
  {
    digitalWrite(LED_BUILTIN, LOW);
    isLedOn = false;
  }
  else
  {
    digitalWrite(LED_BUILTIN, HIGH);
    isLedOn = true;
  }
}
