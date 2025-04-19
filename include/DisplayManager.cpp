#include "DisplayManager.h"
#include "Constants.h"

DisplayManager::DisplayManager(int width, int height, const double &version, uint8_t i2cAddr)
    : display(width, height, &Wire, OLED_RESET),
      screenWidth(width),
      screenHeight(height),
      versionCode(version),
      i2cAddress(i2cAddr) {}

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
    return true;
}

void DisplayManager::drawHeader(String ipAddr)
{
    String text = "CROASTER V" + String(versionCode);
    if (showIp && !ipAddr.isEmpty())
    {
        text = ipAddr;
    }

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(text);

    showIp = !showIp;
}

void DisplayManager::drawTemperature(String label, float temp, int yCursor, String tempUnit)
{
    String tempText = isnan(temp) ? "N/A" : String(temp, 1) + tempUnit;

    int tempX = display.width() - (18 * tempText.length());

    display.setTextSize(1);
    display.setCursor(0, yCursor);
    display.print(label);

    display.setTextSize(3);
    display.setCursor(tempX, yCursor);
    display.print(tempText);
}

void DisplayManager::testDrawLine()
{
    display.clearDisplay();
    for (int i = 0; i < display.width(); i += 4)
    {
        display.drawLine(0, 0, i, display.height() - 1, SSD1306_WHITE);
        display.display();
        delay(1);
    }
    for (int i = 0; i < display.height(); i += 4)
    {
        display.drawLine(0, 0, display.width() - 1, i, SSD1306_WHITE);
        display.display();
        delay(1);
    }
    delay(250);
    display.clearDisplay();
}

void DisplayManager::updateDisplay(TempsManager &croaster, String ipAddr)
{
    if (millis() - lastUpdate < croaster.intervalSendData)
        return;

    et = croaster.tempET;
    bt = croaster.tempBT;
    unit = croaster.temperatureUnit;
    ip = ipAddr;

    lastUpdate = millis();

    display.clearDisplay();
    drawHeader(ip);
    drawTemperature("ET", et, 16, unit);
    drawTemperature("BT", bt, 43, unit);
    display.display();
}
