#include "DisplayManager.h"
#include "Constants.h"
#include "DeviceIdentity.h"
#include <Wire.h>

DisplayManager::DisplayManager(CroasterCore &croaster, uint8_t i2cAddr)
    : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET),
      croaster(&croaster),
      i2cAddress(i2cAddr)
{
}

void DisplayManager::begin()
{
    hasDisplay = isOledPresent();

    if (!hasDisplay)
    {
        debugln(F("# No display found"));
        return;
    }

    if (!display.begin(SSD1306_SWITCHCAPVCC, i2cAddress))
    {
        debugln(F("# SSD1306 allocation failed"));
        return;
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    debugln(F("# SSD1306 initialization succeed"));
    splash();

    display.setTextWrap(false);
}

void DisplayManager::drawHeader()
{
    if (!hasDisplay)
        return;

    String text = "CROASTER V" + String(version);

    if (isIpShowed && !ipAddr.isEmpty())
    {
        text = ipAddr;
    }

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(text);
}

void DisplayManager::drawTemperature(String label, double temp, double ror, int yCursor)
{
    if (!hasDisplay)
        return;

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
        String rorText = String((int)round(ror));

        if (ror >= 0 && ror < 10)
            rorText = String((double)ror, 1);

        display.setTextSize(1);
        display.setCursor(0, yCursor + 14);
        display.print(rorText);
    }
}

void DisplayManager::splash()
{
    if (!hasDisplay)
        return;

    display.clearDisplay();

    for (int16_t i = 0; i < max(display.width(), display.height()) / 2; i += 2)
    {
        display.drawCircle(display.width() / 2, display.height() / 2, i, SSD1306_WHITE);
        display.display();
        delay(1);
    }

    delay(1000);

    display.clearDisplay();

    for (int16_t i = max(display.width(), display.height()) / 2; i > 0; i -= 3)
    {
        // The INVERSE color is used so circles alternate white/black
        display.fillCircle(display.width() / 2, display.height() / 2, i, SSD1306_INVERSE);
        display.display(); // Update screen with each newly-drawn circle
        delay(1);
    }

    delay(1000);
}

bool DisplayManager::isOledPresent()
{
    Wire.begin();
    delay(100);

    Wire.beginTransmission(i2cAddress);
    return Wire.endTransmission() == 0;
}

void DisplayManager::loop()
{
    if (!hasDisplay)
        return;

    unsigned long now = millis();

    // === Invert screen ===
    if (now - lastInversionToggle >= (isDisplayInverted ? inversionDuration : inversionInterval))
    {
        isDisplayInverted = !isDisplayInverted;
        lastInversionToggle = now;
        display.invertDisplay(isDisplayInverted);
        debugln(isDisplayInverted ? "# Display Inverted to Prevent Burn-In" : "# Display Reverted to Normal");
    }

    // === Update screen ===
    if (now - lastUpdate >= 1000)
    {
        lastUpdate = now;

        et = croaster->tempEt;
        rorEt = croaster->rorEt;
        bt = croaster->tempBt;
        rorBt = croaster->rorBt;
        tempUnit = croaster->temperatureUnit();
        ipAddr = getIpAddress();

        display.clearDisplay();
        drawHeader();

        drawTemperature("BT", bt, rorBt, 16);
        drawTemperature("ET", et, rorEt, 43);

        display.display();
    }

    // === Show ip address ===
    if (now - lastShowIpToggle >= (isIpShowed ? 5000 : 10000))
    {
        isIpShowed = !isIpShowed;
        lastShowIpToggle = now;
    }
}

void DisplayManager::rotateScreen()
{
    if (!hasDisplay)
        return;

    if (screenRotation > 0)
        screenRotation = 0;
    else
        screenRotation = 2;

    display.setRotation(screenRotation);
    display.display();
}

void DisplayManager::blinkIndicator(bool state)
{
    if (!hasDisplay)
        return;

    display.fillCircle(124, 3, 3, state ? 1 : 0);
    display.display();
}
