#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Constants.h"
#include "CroasterCore.h"

/**
 * Handles rendering temperature and status data to the OLED display.
 */
class DisplayManager
{
private:
    Adafruit_SSD1306 display;
    const int screenWidth;
    const int screenHeight;
    double versionCode;
    uint8_t i2cAddress;
    bool showIp = false;
    unsigned long lastUpdate = 0;

    String ip;
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

    void drawHeader(String ip = "");
    void drawTemperature(String label, float temp, float ror, int yCursor, String tempUnit);
    void testDrawLine();

public:
    DisplayManager(int width, int height, const double &version, uint8_t i2cAddress = 0x3C);

    /**
     * Initializes the OLED display. Returns false if failed.
     */
    bool begin();

    /**
     * Should be called inside loop() to handle timed updates.
     */
    void loop(CroasterCore &croaster, String ip);

    /**
     * Rotates the screen display.
     */
    void rotateScreen();
};
