#include "CommandHandler.h"
#include "WiFiManagerUtil.h"
#include "Constants.h"
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

CommandHandler::CommandHandler(CroasterCore &core, DisplayManager &display)
    : croaster(core), displayManager(display) {}

void CommandHandler::init()
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LED_OFF);
}

void CommandHandler::loop()
{
    unsigned long now = millis();

    if (blinking && now - lastBlinkTime >= blinkDelay)
    {
        ledState = !ledState;
        digitalWrite(LED_BUILTIN, ledState ? LED_ON : LED_OFF);
        displayManager.blinkIndicator(ledState);
        lastBlinkTime = now;
        blinkCount++;

        if (blinkCount >= blinkTotal)
        {
            blinking = false;
            digitalWrite(LED_BUILTIN, LED_OFF);   // turn off when done
            displayManager.blinkIndicator(false); // turn off when done
        }
    }
}

bool CommandHandler::handle(const String &json, String &responseOut, bool &restart, bool &erase)
{
    StaticJsonDocument<192> doc;

    if (deserializeJson(doc, json))
    {
        debugln("# Invalid JSON command");
        return false;
    }

    restart = false;
    erase = false;
    responseOut = "";

    if (doc["command"].is<String>())
    {
        JsonObject json = doc.as<JsonObject>();

        handleBasicCommand(json, responseOut, restart, erase);
        return true;
    }

    if (doc["command"].is<JsonObject>())
    {
        JsonObject obj = doc["command"];
        handleJsonCommand(obj, responseOut);
        return true;
    }

    return false;
}

void CommandHandler::handleBasicCommand(const JsonObject &json, String &responseOut, bool &restart, bool &erase)
{
    if (!json["command"].is<String>())
        return;

    String command = json["command"].as<String>();

    if (command == "getDataForArtisan")
    {
        croaster.idJsonData = json["id"];

        responseOut = croaster.getJsonData(command, true);
    }
    else if (command == "getDataForICRM")
    {
        responseOut = croaster.getJsonData(command);
    }
    else if (command == "restartesp")
    {
        restart = true;

        responseOut = croaster.getJsonData(command);
    }
    else if (command == "erase")
    {
        erase = true;

        responseOut = croaster.getJsonData(command);
    }
    else if (command == "dummyOn")
    {
        croaster.useDummyData = true;
        croaster.resetHistory();
    }
    else if (command == "dummyOff")
    {
        croaster.useDummyData = false;
        croaster.resetHistory();
    }
    else if (command == "rotateScreen")
    {
        displayManager.rotateScreen();
    }
    else if (command == "blink")
    {
        blinkBuiltinLED();
    }
}

void CommandHandler::handleJsonCommand(const JsonObject &json, String &responseOut)
{
    if (json.containsKey("tempUnit"))
    {
        String unit = json["tempUnit"];
        croaster.changeTemperatureUnit(unit);
    }

    if (json.containsKey("interval"))
    {
        int interval = json["interval"].as<int>();

        if (interval >= 1)
            croaster.intervalSendData = interval;
    }

    if (json.containsKey("wifiConnect"))
    {
        if (json["ssid"].is<String>() && json["pass"].is<String>())
        {
            String ssid = json["ssid"].as<String>();
            String pass = json["pass"].as<String>();

            WiFi.begin(ssid.c_str(), pass.c_str());
        }
    }
}

void CommandHandler::blinkBuiltinLED(uint8_t times, unsigned long blinkDelay)
{

    blinkTotal = times * 2; // on/off = 2 toggles per blink
    blinkCount = 0;
    lastBlinkTime = millis();
    blinkDelay = blinkDelay;
    ledState = false;

    blinking = true;
}
