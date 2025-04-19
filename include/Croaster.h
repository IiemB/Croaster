#include <Arduino.h>
#include <max6675.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

#define debugln(x) Serial.println(x)

// Pin Definitions
#define SCK_PIN 4
#define SO_PIN 5
#define CS_PIN_BT 7
#define CS_PIN_ET 6

MAX6675 thermocoupleBT(SCK_PIN, CS_PIN_BT, SO_PIN);
MAX6675 thermocoupleET(SCK_PIN, CS_PIN_ET, SO_PIN);

// Croaster Class Definition
class Croaster
{
private:
    float etHistory[60] = {}, btHistory[60] = {}, timeHistory[60] = {};
    bool historyInitialized = false;

    unsigned long lastSensorRead = 0;
    unsigned long lastRORUpdate = 0;

    float convertTemperature(float tempCelsius)
    {
        if (isnan(tempCelsius)) // Handle invalid input
            return NAN;
        if (temperatureUnit == "F") // Fahrenheit
        {
            return (tempCelsius * 9.0 / 5.0) + 32.0;
        }
        else if (temperatureUnit == "K") // Kelvin
        {
            return tempCelsius + 273.15;
        }
        else // Celsius
        {
            return tempCelsius;
        }
    }

    void readSensors()
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

            if (isnan(bt))
            {
                debugln("# Error: Failed to read BT! Setting to null.");
                tempBT = NAN; // Indicate invalid reading
            }
            else
            {
                tempBT = convertTemperature(bt);
            }

            if (isnan(et))
            {
                debugln("# Error: Failed to read ET! Setting to null.");
                tempET = NAN; // Indicate invalid reading
            }
            else
            {
                tempET = convertTemperature(et);
            }
        }
    }

    void updateROR()
    {
        if (millis() - lastRORUpdate < 1000)
            return;

        lastRORUpdate = millis();

        if (!historyInitialized)
        {
            // Initialize history with 60 seconds of data
            for (int i = 0; i < 60; i++)
            {
                etHistory[i] = isnan(tempET) ? 0 : tempET;
                btHistory[i] = isnan(tempBT) ? 0 : tempBT;
                timeHistory[i] = timer - (59 - i); // Simulate 60 seconds of past timestamps
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

        // Update ET history and RoR if valid
        if (!isnan(tempET))
        {
            etHistory[59] = tempET;
            float deltaET = etHistory[59] - etHistory[0];
            float deltaTimer = timeHistory[59] - timeHistory[0];
            if (deltaTimer > 0) // Prevent division by near-zero
                rorET = (deltaET / deltaTimer) * 60;
            else
            {
                rorET = 0;
            }
        }
        else
        {
            etHistory[59] = etHistory[58]; // Retain last valid value
            rorET = 0;                     // Reset RoR for invalid reading
            debugln("# Skipping ET ROR update due to invalid reading.");
        }

        // Update BT history and RoR if valid
        if (!isnan(tempBT))
        {
            btHistory[59] = tempBT;
            float deltaBT = btHistory[59] - btHistory[0];
            float deltaTimer = timeHistory[59] - timeHistory[0];
            if (deltaTimer > 0) // Prevent division by near-zero
                rorBT = (deltaBT / deltaTimer) * 60;
            else
            {
                rorBT = 0;
            }
        }
        else
        {
            btHistory[59] = btHistory[58]; // Retain last valid value
            rorBT = 0;                     // Reset RoR for invalid reading
            debugln("# Skipping BT ROR update due to invalid reading.");
        }
    }

public:
    Croaster(const double &version, bool dummyMode = false)
        : useDummyData(dummyMode),
          versionCode(version),
          ssidName("Croaster V" + String(version))
    {
    }

    bool useDummyData;

    String temperatureUnit = "C";

    double versionCode;

    String ssidName;

    float timer = 0, rorET = 0, rorBT = 0, tempET = 0, tempBT = 0;
    unsigned long intervalSendData = 3000;
    int idJsonData = 0;

    String ipAddress = "";

    void loop()
    {
        readSensors();
        updateROR();
    }

    void changeTemperatureUnit(String unit)
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

    String getJsonData(const String &message = "", const bool &skipCroaster = false)
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
            data["BT"].set(nullptr); // Set null for invalid BT
        else
            data["BT"] = tempBT;

        if (isnan(tempET))
            data["ET"].set(nullptr); // Set null for invalid ET
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
                croaster["tempBT"].set(nullptr); // Set null for invalid BT
            else
                croaster["tempBT"] = tempBT;

            if (isnan(tempET))
                croaster["tempET"].set(nullptr); // Set null for invalid ET
            else
                croaster["tempET"] = tempET;

            if (isnan(tempET))
                croaster["rorET"].set(nullptr); // Set null for invalid ET
            else
                croaster["rorET"] = rorET;

            if (isnan(tempBT))
                croaster["rorBT"].set(nullptr); // Set null for invalid BT
            else
                croaster["rorBT"] = rorBT;

            croaster["tempUnit"] = String(temperatureUnit);
        }

        String jsonOutput;

        serializeJson(doc, jsonOutput);

        return jsonOutput;
    }

    // Reset history
    void resetHistory()
    {
        for (int i = 0; i < 60; i++)
        {
            etHistory[i] = isnan(tempET) ? 0 : tempET;
            btHistory[i] = isnan(tempBT) ? 0 : tempBT;
            timeHistory[i] = timer - (59 - i); // Simulate 60 seconds of past timestamps
        }

        historyInitialized = false;

        debugln("# Temperature histories reset due to unit change.");
    }
};