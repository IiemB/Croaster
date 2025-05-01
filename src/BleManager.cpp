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
    BleManager *parent;

public:
    CharacteristicCallbacks(BleManager *parent, CommandHandler *handler) : parent(parent), commandHandler(handler) {}
    void onWrite(BLECharacteristic *pCharacteristic) override
    {
        String raw = pCharacteristic->getValue().c_str();

        if (parent->otaHandler)
        {
            std::vector<uint8_t> rx(pCharacteristic->getValue().begin(), pCharacteristic->getValue().end());
            const uint8_t *data = rx.data();
            size_t len = rx.size();

            String text = String(reinterpret_cast<const char *>(data), len);

            if (parent->otaHandler->getState() == BleOtaHandler::State::Idle &&
                parent->otaHandler->beginOtaSession(text.c_str()))
            {
                return;
            }

            if (parent->otaHandler->getState() == BleOtaHandler::State::Receiving)
            {
                int result = parent->otaHandler->handlePacket(data, len);
                if (result < 0)
                {
                    debugln("# OTA update failed. Restarting...");

                    restartESP();
                }

                return;
            }
        }

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
    pDataCharacteristic->setCallbacks(new CharacteristicCallbacks(this, commandHandler));

    pService->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    BLEDevice::startAdvertising();

    otaHandler = new BleOtaHandler(pDataCharacteristic);

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

    unsigned long interval = croaster->intervalSendData * 1000;

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
