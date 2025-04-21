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

// === Global Instances ===
DisplayManager displayManager(SCREEN_WIDTH, SCREEN_HEIGHT, version);
CroasterCore croaster(version, dummyMode);

// Shared state flags
bool bleDeviceConnected = false;
bool wifiConnected = false;

// External globals (defined elsewhere)
extern WebSocketsServer webSocket;
extern WiFiManager wifiManager;

// LED status control
unsigned long lastBlinkLed = 0;
bool isLedOn = false;

// Function Prototypes
void blinkLED();

// === Arduino Setup ===
void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);

  // Initialize managers
  setupWiFiManager(croaster.ssidName);
#if defined(ESP32)
  setupBLE(croaster, bleDeviceConnected);
#endif
  setupWebSocket(croaster);
  displayManager.begin();
}

// === Arduino Loop ===
void loop()
{
  wifiManager.process();                                                         // Non-blocking WiFi config portal
  webSocket.loop();                                                              // Handle WebSocket messages
  croaster.loop();                                                               // Read sensors, compute ROR
  broadcastData(croaster);                                                       // Send data
  displayManager.loop(croaster, wifiConnected ? WiFi.localIP().toString() : ""); // Update display values to show
  blinkLED();                                                                    // Blink based on connection state
}

// === LED Blinker for Status Feedback ===
void blinkLED()
{
  unsigned long intervalLED = 500;
  wifiConnected = WiFi.isConnected();

  if (wifiConnected && bleDeviceConnected)
  {
    intervalLED = 1000;
  }
  else if (wifiConnected)
  {
    intervalLED = 3000;
  }
  else if (bleDeviceConnected)
  {
    intervalLED = 2000;
  }

  if (millis() - lastBlinkLed < intervalLED)
    return;

  lastBlinkLed = millis();
  digitalWrite(LED_BUILTIN, isLedOn ? LOW : HIGH);
  isLedOn = !isLedOn;
}