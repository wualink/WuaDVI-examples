# Demo catalogue — working tracker

The plan for covering LVGL 9.5 from the WuaDVI board, one demo at a time.

Each demo is built, flashed and **checked on hardware** before the next one
starts. A demo that only compiles proves nothing: every defect worth finding in
this catalogue — overlapping headers, choppy animation, widgets that vanish in
the monochrome modes — is invisible to the compiler.

## Status

| | Demo | State |
|---|---|---|
| 01 | hello | ✅ verified after the primitive migration, serial closed |
| 02 | dashboard | ✅ verified after the primitive migration, serial closed |
| 03 | big-readout | ⚠️ re-check needed (defect found and fixed in the library) |
| 04 | console | ⚠️ re-check needed (migrated to primitives) |
| 05 | widgets | ✅ verified in all five modes; serial-timeout fix unverified |
| 06 | chart | ⬜ |
| 07 | table-list | ⬜ |
| 08 | styles | ⬜ |
| 09 | animations | ⬜ |
| 10 | images | ⬜ |
| 11 | layouts | ⬜ |
| 12 | fonts-text | ⬜ |
| 13 | scale | ⬜ |
| 14 | screens | ⬜ |
| 15 | canvas | ⬜ |
| 16 | overlays | ⬜ |
| 17 | observer | ⬜ |
| 18 | stress | ⬜ |

---

## The constraint that shapes the catalogue

**The board is a display, not a terminal.** HDMI goes out; nothing comes back.
There is no touchscreen, mouse or encoder, so LVGL has no input device
registered.

Every interactive widget is therefore driven from code — a timer, an animation
or `lv_obj_send_event()`. That is not a workaround for the demos' benefit: it is
how the board will actually be used, with state arriving from the application
and, later, from the network. A demo that pretended a finger was available would
be teaching the wrong thing.

The consequence to keep in mind per widget: anything whose *whole point* is
entering data — `keyboard`, `textarea`, `ime` — can be rendered but not
exercised, and is left out.

---

## The demos

### 05 — widgets

A gallery of the basic widget set: `button`, `checkbox`, `switch`, `slider`,
`bar`, `led`, `spinner`, `arc`, `roller`, `dropdown`. Discrete widgets cycle
their state on a slow timer; continuous ones are animated.

**Proves:** the basic set renders and picks up the theme, and — the part worth
flashing for — which widgets stay legible once the 1-bit modes threshold every
colour to black or white. A slider track and its indicator that collapse to the
same shade is a real failure this demo makes visible.

### 06 — chart

`lv_chart` with a line series, a bar series and a scatter plot; samples entering
from the right, axis ticks and labels, a cursor.

**Proves:** the chart API, and the redraw cost of a widget that invalidates a
large area on every sample — the honest stress case for the rectangle stream,
and a useful counterpoint to the small dirty rects the dashboard produces.

### 07 — table-list

`lv_table` with columns, per-cell styling and merged cells, plus `lv_list` with
symbol icons.

**Proves:** dense text layout, per-cell style resolution, and scrolling a
container taller than the screen — which on this board means scroll position has
to survive a pipeline rebuild.

### 08 — styles

Linear and radial gradients, shadows, borders, corner radii, opacity, blend
modes, and state-based styling.

**Proves:** the style engine, and above all **what survives the 1-bit
threshold** versus RGB565. This is the demo that tells an application author
which visual effects are usable on this hardware and which are decoration that
disappears the moment someone selects a monochrome mode.

### 09 — animations

`lv_anim` across the easing paths, with playback, repeat counts, delays and
timelines, plus style transitions between states.

**Proves:** the animation engine, and the stream under sustained motion — where
the frame-rate ceiling sits in each mode. Related: continuous values must be
animated rather than stepped from a slow timer, which is both smoother and
honest about a stream that has stopped.

### 10 — images

An image from a C array, recoloured, zoomed and rotated about a pivot, plus
`animimage` cycling frames.

**Proves:** the decoder, transform quality, flash cost, and how a photograph
dithers down to one bit.

### 11 — layouts

Flex and grid side by side: alignment, grow, gaps, spans and nesting, with the
same screen running in every mode.

**Proves:** the layout engine and genuine resolution independence — that a
screen written in percentages holds from 320×240 to 1280×720 without a single
pixel constant.

### 12 — fonts-text

The font ladder, `LV_SYMBOL` glyphs, the long-text modes (wrap, dot, scroll,
clip), `lv_span` for mixed runs, and UTF-8 accents.

**Proves:** text rendering, and how much *legible* text each mode actually
holds — the number that decides whether a design needs 1280×720.

### 13 — scale

`lv_scale` with needles, major and minor ticks, arc ranges and coloured
sections: industrial instrument faces.

**Proves:** the 9.x replacement for the removed `lv_meter`, and the quality of
arcs and thin lines at each resolution, where 320×240 is unforgiving.

### 14 — screens

Several screens swapped with `lv_screen_load_anim`, plus `tabview`, `tileview`
and `menu`, navigated by a timer.

**Proves:** screen management and load animations — and full-screen repaint
cost, which is the worst case the pixel link ever sees.

### 15 — canvas

`lv_canvas` with lines, arcs, polygons, text and images drawn by hand: a
waveform or hand-plotted trace.

**Proves:** the direct draw API and canvas memory, which is the tightest RAM
case on the ESP32-C3 — the demo most likely to hit a real allocation ceiling.

### 16 — overlays

`lv_msgbox` and `lv_win` over a live background, using the top and system
layers.

**Proves:** layering and z-order, and partial redraw as an overlay appears and
disappears over moving content.

### 17 — observer

LVGL 9's subject/observer binding: one value driving a label, a bar and an arc
at once, with visibility following state.

**Proves:** the data-binding API — **the piece that matters most for
WuaDVI-net**, because it is how remote state will reach widgets without the
network layer knowing what a widget is.

### 18 — stress

A frame-rate and throughput readout, N objects animating at once, with
rectangles sent and failed measured per mode.

**Proves:** the real limits of the pixel link per resolution, and specifically
where 800×600 — the mode with the least scanout margin — begins to break.

---

## Deliberately out of scope

- **`keyboard`, `textarea`, `ime`** — meaningless without an input device.
- **`gif`, `lottie`, `3dtexture`** — decoders too heavy in RAM and flash for an
  ESP32-C3 already running at ~40 % RAM.
- **`calendar`, `spinbox`, `buttonmatrix`, `imagebutton`, `arclabel`** — folded
  into 05 and 07 as secondary widgets rather than given demos of their own.

---

## Open

- **Demos 03 and 04 still need re-checking on hardware.** 01 and 02 have now
  been re-run after the primitive migration and both pass, with the serial
  monitor closed — which also covers the USB-CDC timeout fix for those two.

  The re-check was worth doing: **03 failed.** Its counter runs 0-999, and at
  440 and 940 the final `0` wrapped onto a second line and pushed the rest of
  the screen down. The cause was in the library, not the demo — `wua_value_label`
  froze its width at the literal sample `"999"`, which in proportional Montserrat
  is narrower than `"440"`, and never set a long mode, so LVGL's default WRAP
  reflowed the layout. Fixed in wualink/WuaDVI-lib#4: digits are reserved at the
  widest digit's width, and the label clips instead of wrapping.

  This is exactly why a tick that predates the change it should have covered is
  worse than no tick at all. The defect was in code that had been marked
  verified, and only re-running it on the board found it.

  **After the repin, 01, 02 and 05 are worth a second glance too** — the fix
  makes value labels reserve slightly more width wherever the sample held a
  narrow digit, which touches the gauge readout, the clock and the temperature
  readouts. Reserving more is the safe direction, but it can tighten a layout
  that was already close.

- **The USB-CDC timeout fix is unverified on all five**, 05 included: it landed
  after the last hardware run. What it should fix is the board sitting on the
  display engine's splash until a terminal is opened. Test it by powering the
  board with no monitor attached.

## Solved, worth remembering

- **1280×720 restarting forever** was LVGL's own memory pool, not the ESP heap.
  The log gave it away: bring-up completed, `[LVGL] ready` printed, and then the
  whole sequence began again *without* `setup()` ever reaching its `[OK]` line —
  so the crash was in building widgets, after the display was up.

  `LV_MEM_SIZE` is a static pool, and widgets are sized in pixels, so a screen
  costs roughly four times as much at 1280×720 as at 640×480. 96 KB was enough
  for a dashboard and not for a gallery. Exhausting it does not return an
  error: `LV_USE_ASSERT_MALLOC` parks in a spin loop and the task watchdog
  reboots, which reads as a bring-up loop and points at the wrong subsystem
  entirely. Now 128 KB.

- **The generated `lv_conf.h` was never refreshed.** Raising `LV_MEM_SIZE` in
  the library changed nothing, because the build hook writes the config only
  when absent and the copy in `.pio/libdeps/` was already there. A library
  upgrade could silently not apply. The hook now keeps a hash of what it wrote:
  an unmodified copy is refreshed, an edited one is left alone and reported.
  Existing checkouts have no hash, so clear the stale copy once:

  ```bash
  rm -f .pio/libdeps/*/lv_conf.h
  ```

## Conventions

- **Resolution is fixed per demo**, selected once with `dvi.setResolution()` in
  `setup()` before `dvi.begin()`. Demos never change mode at runtime — that
  belongs to WuaDVI-net.
- **No serial console.** A demo shows how the library is used; it is not a tool.
- **Percentages, never pixel constants.** The primitives resolve pixels, fonts
  and padding for the active mode.
- **No LVGL in a demo. None.** Not a call, a type or a constant: `grep -c "lv_\|LV_"`
  over a demo must return 0. Screen, layout, widgets, values, timers and
  animations all come from `wua_*`.

  This is not style. A stock LVGL widget is styled in shades of one hue, and
  the monochrome modes threshold by luminance, so track and indicator collapse
  into one flat shape — a sketch author cannot be expected to know that, and
  the primitives already do. Every `lv_` call in a demo is a place where
  somebody copying it inherits a bug.

  If a demo needs something the library has no primitive for, **add the
  primitive**. Hand-rolling it in the demo hides the gap from every other user.

  Current debt, to be cleared as each demo is revisited:

  | Demo | `lv_`/`LV_` occurrences |
  |---|---|
  | 05-widgets | 0 ✅ |
  | 01-hello | 4 |
  | 03-big-readout | 12 |
  | 04-console | 14 |
  | 02-dashboard | 43 |
- **Animate what should look smooth.** An `lv_anim` steps at the refresh rate; a
  value moved by a one-second timer looks choppy and hides a dead stream.
- One folder per demo under `examples/`, one env in `platformio.ini`:

  ```ini
  [env:05-widgets]
  build_src_filter = +<05-widgets/>
  ```
