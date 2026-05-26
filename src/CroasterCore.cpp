#include "CroasterCore.h"
#include "DeviceIdentity.h"
#include "Constants.h"
#include <ArduinoJson.h>

CroasterCore::CroasterCore(bool dummyMode)
    : useDummyData(dummyMode)
{
    thermocoupleBT = new SmoothThermocouple(new MAX6675_Thermocouple(SCK_PIN, CS_PIN_BT, SO_PIN), SMOOTHING_FACTOR);
    thermocoupleET = new SmoothThermocouple(new MAX6675_Thermocouple(SCK_PIN, CS_PIN_ET, SO_PIN), SMOOTHING_FACTOR);
}

double CroasterCore::convertTemperature(double tempCelsius)
{
    if (isnan(tempCelsius))
        return NAN;
    if (tempUnit == "F")
        return (tempCelsius * 9.0 / 5.0) + 32.0;
    if (tempUnit == "K")
        return tempCelsius + 273.15;
    return tempCelsius;
}

double CroasterCore::readCelcius(Thermocouple *thermocouple)
{
    double value = thermocouple->readCelsius();

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
        tempBt = random(30, 40) + correctionBt;
        tempEt = random(30, 40) + correctionEt;

        return;
    }

    double bt = readCelcius(thermocoupleBT);
    double et = readCelcius(thermocoupleET);

    if (isnan(bt))
    {
        tempBt = NAN; // Indicate invalid reading
    }
    else
    {
        tempBt = convertTemperature(bt) + correctionBt;
    }

    if (isnan(et))
    {
        tempEt = NAN; // Indicate invalid reading
    }
    else
    {
        tempEt = convertTemperature(et) + correctionEt;
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
            etHistory[i] = isnan(tempEt) ? 0 : tempEt; // Use 0 if NAN to initialize
            btHistory[i] = isnan(tempBt) ? 0 : tempBt; // Use 0 if NAN to initialize
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

    double deltaTimer = timeHistory[59] - timeHistory[0];

    // Update ET history and RoR if valid
    if (!isnan(tempEt))
    {
        etHistory[59] = tempEt;
        double deltaET = etHistory[59] - etHistory[0];

        bool validDelta = deltaET > 0 && deltaTimer > 0;

        rorEt = validDelta ? (deltaET / deltaTimer) * 60 : 0;
    }
    else
    {
        etHistory[59] = etHistory[58]; // Retain last valid value
        rorEt = 0;                     // Reset RoR for invalid reading
    }

    // Update BT history and RoR if valid
    if (!isnan(tempBt))
    {
        btHistory[59] = tempBt;
        double deltaBT = btHistory[59] - btHistory[0];

        bool validDelta = deltaBT > 0 && deltaTimer > 0;

        rorBt = validDelta ? (deltaBT / deltaTimer) * 60 : 0;
    }
    else
    {
        btHistory[59] = btHistory[58]; // Retain last valid value
        rorBt = 0;                     // Reset RoR for invalid reading
    }
}

void CroasterCore::loop()
{
    readSensors();
    updateROR();
}

String CroasterCore::temperatureUnit()
{
    return tempUnit;
}

void CroasterCore::changeTemperatureUnit(String unit)
{
    if (tempUnit == unit)
    {
        debugln("# tempUnit not changed because it same with current value");

        return;
    }

    if (unit == "C" || unit == "F" || unit == "K")
    {
        tempUnit = unit;

        resetHistory("tempUnit");
    }
}

unsigned long CroasterCore::intervalSendData()
{
    return intervalSend;
}

void CroasterCore::changeIntervalSendData(unsigned long interval)
{
    if (intervalSend == interval)
    {
        debugln("# intervalSend not changed because it same with current value");

        return;
    }

    intervalSend = interval;

    resetHistory("intervalSend");
}

void CroasterCore::changeDummyData(bool useDummy)
{
    if (useDummyData == useDummy)
    {
        debugln("# useDummyData not changed because it same with current value");

        return;
    }

    useDummyData = useDummy;

    resetHistory("useDummyData");
}

void CroasterCore::changeCorrectionBt(double value)
{
    if (correctionBt == value)
    {
        debugln("# correctionBt not changed because it same with current value");

        return;
    }

    correctionBt = value;

    resetHistory("correctionBt");
}

void CroasterCore::changeCorrectionEt(double value)
{
    if (correctionEt == value)
    {
        debugln("# correctionEt not changed because it same with current value");

        return;
    }

    correctionEt = value;

    resetHistory("correctionEt");
}

void CroasterCore::resetHistory(String item)
{
    for (int i = 0; i < 60; i++)
    {
        etHistory[i] = isnan(tempEt) ? 0 : tempEt; // Use 0 if NAN
        btHistory[i] = isnan(tempBt) ? 0 : tempBt; // Use 0 if NAN
        timeHistory[i] = timer;
    }

    historyInitialized = false;

    debugln("# Histories reset due to " + item + " change.");
}

String CroasterCore::genRandomString(int length)
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

double CroasterCore::roundTo2(double value)
{
    return round(value * 100.0) / 100.0;
}

String CroasterCore::ssidName()
{
    return getDeviceName("[", "] Croaster V" + String(version, 3));
}

String CroasterCore::getJsonData(int id)
{
    JsonDocument doc;

    doc["id"] = id;
    doc["name"] = ssidName();

    JsonObject data = doc["data"].to<JsonObject>();

    data["interval"] = intervalSend;
    data["timer"] = timer;
    if (!isnan(tempBt))
    {
        data["BT"] = roundTo2(tempBt);
        data["DBT"] = roundTo2(rorBt);
    }
    if (!isnan(tempEt))
    {
        data["ET"] = roundTo2(tempEt);
        data["DET"] = roundTo2(rorEt);
    }

    data["cBT"] = roundTo2(correctionBt);
    data["cET"] = roundTo2(correctionEt);

    data["temp"] = tempUnit;

    /*
    NOTE
    You can add more fields to the JSON document that will be readed by ICRM app,
    such as historical data or other sensor readings.
    */

    JsonObject extra = data["extra"].to<JsonObject>();

    extra["string"] = genRandomString(10);
    extra["number"] = random(100, 999);
    extra["boolean"] = random(0, 2) == 1;

    String jsonOutput;

    serializeJson(doc, jsonOutput);

    return jsonOutput;
}

String CroasterCore::getDeviceInfo()
{
    String ipAddress = getIpAddress();

    String ssid = getSsidName();

    JsonDocument doc;

    doc["ver"] = version;

    if (!ipAddress.isEmpty())
        doc["ip"] = ipAddress;

    if (!ssid.isEmpty())
        doc["ssid"] = ssid;

    doc["name"] = ssidName();

    doc["timer"] = timer;

    String jsonOutput;

    serializeJson(doc, jsonOutput);

    return jsonOutput;
}
