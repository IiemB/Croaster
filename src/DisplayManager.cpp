#include "DisplayManager.h"
#include "Constants.h"

DisplayManager::DisplayManager(int width, int height, uint8_t i2cAddr)
    : display(width, height, &Wire, OLED_RESET),
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
    splash();

    display.setTextWrap(false);
    return true;
}

void DisplayManager::drawHeader()
{
    String text = "CROASTER V" + String(version);

    if (isIpShowed && !ipAddr.isEmpty())
    {
        text = ipAddr;
    }

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(text);
}

void DisplayManager::drawTemperature(String label, float temp, float ror, int yCursor)
{
    String tempText = isnan(temp) ? "N/A" : String(temp, 1) + unit;
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
            rorText = String((float)ror, 1);

        display.setTextSize(1);
        display.setCursor(0, yCursor + 14);
        display.print(rorText);
    }
}

void DisplayManager::splash()
{
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

void DisplayManager::loop(CroasterCore &croaster)
{
    unsigned long now = millis();

    int croasterInterval = croaster.intervalSendData * 1000;

    // === Invert screen ===
    if (now - lastInversionToggle >= (isDisplayInverted ? inversionDuration : inversionInterval))
    {
        isDisplayInverted = !isDisplayInverted;
        lastInversionToggle = now;
        display.invertDisplay(isDisplayInverted);
        debugln(isDisplayInverted ? "# Display Inverted to Prevent Burn-In" : "# Display Reverted to Normal");
    }

    // === Update screen ===
    if (now - lastUpdate >= croasterInterval)
    {
        et = croaster.tempET;
        rorET = croaster.rorET;
        bt = croaster.tempBT;
        rorBT = croaster.rorBT;
        unit = croaster.temperatureUnit;
        ipAddr = getIpAddress();

        lastUpdate = now;

        display.clearDisplay();
        drawHeader();
        drawTemperature("ET", et, rorET, 16);
        drawTemperature("BT", bt, rorBT, 43);
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
    if (screenRotation < 3)
        screenRotation++;
    else
        screenRotation = 0;

    display.setRotation(screenRotation);
}

void DisplayManager::blinkIndicator(bool state)
{
    display.fillCircle(124, 3, 3, state ? 1 : 0);
    display.display();
}
