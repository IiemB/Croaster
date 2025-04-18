#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED Display Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 // Reset pin not used

// DisplayManager Class
class DisplayManager
{
private:
    Adafruit_SSD1306 display;
    const int screenWidth;
    const int screenHeight;

    void drawHeader()
    {
        display.setTextSize(1); // 8px height
        display.setCursor(0, 0);
        display.println("CROASTER V" + String(versionCode));
    }

    void drawTemperature(String label, float temp, int yCursor, String tempUnit)
    {

        // Format temperature text
        String tempText;

        if (isnan(temp))
            tempText = "N/A";
        else
            tempText = String(temp, 1) + tempUnit;

        int textLenght = tempText.length();

        int tempX = 128 - (18 * textLenght);

        display.setTextSize(1);
        display.setCursor(0, yCursor); // Left-aligned label
        display.print(label);

        display.setTextSize(3); // 24px height for digits
        display.setCursor(tempX, yCursor);
        display.print(tempText);
    }

public:
    DisplayManager(int width, int height, const double &version, uint8_t i2cAddress = 0x3C)
        : display(width, height, &Wire, OLED_RESET),
          screenWidth(width),
          screenHeight(height),
          versionCode(version),
          i2cAddress(i2cAddress)
    {
    }

    uint8_t i2cAddress;

    double versionCode;

    bool begin()
    {
        // Initialize OLED display
        if (!display.begin(SSD1306_SWITCHCAPVCC, i2cAddress))
        {
            debugln(F("SSD1306 allocation failed"));
            return false;
        }

        // Clear the display
        display.clearDisplay();
        display.display();

        // Set text color to white
        display.setTextColor(SSD1306_WHITE);

        debugln(F("SSD1306 initialization succeed"));

        return true;
    }

    void updateDisplay(float tempET, float tempBT, String tempUnit)
    {

        // Clear the display buffer
        display.clearDisplay();

        // Draw all components
        drawHeader();
        drawTemperature("ET", tempET, 16, tempUnit);
        drawTemperature("BT", tempBT, 43, tempUnit);

        // Update the display
        display.display();
    }
};