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

    bool showIp = false;

    void drawHeader(String ip = "")
    {
        String text = "CROASTER V" + String(versionCode);

        if (showIp && !ip.isEmpty())
        {
            text = ip;
        }

        display.setTextSize(1); // 8px height
        display.setCursor(0, 0);
        display.print(text);

        showIp = !showIp;
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

        int tempX = display.width() - (18 * textLenght);

        display.setTextSize(1);
        display.setCursor(0, yCursor); // Left-aligned label
        display.print(label);

        display.setTextSize(3); // 24px height for digits
        display.setCursor(tempX, yCursor);
        display.print(tempText);
    }

    void testdrawline()
    {
        int16_t i;

        display.clearDisplay(); // Clear display buffer

        for (i = 0; i < display.width(); i += 4)
        {
            display.drawLine(0, 0, i, display.height() - 1, SSD1306_WHITE);
            display.display(); // Update screen with each newly-drawn line
            delay(1);
        }
        for (i = 0; i < display.height(); i += 4)
        {
            display.drawLine(0, 0, display.width() - 1, i, SSD1306_WHITE);
            display.display();
            delay(1);
        }
        delay(250);

        display.clearDisplay();

        for (i = 0; i < display.width(); i += 4)
        {
            display.drawLine(0, display.height() - 1, i, 0, SSD1306_WHITE);
            display.display();
            delay(1);
        }
        for (i = display.height() - 1; i >= 0; i -= 4)
        {
            display.drawLine(0, display.height() - 1, display.width() - 1, i, SSD1306_WHITE);
            display.display();
            delay(1);
        }
        delay(250);

        display.clearDisplay();

        for (i = display.width() - 1; i >= 0; i -= 4)
        {
            display.drawLine(display.width() - 1, display.height() - 1, i, 0, SSD1306_WHITE);
            display.display();
            delay(1);
        }
        for (i = display.height() - 1; i >= 0; i -= 4)
        {
            display.drawLine(display.width() - 1, display.height() - 1, 0, i, SSD1306_WHITE);
            display.display();
            delay(1);
        }
        delay(250);

        display.clearDisplay();

        for (i = 0; i < display.height(); i += 4)
        {
            display.drawLine(display.width() - 1, 0, 0, i, SSD1306_WHITE);
            display.display();
            delay(1);
        }
        for (i = 0; i < display.width(); i += 4)
        {
            display.drawLine(display.width() - 1, 0, i, display.height() - 1, SSD1306_WHITE);
            display.display();
            delay(1);
        }

        delay(2000); // Pause for 2 seconds
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
            debugln(F("# SSD1306 allocation failed"));
            return false;
        }

        // Clear the display
        display.clearDisplay();
        display.display();

        // Set text color to white
        display.setTextColor(SSD1306_WHITE);

        debugln(F("# SSD1306 initialization succeed"));

        testdrawline();

        return true;
    }

    void updateDisplay(float tempET, float tempBT, String tempUnit, String ip)
    {
        // Clear the display buffer
        display.clearDisplay();

        // Draw all components
        drawHeader(ip);
        drawTemperature("ET", tempET, 16, tempUnit);
        drawTemperature("BT", tempBT, 43, tempUnit);

        // Update the display
        display.display();
    }
};