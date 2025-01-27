#include <Arduino.h>
#include <DHT.h>
#include <MAX6675_Thermocouple.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

// Debug Macros
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)

// Pin Definitions
#define SCK_PIN D8
#define SO_PIN D7
#define CS_PIN_BT D5
#define CS_PIN_ET D6
#define DHT_PIN D4
#define DHTTYPE DHT11

// Croaster Class Definition
class Croaster
{
private:
    Thermocouple *thermocoupleBT;
    Thermocouple *thermocoupleET;
    DHT dht;

    float etHistory[60] = {}, btHistory[60] = {}, timeHistory[60] = {};
    bool historyInitialized = false;

    unsigned long lastSensorRead = 0;
    unsigned long lastRORUpdate = 0;

    bool useDummyData;

    float convertTemperature(float tempCelsius)
    {
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

    // Reset history
    void resetHistory()
    {
        for (int i = 0; i < 60; i++)
        {
            etHistory[i] = tempET;
            btHistory[i] = tempBT;
            timeHistory[i] = timer;
        }

        historyInitialized = false;

        debugln("# Temperature histories reset due to unit change.");
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
            humidity = random(30, 50);
            tempR = random(20, 30);
        }
        else
        {
            tempBT = thermocoupleBT->readCelsius();
            tempET = thermocoupleET->readCelsius();
            humidity = dht.readHumidity();
            tempR = dht.readTemperature();

            if (isnan(tempBT))
            {
                debugln("# Error: Failed to read BT!");
                tempBT = 0;
            }

            if (isnan(tempET))
            {
                debugln("# Error: Failed to read ET!");
                tempET = 0;
            }

            if (isnan(humidity) || isnan(tempR))
            {
                debugln("# Error: Failed to read DHT sensor!");
                humidity = 0;
                tempR = 0;
            }

            tempBT = convertTemperature(tempBT);
            tempET = convertTemperature(tempET);
            tempR = convertTemperature(tempR);
        }
    }

    void updateROR()
    {
        if (millis() - lastRORUpdate < 1000)
            return;

        lastRORUpdate = millis();

        if (!historyInitialized)
        {
            for (int i = 0; i < 60; i++)
            {
                etHistory[i] = tempET;
                btHistory[i] = tempBT;
                timeHistory[i] = timer;
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

        etHistory[59] = tempET;
        btHistory[59] = tempBT;
        timeHistory[59] = timer;

        float deltaET = etHistory[59] - etHistory[0];
        float deltaBT = btHistory[59] - btHistory[0];
        float deltaTimer = timeHistory[59] - timeHistory[0];

        rorET = (deltaET / deltaTimer) * 60;
        rorBT = (deltaBT / deltaTimer) * 60;
    }

public:
    Croaster(bool dummyMode, const float &version)
        : dht(DHT_PIN, DHTTYPE),
          useDummyData(dummyMode),
          versionCode(version),
          ssidName("Croaster V" + String(version))
    {
        thermocoupleBT = new MAX6675_Thermocouple(SCK_PIN, CS_PIN_BT, SO_PIN);
        thermocoupleET = new MAX6675_Thermocouple(SCK_PIN, CS_PIN_ET, SO_PIN);
    }

    ~Croaster()
    {
        delete thermocoupleBT;
        delete thermocoupleET;
    }

    String temperatureUnit = "C";

    float versionCode;

    String ssidName;

    float timer = 0, rorET = 0, rorBT = 0, tempET = 0, tempBT = 0, humidity = 0, tempR = 0;
    unsigned long intervalSendData = 3000;
    int idJsonData = 0;

    void init()
    {
        dht.begin();
        debugln("# Croaster Initialized.");
    }

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

        if (!message.isEmpty())
            doc["message"] = message;

        JsonObject data = doc.createNestedObject("data");
        data["BT"] = tempBT;
        data["ET"] = tempET;

        if (!skipCroaster)
        {
            JsonObject croaster = doc.createNestedObject("croaster");

            croaster["version"] = "V" + String(versionCode);
            croaster["versionCode"] = versionCode;
            croaster["interval"] = intervalSendData;
            croaster["timer"] = timer;
            croaster["tempET"] = tempET;
            croaster["tempBT"] = tempBT;
            croaster["rorET"] = rorET;
            croaster["rorBT"] = rorBT;
            croaster["humidity"] = humidity;
            croaster["tempR"] = tempR;
            croaster["tempUnit"] = String(temperatureUnit);
        }

        String jsonOutput;

        serializeJson(doc, jsonOutput);

        return jsonOutput;
    }
};