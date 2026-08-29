#if defined(ESP8266)
#pragma message "ESP8266 stuff happening!"
#elif defined(ESP32)
#pragma message "ESP32 stuff happening!"
#else
#error "This ain't a ESP8266 or ESP32!"
#endif

#include <Arduino.h>

// Croaster library
#include <CroasterConstants.h>
#include <CroasterCore.h>
#include <CroasterCommandHandler.h>
#include <CroasterWiFiManager.h>
#include <CroasterWebSocketManager.h>
#include <CroasterBleManager.h>

// Reference SSD1306 display (part of this example, not the library)
#include "CroasterDisplaySSD1306.h"

// === Global Instances ===
CroasterCore croaster(dummyMode);

CroasterDisplaySSD1306 displayManager(croaster);

CroasterCommandHandler commandHandler(croaster, &displayManager);

#if CROASTER_HAS_BLE
CroasterBleManager bleManager(croaster, commandHandler, &displayManager);
#endif

CroasterWebSocketManager wsManager(croaster, commandHandler, &displayManager);

// === Arduino Setup ===
void setup()
{
  Serial.begin(115200);

  // Initialize managers
  CroasterWiFiManager::setup(croaster.ssidName());

  commandHandler.begin();

#if CROASTER_HAS_BLE
  bleManager.begin();
#endif

  wsManager.begin();

  displayManager.begin();
}

// === Arduino Loop ===
void loop()
{
  CroasterWiFiManager::process();

  croaster.loop();

  wsManager.loop();

  displayManager.loop();

#if CROASTER_HAS_BLE
  bleManager.loop();
#endif

  commandHandler.loop();
}
