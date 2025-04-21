#include "CommandHandler.h"
#include "Constants.h"
#include "WiFiManagerUtil.h"

bool handleCommand(const String &json, CroasterCore &croaster, String &responseOut, bool &restart, bool &erase)
{
    StaticJsonDocument<96> doc;

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
        String cmd = doc["command"];

        if (cmd == "getData")
        {
            responseOut = croaster.getJsonData(cmd);
        }
        else if (cmd == "restartesp")
        {
            responseOut = croaster.getJsonData(cmd);
            restart = true;
        }
        else if (cmd == "erase")
        {
            responseOut = croaster.getJsonData(cmd);
            erase = true;
        }
        else if (cmd == "dummyOn")
        {
            croaster.useDummyData = true;
            croaster.resetHistory();
        }
        else if (cmd == "dummyOff")
        {
            croaster.useDummyData = false;
            croaster.resetHistory();
        }

        return true;
    }

    if (doc["command"].is<JsonObject>())
    {
        JsonObject cmd = doc["command"];

        if (cmd.containsKey("tempUnit"))
        {
            String unit = cmd["tempUnit"];
            croaster.changeTemperatureUnit(unit);
            responseOut = croaster.getJsonData(unit);
            return true;
        }

        if (cmd.containsKey("interval"))
        {
            croaster.intervalSendData = cmd["interval"].as<int>() * 1000;
            return true;
        }
    }

    return false;
}
