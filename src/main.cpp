#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <Croaster.h>

// BLE UUIDs
#define SERVICE_UUID "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "abcd1234-5678-90ab-cdef-1234567890ab"

// Global Variables
BLEServer *pServer = nullptr;
BLECharacteristic *pCharacteristic = nullptr;
bool deviceConnected = false;

uint32_t passkey = 123456; // Set your PIN here

void ledToggle(bool isOn)
{
    if (isOn)
    {
        digitalWrite(PIN_NEOPIXEL, LOW);

        return;
    }

    digitalWrite(PIN_NEOPIXEL, HIGH);
}

// Callback for Client Connection
class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer) override
    {
        deviceConnected = true;
        debugln("✅ Client Connected");
    }

    void onDisconnect(BLEServer *pServer) override
    {
        deviceConnected = false;
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
            String command = "";
            for (char c : receivedData)
            {
                command += (char)c;
            }

            Serial.print("📥 Received: ");
            debugln(command);

            // Handle Commands and Send Responses
            if (command == "LED_ON")
            {
                ledToggle(true);

                pCharacteristic->setValue("💡 LED is ON");

                pCharacteristic->notify();

                debugln("💡 Turning LED ON");
            }
            else if (command == "LED_OFF")
            {
                ledToggle(false);

                pCharacteristic->setValue("💡 LED is OFF");
                pCharacteristic->notify();
                debugln("💡 Turning LED OFF");
            }
            else
            {
                pCharacteristic->setValue("⚠️ Unknown Command");
                pCharacteristic->notify();
                debugln("⚠️ Unknown Command");
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
    pinMode(PIN_NEOPIXEL, OUTPUT);

    ledToggle(false);

    delay(1000);
    debugln("🚀 Starting BLE Server...");

    // Initialize BLE
    BLEDevice::init("Makergo C3 Logger");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create BLE Service and Characteristic
    BLEService *pService = pServer->createService(SERVICE_UUID);

    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_WRITE_NR |
            BLECharacteristic::PROPERTY_NOTIFY);

    pCharacteristic->addDescriptor(new BLE2902());

    // Enable Security
    BLESecurity *pSecurity = new BLESecurity();
    pSecurity->setStaticPIN(passkey);
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);

    // Add Descriptor for Notifications
    pCharacteristic->addDescriptor(new BLE2902());
    pCharacteristic->setValue("Hello from Makergo C3!");
    pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

    // Start Service and Advertising
    pService->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    BLEDevice::startAdvertising();

    debugln("📡 Bluetooth Logger ready and waiting for commands");
}

void loop()
{
    if (deviceConnected)
    {
        std::string logMessage = "Log: " + std::to_string(millis());
        pCharacteristic->setValue(logMessage);
        pCharacteristic->notify();
        debugln(("[BLE] " + logMessage).c_str());
        delay(1000);
    }

    Serial.flush();
}
