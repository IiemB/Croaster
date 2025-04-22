#include "DisplayManager.h"
#include "Constants.h"

DisplayManager::DisplayManager(int width, int height, uint8_t i2cAddr)
    : display(width, height, &Wire, OLED_RESET),
      screenWidth(width),
      screenHeight(height),
      i2cAddress(i2cAddr)
{
}

bool DisplayManager::begin()
{
    if (!display.begin(SSD1306_SWITCHCAPVCC, i2cAddress))
    {
        debugln(F("# SSD1306 allocation failed"));
        return false;
    }
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    debugln(F("# SSD1306 initialization succeed"));
    testDrawLine();
    display.setTextWrap(false);
    return true;
}

void DisplayManager::drawHeader(String ipAddr)
{
    String text = "CROASTER V" + String(version);
    if (showIp && !ipAddr.isEmpty())
    {
        text = ipAddr;
    }

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(text);

    showIp = !showIp;
}

void DisplayManager::drawTemperature(String label, float temp, float ror, int yCursor, String tempUnit)
{
    String tempText = isnan(temp) ? "N/A" : String(temp, 1) + tempUnit;
    int tempX = display.width() - (18 * tempText.length()) + 3;

    display.setTextSize(1);
    display.setCursor(0, yCursor);
    display.print(label);

    display.setTextSize(3);
    display.setCursor(tempX, yCursor);
    display.print(tempText);

    if (!isnan(temp))
    {
        display.setTextSize(1);
        display.setCursor(0, yCursor + 14);
        display.print(String((int)round(ror)));
    }
}

void DisplayManager::testDrawLine()
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
}

void DisplayManager::loop(CroasterCore &croaster, String ipAddr)
{

    et = croaster.tempET;
    rorET = croaster.rorET;
    bt = croaster.tempBT;
    rorBT = croaster.rorBT;
    unit = croaster.temperatureUnit;
    ip = ipAddr;

    unsigned long now = millis();

    // === Invert screen ===
    if (now - lastInversionToggle >= (isDisplayInverted ? inversionDuration : inversionInterval))
    {
        isDisplayInverted = !isDisplayInverted;
        lastInversionToggle = now;
        display.invertDisplay(isDisplayInverted);
        debugln(isDisplayInverted ? "# Display Inverted to Prevent Burn-In" : "# Display Reverted to Normal");
    }

    if (now - lastUpdate < croaster.intervalSendData)
        return;

    lastUpdate = now;

    display.clearDisplay();
    drawHeader(ip);
    drawTemperature("ET", et, rorET, 16, unit);
    drawTemperature("BT", bt, rorBT, 43, unit);
    display.display();
}

void DisplayManager::rotateScreen()
{
    if (screenRotation < 3)
        screenRotation = screenRotation + 1;
    else
        screenRotation = 0;

    display.setRotation(screenRotation);
}
