#pragma once

#if defined(ESP32)

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "CroasterCore.h"
#include "DisplayManager.h"
#include "Constants.h"
#include "CommandHandler.h"

class BleManager
{
public:
    BleManager(CroasterCore &croaster, CommandHandler &commandHandler);

    void begin();

    void loop();

    bool isClientConnected() const;

private:
    BLEServer *pServer = nullptr;
    BLECharacteristic *pDataCharacteristic = nullptr;

    CommandHandler *commandHandler = nullptr;
    CroasterCore *croaster = nullptr;

    unsigned long lastSend = 0;

    bool clientConnected = false;

    void broadcastData();

    void sendData(const String &data);

    class ServerCallbacks;
    class CharacteristicCallbacks;
};

#endif
