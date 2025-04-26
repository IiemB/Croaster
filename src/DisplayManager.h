#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Constants.h"
#include "CroasterCore.h"
#include "WiFiManagerUtil.h"

/**
 * Handles rendering temperature and status data to the OLED display.
 */

class DisplayManager
{
private:
    Adafruit_SSD1306 display;

    CroasterCore *croaster = nullptr;

    uint8_t i2cAddress;

    unsigned long lastUpdate = 0;

    String ipAddr;

    float et = NAN;
    float bt = NAN;
    float rorET = NAN;
    float rorBT = NAN;
    String unit = "C";

    int screenRotation = 0;

    unsigned long lastInversionToggle = 0;
    bool isDisplayInverted = false;
    const unsigned long inversionInterval = 60000;
    const unsigned long inversionDuration = 5000;

    unsigned long lastShowIpToggle = 0;
    bool isIpShowed = false;

    void drawHeader();
    void drawTemperature(String label, float temp, float ror, int yCursor);
    void splash();

public:
    DisplayManager(CroasterCore &croaster, uint8_t i2cAddress = 0x3C);

    /**
     * Initializes the OLED display. Returns false if failed.
     */
    bool begin();

    /**
     * Should be called inside loop() to handle timed updates.
     */
    void loop();

    /**
     * Rotates the screen display.
     */
    void rotateScreen();

    void blinkIndicator(bool state);
};
