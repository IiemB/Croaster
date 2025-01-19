#include <Thermocouple.h>
#include <MAX6675_Thermocouple.h>
#include <SPI.h>
#include "DHT.h"
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <ArduinoJson.h>

#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)

//  Setup MAX6675
#define SCK_PIN D8
#define SO_PIN D7
#define CS_PIN_BT D5
#define CS_PIN_ET D6

Thermocouple *thermocouple_bt;
Thermocouple *thermocouple_et;

//  Setup DHT11
#define DHT_PIN D4
#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);

class Croaster
{
private:
    unsigned long millisReadSensors = 0;
    unsigned long millisUpdateROR = 0;

    float timer = 0;
    float ror_et = 0;
    float ror_bt = 0;
    int arrEt[61] = {};
    int arrBt[61] = {};
    float arrTimer[61] = {};
    bool isArrInitialized = false;

    float hic = 0;

    bool useDummyData;

    void updateROR()
    {
        if (!(millis() - millisUpdateROR >= 1000))
            return;

        millisUpdateROR = millis();

        if (!isArrInitialized)
        {
            for (int i = 0; i <= 59; i++)
            {
                arrEt[i] = temp_et;
                arrBt[i] = temp_bt;
                arrTimer[i] = timer;
            }

            isArrInitialized = true;

            return;
        }

        for (int i = 0; i <= 59; i++)
        {
            arrEt[i] = arrEt[i + 1];
            arrBt[i] = arrBt[i + 1];
            arrTimer[i] = arrTimer[i + 1];
        }

        arrEt[59] = temp_et;
        arrBt[59] = temp_bt;
        arrTimer[59] = timer;

        int32_t dEt = arrEt[59] - arrEt[0];
        int32_t dBt = arrBt[59] - arrBt[0];
        float dT = arrTimer[59] - arrTimer[0];

        ror_et = (dEt / dT) * 60;
        ror_bt = (dBt / dT) * 60;

        updateJsonData();
    }

    void readSensorData()
    {
        if (!(millis() - millisReadSensors >= 250))
            return;

        millisReadSensors = millis();

        timer = millis() * 0.001;

        if (useDummyData)
        {
            // NOTE DUMMY
            temp_et = random(30, 36);
            temp_bt = random(30, 36);

            humd = random(30, 40);
            temp = random(30, 40);
            hic = dht.computeHeatIndex(temp, humd, false);
        }
        else
        {
            temp_bt = thermocouple_et->readCelsius();
            temp_et = thermocouple_bt->readCelsius();

            if (isnan(temp_bt) || temp_bt > 9000)
            {
                debugln("# Failed to read BT!");
                temp_bt = 0;
            }

            if (isnan(temp_et) || temp_et > 9000)
            {
                debugln("# Failed to read ET!");
                temp_et = 0;
            }

            humd = dht.readHumidity();
            temp = dht.readTemperature();

            if (isnan(humd) || isnan(temp))
            {
                debugln("# Failed to read from DHT sensor!");
                humd = 0;
                temp = 0;
                hic = 0;
            }
            else
            {
                hic = dht.computeHeatIndex(temp, humd, false);
            }
        }
    }

public:
    Croaster(bool dummy, String version)
    {
        useDummyData = dummy;

        versionName = version;
    }

    String versionName;

    String ssidNameStr = "Croaster v" + String(versionName);

    const char *ssidName = ssidNameStr.c_str();

    unsigned long intervalSendData = 3000;

    int temp_et = 0;
    int temp_bt = 0;

    float humd = 0;
    float temp = 0;

    String jsonData = "";
    int idJsonData = 0;

    void init()
    {
        thermocouple_bt = new MAX6675_Thermocouple(SCK_PIN, CS_PIN_BT, SO_PIN);
        thermocouple_et = new MAX6675_Thermocouple(SCK_PIN, CS_PIN_ET, SO_PIN);

        dht.begin();
    }

    void loop()
    {
        readSensorData();
        updateROR();
    }

    void updateJsonData(String message = "")
    {
        StaticJsonDocument<384> doc;

        doc["id"] = idJsonData;
        doc["roasterID"] = ssidNameStr;

        if (message != "")
        {
            doc["message"] = message;
        }

        doc["intervalSendData"] = intervalSendData;

        JsonObject data = doc.createNestedObject("data");
        data["BT"] = temp_bt;
        data["ET"] = temp_et;

        JsonObject croaster = doc.createNestedObject("croaster");
        croaster["fv"] = versionName;
        croaster["timer"] = timer;
        croaster["et"] = temp_et;
        croaster["bt"] = temp_bt;
        croaster["rorEt"] = ror_et;
        croaster["rorBt"] = ror_bt;
        croaster["humd"] = humd;
        croaster["temp"] = temp;
        croaster["hic"] = hic;

        String tempJsonData;

        serializeJson(doc, tempJsonData);

        jsonData = tempJsonData;
    }
};
