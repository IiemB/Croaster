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

bool CommandHandler::handle(const String &json, String &responseOut)
{
    JsonDocument doc;

    if (deserializeJson(doc, json))
    {
        debugln("# Invalid JSON command : " + json);
        return false;
    }

    responseOut = "";

    if (doc["command"].is<String>())
    {
        JsonObject json = doc.as<JsonObject>();

        handleBasicCommand(json, responseOut);

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

void CommandHandler::handleBasicCommand(const JsonObject &json, String &responseOut)
{
    if (!json["command"].is<String>())
        return;

    String command = json["command"].as<String>();

    if (command == "getArtisanData")
    {
        int id = json["id"].as<int>();

        responseOut = croaster.getJsonData(id);
    }
    else if (command == "restartesp")
    {
        ESP.restart();
    }
    else if (command == "erase")
    {
        eraseESP();
    }
    else if (command == "dummyToggle")
    {
        croaster.toggleDummyData();
    }
    else if (command == "rotateScreen")
    {
        displayManager.rotateScreen();
    }
    else if (command == "blink")
    {
        blinkBuiltinLED();
    }
    else if (command == "displayToggle")
    {
        displayManager.displayToggle();
    }
    else if (command == "getDeviceInfo")
    {
        responseOut = genResponseCommand(command, croaster.getDeviceInfo());
    }
    else if (command == "getExtra")
    {
        responseOut = genResponseCommand(command, getExtraData());
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

String CommandHandler::genResponseCommand(const String command, const String response)
{
    JsonDocument doc;

    JsonDocument responseDoc;

    doc["command"] = command;

    if (!response.isEmpty() && !deserializeJson(responseDoc, response))
    {
        doc["response"] = responseDoc;
    }

    String jsonOutput;

    serializeJson(doc, jsonOutput);

    return jsonOutput;
}

String CommandHandler::genRandomString(int length)
{
    const char charset[] = "0123456789"
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "abcdefghijklmnopqrstuvwxyz";

    String result;

    for (size_t i = 0; i < length; i++)
    {
        result += charset[random(0, sizeof(charset) - 1)];
    }

    return result;
}

String CommandHandler::getExtraData()
{

    /*
   NOTE
   You can add more fields to the JSON document that will be readed by ICRM app,
   such as historical data or other sensor readings.
   */

    JsonDocument doc;

    doc["string"] = genRandomString(10);
    doc["number"] = random(100, 999);
    doc["boolean"] = random(0, 2) == 1;

    String jsonOutput;

    serializeJson(doc, jsonOutput);

    return jsonOutput;
}
