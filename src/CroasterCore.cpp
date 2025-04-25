#include "CroasterCore.h"
#include "DeviceIdentity.h"
#include "Constants.h"

CroasterCore::CroasterCore(bool dummyMode)
    : useDummyData(dummyMode)
{

    thermocoupleBT = new SmoothThermocouple(new MAX6675_Thermocouple(SCK_PIN, CS_PIN_BT, SO_PIN), SMOOTHING_FACTOR);
    thermocoupleET = new SmoothThermocouple(new MAX6675_Thermocouple(SCK_PIN, CS_PIN_ET, SO_PIN), SMOOTHING_FACTOR);
}

float CroasterCore::convertTemperature(float tempCelsius)
{
    if (isnan(tempCelsius))
        return NAN;
    if (temperatureUnit == "F")
        return (tempCelsius * 9.0 / 5.0) + 32.0;
    if (temperatureUnit == "K")
        return tempCelsius + 273.15;
    return tempCelsius;
}

float CroasterCore::readCelcius(Thermocouple *thermocouple)
{
    float value = thermocouple->readCelsius();

    if (value > 0.0)
    {
        return value;
    }

    return NAN;
}

void CroasterCore::readSensors()
{
    unsigned long now = millis();

    if (now - lastSensorRead < 250)
        return;

    lastSensorRead = now;
    timer = lastSensorRead / 1000.0;

    if (useDummyData)
    {
        tempBT = random(30, 40);
        tempET = random(30, 40);

        return;
    }

    float bt = readCelcius(thermocoupleBT);
    float et = readCelcius(thermocoupleET);

    if (isnan(bt))
    {
        tempBT = NAN; // Indicate invalid reading
    }
    else
    {
        tempBT = convertTemperature(bt);
    }

    if (isnan(et))
    {
        tempET = NAN; // Indicate invalid reading
    }
    else
    {
        tempET = convertTemperature(et);
    }
}

void CroasterCore::updateROR()
{
    unsigned long now = millis();

    if (now - lastRORUpdate < 1000)
        return;

    lastRORUpdate = now;

    if (!historyInitialized)
    {
        for (int i = 0; i < 60; i++)
        {
            etHistory[i] = isnan(tempET) ? 0 : tempET; // Use 0 if NAN to initialize
            btHistory[i] = isnan(tempBT) ? 0 : tempBT; // Use 0 if NAN to initialize
            timeHistory[i] = timer;
        }

        historyInitialized = true;

        return;
    }

    // Shift history
    for (int i = 0; i < 59; i++)
    {
        etHistory[i] = etHistory[i + 1];
        btHistory[i] = btHistory[i + 1];
        timeHistory[i] = timeHistory[i + 1];
    }

    timeHistory[59] = timer; // Update timer history

    float deltaTimer = timeHistory[59] - timeHistory[0];

    // Update ET history and RoR if valid
    if (!isnan(tempET))
    {
        etHistory[59] = tempET;
        float deltaET = etHistory[59] - etHistory[0];

        bool validDelta = deltaET > 0 && deltaTimer > 0;

        rorET = validDelta ? (deltaET / deltaTimer) * 60 : 0;
    }
    else
    {
        etHistory[59] = etHistory[58]; // Retain last valid value
        rorET = 0;                     // Reset RoR for invalid reading
    }

    // Update BT history and RoR if valid
    if (!isnan(tempBT))
    {
        btHistory[59] = tempBT;
        float deltaBT = btHistory[59] - btHistory[0];

        bool validDelta = deltaBT > 0 && deltaTimer > 0;

        rorBT = validDelta ? (deltaBT / deltaTimer) * 60 : 0;
    }
    else
    {
        btHistory[59] = btHistory[58]; // Retain last valid value
        rorBT = 0;                     // Reset RoR for invalid reading
    }
}

void CroasterCore::loop()
{
    readSensors();
    updateROR();
}

void CroasterCore::changeTemperatureUnit(String unit)
{
    if (unit == "C" || unit == "F" || unit == "K")
    {
        if (temperatureUnit == unit)
            resetHistory();

        temperatureUnit = unit;
        debugln("# Temperature unit set to " + unit);
    }
    else
    {
        debugln("# Invalid temperature unit. Use C, F, or K.");
    }
}

void CroasterCore::resetHistory()
{
    for (int i = 0; i < 60; i++)
    {
        etHistory[i] = isnan(tempET) ? 0 : tempET; // Use 0 if NAN
        btHistory[i] = isnan(tempBT) ? 0 : tempBT; // Use 0 if NAN
        timeHistory[i] = timer;
    }

    historyInitialized = false;

    debugln("# Temperature histories reset due to unit change.");
}

String CroasterCore::ssidName()
{
    return getDeviceName("[", "] Croaster V" + String(version));
}

String CroasterCore::getJsonData(const String &message, const bool &skipCroaster)
{
    StaticJsonDocument<384> doc;

    doc["id"] = idJsonData;

    if (!ipAddress.isEmpty())
        doc["ipAddress"] = ipAddress;

    if (!message.isEmpty())
        doc["message"] = message;

    JsonObject data = doc.createNestedObject("data");
    if (!isnan(tempBT))
        data["BT"] = tempBT;
    if (!isnan(tempET))
        data["ET"] = tempET;

    if (!skipCroaster)
    {
        JsonObject croaster = doc.createNestedObject("croaster");

        croaster["versionCode"] = version;
        croaster["interval"] = intervalSendData;
        croaster["timer"] = timer;
        if (!isnan(tempBT))
        {
            croaster["tempBT"] = tempBT;
            croaster["rorBT"] = rorBT;
        }
        if (!isnan(tempET))
        {
            croaster["tempET"] = tempET;
            croaster["rorET"] = rorET;
        }
        croaster["tempUnit"] = temperatureUnit;
    }

    String jsonOutput;

    serializeJson(doc, jsonOutput);

    return jsonOutput;
}
