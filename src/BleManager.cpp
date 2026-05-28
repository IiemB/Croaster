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

        if (parent->otaHandler.isReceiving())
        {
            debugln("# [OTA] BLE disconnected during OTA - restarting...");
            parent->displayManager->updatingStatusToggle(false);
            restartESP();
            return;
        }

        BLEDevice::startAdvertising();
    }
};

class BleManager::CharacteristicCallbacks : public BLECharacteristicCallbacks
{
    BleManager *parent;

public:
    CharacteristicCallbacks(BleManager *parent) : parent(parent) {}
    void onWrite(BLECharacteristic *pCharacteristic) override
    {
        // If OTA is in progress, treat incoming data as a binary firmware chunk
        if (parent->otaHandler.isReceiving())
        {
            uint8_t *data = pCharacteristic->getData();
            size_t len = pCharacteristic->getLength();

            bool handled = parent->otaHandler.handleBinaryBle(data, len, pCharacteristic);

            if (handled)
            {
                int progress = int((double(parent->otaHandler.getWritten()) / double(parent->otaHandler.getTotal())) * 100.0);

                parent->displayManager->updatingStatusToggle(true);

                parent->displayManager->updateFirmwareUpdateProgress(progress);
            }

            return;
        }

        String raw = pCharacteristic->getValue().c_str();

        // Handle OTA begin command
        if (raw.startsWith("OTA_BEGIN:"))
        {
            uint32_t size = raw.substring(10).toInt();

            parent->otaHandler.begin(size);

            debugln("# [BLE] " + raw);

            return;
        }

        String response;

        if (parent->commandHandler->handle(raw, response))
        {
            if (!response.isEmpty())
            {
                pCharacteristic->setValue(response.c_str());
                pCharacteristic->notify();
            }

            debugln("# [CMD-BLE] " + raw);
        }
    }
};

BleManager::BleManager(CroasterCore &croaster, CommandHandler &commandHandler, DisplayManager &displayManager)
    : croaster(&croaster), commandHandler(&commandHandler), displayManager(&displayManager) {}

void BleManager::begin()
{
    BLEDevice::init(croaster->ssidName().c_str());

    BLEDevice::setMTU(517);

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
    pDataCharacteristic->setCallbacks(new CharacteristicCallbacks(this));

    pService->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    BLEDevice::startAdvertising();

    debugln("# BLE Server ready");
}

void BleManager::loop()
{
    otaHandler.checkTimeout();

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

    if (otaHandler.isReceiving())
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
