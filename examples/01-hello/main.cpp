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
    Serial.setTxTimeoutMs(10); /* never wait on an absent host; 0 would hang */
    delay(1500);

    dvi.setResolution(WUA_RES_640x480x1);
    if (!dvi.begin()) {
        Serial.printf("[ERROR] %s\n", dvi.lastError());
        return; /* loop() keeps retrying */
    }
    wua_ui_init();

    wua_obj_t *scr = wua_screen();

    wua_obj_t *tile = wua_tile(scr, nullptr, 100, 100, wua_theme()->tile);
    wua_label(tile, "Hello, WuaDVI", 12, wua_theme()->accent);
    wua_label(tile, dvi.resolutionName(), 6, wua_theme()->dim);

    Serial.printf("[OK] %s running\n", dvi.resolutionName());
}

void loop() {
    dvi.loop();
}
