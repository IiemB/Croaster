#include "CommandHandler.h"
#include "WiFiManagerUtil.h"
#include "Constants.h"

CommandHandler::CommandHandler(CroasterCore &core, DisplayManager &display)
    : croaster(core), displayManager(display) {}

void CommandHandler::init()
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LED_OFF);
}

void CommandHandler::loop()
{
    if (blinking)
    {
        unsigned long now = millis();
        if (now - lastBlinkTime >= blinkDelay)
        {
            ledState = !ledState;
            digitalWrite(LED_BUILTIN, ledState ? LED_ON : LED_OFF);
            lastBlinkTime = now;
            blinkCount++;

            if (blinkCount >= blinkTotal)
            {
                blinking = false;
                digitalWrite(LED_BUILTIN, LED_OFF); // turn off when done
            }
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
    else if (command == "blink")
    {
        blinkBuiltinLED();
    }
    else if (command == "rotateScreen")
    {
        displayManager.rotateScreen();
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
        int interval = json["interval"].as<int>() * 1000;
        croaster.intervalSendData = interval;
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
