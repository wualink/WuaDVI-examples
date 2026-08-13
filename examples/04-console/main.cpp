/**
 * @file main.cpp
 * @brief 04-console — a scrolling text log on the HDMI output.
 *
 * Text-heavy rather than widget-heavy: the board as a status console for a
 * machine that has no screen of its own. Anything you type in the serial
 * monitor is echoed to the display.
 *
 * This one leans on the small end of the font ladder, where a dashboard leans
 * on the large end — between the two, the whole range gets exercised.
 *
 * Copyright (c) 2026 Wualabs LTD. MIT licensed.
 */
#include <WuaDVI.h>

WuaDVI dvi;
static wua_obj_t *s_log = nullptr;

/** Longest history the screen can usefully hold. */
#define LOG_LINES 14
static char s_lines[LOG_LINES][48];
static uint8_t s_count = 0;

/** Append a line, dropping the oldest when full, and repaint the log. */
static void log_line(const char *text) {
    if (s_count == LOG_LINES) {
        for (uint8_t i = 1; i < LOG_LINES; ++i)
            memcpy(s_lines[i - 1], s_lines[i], sizeof(s_lines[0]));
        --s_count;
    }
    snprintf(s_lines[s_count], sizeof(s_lines[0]), "%s", text);
    ++s_count;

    /* One label holding every line: a single object rewritten wholesale beats
     * LOG_LINES objects created and destroyed, both for memory and for the
     * amount of screen the stream has to repaint. */
    static char buf[LOG_LINES * sizeof(s_lines[0])];
    buf[0] = '\0';
    for (uint8_t i = 0; i < s_count; ++i) {
        strncat(buf, s_lines[i], sizeof(buf) - strlen(buf) - 2);
        if (i + 1 < s_count)
            strncat(buf, "\n", sizeof(buf) - strlen(buf) - 1);
    }
    if (s_log != nullptr)
        wua_label_set(s_log, buf);
}

static void tick_cb(void) {
    static uint32_t n = 0;
    char line[48];
    int16_t temp;
    if (dvi.temperature(&temp)) {
        snprintf(line, sizeof(line), "[%04lu] engine %d.%d C, %lu packets",
                 (unsigned long)++n, temp / 10, abs(temp % 10),
                 (unsigned long)dvi.rectsSent());
    } else {
        snprintf(line, sizeof(line), "[%04lu] waiting for telemetry",
                 (unsigned long)++n);
    }
    log_line(line);
}

void setup() {
    Serial.begin(115200);
    Serial.setTxTimeoutMs(10); /* never wait on an absent host; 0 would hang */
    delay(1500);

    /* 1280x720 gives the most text on screen; at 30 Hz, which a log does not
     * care about. */
    dvi.setResolution(WUA_RES_1280x720x1);
    if (!dvi.begin()) {
        Serial.printf("[ERROR] %s\n", dvi.lastError());
        return;
    }
    wua_ui_init();

    wua_obj_t *scr = wua_screen();

    wua_obj_t *head = wua_header(scr);
    wua_label(head, "System console", 7, wua_theme()->accent);

    wua_obj_t *panel = wua_tile(scr, nullptr, 100, 88, wua_theme()->tile);
    wua_align(panel, WUA_ALIGN_START);
    s_log = wua_label(panel, "", 4, wua_theme()->text);

    log_line("console ready - type in the serial monitor");
    wua_timer(2000, tick_cb);
    Serial.printf("[OK] %s running - type to echo on screen\n",
                  dvi.resolutionName());
}

void loop() {
    dvi.loop();

    /* Echo whatever arrives on the console, a line at a time. */
    static char in[40];
    static uint8_t len = 0;
    while (Serial.available() > 0) {
        const char c = (char)Serial.read();
        if (c == '\n' || c == '\r' || len == sizeof(in) - 1) {
            if (len > 0) {
                in[len] = '\0';
                char line[48];
                snprintf(line, sizeof(line), "> %s", in);
                log_line(line);
                len = 0;
            }
        } else {
            in[len++] = c;
        }
    }
}
