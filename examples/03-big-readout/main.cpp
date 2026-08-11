/**
 * @file main.cpp
 * @brief 03-big-readout — one enormous number, the whole screen.
 *
 * The opposite of a dashboard: a single value read from across a room. This is
 * the layout a machine-status sign or a production counter wants, and it is
 * where the primitives' automatic font scaling has to work hardest — the same
 * code must fill 320x240 and 1280x720 without clipping.
 *
 * Copyright (c) 2026 Wualabs LTD. MIT licensed.
 */
#include <WuaDVI.h>

WuaDVI dvi;
static lv_obj_t *s_value = nullptr;
static lv_obj_t *s_unit = nullptr;

static void tick_cb(lv_timer_t *t) {
    LV_UNUSED(t);
    static int32_t v = 0;
    v = (v + 7) % 1000;
    if (s_value != nullptr)
        lv_label_set_text_fmt(s_value, "%ld", (long)v);
}

void setup() {
    Serial.begin(115200);
    Serial.setTxTimeoutMs(0);
    delay(1500);

    /* 800x600 suits a read-from-afar sign: native pixels, no doubling. */
    dvi.setResolution(WUA_RES_800x600x1);
    if (!dvi.begin()) {
        Serial.printf("[ERROR] %s\n", dvi.lastError());
        return;
    }
    wua_ui_init();

    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, wua_theme()->bg, 0);
    lv_obj_set_style_pad_all(scr, wua_pad(), 0);
    lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(scr, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    wua_label(scr, "PRODUCED TODAY", 6, wua_theme()->dim);

    /* 40 % of the screen height: wua_value_label steps the font down if the
     * widest content would not fit the width, so "999" never clips. */
    s_value = wua_value_label(scr, "999", 40);

    s_unit = wua_label(scr, "units", 7, wua_theme()->accent);

    lv_timer_create(tick_cb, 500, nullptr);
    Serial.printf("[OK] %s running\n", dvi.resolutionName());
}

void loop() {
    dvi.loop();
}
