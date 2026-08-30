#include "CroasterDeviceIdentity.h"
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

namespace
{
    // Board id reported by boardName(). Each implementation hardcodes its own
    // value at startup via setBoardName() (e.g. "esp32s3") in its own main.cpp,
    // so the shared library core stays generic. Defaults to "unknown" until set.
    String s_boardName = "unknown";
} // namespace

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
    return s_boardName;
}

void CroasterDeviceIdentity::setBoardName(const String &name)
{
    s_boardName = name;
}
