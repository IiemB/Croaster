#pragma once
#include <Adafruit_ILI9341.h>
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <lvgl.h>

#include <functional>

#include "LvglTouch.h"

/**
 * @class LvglUi
 * @brief LVGL-based UI for the built-in ILI9341 display on the ES3C28P (ESP32-S3).
 *
 * Compact, instrument-style coffee-roaster monitor (landscape 320x240):
 *  - top bar: brand with IP below it, WiFi/BLE status chips
 *  - two temperature cards (BT / ET) with RoR
 *  - bottom bar: one container holding the roast timer and a single control
 *    button that cycles START → STOP → RESET
 *
 * A second screen shows a rolling BT/ET roast-profile chart (see
 * `buildChartScreen`), opened with a left swipe and left with a right swipe.
 *
 * A double tap on the touchscreen toggles the display (backlight) on/off.
 *
 * LVGL rendering runs on a dedicated FreeRTOS task pinned to Core 0
 * (dual-core: the app loop on Core 1 handles sensors/WiFi/BLE/WebSocket in
 * parallel). All public LVGL-mutating methods take a recursive mutex so the
 * two cores never touch LVGL at the same time.
 */
class LvglUi {
public:
    /** @brief Callback for UI commands ("timerStart", "timerPause", "timerReset"). */
    using CommandCb = std::function<void(const String& cmd)>;

    /** @brief The three states of the single timer control button. */
    enum class TimerState : uint8_t {
        START, ///< not running, timer at 0 → start the roast
        STOP,  ///< running → pause the roast
        RESET, ///< paused with elapsed time → zero it
    };

    bool begin();
    void loop();

    /** @brief Refreshes all live widgets. */
    void updateData(double bt, double rorBt, double et, double rorEt, const String& unit, const String& title,
                    unsigned long timerSecs, const String& ipAddr, bool timerRunning);

    /** @brief Registers the callback fired by the timer control buttons. */
    void setCommandCallback(CommandCb cb) {
        commandCb = std::move(cb);
    }

    /** @brief Registers the callback fired on a double tap (display toggle). */
    void setDisplayToggleCallback(std::function<void()> cb) {
        displayToggleCb = std::move(cb);
    }

    /** @brief Switches to the roast-profile chart page (BT/ET over time). */
    void showChartPage();

    /** @brief Returns to the main instrument page. */
    void showMainPage();

    /** @brief Shows/hides the full-page firmware-update status screen. */
    void showFirmwarePage(bool updating);

    /** @brief Ends the boot splash (after a minimum timed duration). Called at
     *         the end of setup(); the LVGL task switches to the main page. */
    void finishSplash();

    /** @brief Updates the WiFi connection indicator. */
    void setWiFiConnected(bool connected);

    /** @brief Updates the BLE connection indicator. */
    void setBtConnected(bool connected);

    void displayOn(bool on);

    /** @brief Sets the backlight brightness (10-100 percent). Minimum 10% to
     *         avoid turning the panel fully off via the slider. */
    void setBrightness(int percent);

    /** @brief Sets the light/dark theme (dark = true, light = false). */
    void setDarkMode(bool dark);

    /** @brief Returns whether the dark theme is active. */
    bool isDarkMode() const {
        return darkMode;
    }

    /** @brief Returns the current backlight brightness as 0-100 percent (exact). */
    int brightnessPercent() const {
        return brightnessPct;
    }

    void updateFirmwareProgress(int progress);
    void rotateScreen();

private:
    /** @brief Live top-bar widgets (brand/IP/status chips) for one screen. */
    struct TopBar {
        lv_obj_t* headerLabel = nullptr;
        lv_obj_t* ipLabel = nullptr;
        lv_obj_t* wifiDot = nullptr;
        lv_obj_t* btDot = nullptr;
    };

    void createUi();
    void buildMainScreen(lv_obj_t* scr);
    void buildChartScreen(lv_obj_t* scr);
    void buildFirmwareScreen(lv_obj_t* scr);
    void buildSplashScreen(lv_obj_t* scr);
    void buildTopBar(lv_obj_t* scr, TopBar& bar);
    void buildAboutScreen(lv_obj_t* scr);
    void showAboutPage();
    static String rorText(double ror);
    static void btnCb(lv_event_t* e);
    static void flushCb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map);

    /** @brief Maps the timer state to the control button's label/style. */
    static TimerState timerStateFrom(bool running, unsigned long secs);

    /** @brief Applies the given state to the control button (label + colors). */
    void applyTimerState(TimerState st);

    /** @brief Re-fills the chart series from the ring buffer (theme rebuild). */
    void refreshChart();

    /** @brief Rebuilds the UI for the active theme. Runs on the LVGL task
     *         (picked up by lvglTaskEntry when a theme change is queued). */
    void rebuildTheme();

    /** @brief Creates a shared-style rolling line chart widget (no series yet). */
    lv_obj_t* createChartWidget(lv_obj_t* parent);

    /** @brief Color palette for the active theme (dark or light). */
    struct Theme {
        lv_color_t bg, cardBg, border, textPrimary, textMuted, accentBt, accentEt;
        lv_color_t dotOnWifi, dotOnBt, dotOff;
        lv_color_t btnStartBg, btnStartText, btnStartPress;
        lv_color_t btnStopBg, btnStopText, btnStopPress;
        lv_color_t btnResetBg, btnResetText, btnResetBorder, btnResetPress;
    };

    /** @brief Fills the active theme palette from the darkMode flag. */
    void loadTheme();

    /** @brief Converts a 0-100 brightness to an 8-bit PWM duty (rounded). */
    static uint8_t dutyFromPct(int pct) {
        return (uint8_t)((pct * 255 + 50) / 100);
    }

    Adafruit_ILI9341* tft = nullptr;
    lv_display_t* disp = nullptr;
    LvglTouch touch;

    CommandCb commandCb;
    std::function<void()> displayToggleCb;

    // Main instrument screen
    lv_obj_t* scrMain = nullptr;

    // Top bars (brand/IP/status chips) — one per screen so the chart page
    // shows the same appbar as the main page.
    TopBar mainBar_;
    TopBar chartBar_;

    // Temperature cards
    lv_obj_t* btLabel = nullptr;
    lv_obj_t* btRorLabel = nullptr;
    lv_obj_t* etLabel = nullptr;
    lv_obj_t* etRorLabel = nullptr;

    // Bottom bar (timer + single control button in one container)
    lv_obj_t* timerBar = nullptr;
    lv_obj_t* timerLabel = nullptr;
    lv_obj_t* stateBtn = nullptr;
    lv_obj_t* stateBtnLabel = nullptr;

    // Roast profile chart page (second screen) — one card per thermocouple.
    lv_obj_t* scrChart = nullptr;
    lv_obj_t* chartBt = nullptr; ///< BT line chart (inside its own card).
    lv_obj_t* chartEt = nullptr; ///< ET line chart (inside its own card).
    lv_chart_series_t* chartSerBt = nullptr;
    lv_chart_series_t* chartSerEt = nullptr;
    lv_obj_t* chartBtLabel = nullptr; ///< Live BT readout (BT card header).
    lv_obj_t* chartEtLabel = nullptr; ///< Live ET readout (ET card header).

    // Chart data ring buffer (kept outside the widgets so a theme rebuild can
    // re-fill freshly created charts without losing the roast history).
    static constexpr int kChartPoints = 120;              ///< Rolling window width (points).
    static constexpr unsigned long kChartSampleMs = 2000; ///< One sample every 2 s.
    int16_t btHistory[kChartPoints] = {0};
    int16_t etHistory[kChartPoints] = {0};
    int chartHead = 0;  ///< Index of the most recent sample.
    int chartCount = 0; ///< Samples accumulated (<= kChartPoints).
    unsigned long lastChartSampleMs = 0;
    bool unitF = false;       ///< °F → wider chart Y range.
    bool onChartPage = false; ///< Which screen is currently shown.

    // Firmware update screen (full page, shown during OTA)
    lv_obj_t* scrFw = nullptr;
    lv_obj_t* fwBar = nullptr;
    lv_obj_t* fwLabel = nullptr;
    lv_obj_t* fwTitleLabel = nullptr;
    bool onFirmwarePage = false;   ///< Firmware page is currently shown.
    bool prevPageWasChart = false; ///< Page to return to after the update.

    // About page
    lv_obj_t* scrAbout = nullptr;
    TopBar aboutBar_;
    bool onAboutPage = false;

    // Boot splash screen (shown during setup(); switches to the main page
    // after finishSplash() + a minimum timed duration, driven by the LVGL task).
    lv_obj_t* scrSplash = nullptr;
    static constexpr unsigned long kSplashMinMs = 2000; ///< Minimum splash time.
    unsigned long splashStartMs = 0;
    bool splashDone = false;

    // State
    TimerState timerState = TimerState::START;
    bool wifiConnected = false;
    bool btConnected = false;
    bool screenOn = true;    ///< Backlight state (touches suppressed while off).
    int brightnessPct = 100; ///< Current backlight brightness (0-100, exact).

    // Dedicated LVGL render task (pinned to Core 0). A recursive mutex
    // serializes LVGL access between this task and the app loop on Core 1
    // (recursive so LVGL-internal touch callbacks can safely re-enter).
    SemaphoreHandle_t lvglMutex = nullptr;
    TaskHandle_t lvglTaskHandle = nullptr;
    static void lvglTaskEntry(void* arg);
    void checkSplashTransition();

    // Theme + timer state (kept across screen rebuilds so visuals can be re-applied).
    Theme theme_;
    bool darkMode = false;
    bool timerRunning = false;
    unsigned long timerSecs = 0;

    // Current page tracker for roll navigation (Main -> Chart -> About)
    enum class Page : uint8_t { MAIN = 0, CHART = 1, ABOUT = 2 };
    Page currentPage = Page::MAIN;

    // Deferred theme rebuild — queued by setDarkMode(), executed on the LVGL
    // task before its next lv_timer_handler() pass (see rebuildTheme).
    bool themeRebuildPending = false;  ///< A theme rebuild is queued.
    bool themeRebuildWasChart = false; ///< Page active when the rebuild was queued.
    bool themeRebuildWasAbout = false; ///< About page active when rebuild queued.
};
