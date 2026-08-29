#pragma once
#include <CroasterConstants.h>
#include <CroasterCore.h>
#include <CroasterPinConfig.h>
#include <CroasterCommandHandler.h>
#include <CroasterWebSocketManager.h>
#include <CroasterBleManager.h>
#include "CroasterDisplaySSD1306.h"

/**
 * @class CroasterApp
 * @brief Single entry-point wrapper for the reference implementation.
 *
 * Owns every subsystem (core, display, command handler, WebSocket, BLE, WiFi)
 * and exposes only begin()/loop(). The sketch is reduced to:
 *
 *   CroasterApp app;
 *   void setup() { app.begin(); }
 *   void loop()  { app.loop(); }
 *
 * Pin layout, display and dummy-mode configuration live in this
 * implementation (see config.h / CroasterApp), never in the Croaster library.
 */
class CroasterApp
{
public:
    /**
     * @brief Constructs the app.
     * @param dummyMode Use dummy sensor data instead of real thermocouples.
     * @param pins Thermocouple (MAX6675) pin layout for this board.
     */
    CroasterApp(bool dummyMode = false, CroasterPinConfig pins = CroasterPinConfig::defaults());

    /**
     * @brief Initializes WiFi, display, command handler and communication managers.
     */
    void begin();

    /**
     * @brief Drives all subsystems (call repeatedly from the main loop).
     */
    void loop();

    /**
     * @brief Access to the command handler (e.g. to register custom commands).
     */
    CroasterCommandHandler &commands() { return commandHandler; }

    /**
     * @brief Access to the core (sensor data, settings).
     */
    CroasterCore &core() { return croaster; }

private:
    // Declaration order matters: core must exist before the subsystems that
    // reference it, and the display before the handlers that use it.
    CroasterCore croaster;
    CroasterDisplaySSD1306 display;
    CroasterCommandHandler commandHandler;
#if CROASTER_HAS_BLE
    CroasterBleManager bleManager;
#endif
    CroasterWebSocketManager wsManager;
};
