#include "DeviceIdentity.h"

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

String getDeviceName(String prefix, String suffix)
{
    return prefix + getUniqueChipId() + suffix;
}
