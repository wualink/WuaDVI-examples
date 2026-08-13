/**
 * @file main.cpp
 * @brief 05-widgets — a gallery of the library's widget primitives.
 *
 * Ten widgets, one per panel, so the whole basic set can be judged at a glance
 * on real hardware.
 *
 * There is no LVGL in this file. Not one call, one type or one constant — the
 * screen, the layout, the widgets, the values, the timer and the animations all
 * come from `wua_*`. That is the demo as much as the widgets are: a sketch that
 * reaches into LVGL directly has to know which styling survives the monochrome
 * modes, where every colour collapses to black or white by luminance and a
 * stock slider becomes an empty rectangle. The primitives already know.
 *
 * Nothing here is touched by a finger — the board has no input device, HDMI
 * goes out and nothing comes back. Discrete widgets are stepped by a timer and
 * continuous ones are swept, which is how they will be driven in a real
 * application anyway: by state arriving from the program.
 *
 * Copyright (c) 2026 Wualabs LTD. MIT licensed.
 */
#include <WuaDVI.h>

WuaDVI dvi;

/* Widgets the timer steps. */
static wua_obj_t *s_button = nullptr;
static wua_obj_t *s_checkbox = nullptr;
static wua_obj_t *s_switch = nullptr;
static wua_obj_t *s_led = nullptr;
static wua_obj_t *s_roller = nullptr;
static wua_obj_t *s_dropdown = nullptr;

/* Widgets the sweeps drive. */
static wua_obj_t *s_slider = nullptr;
static wua_obj_t *s_bar = nullptr;
static wua_obj_t *s_arc = nullptr;

/* Panel colours: distinct enough to tell apart, dark enough that white content
 * contrasts and the fill thresholds to black in the monochrome modes. */
#define COL_A wua_color(0x0E5A50)
#define COL_B wua_color(0x0A5570)
#define COL_C wua_color(0x0C4478)
#define COL_D wua_color(0x1E3878)
#define COL_E wua_color(0x322E70)

/* Indicator colour. Deliberately NOT one of the panel colours above: an
 * indicator drawn in the shade of the panel it sits on is invisible in the
 * colour modes, and the monochrome ones hide the mistake by forcing every
 * indicator to white anyway. */
#define ON_COLOR wua_color(0x00D0FF)

/* Five across, two down. Two tall rows rather than three short ones: height is
 * the axis that clips first, and the roller and the arc need it most. */
#define TILE_W 18
#define TILE_H 45

/**
 * @brief Step every discrete widget, once a second.
 *
 * State, not value: for these widgets the checked/unchecked and
 * pressed/released styling is the thing under test, because that is what the
 * 1-bit reduction attacks.
 */
static void step_cb(void) {
    static int32_t step = 0;
    const int32_t on = (++step & 1);

    wua_button_set_pressed(s_button, on);
    wua_checkbox_set(s_checkbox, on);
    wua_switch_set(s_switch, on);
    wua_led_set(s_led, on);

    wua_roller_select(s_roller, step % 4);
    wua_dropdown_select(s_dropdown, step % 4);
}

void setup() {
    Serial.begin(115200);
    Serial.setTxTimeoutMs(10); /* never wait on an absent host; 0 would hang */
    delay(1500);

    /* 640x480 mono: ten panels want the room, and this mode has the most
     * scanout margin of the set. Every size below is a percentage, so the
     * gallery holds together in the other four as well. */
    dvi.setResolution(WUA_RES_400x240);
    if (!dvi.begin()) {
        Serial.printf("[ERROR] %s\n", dvi.lastError());
        return; /* loop() keeps retrying */
    }
    wua_ui_init();

    wua_obj_t *scr = wua_screen();

    wua_obj_t *head = wua_header(scr);
    wua_label(head, "LVGL widget gallery", 6, wua_theme()->accent);

    wua_obj_t *grid = wua_grid(scr);

    /* — Discrete: state, not value — */

    wua_obj_t *p = wua_tile(grid, "Button", TILE_W, TILE_H, COL_A);
    s_button = wua_button(p, "OK", 85, 34, COL_A);

    p = wua_tile(grid, "Checkbox", TILE_W, TILE_H, COL_B);
    s_checkbox = wua_checkbox(p, "Enabled", 26);

    p = wua_tile(grid, "Switch", TILE_W, TILE_H, COL_C);
    s_switch = wua_switch(p, 70, 26);

    p = wua_tile(grid, "LED", TILE_W, TILE_H, COL_D);
    s_led = wua_led(p, 45, ON_COLOR);

    p = wua_tile(grid, "Spinner", TILE_W, TILE_H, COL_E);
    wua_spinner(p, 70, 2000);

    /* — Continuous: swept — */

    p = wua_tile(grid, "Slider", TILE_W, TILE_H, COL_A);
    s_slider = wua_slider(p, 88, 16, 0, 100, ON_COLOR);

    p = wua_tile(grid, "Bar", TILE_W, TILE_H, COL_B);
    s_bar = wua_bar(p, 88, 18, 0, 100, ON_COLOR);

    p = wua_tile(grid, "Arc", TILE_W, TILE_H, COL_C);
    s_arc = wua_arc(p, 80, 0, 100, ON_COLOR);

    /* — Selection — */

    p = wua_tile(grid, "Roller", TILE_W, TILE_H, COL_D);
    s_roller = wua_roller(p, "Idle\nRun\nHold\nFault", 92, 2);

    p = wua_tile(grid, "Dropdown", TILE_W, TILE_H, COL_E);
    s_dropdown = wua_dropdown(p, "Auto\nManual\nService\nOff", 92);

    /* A setter is exactly the shape a sweep wants, so it goes in directly. */
    wua_timer(1000, step_cb);
    wua_sweep(s_slider, wua_slider_set, 0, 100, 2600);
    wua_sweep(s_bar, wua_bar_set, 0, 100, 3400);
    wua_sweep(s_arc, wua_arc_set, 0, 100, 4200);

    Serial.printf("[OK] %s running - free heap %u B\n", dvi.resolutionName(),
                  ESP.getFreeHeap());
}

void loop() {
    dvi.loop();
}
