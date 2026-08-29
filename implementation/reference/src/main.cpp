#include <Arduino.h>
#include <ArduinoJson.h>

// Implementation configuration (pins, dummy mode) — edit config.h.
#include "config.h"

// Single entry-point application wrapper (begin()/loop() only).
#include "CroasterApp.h"

CroasterApp app(dummyMode, pins);

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
                             {
        return "{\"pong\": true}";
    });

    // Nested JSON command: {"command":{"mySetting":42}} -> {"mySetting":42}
    app.commands().onJsonCommand("mySetting", [](const JsonObject &json) -> String
                                 {
        return "{\"mySetting\": " + String(json["mySetting"].as<int>()) + "}";
    });
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
