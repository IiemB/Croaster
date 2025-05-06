#if defined(ESP32)

#include "BleManager.h"
#include "WiFiManagerUtil.h"

class BleManager::ServerCallbacks : public BLEServerCallbacks
{
    BleManager *parent;

public:
    ServerCallbacks(BleManager *parent) : parent(parent) {}
    void onConnect(BLEServer *) override
    {
        debugln("# BLE Client Connected");
        parent->clientConnected = true;
    }

    void onDisconnect(BLEServer *) override
    {
        debugln("# BLE Client Disconnected");
        parent->clientConnected = false;
        BLEDevice::startAdvertising();
    }
};

class BleManager::CharacteristicCallbacks : public BLECharacteristicCallbacks
{
    CommandHandler *commandHandler;

public:
    CharacteristicCallbacks(CommandHandler *handler) : commandHandler(handler) {}
    void onWrite(BLECharacteristic *pCharacteristic) override
    {
        String raw = pCharacteristic->getValue().c_str();

        bool restart = false, erase = false;
        String response;

        if (commandHandler->handle(raw, response, restart, erase))
        {
            if (!response.isEmpty())
            {
                pCharacteristic->setValue(response.c_str());
                pCharacteristic->notify();
            }

            if (erase)
                eraseESP();

            if (restart)
                restartESP();

            debugln("# [CMD-BLE] " + raw);
        }
    }
};

BleManager::BleManager(CroasterCore &croaster, CommandHandler &commandHandler)
    : croaster(&croaster), commandHandler(&commandHandler) {}

void BleManager::begin()
{
    BLEDevice::init(croaster->ssidName().c_str());

    pServer = BLEDevice::createServer();

    pServer->setCallbacks(new ServerCallbacks(this));

    BLEService *pService = pServer->createService(SERVICE_UUID);
    pDataCharacteristic = pService->createCharacteristic(
        DATA_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_WRITE_NR);

    pDataCharacteristic->addDescriptor(new BLE2902());
    pDataCharacteristic->setCallbacks(new CharacteristicCallbacks(commandHandler));

    pService->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    BLEDevice::startAdvertising();

    debugln("# BLE Server ready");
}

void BleManager::loop()
{
    broadcastData();
}

bool BleManager::isClientConnected() const
{
    return clientConnected;
}

void BleManager::broadcastData()
{
    if (!clientConnected || !pDataCharacteristic)
        return;

    unsigned long now = millis();

    unsigned long interval = croaster->intervalSendData() * 1000;

    if (now - lastSend >= interval)
    {
        lastSend = now;

        String jsonData = croaster->getJsonData();

        sendData(jsonData);

        debugln("# [BLE-JSON] " + jsonData);
        debugln("");
    }
}

void BleManager::sendData(const String &data)
{
    if (clientConnected && pDataCharacteristic)
    {
        pDataCharacteristic->setValue(data.c_str());
        pDataCharacteristic->notify();
    }
}

#endif
