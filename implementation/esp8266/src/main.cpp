#include <Arduino.h>
#include <ArduinoJson.h>

// Implementation configuration (pins, dummy mode, LED) — edit config.h.
#include "config.h"

// Shared SSD1306 display (implementation/common) used by this board.
#include <CroasterDisplaySSD1306.h>

// Single entry-point application wrapper (begin()/loop() only).
#include <CroasterApp.h>

// Board id reported to the app (getDeviceInfo "board") — hardcoded here.
#include <CroasterDeviceIdentity.h>

// Board wiring: the core and the display are created here so CroasterApp never
// needs to know the display specification.
CroasterCore core(dummyMode, pins);
CroasterDisplaySSD1306 display(core);
CroasterApp app(core, &display, ledPin, ledOnLevel);

// ---------------------------------------------------------------------------
// Optional: register custom commands WITHOUT touching the Croaster library.
//   onCommand("name", fn)      -> basic string command:  {"command":"name"}
//   onJsonCommand("key", fn)   -> nested JSON command:   {"command":{"key":...}}
// ---------------------------------------------------------------------------
void registerCustomCommands()
{
}

void setup()
{
    // Hardcode this board's id (matches the app's CroasterBoardTypes enum).
    CroasterDeviceIdentity::setBoardName("esp8266");

    registerCustomCommands();
    app.begin();
}

void loop()
{
    app.loop();
}
