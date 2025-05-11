#if defined(ESP8266)
#pragma message "ESP8266 stuff happening!"
#elif defined(ESP32)
#pragma message "ESP32 stuff happening!"
#else
#error "This ain't a ESP8266 or ESP32!"
#endif

#include <Arduino.h>
#include "Constants.h"
#include "BleManager.h"
#include "WiFiManagerUtil.h"
#include "WebSocketManager.h"
#include "CroasterCore.h"
#include "DisplayManager.h"
#include "CommandHandler.h"

// === Global Instances ===
CroasterCore croaster(dummyMode);

DisplayManager displayManager(croaster);

CommandHandler commandHandler(croaster, displayManager);

#if defined(ESP32)
BleManager bleManager(croaster, commandHandler);
#endif

WebSocketManager wsManager(croaster, commandHandler);

// === Arduino Setup ===
void setup()
{
  Serial.begin(115200);

  // Initialize managers
  setupWiFiManager(croaster.ssidName());

  commandHandler.begin();

#if defined(ESP32)
  bleManager.begin();
#endif

  wsManager.begin();

  displayManager.begin();
}

// === Arduino Loop ===
void loop()
{
  processWiFiManager();

  croaster.loop();

  wsManager.loop();

  displayManager.loop();

#if defined(ESP32)
  bleManager.loop();
#endif

  commandHandler.loop();
}
