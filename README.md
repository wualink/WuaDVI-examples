# WuaDVI-examples

Demo sketches for the **WuaDVI board**, built on
[WuaDVI-lib](https://github.com/wualink/WuaDVI-lib).

Wualink, member of Wualabs — [wualabs.com](https://wualabs.com)

```bash
pio run -e 02-dashboard -t upload
pio device monitor
```

---

## The demos

Deliberately different from each other. A catalogue of five variations on one
layout would prove nothing; these exercise the library from different angles.

| Env | What it is | Starts in |
|---|---|---|
| `01-hello` | One panel, two lines. The "does my board work" sketch. | 640×480 mono |
| `02-dashboard` | Five instrument panels: readouts, clock, round gauge, linear meter. Installs its own theme. | 640×480 mono |
| `03-big-readout` | A single enormous number filling the screen — a production counter or machine-status sign. | 800×600 mono |
| `04-console` | A scrolling text log; whatever you type in the serial monitor appears on screen. | 1280×720 mono |

---

## Choosing the resolution

Every demo runs at **every** resolution. It is a runtime value, set by two
function calls and nothing else:

```cpp
dvi.begin(WUA_RES_800x600x1);          // start here
dvi.setResolution(WUA_RES_320x240);    // change — restarts into the new mode
```

Each demo passes the mode that suits it to `begin()`, which is the one line to
edit to see a demo at another resolution:

| Env | `begin()` |
|---|---|
| `01-hello`, `02-dashboard` | `WUA_RES_640x480x1` |
| `03-big-readout` | `WUA_RES_800x600x1` |
| `04-console` | `WUA_RES_1280x720x1` |

`setResolution()` **does not return** — it restarts the board into the new mode,
because the display engine reboots to change mode and the widget primitives
resolve their pixel sizes when they are created, so the interface has to be
rebuilt at the new size anyway. `setup()` runs again and does exactly that.

The mode it requests is a **one-shot**, consumed by that restart. A power cycle
brings the board back up in whatever the demo's `begin()` asks for, so a demo
always looks the way it was written to look. To remember a choice instead, store
it in your sketch and pass it to `begin()` — the
[library README](https://github.com/wualink/WuaDVI-lib#resolution) shows the
pattern.

### Why these four

- **`01-hello`** is the smallest thing that can fail, so it is the first thing
  to flash on a board that is not behaving.
- **`02-dashboard`** is the widget showcase, and the one that demonstrates
  theming: its palette is cyan rather than the library's amber, which is the
  visible proof that identity belongs to the application and mechanism to the
  library.
- **`03-big-readout`** pushes the font ladder to its top end. A single value at
  40 % of the screen height is where automatic scaling has to work hardest, and
  where clipping would show first.
- **`04-console`** pushes it to the bottom end, with many small lines instead of
  a few large widgets. Between it and the dashboard the whole range is covered.

---

## Requirements

| | |
|---|---|
| Board | WuaDVI (ESP32-C3 + RP2354B) |
| Toolchain | PlatformIO (the library's build hook does not run in the Arduino IDE) |
| Monitor | HDMI. Some monitors reject the 30 Hz of `1280x720x1` |

Nothing else to install: the library is pulled in by `lib_deps`, and it brings
LVGL, a working `lv_conf.h` and the display-engine firmware with it. The first
build needs a network connection to fetch that firmware; later builds use the
cached copy.

The library is pinned to an exact commit rather than a branch — it pins the
display-engine firmware in turn, and a reproducible build wants the whole chain
fixed.

---

## Writing your own

Start from `01-hello`. The shape is always the same:

```cpp
#include <WuaDVI.h>
WuaDVI dvi;

void setup() {
    dvi.begin(WUA_RES_640x480x1);
    wua_ui_init();
    // ... build a screen with the wua_* primitives ...
}

void loop() { dvi.loop(); }
```

Two habits worth copying from these demos:

- **Size everything in percentages.** The primitives resolve pixels, fonts and
  padding for whichever mode is running, which is what lets one layout serve
  320×240 and 1280×720. A sketch that hard-codes pixels gives that up.
- **Animate what should look smooth.** LVGL animations step at the display
  refresh rate; a value moved from a one-second timer looks choppy no matter how
  healthy the pixel link is, and — worse — hides a link that has actually
  stopped.

To add a demo: a folder under `examples/`, and an env in `platformio.ini`:

```ini
[env:05-mine]
build_src_filter = +<05-mine/>
```

---

## License

Released under the [MIT License](LICENSE) — © 2026 Wualabs LTD.
