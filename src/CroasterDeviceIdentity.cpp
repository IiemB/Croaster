#include "CroasterDeviceIdentity.h"
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

String CroasterDeviceIdentity::uniqueChipId()
{
#if defined(ESP32)
    uint64_t chipId = ESP.getEfuseMac();
    char idStr[13];
    sprintf(idStr, "%04X%08X", (uint32_t)(chipId >> 32), (uint32_t)chipId);
    return String(idStr);
#elif defined(ESP8266)
    char idStr[7];
    sprintf(idStr, "%06X", ESP.getChipId());
    return String(idStr);
#else
    return "UNKNOWN";
#endif
}

String CroasterDeviceIdentity::shortChipId(uint8_t length)
{
    String fullId = uniqueChipId();
    if (length >= fullId.length())
        return fullId;

    return fullId.substring(0, length);
}

String CroasterDeviceIdentity::deviceName(String prefix, String suffix, uint8_t length)
{
    return prefix + shortChipId(length) + suffix;
}

String CroasterDeviceIdentity::ipAddress()
{
    return WiFi.isConnected() ? WiFi.localIP().toString() : "";
}

String CroasterDeviceIdentity::ssidName()
{
    return WiFi.isConnected() ? WiFi.SSID() : "";
}

String CroasterDeviceIdentity::boardName()
{
#ifdef CROASTER_BOARD_NAME
    // Defined per implementation (platformio.ini build flag), e.g. "esp32s3",
    // "esp32c3", "esp8266" — matches the app's CroasterBoardTypes enum.
    return CROASTER_BOARD_NAME;
#else
    return "unknown";
#endif
}
