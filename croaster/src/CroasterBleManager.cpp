#include "CroasterBleManager.h"
#include "CroasterWiFiManager.h"

#if CROASTER_HAS_BLE

class CroasterBleManager::ServerCallbacks : public BLEServerCallbacks
{
    CroasterBleManager *parent;

public:
    ServerCallbacks(CroasterBleManager *parent) : parent(parent) {}
    void onConnect(BLEServer *) override
    {
        debugln("# BLE Client Connected");
        parent->clientConnected = true;
    }

    void onDisconnect(BLEServer *) override
    {
        debugln("# BLE Client Disconnected");
        parent->clientConnected = false;

        if ((parent->display && parent->display->isFirmwareUpdating()) || parent->otaHandler.isReceiving())
        {
            debugln("# [OTA] BLE disconnected during OTA - restarting...");
            if (parent->display)
                parent->display->updatingStatusToggle(false);
            CroasterWiFiManager::restart();
            return;
        }

        BLEDevice::startAdvertising();
    }
};

class CroasterBleManager::CharacteristicCallbacks : public BLECharacteristicCallbacks
{
    CroasterBleManager *parent;

public:
    CharacteristicCallbacks(CroasterBleManager *parent) : parent(parent) {}
    void onWrite(BLECharacteristic *pCharacteristic) override
    {
        // Check OTA first to avoid parsing binary firmware bytes as a String.
        if (parent->otaHandler.isReceiving())
        {
            String result = parent->otaHandler.handleBinary(pCharacteristic->getData(), pCharacteristic->getLength());

            pCharacteristic->setValue(result.c_str());
            pCharacteristic->notify();

            int progress = int((double(parent->otaHandler.getWritten()) / double(parent->otaHandler.getTotal())) * 100.0);

            if (parent->display)
            {
                parent->display->updatingStatusToggle(true);
                parent->display->updateFirmwareUpdateProgress(progress);
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

        // Defer JSON command handling to loop(). This callback runs on the
        // Bluedroid BTC task (~3KB stack); parsing with ArduinoJson here
        // overflows it and crashes as an unrelated "xQueueGenericSend" assert
        // (esp-idf bluedroid bug that only shows on debug builds / single-core).
        parent->pendingWriteData = raw;
        parent->hasPendingWrite = true;
    }
};

CroasterBleManager::CroasterBleManager(CroasterCore &croaster, CroasterCommandHandler &commandHandler, CroasterDisplay *display)
    : croaster(&croaster), commandHandler(&commandHandler), display(display) {}

void CroasterBleManager::begin()
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

void CroasterBleManager::loop()
{
    // Handle a deferred BLE write (see onWrite): run the JSON command handler
    // on the Arduino loop task, which has a large stack, never on the
    // Bluedroid BTC task.
    if (hasPendingWrite)
    {
        hasPendingWrite = false;

        String raw = pendingWriteData;
        String response;

        if (commandHandler->handle(raw, response))
        {
            if (!response.isEmpty())
            {
                pDataCharacteristic->setValue(response.c_str());
                pDataCharacteristic->notify();
            }

            debugln("# [CMD-BLE] " + raw);

            if (!response.isEmpty())
                debugln("# [CMD-BLE-RESP] " + response);
        }
    }

    broadcastData();

    otaHandler.handleState();
}

bool CroasterBleManager::isClientConnected() const
{
    return clientConnected;
}

void CroasterBleManager::broadcastData()
{
    if (!clientConnected || !pDataCharacteristic || otaHandler.isReceiving())
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

void CroasterBleManager::sendData(const String &data)
{
    if (clientConnected && pDataCharacteristic)
    {
        pDataCharacteristic->setValue(data.c_str());
        pDataCharacteristic->notify();
    }
}

#endif
