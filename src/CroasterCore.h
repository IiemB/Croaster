#pragma once
#include <Arduino.h>
#include <Thermocouple.h>
#include <MAX6675_Thermocouple.h>
#include <SmoothThermocouple.h>
#include "Constants.h"
#include "PinConfig.h"

/**
 * @class CroasterCore
 * @brief Core class for managing the Croaster functionality, including sensor readings,
 *        temperature conversions, rate of rise (ROR) calculations, and data handling.
 *
 * This class provides methods to interact with thermocouples, manage temperature data,
 * and generate JSON data for external use. It supports both real sensor data and dummy data
 * for testing purposes.
 *
 * @brief Constructor for the CroasterCore class.
 * @param dummyMode If true, the class operates in dummy data mode for testing.
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
     * @private
     * @brief Converts a temperature from Celsius to the currently selected unit.
     * @param tempCelsius The temperature in Celsius.
     * @return The converted temperature in the selected unit.
     */
    float convertTemperature(float tempCelsius);

    /**
     * @private
     * @brief Reads the temperature from a given thermocouple.
     * @param thermocouple Pointer to the thermocouple to read from.
     * @return The temperature in Celsius.
     */
    float readCelcius(Thermocouple *thermocouple);

    /**
     * @private
     * @brief Reads sensor data and updates the current temperature values.
     */
    void readSensors();

    /**
     * @private
     * @brief Updates the rate of rise (ROR) values for ET and BT.
     */
    void updateROR();

public:
    CroasterCore(bool dummyMode = false);

    bool useDummyData;
    String temperatureUnit = "C";
    float timer = 0, rorET = 0, rorBT = 0, tempET = 0, tempBT = 0;
    unsigned long intervalSendData = 3;
    int idJsonData = 0;
    String ipAddress = "";

    /**
     * @brief Main loop function to handle periodic tasks such as sensor readings and ROR updates.
     */
    void loop();

    /**
     * @brief Changes the temperature unit used by the system.
     * @param unit The desired temperature unit ("C" for Celsius, "F" for Fahrenheit, etc.).
     */
    void changeTemperatureUnit(String unit);

    /**
     * @brief Generates a JSON string containing the current system data.
     * @param message Optional message to include in the JSON data.
     * @param skipCroaster If true, skips including Croaster-specific data in the JSON.
     * @return A JSON-formatted string representing the current system state.
     */
    String getJsonData(const String &message = "", const bool &skipCroaster = false);

    /**
     * @brief Resets the historical data for temperature and time.
     */
    void resetHistory();

    /**
     * @brief Retrieves the SSID (Service Set Identifier) name.
     *
     * @return String The SSID name as a string.
     */
    String ssidName();
};
