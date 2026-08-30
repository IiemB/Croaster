#pragma once
#include <Arduino.h>
#include <MAX6675_Thermocouple.h>
#include <SmoothThermocouple.h>
#include <Thermocouple.h>

#include "CroasterConstants.h"
#include "CroasterPinConfig.h"

/**
 * @class CroasterCore
 * @brief Core functionality for managing sensors and data in the Croaster device.
 */
class CroasterCore {
private:
    Thermocouple* thermocoupleBT;
    Thermocouple* thermocoupleET;

    CroasterPinConfig _pins;

    double etHistory[60] = {}, btHistory[60] = {}, timeHistory[60] = {};
    bool historyInitialized = false;
    unsigned long lastSensorRead = 0;
    unsigned long lastRORUpdate = 0;

    double correctionBt = 0, correctionEt = 0;

    String tempUnit = "C";

    unsigned long intervalSend = 3;

    bool useDummyData;

    // Roast timer state (start / pause / reset).
    bool roastTimerRunning = false;      ///< Whether the roast timer is counting.
    unsigned long roastTimerStartMs = 0; ///< millis() when the roast timer was (re)started.
    unsigned long roastTimerAccumMs = 0; ///< Accumulated running time before the last pause.

    /**
     * @brief Returns the elapsed roast time in milliseconds.
     * @return Elapsed time in milliseconds.
     */
    unsigned long roastTimerElapsedMs() const;

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
    double readCelcius(Thermocouple* thermocouple);

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
     * @brief Rounds a double value to 2 decimal places.
     * @param value The value to round.
     * @return The rounded value.
     */
    double roundTo2(double value);

public:
    double timer = 0;      ///< Device uptime in seconds (unaffected by the roast timer).
    double roastTimer = 0; ///< Roast elapsed time in seconds (start/pause/reset).
    double rorEt = 0, rorBt = 0, tempEt = 0, tempBt = 0;

    /**
     * @brief Constructs a CroasterCore instance.
     * @param dummyMode If true, uses dummy data instead of real sensor data.
     * @param pins The thermocouple pin layout for the target board (provided by
     *             the implementation's config.h — there is no library default).
     */
    CroasterCore(bool dummyMode, CroasterPinConfig pins);

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
     * @brief Toggles the use of dummy data for testing purposes.
     */
    void toggleDummyData();

    /**
     * @brief Starts (or resumes) the roast timer.
     */
    void roastTimerStart();

    /**
     * @brief Pauses the roast timer.
     */
    void roastTimerPause();

    /**
     * @brief Resets the roast timer to zero (stopped, ready for a new roast).
     */
    void roastTimerReset();

    /**
     * @brief Checks whether the roast timer is currently running.
     * @return True if running, false if paused.
     */
    bool roastTimerIsRunning() const;

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
     * @param darkMode Current UI theme (true = dark).
     * @param brightness Current backlight brightness (0-100).
     * @return A string containing the device information.
     */
    String getDeviceInfo(bool darkMode = true, int brightness = 100);
};
