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

    double etHistory[60] = {}, btHistory[60] = {}, timeHistory[60] = {};
    bool historyInitialized = false;
    unsigned long lastSensorRead = 0;
    unsigned long lastRORUpdate = 0;

    double correctionBt = 0, correctionEt = 0;

    String tempUnit = "C";

    unsigned long intervalSend = 3;

    bool useDummyData;

    /**
     * @brief Converts a temperature value from Celsius to the configured unit.
     * @param tempCelsius The temperature in Celsius.
     * @return The converted temperature.
     */
    double convertTemperature(double tempCelsius);

    /**
     * @brief Reads the temperature in Celsius from a thermocouple.
     * @param thermocouple Pointer to the thermocouple instance.
     * @return The temperature in Celsius.
     */
    double readCelcius(Thermocouple *thermocouple);

    /**
     * @brief Reads sensor data and updates internal state.
     */
    void readSensors();

    /**
     * @brief Updates the rate of rise (RoR) values.
     */
    void updateROR();

    /**
     * @brief Resets the historical data for sensors.
     */
    void resetHistory(String item = "something");

    /**
     * @brief Generates a random string of a specified length.
     * @param length The length of the random string to generate.
     * @return A random string of the specified length.
     */
    String genRandomString(int length);

    /**
     * @brief Rounds a double value to 2 decimal places.
     * @param value The value to round.
     * @return The rounded value.
     */
    double roundTo2(double value);

public:
    double timer = 0, rorEt = 0, rorBt = 0, tempEt = 0, tempBt = 0;

    /**
     * @brief Constructs a CroasterCore instance.
     * @param dummyMode If true, uses dummy data instead of real sensor data.
     */
    CroasterCore(bool dummyMode = false);

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
     * @brief Retrieves the temperature unit used in the application.
     *
     * @return String representing the temperature unit (e.g., "Celsius" or "Fahrenheit").
     */
    String temperatureUnit();

    /**
     * @brief Retrieves the interval for sending data.
     *
     * @return The interval in seconds as an unsigned long.
     */
    unsigned long intervalSendData();

    /**
     * @brief Changes the interval for sending data.
     *
     * @param interval The new interval in seconds.
     */
    void changeIntervalSendData(unsigned long interval);

    /**
     * @brief Changes the state of using dummy data.
     *
     * @param useDummy Set to true to use dummy data, false otherwise.
     */
    void changeDummyData(bool useDummy);

    /**
     * @brief Changes the correction value for BT.
     *
     * @param value The new correction value to be set.
     */
    void changeCorrectionBt(double value);

    /**
     * @brief Changes the correction value for ET.
     *
     * @param value The new correction value to be set.
     */
    void changeCorrectionEt(double value);

    /**
     * @brief Retrieves JSON data based on the provided parameters.
     * @param id Identifier used to fetch specific JSON data. Defaults to 0.
     * @return A String containing the JSON data.
     */
    String getJsonData(int id = 0);

    /**
     * @brief Retrieves the SSID name for the device.
     * @return The SSID name as a string.
     */
    String ssidName();

    /**
     * @brief Retrieves the device information as a string.
     * @return A string containing the device information.
     */
    String getDeviceInfo();
};
