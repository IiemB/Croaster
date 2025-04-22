#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Thermocouple.h>
#include <MAX6675_Thermocouple.h>
#include <SmoothThermocouple.h>
#include "Constants.h"
#include "PinConfig.h"

class CroasterCore
{
private:
    Thermocouple *thermocoupleBT;
    Thermocouple *thermocoupleET;

    float etHistory[60] = {}, btHistory[60] = {}, timeHistory[60] = {};
    bool historyInitialized = false;
    unsigned long lastSensorRead = 0;
    unsigned long lastRORUpdate = 0;

    float convertTemperature(float tempCelsius);
    float readCelcius(Thermocouple *thermocouple);
    void readSensors();
    void updateROR();

public:
    CroasterCore(bool dummyMode = false);

    bool useDummyData;
    String temperatureUnit = "C";
    String ssidName;
    float timer = 0, rorET = 0, rorBT = 0, tempET = 0, tempBT = 0;
    unsigned long intervalSendData = 3000;
    int idJsonData = 0;
    String ipAddress = "";

    void loop();
    void changeTemperatureUnit(String unit);
    String getJsonData(const String &message = "", const bool &skipCroaster = false);
    void resetHistory();
};
