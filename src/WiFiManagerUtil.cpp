#include "WiFiManagerUtil.h"
#include "Constants.h"
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

WiFiManager wifiManager;

void configModeCallback(WiFiManager *myWiFiManager)
{
    debugln("# Config mode: " + WiFi.softAPIP().toString());
}

void setupWiFiManager(const String &apName)
{

    debugln("# Setting up WiFi Manager");

    WiFi.mode(WIFI_STA);

    wifiManager.setDebugOutput(true);
    wifiManager.setConfigPortalBlocking(false);
    wifiManager.setAPCallback(configModeCallback);
    wifiManager.setAPStaticIPConfig(IPAddress(10, 0, 1, 1), IPAddress(10, 0, 1, 1), IPAddress(255, 255, 255, 0));
    wifiManager.setClass("invert");
    wifiManager.setConnectTimeout(10);

    if (wifiManager.autoConnect(apName.c_str()))
    {
        debugln("# WiFi Connected");
    }
}

void processWiFiManager()
{
    wifiManager.process();
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
