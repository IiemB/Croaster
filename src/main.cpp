#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <SPI.h>
#include <WiFiManager.h>
#include <WebSocketsServer.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <Croaster.h>

// BLE UUIDs
#define SERVICE_UUID "1cc9b045-a6e9-4bd5-b874-07d4f2d57843"
#define CHARACTERISTIC_UUID "d56d0059-ad65-43f3-b971-431d48f89a69"

// Global Variables
BLEServer *pServer = nullptr;
BLECharacteristic *pCharacteristic = nullptr;
bool bleDeviceConnected = false;

Croaster croaster(2.43, true);

WiFiManager wifiManager;
WebSocketsServer webSocket(81);
LiquidCrystal_I2C display(0x27, 16, 2);

uint32_t passkey = 123456; // Set your PIN here

unsigned long lastWebSocketSend = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastShowBleLogs = 0;

bool showIp = false;

String socketEventMessage = "";

// Function Prototypes
void updateDisplay();
void broadcastData();
void handleWebSocketEvent(const String &cmd, uint8_t num);
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
void configModeCallback(WiFiManager *myWiFiManager);
void showBLELogs();
void restartESP();
void eraseESP();

// Callback for Client Connection
class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer) override
    {
        bleDeviceConnected = true;
        debugln("✅ Client Connected");
    }

    void onDisconnect(BLEServer *pServer) override
    {
        bleDeviceConnected = false;
        debugln("❌ Client Disconnected");
        BLEDevice::startAdvertising(); // Restart advertising
    }
};

// Callback for Data Written from Client
class MyCharacteristicCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic) override
    {
        std::string receivedData = pCharacteristic->getValue();

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
        debugln("🔒 Passkey Request");
        return passkey;
    }
    void onPassKeyNotify(uint32_t pass_key)
    {
        debugf("🔑 Passkey: %06d\n", pass_key);
    }
    bool onConfirmPIN(uint32_t pin)
    {
        debugln("✅ PIN Confirmed");
        return pin == passkey;
    }
    bool onSecurityRequest()
    {
        debugln("🔐 Security Request");
        return true;
    }
    void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl)
    {
        if (cmpl.success)
        {
            debugln("✅ Authentication Success");
        }
        else
        {
            debugln("❌ Authentication Failed");
        }
    }
};

void setup()
{
    Serial.begin(115200);

    while (!Serial)
        ;

    delay(1000);

    display.init();

    display.backlight();

    debugln("🚀 Starting BLE Server...");

    // Initialize BLE
    BLEDevice::init(croaster.ssidName.c_str());
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create BLE Service and Characteristic
    BLEService *pService = pServer->createService(SERVICE_UUID);

    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_WRITE_NR);

    // Enable Security
    BLESecurity *pSecurity = new BLESecurity();
    pSecurity->setStaticPIN(passkey);
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);

    // Add Descriptor for Notifications
    pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

    // Start Service and Advertising
    pService->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    BLEDevice::startAdvertising();

    debugln("📡 Bluetooth Logger ready and waiting for commands");

    wifiManager.setDebugOutput(false);
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
    showBLELogs();
}

// Function Implementations

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

            eraseESP();

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

void showBLELogs()
{
    if (millis() - lastShowBleLogs < 1500)
        return;

    lastShowBleLogs = millis();

    if (bleDeviceConnected)
    {
        std::string logMessage = "Log: " + std::to_string(millis());
        pCharacteristic->setValue(logMessage);
        pCharacteristic->notify();
        debugln(("[BLE] " + logMessage).c_str());
        delay(1000);
    }
}

void restartESP()
{
    ESP.restart();
}

void eraseESP()
{
    wifiManager.erase();

    restartESP();
}
