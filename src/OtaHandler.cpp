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

OtaResult OtaHandler::handleBinary(uint8_t *data, size_t len)
{
    JsonDocument doc;
    String jsonOutput;

    if (state != State::Receiving)
    {
        doc["status"] = "failed";

        serializeJson(doc, jsonOutput);

        return {jsonOutput, true};
    }

    size_t writtenChunk = Update.write(data, len);
    if (writtenChunk != len)
    {
        doc["status"] = "failed";

        serializeJson(doc, jsonOutput);

        return {jsonOutput, true};
    }

    written += writtenChunk;

    bool isFinished = written >= totalSize;

    if (isFinished)
        doc["status"] = "done";
    else
        doc["status"] = "receiving";

    doc["written"] = double(written);
    doc["totalSize"] = double(totalSize);

    serializeJson(doc, jsonOutput);

    debugln("# [OTA-JSON] " + jsonOutput);

    if (isFinished)
        state = State::Done;

    return {jsonOutput, false};
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
}

bool OtaHandler::isReceiving() const
{
    return state == State::Receiving;
}

bool OtaHandler::isDone() const
{
    return state == State::Done;
}

uint32_t OtaHandler::getTotal() const
{
    return totalSize;
}

uint32_t OtaHandler::getWritten() const
{
    return written;
}
