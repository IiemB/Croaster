#include "CroasterDisplayS3.h"
#include <WiFi.h>
#include <CroasterConstants.h>
#include <CroasterDeviceIdentity.h>

CroasterDisplayS3::CroasterDisplayS3(CroasterCore &core)
    : CroasterDisplay(core)
{
}

void CroasterDisplayS3::begin()
{
    if (!lvgl.begin())
    {
        debugln(F("# LVGL UI failed to start"));
        return;
    }

    // Route UI commands (timer controls) and the double-tap display toggle.
    lvgl.setCommandCallback([this](const String &cmd)
                            {
        if (cmd == "timerStart")
            _core->roastTimerStart();
        else if (cmd == "timerPause")
            _core->roastTimerPause();
        else if (cmd == "timerReset")
            _core->roastTimerReset(); });
    lvgl.setDisplayToggleCallback([this]()
                                  { displayToggle(); });
}

void CroasterDisplayS3::loop()
{
    unsigned long now = millis();

    // === Update live data (skipped while a firmware update is on screen) ===
    if (!isUpdatingFirmware && now - lastUpdate >= 1000)
    {
        lastUpdate = now;

        double et = _core->tempEt;
        double rorEt = _core->rorEt;
        double bt = _core->tempBt;
        double rorBt = _core->rorBt;
        String tempUnit = _core->temperatureUnit();
        String ipAddr = CroasterDeviceIdentity::ipAddress();
        String title = "CROASTER V" + String(version);

        lvgl.updateData(bt, rorBt, et, rorEt, tempUnit, title,
                        (unsigned long)_core->roastTimer, ipAddr,
                        _core->roastTimerIsRunning());
        lvgl.setWiFiConnected(WiFi.status() == WL_CONNECTED);
    }

    // === Run the LVGL timer handler so widgets keep rendering ===
    lvgl.loop();
}

void CroasterDisplayS3::rotateScreen()
{
    lvgl.rotateScreen();
}

void CroasterDisplayS3::blinkIndicator(bool state)
{
    // No on-screen blink indicator on the LVGL UI (the builtin LED is handled
    // by CroasterApp via ledPin/ledOnLevel).
    (void)state;
}

void CroasterDisplayS3::setBtConnected(bool connected)
{
    lvgl.setBtConnected(connected);
}

void CroasterDisplayS3::displayToggle()
{
    isDisplayOn = !isDisplayOn;
    lvgl.displayOn(isDisplayOn);
}

void CroasterDisplayS3::setBrightness(int percent)
{
    lvgl.setBrightness(percent);
}

void CroasterDisplayS3::setDarkMode(bool dark)
{
    lvgl.setDarkMode(dark);
}

bool CroasterDisplayS3::isDarkMode() const
{
    return lvgl.isDarkMode();
}

int CroasterDisplayS3::getBrightness() const
{
    return lvgl.brightnessPercent();
}

void CroasterDisplayS3::updateFirmwareUpdateProgress(int progress)
{
    lvgl.updateFirmwareProgress(progress);
}

void CroasterDisplayS3::updatingStatusToggle(bool isUpdating)
{
    if (isUpdatingFirmware == isUpdating)
        return;

    isUpdatingFirmware = isUpdating;
    lvgl.showFirmwarePage(isUpdating);
}

void CroasterDisplayS3::finishSplash()
{
    lvgl.finishSplash();
}

bool CroasterDisplayS3::isFirmwareUpdating() const
{
    return isUpdatingFirmware;
}
