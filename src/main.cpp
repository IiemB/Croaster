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
DisplayManager displayManager(SCREEN_WIDTH, SCREEN_HEIGHT);
CroasterCore croaster(dummyMode);
CommandHandler commandHandler(croaster, displayManager);

// External globals (defined elsewhere)
extern WebSocketsServer webSocket;
extern WiFiManager wifiManager;

// === Arduino Setup ===
void setup()
{
  Serial.begin(115200);

  // Initialize managers
  commandHandler.init();
  setupWiFiManager(croaster.ssidName());
#if defined(ESP32)
  setupBLE(croaster, displayManager);
#endif
  setupWebSocket(commandHandler);
  displayManager.begin();
}

// === Arduino Loop ===
void loop()
{
  wifiManager.process();         // Non-blocking WiFi config portal
  webSocket.loop();              // Handle WebSocket messages
  croaster.loop();               // Read sensors, compute ROR
  broadcastData(croaster);       // Send data
  displayManager.loop(croaster); // Update display values to show
  commandHandler.loop();         // Command handler
}
