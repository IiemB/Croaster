#include "WiFiManagerUtil.h"
#include "Constants.h"

WiFiManager wifiManager;

void configModeCallback(WiFiManager *myWiFiManager)
{
    debugln("# Config mode: " + WiFi.softAPIP().toString());
}

void setupWiFiManager(const String &apName)
{
    debugln("# Setting up WiFi Manager");

    wifiManager.setDebugOutput(true);
    wifiManager.setConfigPortalBlocking(false);
    wifiManager.setAPCallback(configModeCallback);
    wifiManager.setClass("invert");
    wifiManager.setConnectTimeout(10);
    wifiManager.setAPStaticIPConfig(
        IPAddress(10, 0, 1, 1),
        IPAddress(10, 0, 1, 1),
        IPAddress(255, 255, 255, 0));

    if (wifiManager.autoConnect(apName.c_str()))
    {
        debugln("# WiFi Connected");
    }
}

void restartESP()
{
    wifiManager.reboot();
}

void eraseESP()
{
    wifiManager.erase();
    restartESP();
}
