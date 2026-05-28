#include "OtaHandler.h"
#include <ArduinoJson.h>

OtaHandler::OtaHandler() {}

void OtaHandler::begin(uint32_t totalSize)
{
    resetState();

    if (Update.begin(totalSize))
    {
        this->totalSize = totalSize;
        written = 0;
        lastChunkTime = millis();
        state = State::Receiving;
        debugln("# [OTA] Begin, total size: " + String(totalSize));
    }
    else
    {
        debugln("# [OTA] Failed to begin update");
        debugln("# [OTA] Restarting device...");
        restartESP();
    }
}

bool OtaHandler::handleBinary(uint8_t *data, size_t len, WebSocketsServer &server, uint8_t clientId)
{
    JsonDocument doc;

    String jsonOutput;

    if (state != State::Receiving)
        return false;

    size_t writtenChunk = Update.write(data, len);
    if (writtenChunk != len)
    {

        doc["status"] = "failed";

        serializeJson(doc, jsonOutput);

        server.sendTXT(clientId, jsonOutput);

        finalize(true);

        return false;
    }

    written += writtenChunk;

    lastChunkTime = millis();

    bool isFinished = written >= totalSize;

    doc["status"] = "receiving";

    if (isFinished)
        doc["status"] = "done";
    else
        doc["status"] = "receiving";

    doc["written"] = double(written);

    doc["totalSize"] = double(totalSize);

    serializeJson(doc, jsonOutput);

    server.sendTXT(clientId, jsonOutput);

    debugln("# [OTA-JSON] " + jsonOutput);

    if (isFinished)
        finalize();

    return true;
}

void OtaHandler::finalize(bool hasError)
{
    if (hasError)
    {
        debugln("# [OTA] Update failed: Write failed");

#if defined(ESP32)
        Update.abort();
        debugln("# [OTA] Update aborted");
#endif

        debugln("# Restarting...");

        delay(3000);

        restartESP();

        return;
    }

    if (Update.end(true))
    {
        debugln("# [OTA] Update finished");
    }
    else
    {
        debugln("# [OTA] Update failed: OTA finalize failed");
#if defined(ESP32)
        Update.abort();
#endif
    }

    debugln("# [OTA] Restarting...");

    delay(3000);

    restartESP();
}

void OtaHandler::resetState()
{
    state = State::Idle;
    totalSize = 0;
    written = 0;
    lastChunkTime = 0;
}

void OtaHandler::checkTimeout()
{
    if (state != State::Receiving)
        return;

    if (millis() - lastChunkTime > OTA_TIMEOUT_MS)
    {
        debugln("# [OTA] Timeout - no data received, aborting...");
        finalize(true);
    }
}

#if defined(ESP32)
bool OtaHandler::handleBinaryBle(uint8_t *data, size_t len, BLECharacteristic *pChar)
{
    JsonDocument doc;

    String jsonOutput;

    if (state != State::Receiving)
        return false;

    size_t writtenChunk = Update.write(data, len);
    if (writtenChunk != len)
    {
        doc["status"] = "failed";

        serializeJson(doc, jsonOutput);

        pChar->setValue(jsonOutput.c_str());
        pChar->notify();

        finalize(true);

        return false;
    }

    written += writtenChunk;

    lastChunkTime = millis();

    bool isFinished = written >= totalSize;

    if (isFinished)
        doc["status"] = "done";
    else
        doc["status"] = "receiving";

    doc["written"] = double(written);

    doc["totalSize"] = double(totalSize);

    serializeJson(doc, jsonOutput);

    pChar->setValue(jsonOutput.c_str());
    pChar->notify();

    debugln("# [OTA-BLE-JSON] " + jsonOutput);

    if (isFinished)
        finalize();

    return true;
}
#endif

bool OtaHandler::isReceiving() const
{
    return state == State::Receiving;
}

uint32_t OtaHandler::getTotal() const
{
    return totalSize;
}

uint32_t OtaHandler::getWritten() const
{
    return written;
}
