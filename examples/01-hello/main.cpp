/**
 * @file main.cpp
 * @brief 01-hello — the smallest complete WuaDVI sketch.
 *
 * Does my board work? Brings it up and puts one panel on the HDMI output.
 * The RP2354B display engine is probed, flashed if its firmware differs and
 * kept healthy by the library; this sketch never mentions it.
 *
 * Copyright (c) 2026 Wualabs LTD. MIT licensed.
 */
#include <WuaDVI.h>

WuaDVI dvi;

void setup() {
    Serial.begin(115200);
    Serial.setTxTimeoutMs(0);
    delay(1500);

    if (!dvi.begin(WUA_RES_640x480x1)) {
        Serial.printf("[ERROR] %s\n", dvi.lastError());
        return; /* loop() keeps retrying */
    }
    wua_ui_init();

    lv_obj_t *scr = lv_screen_active();
    lv_obj_set_style_bg_color(scr, wua_theme()->bg, 0);
    lv_obj_set_style_pad_all(scr, wua_pad(), 0);

    lv_obj_t *tile = wua_tile(scr, nullptr, 100, 100, wua_theme()->tile);
    wua_label(tile, "Hello, WuaDVI", 12, wua_theme()->accent);
    wua_label(tile, dvi.resolutionName(), 6, wua_theme()->dim);

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
