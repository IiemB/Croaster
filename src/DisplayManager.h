#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Constants.h"
#include "CroasterCore.h"

/**
 * @class DisplayManager
 * @brief Manages the OLED display for the Croaster device.
 */
class DisplayManager
{
private:
    Adafruit_SSD1306 display;

    CroasterCore *croaster = nullptr;

    uint8_t i2cAddress;

    unsigned long lastUpdate = 0;

    String ipAddr;

    double et = NAN;
    double bt = NAN;
    double rorEt = NAN;
    double rorBt = NAN;
    String tempUnit = "C";

    int screenRotation = 0;

    unsigned long lastInversionToggle = 0;
    bool isDisplayInverted = false;
    const unsigned long inversionInterval = 60000;
    const unsigned long inversionDuration = 5000;

    unsigned long lastShowIpToggle = 0;
    bool isIpShowed = false;

    bool hasDisplay = false;

    /**
     * @brief Draws the header section of the display.
     */
    void drawHeader();

    /**
     * @brief Draws temperature data on the display.
     * @param label The label for the temperature (e.g., "ET" or "BT").
     * @param temp The temperature value.
     * @param ror The rate of rise (RoR) value.
     * @param yCursor The vertical position on the display.
     */
    void drawTemperature(String label, double temp, double ror, int yCursor);

    /**
     * @brief Displays the splash screen.
     */
    void splash();

    /**
     * @brief Scans the I2C bus for connected devices.
     *
     * @return true if devices are found on the I2C bus, false otherwise.
     */
    bool isOledPresent();

public:
    /**
     * @brief Constructs a DisplayManager instance.
     * @param croaster Reference to the CroasterCore instance.
     * @param i2cAddress The I2C address of the display (default is 0x3C).
     */
    DisplayManager(CroasterCore &croaster, uint8_t i2cAddress = 0x3C);

    /**
     * @brief Initializes the display.
     * @return True if initialization is successful, false otherwise.
     */
    void begin();

    /**
     * @brief Handles display updates in the main loop.
     */
    void loop();

    /**
     * @brief Rotates the display orientation.
     */
    void rotateScreen();

    /**
     * @brief Toggles the display indicator (e.g., blinking).
     * @param state The state of the indicator (true for on, false for off).
     */
    void blinkIndicator(bool state);
};
