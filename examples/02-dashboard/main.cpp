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

static wua_obj_t *s_counter = nullptr;
static wua_obj_t *s_clock = nullptr;
static wua_obj_t *s_temp = nullptr;
static wua_gauge_t *s_gauge = nullptr;
static wua_meter_t *s_meter = nullptr;

/* This demo's identity. Every foreground stays lit and every background stays
 * dark under the 1-bit reduction of the monochrome modes — wua_theme_set()
 * refuses a palette that would not, so a mistake here is caught at startup
 * rather than after switching resolution. */
static const wua_theme_t kTheme = {
    .bg = WUA_RGB(0x00, 0x00, 0x00),
    .tile = WUA_RGB(0x12, 0x18, 0x20),
    .text = WUA_RGB(0xFF, 0xFF, 0xFF),
    .dim = WUA_RGB(0xB0, 0xB8, 0xC0),
    .accent = WUA_RGB(0x00, 0xD0, 0xFF), /* cyan instead of the amber */
    .track = WUA_RGB(0x24, 0x2C, 0x38),
};

/* Panel colours, distinct enough to tell apart at a glance and dark enough to
 * threshold to black in the monochrome modes. */
#define COL_COUNTER wua_color(0x0E5A50)
#define COL_CLOCK   wua_color(0x0A5570)
#define COL_TEMP    wua_color(0x0C4478)
#define COL_GAUGE   wua_color(0x1E3878)
#define COL_METER   wua_color(0x322E70)

/** Once-a-second readouts. The analogue widgets are animated instead. */
static void tick_cb(void) {
    static uint32_t ticks = 0;
    ++ticks;

    wua_label_setf(s_counter, "%lu", (unsigned long)ticks);
    wua_clock_set(s_clock, millis() / 1000u);

    int16_t c10;
    if (dvi.temperature(&c10)) {
        const int16_t a = (int16_t)(c10 < 0 ? -c10 : c10);
        wua_label_setf(s_temp, "%s%d.%d C", (c10 < 0) ? "-" : "",
                       a / 10, a % 10);
    } else {
        wua_label_set(s_temp, "--.- C");
    }
}

void setup() {
    Serial.begin(115200);
    Serial.setTxTimeoutMs(10); /* never wait on an absent host; 0 would hang */
    delay(1500);

    dvi.setResolution(WUA_RES_640x480x1);
    if (!dvi.begin()) {
        Serial.printf("[ERROR] %s\n", dvi.lastError());
        return; /* loop() keeps retrying */
    }
    wua_ui_init();

    if (!wua_theme_set(&kTheme))
        Serial.println("[WARN] theme rejected as not monochrome-safe");

    wua_obj_t *scr = wua_screen();

    /* wua_column() sizes itself to its children, so the header never steals
     * room from the grid below it. */
    wua_obj_t *head = wua_header(scr);
    wua_label(head, "Instrument dashboard", 7, wua_theme()->accent);
    wua_label(head, dvi.resolutionName(), 5, wua_theme()->dim);

    wua_obj_t *grid = wua_grid(scr);

    /* Percentages only: the library resolves the pixels for the active mode,
     * so this same layout holds from 320x240 to 1280x720. */
    wua_obj_t *c = wua_tile(grid, "Counter", 31, 48, COL_COUNTER);
    s_counter = wua_value_label(c, "99999", 14);

    c = wua_tile(grid, "Uptime", 31, 48, COL_CLOCK);
    s_clock = wua_clock(c, 12);

    c = wua_tile(grid, "Engine temp", 31, 48, COL_TEMP);
    s_temp = wua_value_label(c, "-99.9 C", 12);

    c = wua_tile(grid, "Gauge", 48, 48, COL_GAUGE);
    s_gauge = wua_gauge(c, 85, 0, 100);

    c = wua_tile(grid, "Level", 48, 48, COL_METER);
    s_meter = wua_meter(c, 85, 0, 100);

    wua_timer(1000, tick_cb);
    wua_gauge_sweep(s_gauge, 0, 100, 3000);
    wua_meter_sweep(s_meter, 0, 100, 2200);

    Serial.printf("[OK] %s running\n", dvi.resolutionName());
}

void loop() {
    dvi.loop();
}
