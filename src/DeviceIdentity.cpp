#include "DeviceIdentity.h"
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

String getUniqueChipId()
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

String getShortChipId(uint8_t length)
{
    String fullId = getUniqueChipId();
    if (length >= fullId.length())
        return fullId;

    return fullId.substring(0, length);
}

String getDeviceName(String prefix, String suffix, uint8_t length)
{
    return prefix + getShortChipId(length) + suffix;
}

String getIpAddress()
{
    return WiFi.isConnected() ? WiFi.localIP().toString() : "";
}

String getSsidName()
{
    return WiFi.isConnected() ? WiFi.SSID() : "";
}
