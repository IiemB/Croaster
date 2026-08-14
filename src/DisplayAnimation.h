#pragma once

#include <Arduino.h>
#include <Adafruit_SSD1306.h>

class DisplayAnimation
{
public:
    DisplayAnimation(int16_t x = 40, int16_t y = 8);

    void showFire(Adafruit_SSD1306 &display);
    void reset();

private:
    static constexpr uint16_t FRAME_DELAY = 42;
    static constexpr uint8_t FRAME_WIDTH = 48;
    static constexpr uint8_t FRAME_HEIGHT = 48;
    static constexpr uint8_t FRAME_COUNT = 28;

    uint8_t frame = 0;
    int16_t xPos;
    int16_t yPos;
};