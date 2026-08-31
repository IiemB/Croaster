#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <lvgl.h>

#include <functional>

/**
 * @class LvglTouch
 * @brief Minimal FT6336G capacitive touch driver + LVGL 9 pointer indev
 *        for the ES3C28P built-in touchscreen.
 *
 * The FT6336G sits on the I2C bus (SDA=IO16, SCL=IO15) and reports points in
 * the panel's native portrait orientation (240x320). The read callback maps
 * them to the display orientation (landscape 320x240 by default).
 */
class LvglTouch {
public:
    LvglTouch() = default;

    /**
     * @brief Initializes the touch controller and registers an LVGL pointer indev.
     * @param disp The LVGL display the touch should drive.
     * @return True on success.
     */
    bool begin(lv_display_t* disp);

    /** @brief Set the display rotation so touch coords stay aligned (0-3). */
    void setRotation(uint8_t rotation) {
        this->rotation = rotation & 0x03;
    }

    /**
     * @brief Register a callback fired when a double tap (two quick taps) is
     *        detected on the touchscreen.
     * @param cb The callback to invoke on double tap.
     */
    void setDoubleTapCb(std::function<void()> cb) {
        doubleTapCb = std::move(cb);
    }

    /**
     * @brief Register a callback fired on a horizontal swipe gesture.
     * @param cb Called with +1 for a swipe to the right, -1 for a swipe to
     *           the left. Fires on release when the finger moved a clear
     *           horizontal distance without much vertical travel.
     */
    void setSwipeCb(std::function<void(int dir)> cb) {
        swipeCb = std::move(cb);
    }

private:
    void resetChip();
    uint8_t readByte(uint8_t reg);
    bool readPoints();

    uint8_t i2cAddr = 0x38;     // FT6336 I2C address
    uint16_t panelWidth = 240;  // native portrait width
    uint16_t panelHeight = 320; // native portrait height
    uint8_t rotation = 1;       // must match the ILI9341 rotation (1 = landscape)

    bool touched = false;
    int16_t x = 0;
    int16_t y = 0;

    // Double-tap detection state.
    bool prevPressed = false;
    bool dblArmed = false;
    unsigned long dblArmStartMs = 0;
    std::function<void()> doubleTapCb;

    // Swipe detection state (press start + horizontal move on release).
    bool pressActive = false;
    int16_t pressStartX = 0;
    int16_t pressStartY = 0;
    std::function<void(int dir)> swipeCb;

    lv_indev_t* indev = nullptr;

    static LvglTouch* s_instance;
    static void readCb(lv_indev_t* indev, lv_indev_data_t* data);
};
