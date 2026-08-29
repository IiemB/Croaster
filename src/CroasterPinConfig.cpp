#include "CroasterPinConfig.h"

CroasterPinConfig CroasterPinConfig::defaults()
{
#if defined(ESP8266)
    // NodeMCU / ESP-12E (D5/D7/D8/D6)
    return {D5, D7, D8, D6};
#elif defined(ESP32)
    // ESP32-C3 Super Mini
    return {4, 5, 7, 6};
#else
    return {4, 5, 7, 6};
#endif
}
