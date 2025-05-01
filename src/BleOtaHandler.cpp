#if defined(ESP32)

#include "BleOtaHandler.h"

BleOtaHandler::BleOtaHandler(BLECharacteristic *characteristic)
    : _characteristic(characteristic) {}

void BleOtaHandler::reset()
{
    otaState = State::Idle;
    otaWrittenSize = 0;
    expectedPacketId = 0;
    otaTotalSize = 0;
}

bool BleOtaHandler::beginOtaSession(const char *command)
{
    const char *prefix = "IOT47_BLE_OTA_BEGIN:";
    size_t prefixLen = strlen(prefix);

    if (strncmp(command, prefix, prefixLen) != 0)
        return false;

    otaTotalSize = atoi(command + prefixLen);
    if (otaTotalSize == 0)
        return false;

    bool ok = Update.begin(otaTotalSize);
    if (!ok)
    {
        debugln("# [OTA] Update.begin failed");
        return false;
    }

    otaWrittenSize = 0;
    expectedPacketId = 0;
    otaState = State::Receiving;

    debugf("# [OTA] Started. Expecting %d bytes\n", otaTotalSize);
    sendResponse("OK\r\n");
    return true;
}

int BleOtaHandler::handlePacket(const uint8_t *data, size_t len)
{
    if (otaState != State::Receiving)
        return 0;

    if (len < 4)
        return 0;

    uint16_t packetId = (data[0] << 8) | data[1];
    uint16_t chunkSize = (data[2] << 8) | data[3];

    if (packetId != expectedPacketId || len < 4 + chunkSize)
    {
        debugf("# [OTA] Packet error. Got ID: %d, Expected: %d, Size: %d, Len: %d\n", packetId, expectedPacketId, chunkSize, len);
        sendResponse("Fail\r\n");
        return -1;
    }

    size_t written = Update.write((uint8_t *)(data + 4), chunkSize);
    if (written != chunkSize)
    {
        debugf("# [OTA] Write error: expected %d, wrote %d\n", chunkSize, written);
        sendResponse("Fail\r\n");
        return -1;
    }

    otaWrittenSize += written;
    expectedPacketId++;

    debugf("# [OTA] Written %d / %d bytes\n", otaWrittenSize, otaTotalSize);

    if (otaWrittenSize >= otaTotalSize)
    {
        sendResponse("OTA DONE\r\n");
        otaState = State::Done;
        finishOta();
        return 2;
    }

    return 1;
}

void BleOtaHandler::sendResponse(const char *msg)
{
    if (_characteristic)
    {
        _characteristic->setValue(msg);
        _characteristic->notify();
    }
    debugln(String("# [OTA Response] ") + msg);
}

void BleOtaHandler::finishOta()
{
    if (Update.end() && Update.isFinished())
    {
        debugln("# [OTA] Update finished, restarting...");
        delay(1000);
        ESP.restart();
    }
    else
    {
        debugln("# [OTA] Update failed to finalize.");
    }
}

#endif