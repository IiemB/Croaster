#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "CroasterCore.h"
#include "DisplayManager.h"

#define LED_ON LOW
#define LED_OFF HIGH

/**
 * @class CommandHandler
 * @brief Handles incoming commands and manages device behavior accordingly.
 */
class CommandHandler
{
private:
    CroasterCore &croaster;
    DisplayManager &displayManager;

    bool blinking = false;
    uint8_t blinkCount = 0;
    uint8_t blinkTotal = 0;
    unsigned long lastBlinkTime = 0;
    unsigned long blinkDelay = 250;
    bool ledState = false;

    /**
     * @brief Handles basic commands such as restart or erase.
     * @param json The JSON object containing the command.
     * @param responseOut The response to send back.
     * @param restart Set to true if a restart is requested.
     * @param erase Set to true if an erase is requested.
     */
    void handleBasicCommand(const JsonObject &json, String &responseOut);

    /**
     * @brief Handles JSON-formatted commands.
     * @param json The JSON object containing the command.
     * @param responseOut The response to send back.
     */
    void handleJsonCommand(const JsonObject &json, String &responseOut);

    /**
     * @brief Blinks the built-in LED a specified number of times.
     * @param times The number of times to blink.
     * @param blinkDelay The delay between blinks in milliseconds.
     */
    void blinkBuiltinLED(uint8_t times = 2, unsigned long blinkDelay = 250);

    /**
     * @brief Generates a JSON response for a given command.
     * @param command The command for which to generate the response.
     * @param response The response message to include in the JSON.
     * @return A JSON-formatted string containing the command and response.
     */
    String genResponseCommand(const String command, const String response);

    /**
     * @brief Generates a random string of a specified length.
     * @param length The length of the random string to generate.
     * @return A random string of the specified length.
     */
    String genRandomString(int length);

    /**
     * @brief Retrieves extra data for the "extra" command.
     * @return A JSON-formatted string containing extra data.
     */
    String getExtraData();

public:
    /**
     * @brief Constructs a CommandHandler instance.
     * @param core Reference to the CroasterCore instance.
     * @param display Reference to the DisplayManager instance.
     */
    CommandHandler(CroasterCore &core, DisplayManager &display);

    /**
     * @brief Initializes the CommandHandler.
     */
    void begin();

    /**
     * @brief Handles tasks related to command processing in the main loop.
     */
    void loop();

    /**
     * @brief Processes an incoming command.
     * @param json The command in JSON format.
     * @param responseOut The response to send back.
     * @return True if the command was handled successfully, false otherwise.
     */
    bool handle(const String &json, String &responseOut);
};
