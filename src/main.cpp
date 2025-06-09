#if defined(ESP8266)
#pragma message "ESP8266 stuff happening!"
#elif defined(ESP32)
#pragma message "ESP32 stuff happening!"
#else
#error "This ain't a ESP8266 or ESP32!"
#endif

#define USE_WEBSOCKET false
#define USE_BLE true

#include <Arduino.h>
#include "Constants.h"
#if USE_BLE
#include "BleManager.h"
#endif
#if USE_WEBSOCKET
#include "WiFiManagerUtil.h"
#include "WebSocketManager.h"
#endif
#include "CroasterCore.h"
#include "DisplayManager.h"
#include "CommandHandler.h"


// === Global Instances ===
CroasterCore croaster(dummyMode);

DisplayManager displayManager(croaster);

CommandHandler commandHandler(croaster, displayManager);

#if defined(ESP32) and USE_BLE
BleManager bleManager(croaster, commandHandler);
#endif

#if USE_WEBSOCKET
WebSocketManager wsManager(croaster, commandHandler);
#endif

// === Arduino Setup ===
void setup()
{
  Serial.begin(115200);

  #if USE_WEBSOCKET
  // Initialize managers
  setupWiFiManager(croaster.ssidName());
  #endif

  commandHandler.begin();

#if defined(ESP32) and USE_BLE
  bleManager.begin();
#endif
#if USE_WEBSOCKET
  wsManager.begin();
#endif
  displayManager.begin();
}

// === Arduino Loop ===
void loop()
{
  croaster.loop();
  
#if USE_WEBSOCKET
  processWiFiManager();
  wsManager.loop();
#endif
  displayManager.loop();

#if defined(ESP32) and USE_BLE
  bleManager.loop();
#endif

  commandHandler.loop();
}
