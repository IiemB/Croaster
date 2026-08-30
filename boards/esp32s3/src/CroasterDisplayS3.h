#pragma once
#include <CroasterDisplay.h>

#include "ui/LvglUi.h"

/**
 * @class CroasterDisplayS3
 * @brief CroasterDisplay implementation for the ES3C28P (ESP32-S3) board.
 *
 * Wraps the LVGL UI (LvglUi) on the built-in ILI9341 LCD, wiring the UI's
 * timer-control buttons to the core's roast timer and the double-tap
 * backlight toggle back into this class. Lives in the esp32s3 implementation
 * only — it is never compiled by the shared library.
 */
class CroasterDisplayS3 : public CroasterDisplay {
public:
    explicit CroasterDisplayS3(CroasterCore& core);

    // --- CroasterDisplay interface ---
    void begin() override;
    void loop() override;
    void rotateScreen() override;
    void blinkIndicator(bool state) override;
    void displayToggle() override;
    void updateFirmwareUpdateProgress(int progress) override;
    void updatingStatusToggle(bool isUpdating) override;
    bool isFirmwareUpdating() const override;
    bool isDarkMode() const override;
    int getBrightness() const override;

    // --- S3 extras (not part of the base interface) ---
    void setBtConnected(bool connected);
    void setBrightness(int percent);
    void setDarkMode(bool dark);
    void finishSplash();

private:
    LvglUi lvgl;
    bool isDisplayOn = true;
    bool isUpdatingFirmware = false;
    unsigned long lastUpdate = 0;
};
