#pragma once

#if defined(ESP32)

#include <Arduino.h>
#include <Update.h>
#include <BLECharacteristic.h>
#include "Constants.h"

class BleOtaHandler
{
public:
    enum class State
    {
        Idle,
        Receiving,
        Done
    };

    BleOtaHandler(BLECharacteristic *characteristic);

    void reset();
    bool beginOtaSession(const char *command);
    int handlePacket(const uint8_t *data, size_t len);

    State getState() const { return otaState; }

private:
    BLECharacteristic *_characteristic;
    State otaState = State::Idle;

    uint32_t otaTotalSize = 0;
    uint32_t otaWrittenSize = 0;
    uint16_t expectedPacketId = 0;

    void sendResponse(const char *msg);
    void finishOta();
};

#endif