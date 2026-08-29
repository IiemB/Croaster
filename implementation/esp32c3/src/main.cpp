#include <Arduino.h>
#include <ArduinoJson.h>

// Implementation configuration (pins, dummy mode, LED) — edit config.h.
#include "config.h"

// Concrete display for this board — defined here, in the implementation.
#include "CroasterDisplaySSD1306.h"

// Single entry-point application wrapper (begin()/loop() only).
#include "CroasterApp.h"

// Board wiring: the core and the display are created here so CroasterApp never
// needs to know the display specification.
CroasterCore core(dummyMode, pins);
CroasterDisplaySSD1306 display(core);
CroasterApp app(core, &display, ledPin, ledOnLevel);

// ---------------------------------------------------------------------------
// Optional: register custom commands WITHOUT touching the Croaster library.
//
//   onCommand("name", fn)      -> basic string command:  {"command":"name"}
//   onJsonCommand("key", fn)   -> nested JSON command:   {"command":{"key":...}}
//
// Each callback receives the parsed JSON object and returns a response string
// (empty = no response is sent back). genResponseCommand() can wrap a response
// in the standard {"command","response"} format.
// ---------------------------------------------------------------------------
void registerCustomCommands()
{
    // Basic command: {"command":"ping"} -> {"pong":true}
    app.commands().onCommand("ping", [](const JsonObject &json) -> String
                             { return "{\"pong\": true}"; });

    // Nested JSON command: {"command":{"mySetting":42}} -> {"mySetting":42}
    app.commands().onJsonCommand("mySetting", [](const JsonObject &json) -> String
                                 { return "{\"mySetting\": " + String(json["mySetting"].as<int>()) + "}"; });
}

void setup()
{
    registerCustomCommands();
    app.begin();
}

void loop()
{
    app.loop();
}
