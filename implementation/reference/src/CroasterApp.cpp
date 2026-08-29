#include "CroasterApp.h"
#include <CroasterWiFiManager.h>

CroasterApp::CroasterApp(bool dummyMode, CroasterPinConfig pins)
    : croaster(dummyMode, pins),
      display(croaster),
      commandHandler(croaster, &display),
#if CROASTER_HAS_BLE
      bleManager(croaster, commandHandler, &display),
#endif
      wsManager(croaster, commandHandler, &display)
{
}

void CroasterApp::begin()
{
    Serial.begin(115200);

    // WiFi (WiFiManager captive portal)
    CroasterWiFiManager::setup(croaster.ssidName());

    commandHandler.begin();

#if CROASTER_HAS_BLE
    bleManager.begin();
#endif

    wsManager.begin();

    display.begin();
}

void CroasterApp::loop()
{
    CroasterWiFiManager::process();

    croaster.loop();

    wsManager.loop();

    display.loop();

#if CROASTER_HAS_BLE
    bleManager.loop();
#endif

    commandHandler.loop();
}
