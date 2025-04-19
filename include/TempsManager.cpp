#include "TempsManager.h"

MAX6675 thermocoupleBT(SCK_PIN, CS_PIN_BT, SO_PIN);
MAX6675 thermocoupleET(SCK_PIN, CS_PIN_ET, SO_PIN);

TempsManager::TempsManager(const double &version, bool dummyMode)
    : useDummyData(dummyMode),
      versionCode(version),
      ssidName("Croaster V" + String(version)) {}

float TempsManager::convertTemperature(float tempCelsius)
{
    if (isnan(tempCelsius))
        return NAN;
    if (temperatureUnit == "F")
        return (tempCelsius * 9.0 / 5.0) + 32.0;
    if (temperatureUnit == "K")
        return tempCelsius + 273.15;
    return tempCelsius;
}

void TempsManager::readSensors()
{
    if (millis() - lastSensorRead < 250)
        return;
    lastSensorRead = millis();
    timer = millis() / 1000.0;

    if (useDummyData)
    {
        tempBT = random(30, 40);
        tempET = random(30, 40);
    }
    else
    {
        float bt = thermocoupleBT.readCelsius();
        float et = thermocoupleET.readCelsius();

        tempBT = isnan(bt) ? NAN : convertTemperature(bt);
        tempET = isnan(et) ? NAN : convertTemperature(et);
    }
}

void TempsManager::updateROR()
{
    if (millis() - lastRORUpdate < 1000)
        return;
    lastRORUpdate = millis();

    if (!historyInitialized)
    {
        for (int i = 0; i < 60; i++)
        {
            etHistory[i] = isnan(tempET) ? 0 : tempET;
            btHistory[i] = isnan(tempBT) ? 0 : tempBT;
            timeHistory[i] = timer - (59 - i);
        }
        historyInitialized = true;
        return;
    }

    for (int i = 0; i < 59; i++)
    {
        etHistory[i] = etHistory[i + 1];
        btHistory[i] = btHistory[i + 1];
        timeHistory[i] = timeHistory[i + 1];
    }

    // Update RoR ET
    if (!isnan(tempET))
    {
        etHistory[59] = tempET;
        float deltaET = etHistory[59] - etHistory[0];
        float deltaTimer = timeHistory[59] - timeHistory[0];
        rorET = deltaTimer > 0 ? (deltaET / deltaTimer) * 60 : 0;
    }
    else
    {
        etHistory[59] = etHistory[58];
        rorET = 0;
    }

    // Update RoR BT
    if (!isnan(tempBT))
    {
        btHistory[59] = tempBT;
        float deltaBT = btHistory[59] - btHistory[0];
        float deltaTimer = timeHistory[59] - timeHistory[0];
        rorBT = deltaTimer > 0 ? (deltaBT / deltaTimer) * 60 : 0;
    }
    else
    {
        btHistory[59] = btHistory[58];
        rorBT = 0;
    }
}

void TempsManager::loop()
{
    readSensors();
    updateROR();
}

void TempsManager::changeTemperatureUnit(String unit)
{
    if (unit == "C" || unit == "F" || unit == "K")
    {
        temperatureUnit = unit;
        resetHistory();
        debugln("# Temperature unit set to " + unit);
    }
    else
    {
        debugln("# Invalid temperature unit. Use C, F, or K.");
    }
}

void TempsManager::resetHistory()
{
    for (int i = 0; i < 60; i++)
    {
        etHistory[i] = isnan(tempET) ? 0 : tempET;
        btHistory[i] = isnan(tempBT) ? 0 : tempBT;
        timeHistory[i] = timer - (59 - i);
    }
    historyInitialized = false;
    debugln("# Temperature histories reset due to unit change.");
}

String TempsManager::getJsonData(const String &message, const bool &skipCroaster)
{
    StaticJsonDocument<384> doc;
    doc["id"] = idJsonData;
    doc["roasterID"] = ssidName;

    if (!ipAddress.isEmpty())
        doc["ipAddress"] = ipAddress;
    if (!message.isEmpty())
        doc["message"] = message;

    JsonObject data = doc.createNestedObject("data");

    if (isnan(tempBT))
        data["BT"].set(nullptr);
    else
        data["BT"] = tempBT;

    if (isnan(tempET))
        data["ET"].set(nullptr);
    else
        data["ET"] = tempET;

    if (!skipCroaster)
    {
        JsonObject croaster = doc.createNestedObject("croaster");
        croaster["version"] = "V" + String(versionCode);
        croaster["versionCode"] = versionCode;
        croaster["interval"] = intervalSendData;
        croaster["timer"] = timer;
        if (isnan(tempBT))
            croaster["tempBT"].set(nullptr);
        else
            croaster["tempBT"] = tempBT;
        if (isnan(tempET))
            croaster["tempET"].set(nullptr);
        else
            croaster["tempET"] = tempET;
        if (isnan(tempBT))
            croaster["rorBT"].set(nullptr);
        else
            croaster["rorBT"] = rorBT;
        if (isnan(tempET))
            croaster["rorET"].set(nullptr);
        else
            croaster["rorET"] = rorET;
        croaster["tempUnit"] = temperatureUnit;
    }

    String output;
    serializeJson(doc, output);
    return output;
}
