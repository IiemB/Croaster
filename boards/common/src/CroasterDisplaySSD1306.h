#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <CroasterDisplay.h>

// This display's hardware specifics (defined in the implementation, not the library).
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

/**
 * @class CroasterDisplaySSD1306
 * @brief Concrete 128x64 SSD1306 OLED display implementation (I2C).
 *
 * This is the display implementation for the ESP32-C3 Super Mini. It lives in
 * the implementation, not in the library — a consuming project with a different
 * screen provides its own CroasterDisplay subclass instead.
 */
class CroasterDisplaySSD1306 : public CroasterDisplay {
public:
    /**
     * @brief Constructs a CroasterDisplaySSD1306 instance.
     * @param croaster Reference to the CroasterCore instance.
     * @param i2cAddress The I2C address of the display (default is 0x3C).
     */
    CroasterDisplaySSD1306(CroasterCore& croaster, uint8_t i2cAddress = 0x3C);

    void begin() override;
    void loop() override;
    void rotateScreen() override;
    void blinkIndicator(bool state) override;
    void displayToggle() override;
    void updateFirmwareUpdateProgress(int progress) override;
    void updatingStatusToggle(bool isUpdating) override;
    bool isFirmwareUpdating() const override;

private:
    Adafruit_SSD1306 display;

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
    const unsigned long inversionDuration = 60000;

    unsigned long lastShowIpToggle = 0;
    bool isIpShowed = false;

    bool hasDisplay = false;

    bool isDisplayOn = true;

    bool isUpdatingFirmware = false;

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
};
