#include "CroasterApp.h"

#include <CroasterWiFiManager.h>

CroasterApp::CroasterApp(CroasterCore& core, CroasterDisplay* display, int8_t ledPin, uint8_t ledOnLevel)
    : croaster(core)
    , _display(display)
    , commandHandler(croaster, display, ledPin, ledOnLevel)
    ,
#if CROASTER_HAS_BLE
    bleManager(croaster, commandHandler, display)
    ,
#endif
    wsManager(croaster, commandHandler, display) {
}

void CroasterApp::begin() {
    Serial.begin(115200);

    // WiFi (WiFiManager captive portal)
    CroasterWiFiManager::setup(croaster.ssidName());

    commandHandler.begin();

#if CROASTER_HAS_BLE
    bleManager.begin();
#endif

    wsManager.begin();

    if (_display)
        _display->begin();
}

void CroasterApp::loop() {
    CroasterWiFiManager::process();

    croaster.loop();

    wsManager.loop();

    if (_display)
        _display->loop();

#if CROASTER_HAS_BLE
    bleManager.loop();
#endif

    commandHandler.loop();
}
