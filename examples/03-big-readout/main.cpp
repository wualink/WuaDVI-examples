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
static wua_obj_t *s_value = nullptr;
static wua_obj_t *s_unit = nullptr;

static void tick_cb(void) {
    static int32_t v = 0;
    v = (v + 7) % 1000;
    if (s_value != nullptr)
        wua_label_setf(s_value, "%ld", (long)v);
}

void setup() {
    Serial.begin(115200);
    Serial.setTxTimeoutMs(10); /* never wait on an absent host; 0 would hang */
    delay(1500);

    /* 800x600 suits a read-from-afar sign: native pixels, no doubling. */
    dvi.setResolution(WUA_RES_800x600x1);
    if (!dvi.begin()) {
        Serial.printf("[ERROR] %s\n", dvi.lastError());
        return;
    }
    wua_ui_init();

    wua_obj_t *scr = wua_screen();
    wua_align(scr, WUA_ALIGN_CENTER);

    wua_label(scr, "PRODUCED TODAY", 6, wua_theme()->dim);

    /* 40 % of the screen height.  The sample says "three digits", not those
     * three glyphs: wua_value_label reserves each digit at the widest digit's
     * width, because Montserrat is proportional and "440" is wider than "999".
     * Taking the sample literally used to leave the final 0 of 440 and 940
     * wrapping onto a second line. */
    s_value = wua_value_label(scr, "999", 40);

    s_unit = wua_label(scr, "units", 7, wua_theme()->accent);

    wua_timer(500, tick_cb);
    Serial.printf("[OK] %s running\n", dvi.resolutionName());
}

void loop() {
    dvi.loop();
}
