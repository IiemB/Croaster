#pragma once
#include <Arduino.h>
#include <CroasterConstants.h>
#include <CroasterCore.h>
#include <CroasterPinConfig.h>
#include <CroasterDisplay.h>
#include <CroasterCommandHandler.h>
#include <CroasterWebSocketManager.h>
#include <CroasterBleManager.h>

/**
 * @class CroasterApp
 * @brief Single entry-point wrapper for an implementation.
 *
 * Owns the command handler, WebSocket, BLE and WiFi subsystems and exposes only
 * begin()/loop(). It does NOT know the concrete display type — the
 * implementation creates the core + display and passes them in (as a
 * CroasterDisplay*). Pin, dummy-mode, LED and display configuration all live in
 * the implementation, never in the Croaster library.
 *
 *   CroasterCore core(dummyMode, pins);
 *   MyDisplay display(core);          // display defined in the implementation
 *   CroasterApp app(core, &display, ledPin, ledOnLevel);
 *   void setup() { app.begin(); }
 *   void loop()  { app.loop(); }
 */
class CroasterApp
{
public:
    /**
     * @brief Constructs the app.
     * @param core Reference to the CroasterCore instance (owned by the caller).
     * @param display Pointer to the implementation's CroasterDisplay (not owned; may be nullptr).
     * @param ledPin Built-in LED pin (defaults to LED_BUILTIN; -1 disables the LED).
     * @param ledOnLevel Active level for the LED (LOW = active-low, HIGH = active-high).
     */
    CroasterApp(CroasterCore &core, CroasterDisplay *display,
                int8_t ledPin = LED_BUILTIN, uint8_t ledOnLevel = LOW);

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

    /**
     * @brief Access to the display (may be nullptr).
     */
    CroasterDisplay *display() { return _display; }

#if CROASTER_HAS_BLE
    /**
     * @brief Access to the BLE manager (only available on boards with BLE).
     */
    CroasterBleManager &ble() { return bleManager; }
#endif

private:
    CroasterCore &croaster;    ///< Reference to the caller-owned core.
    CroasterDisplay *_display; ///< Implementation-owned display (may be nullptr).
    CroasterCommandHandler commandHandler;
#if CROASTER_HAS_BLE
    CroasterBleManager bleManager;
#endif
    CroasterWebSocketManager wsManager;
};
