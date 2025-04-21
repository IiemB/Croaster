#if defined(ESP32)

#include "BleManager.h"
#include "WiFiManagerUtil.h"

BLEServer *pServer = nullptr;
BLECharacteristic *pDataCharacteristic = nullptr;
uint32_t passkey = 123456;

class MyServerCallbacks : public BLEServerCallbacks
{
public:
    bool *connectedFlag;

    MyServerCallbacks(bool *flag) : connectedFlag(flag) {}

    void onConnect(BLEServer *) override
    {
        *connectedFlag = true;
        debugln("# BLE Client Connected");
    }

    void onDisconnect(BLEServer *) override
    {
        *connectedFlag = false;
        debugln("# BLE Client Disconnected");
        BLEDevice::startAdvertising();
    }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks
{
private:
    CroasterCore *croaster;

public:
    MyCharacteristicCallbacks(CroasterCore *croaster) : croaster(croaster) {}

    void onWrite(BLECharacteristic *pCharacteristic) override
    {
        String raw = pCharacteristic->getValue().c_str();
        StaticJsonDocument<96> doc;

        if (deserializeJson(doc, raw))
        {
            debugln("# [BLE] Invalid JSON command");
            return;
        }

        if (doc["command"].is<String>())
        {
            String command = doc["command"];
            String res = croaster->getJsonData(command);

            pCharacteristic->setValue(res.c_str());
            pCharacteristic->notify();

            if (command == "restartesp")
                restartESP();
            else if (command == "erase")
                eraseESP();
            else if (command == "dummyOn")
            {
                croaster->useDummyData = true;
                croaster->resetHistory();
            }
            else if (command == "dummyOff")
            {
                croaster->useDummyData = false;
                croaster->resetHistory();
            }
        }

        if (doc["command"].is<JsonObject>())
        {
            JsonObject cmd = doc["command"];
            if (cmd.containsKey("tempUnit"))
            {
                String unit = cmd["tempUnit"];
                croaster->changeTemperatureUnit(unit);
                String res = croaster->getJsonData(unit);
                pCharacteristic->setValue(res.c_str());
                pCharacteristic->notify();
            }
        }
    }
};

void setupBLE(CroasterCore &croaster, bool &bleDeviceConnected)
{
    BLEDevice::init(croaster.ssidName.c_str());
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks(&bleDeviceConnected));

    BLEService *pService = pServer->createService(SERVICE_UUID);
    pDataCharacteristic = pService->createCharacteristic(
        DATA_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_WRITE_NR);

    pDataCharacteristic->addDescriptor(new BLE2902());
    pDataCharacteristic->setCallbacks(new MyCharacteristicCallbacks(&croaster));

    BLESecurity *pSecurity = new BLESecurity();
    pSecurity->setStaticPIN(passkey);
    pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);

    pService->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    BLEDevice::startAdvertising();

    debugln("# BLE Server ready");
}

#endif