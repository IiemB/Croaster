#pragma once
#include <Arduino.h>
#include <Thermocouple.h>
#include <MAX6675_Thermocouple.h>
#include <SmoothThermocouple.h>
#include "Constants.h"
#include "PinConfig.h"

/**
 * @class CroasterCore
 * @brief Core functionality for managing sensors and data in the Croaster device.
 */
class CroasterCore
{
private:
    Thermocouple *thermocoupleBT;
    Thermocouple *thermocoupleET;

    float etHistory[60] = {}, btHistory[60] = {}, timeHistory[60] = {};
    bool historyInitialized = false;
    unsigned long lastSensorRead = 0;
    unsigned long lastRORUpdate = 0;

    /**
     * @brief Converts a temperature value from Celsius to the configured unit.
     * @param tempCelsius The temperature in Celsius.
     * @return The converted temperature.
     */
    float convertTemperature(float tempCelsius);

    /**
     * @brief Reads the temperature in Celsius from a thermocouple.
     * @param thermocouple Pointer to the thermocouple instance.
     * @return The temperature in Celsius.
     */
    float readCelcius(Thermocouple *thermocouple);

    /**
     * @brief Reads sensor data and updates internal state.
     */
    void readSensors();

    /**
     * @brief Updates the rate of rise (RoR) values.
     */
    void updateROR();

public:
    /**
     * @brief Constructs a CroasterCore instance.
     * @param dummyMode If true, uses dummy data instead of real sensor data.
     */
    CroasterCore(bool dummyMode = false);

    bool useDummyData;
    String temperatureUnit = "C";
    float timer = 0, rorET = 0, rorBT = 0, tempET = 0, tempBT = 0;
    unsigned long intervalSendData = 3;
    int idJsonData = 0;

    /**
     * @brief Main loop for handling sensor updates and data processing.
     */
    void loop();

    /**
     * @brief Changes the temperature unit (e.g., Celsius to Fahrenheit).
     * @param unit The new temperature unit ("C" or "F").
     */
    void changeTemperatureUnit(String unit);

    /**
     * @brief Retrieves JSON-formatted data from the device.
     * @param message Optional message to include in the JSON.
     * @param skipCroaster If true, skips including Croaster-specific data.
     * @return A JSON string representing the device data.
     */
    String getJsonData(const String &message = "", const bool &skipCroaster = false);

    /**
     * @brief Resets the historical data for sensors.
     */
    void resetHistory();

    /**
     * @brief Retrieves the SSID name for the device.
     * @return The SSID name as a string.
     */
    String ssidName();
};
