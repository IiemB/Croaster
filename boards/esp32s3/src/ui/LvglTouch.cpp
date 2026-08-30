#include "LvglTouch.h"

#include <CroasterConstants.h>

#include "pins.h"

LvglTouch* LvglTouch::s_instance = nullptr;

// FT6336 registers
#define FT6336_TD_STATUS 0x02         // number of touch points (0-2)
#define FT6336_TOUCH_1 0x03           // first touch: XH XL YH YL weight area
#define FT6336_ID_G_FOCALTECH_ID 0xA8 // chip ID (0x11 for FT6336G)

// Two quick taps within this window (ms) count as a double tap.
#define DOUBLE_TAP_WINDOW_MS 350

// A horizontal finger move of at least this many px counts as a swipe.
#define SWIPE_THRESHOLD_PX 60

bool LvglTouch::begin(lv_display_t* disp) {
    s_instance = this;

    Wire.begin(TOUCH_SDA_PIN, TOUCH_SCL_PIN);
    resetChip();

    // Presence check (not fatal — the panel can still work if ID reads odd).
    uint8_t id = readByte(FT6336_ID_G_FOCALTECH_ID);
    debugln("# Touch chip ID: 0x" + String(id, HEX) + (id == 0x11 ? " (FT6336G)" : " (unexpected)"));

    indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, readCb);
    lv_indev_set_display(indev, disp);

    debugln("# LVGL touch initialized (rotation " + String(rotation) + ")");
    return true;
}

void LvglTouch::resetChip() {
    pinMode(TOUCH_INT_PIN, INPUT);
    pinMode(TOUCH_RST_PIN, OUTPUT);
    digitalWrite(TOUCH_RST_PIN, HIGH);
    delay(20);
    digitalWrite(TOUCH_RST_PIN, LOW);
    delay(20);
    digitalWrite(TOUCH_RST_PIN, HIGH);
    delay(50);
}

uint8_t LvglTouch::readByte(uint8_t reg) {
    Wire.beginTransmission(i2cAddr);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)i2cAddr, (uint8_t)1);
    return Wire.available() ? Wire.read() : 0;
}

bool LvglTouch::readPoints() {
    uint8_t touches = readByte(FT6336_TD_STATUS);
    touched = (touches > 0 && touches < 3);
    if (!touched)
        return false;

    // First touch: 6 bytes at 0x03 (XH XL YH YL weight area).
    uint8_t data[6] = {0};
    Wire.beginTransmission(i2cAddr);
    Wire.write(FT6336_TOUCH_1);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)i2cAddr, (uint8_t)6);
    for (uint8_t i = 0; i < 6 && Wire.available(); i++)
        data[i] = Wire.read();

    // 12-bit coordinates in the panel's native portrait orientation.
    uint16_t tx = (uint16_t)((data[0] & 0x0F) << 8) | data[1];
    uint16_t ty = (uint16_t)((data[2] & 0x0F) << 8) | data[3];

    // Map portrait (240x320) touch coords to the active display orientation.
    switch (rotation) {
    case 0: // portrait
        x = tx;
        y = ty;
        break;
    case 1: // landscape (ILI9341 rotation 1) -> 320x240
        x = ty;
        y = panelWidth - 1 - tx;
        break;
    case 2: // inverted portrait
        x = panelWidth - 1 - tx;
        y = panelHeight - 1 - ty;
        break;
    case 3: // inverted landscape
        x = panelHeight - 1 - ty;
        y = tx;
        break;
    }
    return true;
}

void LvglTouch::readCb(lv_indev_t* indev, lv_indev_data_t* data) {
    LvglTouch* touch = s_instance;
    if (!touch) {
        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }

    touch->readPoints();

    // Detect a horizontal swipe when the finger lifts.
    if (touch->prevPressed && !touch->touched) {
        int32_t dx = touch->x - touch->pressStartX;
        int32_t dy = touch->y - touch->pressStartY;

        // A clear horizontal move (mostly left/right, not vertical).
        if ((dx >= SWIPE_THRESHOLD_PX || dx <= -SWIPE_THRESHOLD_PX) && abs(dx) > abs(dy) * 2) {
            if (touch->swipeCb)
                touch->swipeCb(dx > 0 ? 1 : -1);
            // Consume this release: don't also arm a double tap.
            touch->dblArmed = false;
            touch->prevPressed = false;
            data->state = LV_INDEV_STATE_RELEASED;
            return;
        }

        // Double tap: two quick press→release cycles within the window.
        unsigned long now = millis();
        if (touch->dblArmed && (now - touch->dblArmStartMs) <= DOUBLE_TAP_WINDOW_MS) {
            touch->dblArmed = false;
            if (touch->doubleTapCb)
                touch->doubleTapCb();
        } else {
            touch->dblArmed = true;
            touch->dblArmStartMs = now;
        }
    }
    touch->prevPressed = touch->touched;

    if (touch->touched) {
        // Record where the press started (once per touch) for swipe math.
        if (!touch->pressActive) {
            touch->pressActive = true;
            touch->pressStartX = touch->x;
            touch->pressStartY = touch->y;
        }
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = touch->x;
        data->point.y = touch->y;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
        touch->pressActive = false;
    }
}
