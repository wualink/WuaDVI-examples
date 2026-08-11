/**
 * @file main.cpp
 * @brief 02-dashboard — a five-panel instrument dashboard.
 *
 * The demo the WuaDVI board shipped with, and the one that shows the widget
 * primitives together: readouts, a clock, a round gauge and a linear meter.
 *
 * It also shows the point of theming. The colours below are this demo's
 * identity, not the library's: the primitives own the mechanism (sizing that
 * follows the resolution, the dither that suppresses HDMI banding, redraw
 * suppression that keeps the pixel link quiet) and take their palette from
 * whatever theme the application installs.
 *
 * Copyright (c) 2026 Wualabs LTD. MIT licensed.
 */
#include <WuaDVI.h>

WuaDVI dvi;

static lv_obj_t *s_counter = nullptr;
static lv_obj_t *s_clock = nullptr;
static lv_obj_t *s_temp = nullptr;
static wua_gauge_t *s_gauge = nullptr;
static wua_meter_t *s_meter = nullptr;

/* This demo's identity. Every foreground stays lit and every background stays
 * dark under the 1-bit reduction of the monochrome modes — wua_theme_set()
 * refuses a palette that would not, so a mistake here is caught at startup
 * rather than after switching resolution. */
static const wua_theme_t kTheme = {
    .bg = LV_COLOR_MAKE(0x00, 0x00, 0x00),
    .tile = LV_COLOR_MAKE(0x12, 0x18, 0x20),
    .text = LV_COLOR_MAKE(0xFF, 0xFF, 0xFF),
    .dim = LV_COLOR_MAKE(0xB0, 0xB8, 0xC0),
    .accent = LV_COLOR_MAKE(0x00, 0xD0, 0xFF), /* cyan instead of the amber */
    .track = LV_COLOR_MAKE(0x24, 0x2C, 0x38),
};

/* Panel colours, distinct enough to tell apart at a glance and dark enough to
 * threshold to black in the monochrome modes. */
#define COL_COUNTER lv_color_hex(0x0E5A50)
#define COL_CLOCK   lv_color_hex(0x0A5570)
#define COL_TEMP    lv_color_hex(0x0C4478)
#define COL_GAUGE   lv_color_hex(0x1E3878)
#define COL_METER   lv_color_hex(0x322E70)

/** Once-a-second readouts. The analogue widgets are animated instead. */
static void tick_cb(lv_timer_t *t) {
    LV_UNUSED(t);
    static uint32_t ticks = 0;
    ++ticks;

    lv_label_set_text_fmt(s_counter, "%lu", (unsigned long)ticks);
    wua_clock_set(s_clock, millis() / 1000u);

    int16_t c10;
    if (dvi.temperature(&c10)) {
        const int16_t a = (int16_t)(c10 < 0 ? -c10 : c10);
        lv_label_set_text_fmt(s_temp, "%s%d.%d C", (c10 < 0) ? "-" : "",
                              a / 10, a % 10);
    } else {
        lv_label_set_text(s_temp, "--.- C");
    }
}

static void gauge_cb(void *v, int32_t value) {
    LV_UNUSED(v);
    wua_gauge_set(s_gauge, value);
}

static void meter_cb(void *v, int32_t value) {
    LV_UNUSED(v);
    wua_meter_set(s_meter, value);
}

/**
 * @brief Start an endless 0..100..0 sweep.
 *
 * An animation steps at the display refresh rate, so the needle and the bar
 * move smoothly. Driving them from the one-second timer would look choppy
 * however healthy the pixel stream was — and would hide a stream that had
 * actually stopped.
 */
static void start_sweep(lv_anim_exec_xcb_t exec_cb, uint32_t period_ms) {
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, nullptr);
    lv_anim_set_values(&a, 0, 100);
    lv_anim_set_duration(&a, period_ms);
    lv_anim_set_playback_duration(&a, period_ms);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
    lv_anim_set_exec_cb(&a, exec_cb);
    lv_anim_start(&a);
}

void setup() {
    Serial.begin(115200);
    Serial.setTxTimeoutMs(0);
    delay(1500);

    if (!dvi.begin(WUA_RES_640x480x1)) {
        Serial.printf("[ERROR] %s\n", dvi.lastError());
        return; /* loop() keeps retrying */
    }
    wua_ui_init();

    if (!wua_theme_set(&kTheme))
        Serial.println("[WARN] theme rejected as not monochrome-safe");

    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, wua_theme()->bg, 0);
    lv_obj_set_style_pad_all(scr, wua_pad(), 0);
    lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);

    /* wua_column() sizes itself to its children, so the header never steals
     * room from the grid below it. */
    lv_obj_t *head = wua_column(scr);
    lv_obj_set_width(head, lv_pct(100));
    wua_label(head, "Instrument dashboard", 7, wua_theme()->accent);
    wua_label(head, dvi.resolutionName(), 5, wua_theme()->dim);

    lv_obj_t *grid = wua_row(scr);
    lv_obj_set_width(grid, lv_pct(100));
    lv_obj_set_flex_grow(grid, 1);

    /* Percentages only: the library resolves the pixels for the active mode,
     * so this same layout holds from 320x240 to 1280x720. */
    lv_obj_t *c = wua_tile(grid, "Counter", 31, 48, COL_COUNTER);
    s_counter = wua_value_label(c, "99999", 14);

    c = wua_tile(grid, "Uptime", 31, 48, COL_CLOCK);
    s_clock = wua_clock(c, 12);

    c = wua_tile(grid, "Engine temp", 31, 48, COL_TEMP);
    s_temp = wua_value_label(c, "-99.9 C", 12);

    c = wua_tile(grid, "Gauge", 48, 48, COL_GAUGE);
    s_gauge = wua_gauge(c, 85, 0, 100);

    c = wua_tile(grid, "Level", 48, 48, COL_METER);
    s_meter = wua_meter(c, 85, 0, 100);

    lv_timer_create(tick_cb, 1000, nullptr);
    start_sweep(gauge_cb, 3000);
    start_sweep(meter_cb, 2200);

    Serial.printf("[OK] %s running\n", dvi.resolutionName());
    WuaDVI::printConsoleHelp();
}

void loop() {
    dvi.loop();

    /* Change resolution without rebuilding: 1..5 pick a mode, 'c' forgets the
     * stored one, '?' lists the keys.  Switching stores the choice and restarts
     * the board into it. */
    while (Serial.available() > 0)
        dvi.consoleKey((char)Serial.read());
}
