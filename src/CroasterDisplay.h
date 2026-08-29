#pragma once
#include <Arduino.h>
#include "CroasterCore.h"

/**
 * @class CroasterDisplay
 * @brief Abstract display interface implemented by the consuming project.
 *
 * The firmware core (CroasterCore, CroasterCommandHandler, CroasterBleManager,
 * CroasterWebSocketManager) only ever talks to a CroasterDisplay* — it never
 * depends on a concrete driver or a display library.
 *
 * A project with no display simply passes `nullptr`; a project with an OLED,
 * TFT or LVGL screen provides its own subclass. See the `reference` example for
 * a complete SSD1306 implementation (CroasterDisplaySSD1306).
 */
class CroasterDisplay
{
public:
    explicit CroasterDisplay(CroasterCore &core) : _core(&core) {}
    virtual ~CroasterDisplay() = default;

    // --- Lifecycle ---

    /**
     * @brief Initializes the display hardware.
     */
    virtual void begin() = 0;

    /**
     * @brief Handles display updates in the main loop.
     */
    virtual void loop() = 0;

    // --- Controls ---

    /**
     * @brief Rotates the display orientation.
     */
    virtual void rotateScreen() = 0;

    /**
     * @brief Toggles the display indicator (e.g., blinking).
     * @param state The state of the indicator (true for on, false for off).
     */
    virtual void blinkIndicator(bool state) = 0;

    /**
     * @brief Toggles the display on or off.
     */
    virtual void displayToggle() = 0;

    // --- OTA progress ---

    /**
     * @brief Updates the firmware update progress on the display.
     * @param progress The firmware update progress percentage (0-100).
     */
    virtual void updateFirmwareUpdateProgress(int progress) = 0;

    /**
     * @brief Toggles the firmware updating status display.
     * @param isUpdating The firmware update status (true if updating, false otherwise).
     */
    virtual void updatingStatusToggle(bool isUpdating) = 0;

    /**
     * @brief Checks if the firmware is currently being updated.
     * @return true if firmware is updating, false otherwise.
     */
    virtual bool isFirmwareUpdating() const = 0;

protected:
    CroasterCore *_core; ///< Access to live sensor data (tempBt/tempEt/rorBt/rorEt).
};
