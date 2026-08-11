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

## Changing the resolution

Every demo runs at **every** resolution, and switching does not need a rebuild.
Open the serial monitor and press a number:

```
 1  320x240 RGB565      4  800x600 mono
 2  400x240 RGB565      5  1280x720 mono (30 Hz)
 3  640x480 mono
 c  forget the stored mode   i  status   ?  this list
```

The board stores the choice and restarts into it, which is also how the display
engine changes mode. Every demo prints this list at startup and forwards keys to
the library in one line:

```cpp
void loop() {
    dvi.loop();
    while (Serial.available() > 0)
        dvi.consoleKey((char)Serial.read());
}
```

### Why every demo looked the same

**A stored mode wins over the `begin()` argument.** That is what makes a switch
survive the restart it performs — but it also means that once anything has
stored a mode, every sketch you flash afterwards comes up in *that* mode and
ignores the one it asked for. Four demos that each request a different
resolution will all look identical.

Press **`c`** to forget the stored mode and restart; the sketch's own choice
applies again. Since library commit `f6ec426`, `begin()` also prints a line when
it overrides you, rather than doing it silently.

```cpp
dvi.begin(WUA_RES_800x600x1);            // start here, unless a mode is stored
dvi.setResolution(WUA_RES_320x240);      // change — stores and restarts
WuaDVI::clearStoredResolution();         // forget it, so begin() decides again
```

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
