#include "LvglUi.h"

#include <CroasterConstants.h>
#include <esp_heap_caps.h>

#include "pins.h"

// Partial-screen draw buffer size in pixels (two buffers, allocated from PSRAM).
#define LVGL_BUF_PX (320 * 40)

// LEDC channel used for the LCD backlight PWM (brightness control).
#define LCD_BL_CHANNEL 0

// RAII guard that serializes LVGL access between the app loop (Core 1) and
// the dedicated LVGL render task (Core 0). Recursive so LVGL-internal
// callbacks (touch -> page/display toggle) can safely re-enter the lock.
struct LvglLock {
    SemaphoreHandle_t mtx;
    explicit LvglLock(SemaphoreHandle_t m)
        : mtx(m) {
        if (mtx)
            xSemaphoreTakeRecursive(mtx, portMAX_DELAY);
    }
    ~LvglLock() {
        if (mtx)
            xSemaphoreGiveRecursive(mtx);
    }
};

// Animation exec callback for the splash progress bar. Sets the value with
// LV_ANIM_OFF (the 3rd arg of lv_bar_set_value must not be left as garbage).
static void splashBarAnimCb(void* var, int32_t v) {
    lv_bar_set_value((lv_obj_t*)var, v, LV_ANIM_OFF);
}

bool LvglUi::begin() {
    // Backlight via LEDC PWM (IO45) so brightness can be dimmed (0-100%).
    ledcSetup(LCD_BL_CHANNEL, 5000, 8); // 5 kHz, 8-bit resolution (0-255)
    ledcAttachPin(LCD_BL_PIN, LCD_BL_CHANNEL);
    ledcWrite(LCD_BL_CHANNEL, dutyFromPct(brightnessPct));

    tft = new Adafruit_ILI9341(LCD_CS_PIN, LCD_DC_PIN, LCD_MOSI_PIN, LCD_SCLK_PIN, LCD_RST_PIN, LCD_MISO_PIN);
    tft->begin();
    tft->setRotation(1); // landscape 320x240

    lv_init();

    // Drive the LVGL tick from millis().
    lv_tick_set_cb([]() -> uint32_t {
        return millis();
    });

    // LVGL 9 display driver.
    disp = lv_display_create(320, 240);
    lv_display_set_default(disp);

    // Draw buffers in PSRAM (keep the limited internal SRAM for the stack).
    // LVGL 9 expresses the buffer size in bytes (RGB565 = 2 bytes/pixel).
    uint32_t bufBytes = LVGL_BUF_PX * sizeof(uint16_t);
    void* buf1 = heap_caps_malloc(bufBytes, MALLOC_CAP_SPIRAM);
    void* buf2 = heap_caps_malloc(bufBytes, MALLOC_CAP_SPIRAM);
    if (!buf1 || !buf2) {
        debugln(F("# LVGL draw buffer allocation failed"));
        return false;
    }
    lv_display_set_buffers(disp, buf1, buf2, bufBytes, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(disp, flushCb);
    lv_display_set_user_data(disp, this);

    // Wire the FT6336G touchscreen as the LVGL pointer input device.
    touch.begin(disp);

    // Build the UI widgets (regular open-source LVGL — no Pro/XML).
    createUi();

    // Double tap toggles the display (backlight) on/off.
    touch.setDoubleTapCb([this]() {
        if (displayToggleCb)
            displayToggleCb();
    });

    // Horizontal swipe pages between the instrument and the roast chart.
    touch.setSwipeCb([this](int dir) {
        if (!screenOn)
            return; // display off -> no navigation
        if (dir < 0 && !onChartPage)
            showChartPage(); // swipe left  -> chart
        else if (dir > 0 && onChartPage)
            showMainPage(); // swipe right -> back
    });

    debugln(F("# LVGL UI initialization succeed"));

    // Show the boot splash while the rest of setup() (WiFi/BLE/WebSocket)
    // initializes. The LVGL task renders it on Core 0 without blocking setup;
    // finishSplash() (called at the end of setup) switches to the main page.
    splashStartMs = millis();
    splashDone = false;
    lv_screen_load(scrSplash);

    // Dedicated LVGL render task pinned to Core 0: the app loop (Core 1)
    // keeps handling sensors/WiFi/BLE/WebSocket while the UI renders in
    // parallel. A recursive mutex serializes all LVGL access between cores.
    lvglMutex = xSemaphoreCreateRecursiveMutex();
    xTaskCreatePinnedToCore(lvglTaskEntry, "lvgl", 8192, this, 2, &lvglTaskHandle, 0);

    return true;
}

void LvglUi::loop() {
    // LVGL runs on its own pinned task (see lvglTaskEntry / Core 0); nothing
    // for the app-loop side to do here.
}

void LvglUi::lvglTaskEntry(void* arg) {
    LvglUi* ui = (LvglUi*)arg;
    while (true) {
        uint32_t idleMs = 0;
        if (ui->lvglMutex && xSemaphoreTakeRecursive(ui->lvglMutex, portMAX_DELAY) == pdTRUE) {
            if (ui->themeRebuildPending)
                ui->rebuildTheme(); // full UI rebuild runs here (LVGL owns the context)
            idleMs = lv_timer_handler();
            ui->checkSplashTransition();
            xSemaphoreGiveRecursive(ui->lvglMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(idleMs == 0 ? 1 : idleMs));
    }
}

void LvglUi::flushCb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
    LvglUi* ui = (LvglUi*)lv_display_get_user_data(disp);
    if (!ui || !ui->tft) {
        lv_display_flush_ready(disp);
        return;
    }

    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;

    ui->tft->startWrite();
    ui->tft->setAddrWindow(area->x1, area->y1, w, h);
    ui->tft->writePixels((uint16_t*)px_map, w * h, true);
    ui->tft->endWrite();

    lv_display_flush_ready(disp);
}

void LvglUi::loadTheme() {
    if (!darkMode) {
        // Warm near-black instrument look.
        theme_.bg = lv_color_hex(0x0F0E0D);
        theme_.cardBg = lv_color_hex(0x171615);
        theme_.border = lv_color_hex(0x262522);
        theme_.textPrimary = lv_color_hex(0xEDECE9);
        theme_.textMuted = lv_color_hex(0x75736E);
        theme_.accentBt = lv_color_hex(0x87B8C4); // muted desaturated cyan
        theme_.accentEt = lv_color_hex(0xC89A6C); // muted desaturated orange
        theme_.dotOnWifi = lv_color_hex(0x7FA06B);
        theme_.dotOnBt = lv_color_hex(0x7D9FBF);
        theme_.dotOff = lv_color_hex(0x3A3834);
        theme_.btnStartBg = lv_color_hex(0xEDECE9);
        theme_.btnStartText = lv_color_hex(0x141312);
        theme_.btnStartPress = lv_color_hex(0xD8D7D3);
        theme_.btnStopBg = lv_color_hex(0x8A4A3F);
        theme_.btnStopText = lv_color_hex(0xEDECE9);
        theme_.btnStopPress = lv_color_hex(0x6E3A31);
        theme_.btnResetBg = lv_color_hex(0x171615);
        theme_.btnResetText = lv_color_hex(0xEDECE9);
        theme_.btnResetBorder = lv_color_hex(0x262522);
        theme_.btnResetPress = lv_color_hex(0x22211F);
    } else {
        // Warm paper light look.
        theme_.bg = lv_color_hex(0xF7F6F3);
        theme_.cardBg = lv_color_hex(0xFFFFFF);
        theme_.border = lv_color_hex(0xE3E1DC);
        theme_.textPrimary = lv_color_hex(0x2F3437);
        theme_.textMuted = lv_color_hex(0x787774);
        theme_.accentBt = lv_color_hex(0x4A7D94);
        theme_.accentEt = lv_color_hex(0xB2763C);
        theme_.dotOnWifi = lv_color_hex(0x3F7D3A);
        theme_.dotOnBt = lv_color_hex(0x3A6EA5);
        theme_.dotOff = lv_color_hex(0xC9C6C0);
        theme_.btnStartBg = lv_color_hex(0x2F3437);
        theme_.btnStartText = lv_color_hex(0xFFFFFF);
        theme_.btnStartPress = lv_color_hex(0x1A1C1E);
        theme_.btnStopBg = lv_color_hex(0xB0503F);
        theme_.btnStopText = lv_color_hex(0xFFFFFF);
        theme_.btnStopPress = lv_color_hex(0x8E3D2F);
        theme_.btnResetBg = lv_color_hex(0xFFFFFF);
        theme_.btnResetText = lv_color_hex(0x2F3437);
        theme_.btnResetBorder = lv_color_hex(0xD5D2CC);
        theme_.btnResetPress = lv_color_hex(0xEFEDE9);
    }
}

void LvglUi::createUi() {
    // Palette driven by the active theme (dark by default, light optional).
    loadTheme();

    // Two screens: the main instrument page + the roast-profile chart page.
    scrMain = lv_obj_create(NULL);
    buildMainScreen(scrMain);

    scrChart = lv_obj_create(NULL);
    buildChartScreen(scrChart);

    // Third screen: full-page firmware-update status (shown during OTA).
    scrFw = lv_obj_create(NULL);
    buildFirmwareScreen(scrFw);

    // Boot splash screen (loaded by begin(); switches to scrMain after setup).
    scrSplash = lv_obj_create(NULL);
    buildSplashScreen(scrSplash);

    lv_screen_load(scrMain);
}

void LvglUi::buildFirmwareScreen(lv_obj_t* scr) {
    lv_obj_set_style_bg_color(scr, theme_.bg, 0);

    fwTitleLabel = lv_label_create(scr);
    lv_obj_set_style_text_color(fwTitleLabel, theme_.textPrimary, 0);
    lv_obj_set_style_text_font(fwTitleLabel, &lv_font_montserrat_20, 0);
    lv_label_set_text(fwTitleLabel, "Updating Firmware");
    lv_obj_align(fwTitleLabel, LV_ALIGN_CENTER, 0, -40);

    fwLabel = lv_label_create(scr);
    lv_obj_set_style_text_color(fwLabel, theme_.textPrimary, 0);
    lv_obj_set_style_text_font(fwLabel, &lv_font_montserrat_28, 0);
    lv_label_set_text(fwLabel, "0 %");
    lv_obj_align(fwLabel, LV_ALIGN_CENTER, 0, 0);

    fwBar = lv_bar_create(scr);
    lv_obj_set_size(fwBar, 280, 16);
    lv_obj_align(fwBar, LV_ALIGN_CENTER, 0, 40);
    lv_bar_set_range(fwBar, 0, 100);
    lv_bar_set_value(fwBar, 0, LV_ANIM_OFF);

    lv_obj_t* hint = lv_label_create(scr);
    lv_obj_set_style_text_color(hint, theme_.textMuted, 0);
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_12, 0);
    lv_label_set_text(hint, "Do not disconnect the device");
    lv_obj_align(hint, LV_ALIGN_CENTER, 0, 70);
}

void LvglUi::buildSplashScreen(lv_obj_t* scr) {
    lv_obj_set_style_bg_color(scr, theme_.bg, 0);

    // Brand
    lv_obj_t* brand = lv_label_create(scr);
    lv_obj_set_style_text_color(brand, theme_.textPrimary, 0);
    lv_obj_set_style_text_font(brand, &lv_font_montserrat_32, 0);
    lv_label_set_text(brand, "CROASTER");
    lv_obj_align(brand, LV_ALIGN_CENTER, 0, -36);

    // Version
    lv_obj_t* ver = lv_label_create(scr);
    lv_obj_set_style_text_color(ver, theme_.textMuted, 0);
    lv_obj_set_style_text_font(ver, &lv_font_montserrat_14, 0);
    lv_label_set_text(ver, ("V" + String(version)).c_str());
    lv_obj_align(ver, LV_ALIGN_CENTER, 0, -6);

    // Progress bar, animated 0 → 100 over the minimum splash duration.
    lv_obj_t* bar = lv_bar_create(scr);
    lv_obj_set_size(bar, 220, 6);
    lv_obj_align(bar, LV_ALIGN_CENTER, 0, 24);
    lv_bar_set_range(bar, 0, 100);
    lv_bar_set_value(bar, 0, LV_ANIM_OFF);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, bar);
    lv_anim_set_exec_cb(&a, splashBarAnimCb);
    lv_anim_set_values(&a, 0, 100);
    lv_anim_set_time(&a, kSplashMinMs);
    lv_anim_set_path_cb(&a, lv_anim_path_linear);
    lv_anim_start(&a);

    // Status
    lv_obj_t* status = lv_label_create(scr);
    lv_obj_set_style_text_color(status, theme_.textMuted, 0);
    lv_obj_set_style_text_font(status, &lv_font_montserrat_12, 0);
    lv_label_set_text(status, "Initializing...");
    lv_obj_align(status, LV_ALIGN_CENTER, 0, 46);
}

void LvglUi::buildTopBar(lv_obj_t* scr, TopBar& bar) {
    const lv_color_t cardBg = theme_.cardBg;
    const lv_color_t border = theme_.border;
    const lv_color_t textMuted = theme_.textMuted;

    // Brand (muted, small) with the IP address directly below it.
    bar.headerLabel = lv_label_create(scr);
    lv_obj_set_style_text_color(bar.headerLabel, textMuted, 0);
    lv_obj_set_style_text_font(bar.headerLabel, &lv_font_montserrat_14, 0);
    lv_label_set_text(bar.headerLabel, "CROASTER");
    lv_obj_align(bar.headerLabel, LV_ALIGN_TOP_LEFT, 12, 5);

    bar.ipLabel = lv_label_create(scr);
    lv_obj_set_style_text_color(bar.ipLabel, textMuted, 0);
    lv_obj_set_style_text_font(bar.ipLabel, &lv_font_montserrat_12, 0);
    lv_label_set_text(bar.ipLabel, "");
    lv_obj_align(bar.ipLabel, LV_ALIGN_TOP_LEFT, 12, 24);

    // WiFi status chip (top-right).
    lv_obj_t* wifiChip = lv_obj_create(scr);
    lv_obj_remove_style_all(wifiChip);
    lv_obj_set_size(wifiChip, 52, 22);
    lv_obj_align(wifiChip, LV_ALIGN_TOP_RIGHT, -4, 6);
    lv_obj_set_style_bg_color(wifiChip, cardBg, 0);
    lv_obj_set_style_bg_opa(wifiChip, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(wifiChip, border, 0);
    lv_obj_set_style_border_width(wifiChip, 1, 0);
    lv_obj_set_style_radius(wifiChip, 4, 0);

    bar.wifiDot = lv_obj_create(wifiChip);
    lv_obj_remove_style_all(bar.wifiDot);
    lv_obj_set_size(bar.wifiDot, 8, 8);
    lv_obj_set_pos(bar.wifiDot, 7, 7);
    lv_obj_set_style_radius(bar.wifiDot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(bar.wifiDot, theme_.dotOff, 0);
    lv_obj_set_style_bg_opa(bar.wifiDot, LV_OPA_COVER, 0);

    lv_obj_t* wifiLabel = lv_label_create(wifiChip);
    lv_obj_set_style_text_color(wifiLabel, textMuted, 0);
    lv_obj_set_style_text_font(wifiLabel, &lv_font_montserrat_12, 0);
    lv_label_set_text(wifiLabel, "WIFI");
    lv_obj_align(wifiLabel, LV_ALIGN_LEFT_MID, 20, 0);

    // BLE status chip (next to WiFi).
    lv_obj_t* btChip = lv_obj_create(scr);
    lv_obj_remove_style_all(btChip);
    lv_obj_set_size(btChip, 44, 22);
    lv_obj_align(btChip, LV_ALIGN_TOP_RIGHT, -60, 6);
    lv_obj_set_style_bg_color(btChip, cardBg, 0);
    lv_obj_set_style_bg_opa(btChip, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(btChip, border, 0);
    lv_obj_set_style_border_width(btChip, 1, 0);
    lv_obj_set_style_radius(btChip, 4, 0);

    bar.btDot = lv_obj_create(btChip);
    lv_obj_remove_style_all(bar.btDot);
    lv_obj_set_size(bar.btDot, 8, 8);
    lv_obj_set_pos(bar.btDot, 7, 7);
    lv_obj_set_style_radius(bar.btDot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(bar.btDot, theme_.dotOff, 0);
    lv_obj_set_style_bg_opa(bar.btDot, LV_OPA_COVER, 0);

    lv_obj_t* btChipLabel = lv_label_create(btChip);
    lv_obj_set_style_text_color(btChipLabel, textMuted, 0);
    lv_obj_set_style_text_font(btChipLabel, &lv_font_montserrat_12, 0);
    lv_label_set_text(btChipLabel, "BT");
    lv_obj_align(btChipLabel, LV_ALIGN_LEFT_MID, 20, 0);

    // Subtle divider under the top bar.
    lv_obj_t* topLine = lv_obj_create(scr);
    lv_obj_remove_style_all(topLine);
    lv_obj_set_size(topLine, 320, 1);
    lv_obj_set_pos(topLine, 0, 38);
    lv_obj_set_style_bg_color(topLine, border, 0);
    lv_obj_set_style_bg_opa(topLine, LV_OPA_COVER, 0);
}

void LvglUi::buildMainScreen(lv_obj_t* scr) {
    const lv_color_t bg = theme_.bg;
    const lv_color_t cardBg = theme_.cardBg;
    const lv_color_t border = theme_.border;
    const lv_color_t textPrimary = theme_.textPrimary;
    const lv_color_t accentBt = theme_.accentBt;
    const lv_color_t accentEt = theme_.accentEt;

    lv_obj_set_style_bg_color(scr, bg, 0);

    // --- Top bar (brand/IP/status chips) ------------------------------------
    buildTopBar(scr, mainBar_);

    // --- Temperature cards -------------------------------------------------

    // BT card (left)
    lv_obj_t* btCard = lv_obj_create(scr);
    lv_obj_remove_style_all(btCard);
    lv_obj_set_pos(btCard, 12, 46);
    lv_obj_set_size(btCard, 142, 112);
    lv_obj_set_style_bg_color(btCard, cardBg, 0);
    lv_obj_set_style_bg_opa(btCard, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(btCard, border, 0);
    lv_obj_set_style_border_width(btCard, 1, 0);
    lv_obj_set_style_radius(btCard, 8, 0);

    lv_obj_t* btTag = lv_label_create(btCard);
    lv_obj_set_style_text_color(btTag, accentBt, 0);
    lv_obj_set_style_text_font(btTag, &lv_font_montserrat_14, 0);
    lv_label_set_text(btTag, "BT");
    lv_obj_align(btTag, LV_ALIGN_TOP_LEFT, 14, 8);

    btLabel = lv_label_create(btCard);
    lv_obj_set_style_text_color(btLabel, textPrimary, 0);
    lv_obj_set_style_text_font(btLabel, &lv_font_montserrat_32, 0);
    lv_label_set_text(btLabel, "--");
    lv_obj_align(btLabel, LV_ALIGN_TOP_LEFT, 14, 30);

    btRorLabel = lv_label_create(btCard);
    lv_obj_set_style_text_color(btRorLabel, accentBt, 0);
    lv_obj_set_style_text_font(btRorLabel, &lv_font_montserrat_16, 0);
    lv_label_set_text(btRorLabel, "");
    lv_obj_align(btRorLabel, LV_ALIGN_TOP_LEFT, 14, 80);

    // ET card (right)
    lv_obj_t* etCard = lv_obj_create(scr);
    lv_obj_remove_style_all(etCard);
    lv_obj_set_pos(etCard, 166, 46);
    lv_obj_set_size(etCard, 142, 112);
    lv_obj_set_style_bg_color(etCard, cardBg, 0);
    lv_obj_set_style_bg_opa(etCard, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(etCard, border, 0);
    lv_obj_set_style_border_width(etCard, 1, 0);
    lv_obj_set_style_radius(etCard, 8, 0);

    lv_obj_t* etTag = lv_label_create(etCard);
    lv_obj_set_style_text_color(etTag, accentEt, 0);
    lv_obj_set_style_text_font(etTag, &lv_font_montserrat_14, 0);
    lv_label_set_text(etTag, "ET");
    lv_obj_align(etTag, LV_ALIGN_TOP_LEFT, 14, 8);

    etLabel = lv_label_create(etCard);
    lv_obj_set_style_text_color(etLabel, textPrimary, 0);
    lv_obj_set_style_text_font(etLabel, &lv_font_montserrat_32, 0);
    lv_label_set_text(etLabel, "--");
    lv_obj_align(etLabel, LV_ALIGN_TOP_LEFT, 14, 30);

    etRorLabel = lv_label_create(etCard);
    lv_obj_set_style_text_color(etRorLabel, accentEt, 0);
    lv_obj_set_style_text_font(etRorLabel, &lv_font_montserrat_16, 0);
    lv_label_set_text(etRorLabel, "");
    lv_obj_align(etRorLabel, LV_ALIGN_TOP_LEFT, 14, 80);

    // --- Bottom bar (timer + single control button in one container) --------

    timerBar = lv_obj_create(scr);
    lv_obj_remove_style_all(timerBar);
    lv_obj_set_pos(timerBar, 12, 170);
    lv_obj_set_size(timerBar, 296, 52);
    lv_obj_set_style_bg_color(timerBar, cardBg, 0);
    lv_obj_set_style_bg_opa(timerBar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(timerBar, border, 0);
    lv_obj_set_style_border_width(timerBar, 1, 0);
    lv_obj_set_style_radius(timerBar, 8, 0);

    // Roast timer (prominent, left side of the bar)
    timerLabel = lv_label_create(timerBar);
    lv_obj_set_style_text_color(timerLabel, textPrimary, 0);
    lv_obj_set_style_text_font(timerLabel, &lv_font_montserrat_32, 0);
    lv_label_set_text(timerLabel, "00:00");
    lv_obj_align(timerLabel, LV_ALIGN_LEFT_MID, 20, 0);

    // Single control button (right side of the bar) — cycles START/STOP/RESET
    stateBtn = lv_button_create(timerBar);
    lv_obj_set_pos(stateBtn, 296 - 8 - 120, 6);
    lv_obj_set_size(stateBtn, 120, 40);
    lv_obj_set_style_radius(stateBtn, 6, 0);
    stateBtnLabel = lv_label_create(stateBtn);
    lv_obj_set_style_text_font(stateBtnLabel, &lv_font_montserrat_20, 0);
    lv_label_set_text(stateBtnLabel, "START");
    lv_obj_center(stateBtnLabel);
    lv_obj_add_event_cb(stateBtn, btnCb, LV_EVENT_CLICKED, this);

    applyTimerState(TimerState::START);
}

void LvglUi::buildChartScreen(lv_obj_t* scr) {
    const lv_color_t bg = theme_.bg;
    const lv_color_t cardBg = theme_.cardBg;
    const lv_color_t border = theme_.border;
    const lv_color_t accentBt = theme_.accentBt;
    const lv_color_t accentEt = theme_.accentEt;

    lv_obj_set_style_bg_color(scr, bg, 0);

    // Top bar identical to the main page (brand/IP/status chips); only the
    // body below changes.
    buildTopBar(scr, chartBar_);

    // --- BT card: own container + line chart --------------------------------
    lv_obj_t* btCard = lv_obj_create(scr);
    lv_obj_remove_style_all(btCard);
    lv_obj_set_pos(btCard, 12, 46);
    lv_obj_set_size(btCard, 296, 88);
    lv_obj_set_style_bg_color(btCard, cardBg, 0);
    lv_obj_set_style_bg_opa(btCard, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(btCard, border, 0);
    lv_obj_set_style_border_width(btCard, 1, 0);
    lv_obj_set_style_radius(btCard, 8, 0);

    chartBtLabel = lv_label_create(btCard);
    lv_obj_set_style_text_color(chartBtLabel, accentBt, 0);
    lv_obj_set_style_text_font(chartBtLabel, &lv_font_montserrat_14, 0);
    lv_label_set_text(chartBtLabel, "BT --");
    lv_obj_align(chartBtLabel, LV_ALIGN_TOP_LEFT, 12, 6);

    chartBt = createChartWidget(btCard);
    lv_obj_set_pos(chartBt, 6, 26);
    lv_obj_set_size(chartBt, 284, 56);
    chartSerBt = lv_chart_add_series(chartBt, accentBt, LV_CHART_AXIS_PRIMARY_Y);

    // --- ET card: own container + line chart --------------------------------
    lv_obj_t* etCard = lv_obj_create(scr);
    lv_obj_remove_style_all(etCard);
    lv_obj_set_pos(etCard, 12, 140);
    lv_obj_set_size(etCard, 296, 88);
    lv_obj_set_style_bg_color(etCard, cardBg, 0);
    lv_obj_set_style_bg_opa(etCard, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(etCard, border, 0);
    lv_obj_set_style_border_width(etCard, 1, 0);
    lv_obj_set_style_radius(etCard, 8, 0);

    chartEtLabel = lv_label_create(etCard);
    lv_obj_set_style_text_color(chartEtLabel, accentEt, 0);
    lv_obj_set_style_text_font(chartEtLabel, &lv_font_montserrat_14, 0);
    lv_label_set_text(chartEtLabel, "ET --");
    lv_obj_align(chartEtLabel, LV_ALIGN_TOP_LEFT, 12, 6);

    chartEt = createChartWidget(etCard);
    lv_obj_set_pos(chartEt, 6, 26);
    lv_obj_set_size(chartEt, 284, 56);
    chartSerEt = lv_chart_add_series(chartEt, accentEt, LV_CHART_AXIS_PRIMARY_Y);
}

lv_obj_t* LvglUi::createChartWidget(lv_obj_t* parent) {
    lv_obj_t* ch = lv_chart_create(parent);
    lv_obj_set_style_bg_opa(ch, LV_OPA_TRANSP, 0); // the card provides the bg
    lv_obj_set_style_border_width(ch, 0, 0);
    lv_obj_set_style_radius(ch, 0, 0);
    lv_obj_set_style_pad_all(ch, 4, 0);
    lv_obj_set_style_line_color(ch, theme_.border, LV_PART_MAIN); // grid
    lv_obj_set_style_line_opa(ch, LV_OPA_40, LV_PART_MAIN);

    // Series are plain descriptors (NOT lv_obj_t) in LVGL 9.5; line width is a
    // chart-level style on LV_PART_ITEMS (styling a series cast to lv_obj_t*
    // crashes). Per-series color is set at lv_chart_add_series().
    lv_obj_set_style_line_width(ch, 1, LV_PART_ITEMS); // 1 px traces

    lv_chart_set_type(ch, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(ch, kChartPoints);
    lv_chart_set_update_mode(ch, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_axis_range(ch, LV_CHART_AXIS_PRIMARY_Y, 0, unitF ? 580 : 300);
    lv_chart_set_div_line_count(ch, 3, 4);
    return ch;
}

void LvglUi::showChartPage() {
    LvglLock lock(lvglMutex);

    if (onFirmwarePage || onChartPage || !scrChart)
        return;
    onChartPage = true;
    lv_screen_load_anim(scrChart, LV_SCREEN_LOAD_ANIM_MOVE_LEFT, 200, 0, false);
}

void LvglUi::showMainPage() {
    LvglLock lock(lvglMutex);

    if (onFirmwarePage || !onChartPage || !scrMain)
        return;
    onChartPage = false;
    lv_screen_load_anim(scrMain, LV_SCREEN_LOAD_ANIM_MOVE_RIGHT, 200, 0, false);
}

void LvglUi::showFirmwarePage(bool updating) {
    LvglLock lock(lvglMutex);

    if (updating) {
        // Remember where to return so the update page REPLACES the content
        // instead of stacking on top of it.
        if (!onFirmwarePage) {
            onFirmwarePage = true;
            prevPageWasChart = onChartPage;
        }
        if (scrFw)
            lv_screen_load(scrFw);
    } else {
        if (onFirmwarePage) {
            onFirmwarePage = false;
            onChartPage = prevPageWasChart;
            if (onChartPage && scrChart)
                lv_screen_load(scrChart);
            else if (scrMain)
                lv_screen_load(scrMain);
        }
    }
}

void LvglUi::finishSplash() {
    LvglLock lock(lvglMutex);
    // Flag setup() as done; the LVGL task performs the switch once the
    // minimum splash duration has also elapsed.
    splashDone = true;
}

void LvglUi::checkSplashTransition() {
    // Runs on the LVGL task (caller holds lvglMutex). Switch from the boot
    // splash to the main page once setup() has finished AND the minimum
    // splash duration has elapsed.
    if (!splashDone || !scrSplash || !scrMain)
        return;
    if (millis() - splashStartMs < kSplashMinMs)
        return;

    splashDone = false;
    lv_screen_load(scrMain);
}

void LvglUi::refreshChart() {
    if (!chartBt || !chartEt || !chartSerBt || !chartSerEt)
        return;

    // Reset both charts, then replay the ring buffer (oldest → newest).
    lv_chart_set_all_values(chartBt, chartSerBt, 0);
    lv_chart_set_all_values(chartEt, chartSerEt, 0);

    int oldest = (chartHead - chartCount + 1 + kChartPoints) % kChartPoints;
    for (int i = 0; i < chartCount; i++) {
        int idx = (oldest + i) % kChartPoints;
        lv_chart_set_next_value(chartBt, chartSerBt, btHistory[idx]);
        lv_chart_set_next_value(chartEt, chartSerEt, etHistory[idx]);
    }
    lv_chart_refresh(chartBt);
    lv_chart_refresh(chartEt);
}

void LvglUi::btnCb(lv_event_t* e) {
    LvglUi* ui = (LvglUi*)lv_event_get_user_data(e);
    if (!ui || !ui->commandCb)
        return;

    // Ignore touches while the display is off — the screen must stay untouchable.
    if (!ui->screenOn)
        return;

    switch (ui->timerState) {
    case TimerState::START:
        ui->commandCb("timerStart");
        break;
    case TimerState::STOP:
        ui->commandCb("timerPause");
        break;
    case TimerState::RESET:
        ui->commandCb("timerReset");
        break;
    }
}

void LvglUi::updateData(double bt, double rorBt, double et, double rorEt, const String& unit, const String& title,
                        unsigned long timerSecs, const String& ipAddr, bool timerRunning) {
    LvglLock lock(lvglMutex);

    this->timerRunning = timerRunning;
    this->timerSecs = timerSecs;

    lv_label_set_text(mainBar_.headerLabel, title.c_str());
    lv_label_set_text(chartBar_.headerLabel, title.c_str());

    // Roast timer as mm:ss
    unsigned long mm = timerSecs / 60;
    unsigned long ss = timerSecs % 60;
    String timerTxt = String(mm) + ":" + (ss < 10 ? "0" : "") + String(ss);
    lv_label_set_text(timerLabel, timerTxt.c_str());

    // IP address (show a hint until WiFi connects)
    const char* ipTxt = ipAddr.isEmpty() ? "--" : ipAddr.c_str();
    lv_label_set_text(mainBar_.ipLabel, ipTxt);
    lv_label_set_text(chartBar_.ipLabel, ipTxt);

    // Single control button reflects the timer state (START/STOP/RESET).
    applyTimerState(timerStateFrom(timerRunning, timerSecs));

    if (isnan(bt)) {
        lv_label_set_text(btLabel, "--");
        lv_label_set_text(btRorLabel, "");
    } else {
        lv_label_set_text(btLabel, (String(bt, 1) + unit).c_str());
        lv_label_set_text(btRorLabel, rorText(rorBt).c_str());
    }

    if (isnan(et)) {
        lv_label_set_text(etLabel, "--");
        lv_label_set_text(etRorLabel, "");
    } else {
        lv_label_set_text(etLabel, (String(et, 1) + unit).c_str());
        lv_label_set_text(etRorLabel, rorText(rorEt).c_str());
    }

    // --- Roast-profile charts -----------------------------------------------
    // Sample the temps every kChartSampleMs into the rolling ring buffer.
    unsigned long now = millis();
    if (now - lastChartSampleMs >= kChartSampleMs) {
        lastChartSampleMs = now;

        if (!isnan(bt) && !isnan(et)) {
            int btVal = (int)round(bt);
            int etVal = (int)round(et);

            chartHead = (chartHead + 1) % kChartPoints;
            btHistory[chartHead] = (int16_t)btVal;
            etHistory[chartHead] = (int16_t)etVal;
            if (chartCount < kChartPoints)
                chartCount++;

            if (chartBt) {
                lv_chart_set_next_value(chartBt, chartSerBt, btVal);
                lv_chart_refresh(chartBt);
            }
            if (chartEt) {
                lv_chart_set_next_value(chartEt, chartSerEt, etVal);
                lv_chart_refresh(chartEt);
            }
        }
    }

    // Keep the chart Y-range in step with the temperature unit.
    bool isF = (unit == "F" || unit == "f");
    if (isF != unitF) {
        unitF = isF;
        int rangeMax = unitF ? 580 : 300;
        if (chartBt)
            lv_chart_set_axis_range(chartBt, LV_CHART_AXIS_PRIMARY_Y, 0, rangeMax);
        if (chartEt)
            lv_chart_set_axis_range(chartEt, LV_CHART_AXIS_PRIMARY_Y, 0, rangeMax);
    }

    // Live readouts on the chart footer (safe even when that screen is hidden).
    if (chartBtLabel)
        lv_label_set_text(chartBtLabel, (String("BT ") + (isnan(bt) ? "--" : String(bt, 1) + unit)).c_str());
    if (chartEtLabel)
        lv_label_set_text(chartEtLabel, (String("ET ") + (isnan(et) ? "--" : String(et, 1) + unit)).c_str());
}

String LvglUi::rorText(double ror) {
    if (isnan(ror))
        return "";

    String t = String((int)round(ror));
    if (ror >= 0 && ror < 10)
        t = String((double)ror, 1);

    return "ROR " + t;
}

LvglUi::TimerState LvglUi::timerStateFrom(bool running, unsigned long secs) {
    if (running)
        return TimerState::STOP;
    if (secs > 0)
        return TimerState::RESET;
    return TimerState::START;
}

void LvglUi::applyTimerState(TimerState st) {
    timerState = st;
    if (!stateBtn || !stateBtnLabel)
        return;

    switch (st) {
    case TimerState::START: // primary: start the roast
        lv_obj_set_style_bg_color(stateBtn, theme_.btnStartBg, 0);
        lv_obj_set_style_bg_color(stateBtn, theme_.btnStartPress, LV_STATE_PRESSED);
        lv_obj_set_style_border_width(stateBtn, 0, 0);
        lv_obj_set_style_text_color(stateBtnLabel, theme_.btnStartText, 0);
        lv_label_set_text(stateBtnLabel, "START");
        break;
    case TimerState::STOP: // muted brick red: pause the roast
        lv_obj_set_style_bg_color(stateBtn, theme_.btnStopBg, 0);
        lv_obj_set_style_bg_color(stateBtn, theme_.btnStopPress, LV_STATE_PRESSED);
        lv_obj_set_style_border_width(stateBtn, 0, 0);
        lv_obj_set_style_text_color(stateBtnLabel, theme_.btnStopText, 0);
        lv_label_set_text(stateBtnLabel, "STOP");
        break;
    case TimerState::RESET: // secondary (bordered): zero the timer
        lv_obj_set_style_bg_color(stateBtn, theme_.btnResetBg, 0);
        lv_obj_set_style_bg_color(stateBtn, theme_.btnResetPress, LV_STATE_PRESSED);
        lv_obj_set_style_border_color(stateBtn, theme_.btnResetBorder, 0);
        lv_obj_set_style_border_width(stateBtn, 1, 0);
        lv_obj_set_style_text_color(stateBtnLabel, theme_.btnResetText, 0);
        lv_label_set_text(stateBtnLabel, "RESET");
        break;
    }
}

void LvglUi::setDarkMode(bool dark) {
    LvglLock lock(lvglMutex);

    if (dark == darkMode)
        return;

    darkMode = dark;

    // Defer the heavy UI rebuild to the LVGL render task (Core 0). Running
    // createUi() + lv_obj_delete here would block the Arduino loop task — the
    // same task that must keep broadcasting BLE/WS data (BLE-JSON would pause)
    // — and it would run on a small-stack task. The LVGL task picks up the
    // flag before its next lv_timer_handler() pass, so broadcasts keep flowing.
    themeRebuildWasChart = onChartPage;
    themeRebuildPending = true;
}

void LvglUi::rebuildTheme() {
    // Runs on the LVGL task (lvglTaskEntry → just before lv_timer_handler),
    // already holding the recursive mutex. LVGL owns the context here, so the
    // full rebuild is safe and never stalls the app loop.

    bool wasChart = themeRebuildWasChart;
    themeRebuildPending = false;

    // Remember the old screens. The currently-active one can't be deleted
    // until a new screen is loaded, but the non-active ones can be freed
    // BEFORE building the new UI — this caps peak LVGL heap at ~1x the UI
    // footprint instead of 2x. The old code built all 4 new screens while all
    // 4 old screens were still alive, which could exhaust the LV_MEM_SIZE pool
    // (LV_USE_ASSERT_MALLOC → abort → reboot → BLE client dropped → BLE-JSON
    // stopped).
    lv_obj_t* oldMain = scrMain;
    lv_obj_t* oldChart = scrChart;
    lv_obj_t* oldFw = scrFw;
    lv_obj_t* oldSplash = scrSplash;
    lv_obj_t* active = lv_screen_active();

    // Reset widget pointers — the rebuild recreates them all.
    scrMain = nullptr;
    scrChart = nullptr;
    scrFw = nullptr;
    scrSplash = nullptr;
    fwBar = nullptr;
    fwLabel = nullptr;
    fwTitleLabel = nullptr;
    onFirmwarePage = false;
    prevPageWasChart = false;
    chartBt = nullptr;
    chartEt = nullptr;
    chartSerBt = nullptr;
    chartSerEt = nullptr;
    chartBtLabel = nullptr;
    chartEtLabel = nullptr;
    mainBar_ = TopBar();
    chartBar_ = TopBar();

    // Free the non-active old screens first (the active one waits below).
    if (oldMain && oldMain != active) {
        lv_obj_delete(oldMain);
        oldMain = nullptr;
    }
    if (oldChart && oldChart != active) {
        lv_obj_delete(oldChart);
        oldChart = nullptr;
    }
    if (oldFw && oldFw != active) {
        lv_obj_delete(oldFw);
        oldFw = nullptr;
    }
    if (oldSplash && oldSplash != active) {
        lv_obj_delete(oldSplash);
        oldSplash = nullptr;
    }

    createUi(); // builds all screens and loads scrMain

    // The old active screen is no longer active — safe to free now.
    if (oldMain)
        lv_obj_delete(oldMain);
    if (oldChart)
        lv_obj_delete(oldChart);
    if (oldFw)
        lv_obj_delete(oldFw);
    if (oldSplash)
        lv_obj_delete(oldSplash);

    // Re-apply runtime visuals to the freshly built widgets.
    applyTimerState(timerStateFrom(timerRunning, timerSecs));
    setWiFiConnected(wifiConnected);
    setBtConnected(btConnected);
    refreshChart(); // refill the chart from the ring buffer (kept across themes)

    // Restore whichever page was active before the theme change. createUi()
    // loads the main page, so reset onChartPage before re-asserting it.
    onChartPage = false;
    if (wasChart)
        showChartPage();
}

void LvglUi::setWiFiConnected(bool connected) {
    LvglLock lock(lvglMutex);

    wifiConnected = connected;
    lv_color_t c = connected ? theme_.dotOnWifi : theme_.dotOff;
    if (mainBar_.wifiDot)
        lv_obj_set_style_bg_color(mainBar_.wifiDot, c, 0);
    if (chartBar_.wifiDot)
        lv_obj_set_style_bg_color(chartBar_.wifiDot, c, 0);
}

void LvglUi::setBtConnected(bool connected) {
    LvglLock lock(lvglMutex);

    btConnected = connected;
    lv_color_t c = connected ? theme_.dotOnBt : theme_.dotOff;
    if (mainBar_.btDot)
        lv_obj_set_style_bg_color(mainBar_.btDot, c, 0);
    if (chartBar_.btDot)
        lv_obj_set_style_bg_color(chartBar_.btDot, c, 0);
}

void LvglUi::displayOn(bool on) {
    screenOn = on;
    // Backlit LCD: backlight fully off, or back to the last set brightness.
    if (LCD_BL_PIN >= 0)
        ledcWrite(LCD_BL_CHANNEL, on ? dutyFromPct(brightnessPct) : 0);
}

void LvglUi::setBrightness(int percent) {
    if (percent < 0)
        percent = 0;
    if (percent > 100)
        percent = 100;

    brightnessPct = percent;

    // Apply immediately if the display is currently on.
    if (LCD_BL_PIN >= 0 && screenOn)
        ledcWrite(LCD_BL_CHANNEL, dutyFromPct(brightnessPct));
}

void LvglUi::updateFirmwareProgress(int progress) {
    LvglLock lock(lvglMutex);

    // The widgets live on the dedicated firmware-update screen (built in
    // buildFirmwareScreen); here we only update their values.
    if (!fwBar || !fwLabel)
        return;

    if (progress < 0)
        progress = 0;
    if (progress > 100)
        progress = 100;

    lv_bar_set_value(fwBar, progress, LV_ANIM_OFF);
    lv_label_set_text(fwLabel, (String(progress) + " %").c_str());
}

void LvglUi::rotateScreen() {
    // Runtime rotation in LVGL 8 requires re-registering the display driver;
    // not supported here — the S3 LCD is fixed to landscape.
    debugln(F("# LVGL rotation not supported (fixed landscape)"));
}
