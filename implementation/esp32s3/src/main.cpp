#include <Arduino.h>
#include <ArduinoJson.h>

// Implementation configuration (pins, dummy mode, LED) — edit config.h.
#include "config.h"

// ES3C28P LVGL display on the built-in ILI9341 LCD (impl src/CroasterDisplayS3.h).
#include <CroasterDisplayS3.h>

// Single entry-point application wrapper (begin()/loop() only).
#include <CroasterApp.h>

// Board id reported to the app (getDeviceInfo "board") — hardcoded here.
#include <CroasterDeviceIdentity.h>

// Board wiring: the core and the display are created here so CroasterApp never
// needs to know the display specification.
CroasterCore core(dummyMode, pins);
CroasterDisplayS3 display(core);
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
//
// Here we expose the S3-only display controls (backlight brightness and the
// light/dark theme) so a client can drive them over WebSocket / BLE.
// ---------------------------------------------------------------------------
void registerCustomCommands()
{
}

void setup()
{
    // Hardcode this board's id (matches the app's CroasterBoardTypes enum).
    CroasterDeviceIdentity::setBoardName("esp32s3");

    registerCustomCommands();
    app.begin();

    // Boot splash is driven by the LVGL task; let it switch to the main page
    // once the (minimum timed) splash has finished.
    display.finishSplash();
}

void loop()
{
    // Keep the LVGL status chip in sync with the BLE client connection.
    display.setBtConnected(app.ble().isClientConnected());

    app.loop();
}
