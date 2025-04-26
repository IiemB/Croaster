#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "CroasterCore.h"
#include "DisplayManager.h"

#define LED_ON LOW
#define LED_OFF HIGH

/**
 * CommandHandler class handles parsed BLE/WebSocket JSON commands.
 * Supports internal actions like restart, erase, LED blink, etc.
 */
class CommandHandler
{
private:
    CroasterCore &croaster;
    DisplayManager &displayManager;

    // Blink LED non-blocking state
    bool blinking = false;
    uint8_t blinkCount = 0;
    uint8_t blinkTotal = 0;
    unsigned long lastBlinkTime = 0;
    unsigned long blinkDelay = 250;
    bool ledState = false;

    void handleBasicCommand(const JsonObject &json, String &responseOut, bool &restart, bool &erase);

    void handleJsonCommand(const JsonObject &json, String &responseOut);

    /**
     * This is an example custom function to handle the `blink` command.
     */
    void blinkBuiltinLED(uint8_t times = 2, unsigned long blinkDelay = 250);

public:
    CommandHandler(CroasterCore &core, DisplayManager &display);

    /**
     * Initializes the command handler.
     */
    void begin();

    /**
     * Executes the main loop for handling commands.
     */
    void loop();

    /**
     * Handle a parsed JSON command string.
     *
     * @param json          Incoming JSON string
     * @param responseOut   String to be filled with a JSON reply (if needed)
     * @param restart       Will be set true if restart is requested
     * @param erase         Will be set true if erase is requested
     * @return true if valid command processed, false otherwise
     */
    bool handle(const String &json, String &responseOut, bool &restart, bool &erase);
};
