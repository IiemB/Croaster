#include "CommandHandler.h"
#include "WiFiManagerUtil.h"
#include <ArduinoJson.h>
#include "Constants.h"
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

CommandHandler::CommandHandler(CroasterCore &core, DisplayManager &display)
    : croaster(core), displayManager(display) {}

void CommandHandler::begin()
{
    #ifdef LED_BUILTIN
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LED_OFF);
    #endif
}

void CommandHandler::loop()
{
    unsigned long now = millis();

    if (blinking && now - lastBlinkTime >= blinkDelay)
    {
        ledState = !ledState;
        #ifdef LED_BUILTIN
        digitalWrite(LED_BUILTIN, ledState ? LED_ON : LED_OFF);
        #endif
        displayManager.blinkIndicator(ledState);
        lastBlinkTime = now;
        blinkCount++;

        if (blinkCount >= blinkTotal)
        {
            blinking = false;
        #ifdef LED_BUILTIN
            digitalWrite(LED_BUILTIN, LED_OFF);   // turn off when done
        #endif
            displayManager.blinkIndicator(false); // turn off when done
        }
    }
}

bool CommandHandler::handle(const String &json, String &responseOut, bool &restart, bool &erase)
{
    JsonDocument doc;

    if (deserializeJson(doc, json))
    {
        debugln("# Invalid JSON command : " + json);
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

    if (command == "getArtisanData")
    {
        int id = json["id"].as<int>();

        responseOut = croaster.getJsonData(command, true, id);
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
        croaster.changeDummyData(true);
    }
    else if (command == "dummyOff")
    {
        croaster.changeDummyData(false);
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
    if (json["tempUnit"].is<String>())
    {
        String tempUnit = json["tempUnit"].as<String>();

        croaster.changeTemperatureUnit(tempUnit);
    }

    if (json["interval"].is<int>())
    {
        int interval = json["interval"].as<int>();

        if (interval >= 1)
            croaster.changeIntervalSendData(interval);
    }

    if (json["correctionBt"].is<double>())
    {
        double correctionBt = json["correctionBt"].as<double>();

        croaster.changeCorrectionBt(correctionBt);
    }

    if (json["correctionEt"].is<double>())
    {
        double correctionEt = json["correctionEt"].as<double>();

        croaster.changeCorrectionEt(correctionEt);
    }

    if (json["wifiConnect"].is<JsonObject>())
    {
        JsonObject obj = json["wifiConnect"];

        if (obj["ssid"].is<String>() && obj["pass"].is<String>())
        {
            String ssid = obj["ssid"].as<String>();
            String pass = obj["pass"].as<String>();

            WiFi.begin(ssid.c_str(), pass.c_str());

            debugln("# Connecting to " + ssid);
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
