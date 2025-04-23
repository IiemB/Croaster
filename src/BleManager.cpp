#if defined(ESP32)

#include "CommandHandler.h"
#include "BleManager.h"
#include "WiFiManagerUtil.h"

BLEServer *pServer = nullptr;
BLECharacteristic *pDataCharacteristic = nullptr;
uint32_t passkey = 123456;

class MyServerCallbacks : public BLEServerCallbacks
{
public:
    void onConnect(BLEServer *) override
    {
        debugln("# BLE Client Connected");
    }

    void onDisconnect(BLEServer *) override
    {
        debugln("# BLE Client Disconnected");
        BLEDevice::startAdvertising();
    }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks
{
private:
    CommandHandler *commandHandler;

public:
    MyCharacteristicCallbacks(CommandHandler *commandHandler) : commandHandler(commandHandler)
    {
    }

    void onWrite(BLECharacteristic *pCharacteristic) override
    {
        String raw = pCharacteristic->getValue().c_str();

        bool restart = false, erase = false;
        String response;

        if (commandHandler->handle(raw, response, restart, erase))
        {

            debugln("# [BLE] " + raw);

            if (!response.isEmpty())
            {
                pCharacteristic->setValue(response.c_str());
                pCharacteristic->notify();
            }
            if (erase)
                eraseESP();
            if (restart)
                restartESP();
        }
    }
};

void setupBLE(String name, CommandHandler &commandHandler)
{
    BLEDevice::init(name.c_str());
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);
    pDataCharacteristic = pService->createCharacteristic(
        DATA_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_WRITE_NR);

    pDataCharacteristic->addDescriptor(new BLE2902());
    pDataCharacteristic->setCallbacks(new MyCharacteristicCallbacks(&commandHandler));

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