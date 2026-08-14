# Config Surfaces Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the menu's 31 theme colours editable, animatable and persistent, and give the panel framework four real panels plus the submenu that arranges them.

**Architecture:** A single name-to-pointer colour registry is the spine — the theme submenu lists it, the colour helper edits an entry of it, "Sync With…" copies between entries, and the rainbow animates a subset of it. Panels are separate: four render callbacks registered against the existing framework, behind the `player_valid()` gate that the panel call site is currently missing.

**Tech Stack:** C++17 without exceptions or RTTI, hand-rolled mini-STL (`src/stl/`), OpenOrbis PS4 toolchain via WSL + ninja, host unit tests through a standalone `clang++` invocation.

**Spec:** `docs/superpowers/specs/2026-08-14-config-surfaces-design.md`

## Global Constraints

Every task's requirements implicitly include these. Each has already cost this project a crash or a lost day.

- **No `.init_array`.** Global constructors never run. Namespace-scope data must be constant-initialised (string literals, address constants, `constexpr` constructors) or live behind a function-local static.
- **`stl::function` captures cap at 64 bytes** (`STL_FUNCTION_CAP`, enforced by `static_assert`). Capture a small index or id, never a struct or an entry.
- **`stl::string` is a fixed 128-byte buffer** that truncates silently.
- **No `stl::to_string`.** Use the shared helpers added in Task 8.
- **Nothing may call a native during `menu::build()`.** Option construction sets state; applying state belongs in `feature_update()`.
- **Per-frame work that calls natives must be gated on `game::player_valid()`.**
- **Host tests take C headers only.** MSVC's C++ stdlib rejects the installed clang with `STL1000`, so a file under host test must not include the C++ standard library or any PS4 header.
- **Build:** `wsl.exe bash -lc 'export OO_PS4_TOOLCHAIN=/home/bbc/OpenOrbis-PS4-Toolchain; export PATH="$OO_PS4_TOOLCHAIN/bin/linux:$PATH"; cd /mnt/e/Projects/PS4/InsulinEngine/InsulinGTAV && cmake --build build-wsl'` — build into `build-wsl/`, never `build/`.
- **Bump `INSULIN_BUILD_TAG` on every deploy.** It is the only reliable proof of which `.prx` is running.

**Testing reality.** Only two things in this plan are host-testable: the rainbow's step maths and colour conversion. Everything else is menu behaviour on a console. Tasks that can carry a real test do; the rest end with an explicit on-console verification step, and those steps are the gate — do not mark such a task done on a successful compile alone.

---

### Task 1: Config batch mode

`config::write_color` calls a file-static `save()` that re-serialises the whole document and rewrites `config.json`. Saving a theme is 31 colours, so 31 full serialisations inside one button press. That reads as a freeze on console, and freezes in this project get chased as crashes.

**Files:**
- Modify: `src/util/config.cpp` (the file-static `save()` around line 34, and `write_color` at 142)
- Modify: `src/util/config.h` (public API, after `load()`)

**Interfaces:**
- Consumes: nothing
- Produces: `util::config::begin_batch()`, `util::config::end_batch()` — free functions in `namespace util::config`, both `void`, re-entrant via a depth counter.

- [ ] **Step 1: Add the depth counter and gate `save()`**

In `src/util/config.cpp`, next to the existing file-static `save()`:

```cpp
    // Writes normally persist immediately. A batch defers that to the outermost
    // end_batch(), so a caller writing many keys at once (the theme's 31
    // colours) pays for one serialisation instead of 31.
    static int  g_batch_depth = 0;
    static bool g_batch_dirty = false;

    static void save() {
        if (g_batch_depth > 0) { g_batch_dirty = true; return; }
        g_root.save_to_file(CONFIG_PATH, 2);
    }
```

- [ ] **Step 2: Add the public entry points**

Also in `src/util/config.cpp`, inside `namespace util::config`:

```cpp
    void begin_batch() { g_batch_depth++; }

    void end_batch() {
        if (g_batch_depth > 0) g_batch_depth--;
        if (g_batch_depth == 0 && g_batch_dirty) {
            g_batch_dirty = false;
            g_root.save_to_file(CONFIG_PATH, 2);
        }
    }
```

Declare both in `src/util/config.h` inside `namespace util::config`, directly under `inline void load() { ... }`:

```cpp
    // Defer persistence until the outermost end_batch(). Re-entrant.
    void begin_batch();
    void end_batch();
```

- [ ] **Step 3: Build**

Run the WSL build command from Global Constraints.
Expected: compiles, no new warnings.

- [ ] **Step 4: Commit**

```bash
git add src/util/config.cpp src/util/config.h
git commit -m "perf(config): batch mode so a multi-key write serialises once

write_color and friends each call save(), which re-serialises the whole
document and rewrites config.json. Saving a theme is 31 colours, so 31 full
writes inside one button press - a visible freeze on console. begin_batch /
end_batch defer that to the outermost end_batch via a depth counter; every
existing single-write callsite is unaffected."
```

---

### Task 2: Expose the existing colour registry

`src/menu/base/util/theme.cpp:21` already holds `COLORS[]`, a name-to-pointer table of all 31 theme colours, plus `reset_to_default()` restoring the built-in palette. It is file-static, so nothing outside that file can iterate it. This task makes it reachable. It does NOT build a second table — a duplicate registry drifts against this one the first time a colour is added to `ui_vars`, and the drift is silent.

**Files:**
- Modify: `src/menu/base/util/theme.h` (add three accessors to `namespace menu::theme`)
- Modify: `src/menu/base/util/theme.cpp` (implement them next to `COLORS[]`)

**Interfaces:**
- Consumes: the existing `COLORS[]` table and `struct nc { const char* name; color_rgba* p; }` in `theme.cpp`
- Produces, in `namespace menu::theme`: `int color_count()`, `const char* color_name(int index)`, `color_rgba* color_ptr(int index)`

- [ ] **Step 1: Declare the accessors**

In `src/menu/base/util/theme.h`, inside `namespace menu::theme`, after `stl::vector<stl::string> list();`:

```cpp
    // The colour registry, exposed for editing surfaces. theme.cpp already owns
    // this table for save/load; a second copy elsewhere would drift against it
    // the first time a colour is added to ui_vars.
    int         color_count();
    const char* color_name(int index);   // stable key, e.g. "option_selected"
    color_rgba* color_ptr(int index);    // nullptr for an invalid index
```

- [ ] **Step 2: Implement them**

In `src/menu/base/util/theme.cpp`, directly below the `COLORS[]` definition:

```cpp
    int color_count() { return (int)(sizeof(COLORS) / sizeof(COLORS[0])); }

    const char* color_name(int index) {
        if (index < 0 || index >= color_count()) return "";
        return COLORS[index].name;
    }

    color_rgba* color_ptr(int index) {
        if (index < 0 || index >= color_count()) return nullptr;
        return COLORS[index].p;
    }
```

- [ ] **Step 3: Build**

Run the WSL build command from Global Constraints.
Expected: compiles, no new warnings.

- [ ] **Step 4: Commit**

```bash
git add src/menu/base/util/theme.h src/menu/base/util/theme.cpp
git commit -m "feat(ui): expose the theme colour registry for editing surfaces

theme.cpp has held a name-to-pointer table of all 31 colours since the theme
system landed, but it is file-static, so the menu could save and load colours
without being able to enumerate or edit one.

Three accessors rather than a second table: a duplicate registry drifts against
this one the first time a colour is added to ui_vars, and the drift is silent -
the new colour simply never appears in whichever list was forgotten."
```

---

### Task 3: Move the theme controls into the Themes submenu

There are currently two Save Theme buttons. The pair in `settings.cpp:26-38` works, calling `menu::theme::save` and `reset_to_default`. The pair in `settings_themes.cpp` is a scaffold that emits a `Theme saved` notification and writes nothing — and it sits one level deeper, under Settings → Themes, where a user would reasonably look first. This task makes the Themes submenu the real home and deletes the duplicates.

**Files:**
- Modify: `src/menu/base/submenus/settings_themes.cpp` (replace the stub body)
- Modify: `src/menu/base/submenus/settings_themes.h` (add the update overrides)
- Modify: `src/menu/base/submenus/settings.cpp` (remove the moved options)

**Interfaces:**
- Consumes: `menu::theme::save`, `reset_to_default`, `list`, `load_by_name`
- Produces: nothing new. Task 7 appends the per-colour editor list to this same `load()`.

- [ ] **Step 1: Read what you are moving**

Read `src/menu/base/submenus/settings.cpp` in full first. It carries the Save Theme button, the Reset to Default button, a `g_themes_dirty` flag, a `break_option("Themes")` and a theme-picker list rebuilt in `update()`. All of that moves. The Language option and the Streamer Mode option stay.

- [ ] **Step 2: Write the Themes submenu**

Replace the body of `src/menu/base/submenus/settings_themes.cpp`. Reproduce the save, reset and picker logic as `settings.cpp` had it, including the `g_themes_dirty` deferral — the list must be refreshed from `update()`, never re-entrantly inside a click handler, which is why that flag exists:

```cpp
#include "menu/base/submenus/settings_themes.h"
#include "menu/base/submenus/settings.h"
#include "menu/base/options/button.h"
#include "menu/base/options/break.h"
#include "menu/base/util/notify.h"
#include "menu/base/util/theme.h"
#include <stdio.h>

namespace {
    bool g_themes_dirty = false;
    int  g_built_count  = -1;
}

void settings_themes_menu::load() {
    set_name("Themes");
    set_parent<settings_menu>();

    add_option(button_option("Save Theme")
        .add_tooltip("Save current colours / fonts / positions as theme_N.json (rename the file to taste)")
        .add_click([] {
            char name[32];
            snprintf(name, sizeof(name), "theme_%d", (int)menu::theme::list().size() + 1);
            menu::theme::save(name);
            menu::notify::stacked("Theme", "Saved");
            g_themes_dirty = true;   // list refreshed in update(), not re-entrantly here
        }));

    add_option(button_option("Reset to Default")
        .add_tooltip("Restore the built-in default theme")
        .add_click([] {
            menu::theme::reset_to_default();
            menu::notify::stacked("Theme", "Reset to default");
        }));

    add_option(break_option("Saved Themes").ref());
}

void settings_themes_menu::update() {
    stl::vector<stl::string> themes = menu::theme::list();
    if (g_themes_dirty || g_built_count != (int)themes.size()) {
        g_themes_dirty = false;
        update_once();
    }
}

void settings_themes_menu::update_once() {
    stl::vector<stl::string> themes = menu::theme::list();
    g_built_count = (int)themes.size();
    clear_options(3);

    for (int i = 0; i < g_built_count; i++) {
        add_option(button_option(themes[i])
            .add_tooltip("Apply this theme")
            .add_click([i] {
                stl::vector<stl::string> list = menu::theme::list();
                if (i < (int)list.size()) {
                    menu::theme::load_by_name(list[i].c_str());
                    menu::notify::stacked("Theme", "Applied");
                }
            }));
    }
}

settings_themes_menu* settings_themes_menu::get() {
    static settings_themes_menu instance;
    return &instance;
}
```

The click handler re-reads `menu::theme::list()` instead of capturing the name: the capture has to stay under the 64-byte cap, and the list can change between the option being built and the button being pressed.

In `settings_themes.h`, add to the class if not already present:

```cpp
    void update() override;
    void update_once() override;
```

- [ ] **Step 3: Remove the duplicates from settings.cpp**

Delete from `src/menu/base/submenus/settings.cpp`: the `Save Theme` button, the `Reset to Default` button, the `break_option("Themes")`, the theme-picker list, and the `g_themes_dirty` flag with its rebuild logic. Keep the `Themes` and `Streamer Mode` submenu options and the `Language` option. If removing the picker empties `update()` / `update_once()`, leave them as empty overrides rather than deleting the declarations.

- [ ] **Step 4: Build**

Run the WSL build command.
Expected: compiles, no new warnings, and `grep -n g_themes_dirty src/menu/base/submenus/settings.cpp` returns nothing.

- [ ] **Step 5: Commit**

```bash
git add src/menu/base/submenus/settings_themes.cpp src/menu/base/submenus/settings_themes.h src/menu/base/submenus/settings.cpp
git commit -m "fix(settings): one Save Theme button instead of two, and it works

Settings carried working Save Theme and Reset buttons; Settings -> Themes
carried a scaffold pair with the same names that reported success and wrote
nothing. The stub sat one level deeper, where a user looks first.

The working controls move down into Themes, where they belong now that the
submenu is about to grow a per-colour editor, and the duplicates come out of
Settings. The picker keeps its g_themes_dirty deferral: rebuilding the option
list inside a click handler re-enters the list being iterated."
```

---

### Task 4: Rainbow step maths (host-tested)

**Files:**
- Create: `src/menu/base/util/rainbow_math.h`
- Create: `tests/rainbow_math_test.cpp`

**Interfaces:**
- Consumes: nothing — the file must include no C++ stdlib and no PS4 header, so the host test can compile it
- Produces: `struct menu::rainbow_math::rgb { int r, g, b; }` and `rgb menu::rainbow_math::color_at(int step, int steps, int min, int max)`

- [ ] **Step 1: Write the failing test**

Create `tests/rainbow_math_test.cpp`:

```cpp
// Host unit tests for the rainbow stepping maths.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/rainbow_math_test.cpp -o build/rainbow_math_test.exe
//   ./build/rainbow_math_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
#include "menu/base/util/rainbow_math.h"
#include <stdio.h>

static int g_failed = 0;

static void check(const char* what, int got, int want) {
    if (got != want) { printf("FAIL %s: got %d, want %d\n", what, got, want); g_failed++; }
    else             { printf("ok   %s = %d\n", what, got); }
}

static void check_range(const char* what, int got, int lo, int hi) {
    if (got < lo || got > hi) { printf("FAIL %s: %d outside [%d,%d]\n", what, got, lo, hi); g_failed++; }
    else                      { printf("ok   %s = %d in [%d,%d]\n", what, got, lo, hi); }
}

int main() {
    using namespace menu::rainbow_math;

    // A full cycle returns to where it started.
    rgb first = color_at(0, 80, 25, 250);
    rgb wrapped = color_at(80, 80, 25, 250);
    check("wrap r", wrapped.r, first.r);
    check("wrap g", wrapped.g, first.g);
    check("wrap b", wrapped.b, first.b);

    // Every channel at every step stays inside [min,max]: the cycle must never
    // reach black (invisible text) or full blast.
    for (int s = 0; s < 80; s++) {
        rgb c = color_at(s, 80, 25, 250);
        check_range("r", c.r, 25, 250);
        check_range("g", c.g, 25, 250);
        check_range("b", c.b, 25, 250);
    }

    // Degenerate inputs must not divide by zero or escape the range.
    rgb zero = color_at(5, 0, 25, 250);
    check_range("steps=0 r", zero.r, 25, 250);
    rgb one = color_at(5, 1, 25, 250);
    check_range("steps=1 r", one.r, 25, 250);

    // An inverted range is accepted rather than producing nonsense.
    rgb inverted = color_at(10, 80, 250, 25);
    check_range("inverted r", inverted.r, 25, 250);

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed ? 1 : 0;
}
```

- [ ] **Step 2: Run it to make sure it fails**

Run: `clang++ -std=c++17 -I src tests/rainbow_math_test.cpp -o build/rainbow_math_test.exe`
Expected: FAIL — `'menu/base/util/rainbow_math.h' file not found`.

- [ ] **Step 3: Write the implementation**

Create `src/menu/base/util/rainbow_math.h`:

```cpp
#pragma once

// Rainbow stepping, kept free of the STL and of every PS4 header so the host
// test can include it directly - the same split frame_clock.h uses against
// animated_texture.cpp.
//
// Hue advances 360/steps degrees per call and wraps. Saturation and value are
// full; the resulting channels are then remapped into [min,max] so the cycle
// never reaches black (unreadable menu text) or full saturation. Ozark's
// defaults - min 25, max 250, steps 80 - carry over as ours.
namespace menu::rainbow_math {

    struct rgb { int r, g, b; };

    inline rgb color_at(int step, int steps, int min, int max) {
        if (steps < 1) steps = 1;
        if (min > max) { int t = min; min = max; max = t; }
        if (min < 0) min = 0;
        if (max > 255) max = 255;

        // Hue in sixths, integer maths throughout: no <math.h>, no float drift
        // across a long-running cycle.
        int  wrapped = step % steps;
        if (wrapped < 0) wrapped += steps;

        int  h6      = (wrapped * 6) / steps;          // which sixth, 0..5
        int  within  = (wrapped * 6) % steps;          // position inside it
        int  rising  = (within * 255) / steps;         // 0..254
        int  falling = 255 - rising;

        int r = 0, g = 0, b = 0;
        switch (h6) {
            case 0: r = 255;     g = rising;  b = 0;       break;
            case 1: r = falling; g = 255;     b = 0;       break;
            case 2: r = 0;       g = 255;     b = rising;  break;
            case 3: r = 0;       g = falling; b = 255;     break;
            case 4: r = rising;  g = 0;       b = 255;     break;
            default:r = 255;     g = 0;       b = falling; break;
        }

        // Remap 0..255 into [min,max].
        int span = max - min;
        rgb out;
        out.r = min + (r * span) / 255;
        out.g = min + (g * span) / 255;
        out.b = min + (b * span) / 255;
        return out;
    }
}
```

- [ ] **Step 4: Run the test to verify it passes**

Run:
```bash
clang++ -std=c++17 -I src tests/rainbow_math_test.cpp -o build/rainbow_math_test.exe && ./build/rainbow_math_test.exe
```
Expected: every line `ok`, final line `all passed`, exit code 0.

- [ ] **Step 5: Commit**

```bash
git add src/menu/base/util/rainbow_math.h tests/rainbow_math_test.cpp
git commit -m "feat(ui): rainbow stepping maths, host-tested

Split out of the stateful rainbow so the host test can include it: MSVC's C++
stdlib rejects the installed clang, so anything under test has to compile
against C headers alone. Same split frame_clock.h uses.

Integer maths throughout - a float hue accumulating across a cycle that runs
for hours drifts. Channels are remapped into [min,max] so the animation never
reaches black, which would make menu text unreadable for a frame."
```

---

### Task 5: Rainbow container and per-frame stepping

**Files:**
- Create: `src/menu/base/util/rainbow.h`
- Create: `src/menu/base/util/rainbow.cpp`
- Modify: `src/menu/menu.cpp` (`tick()`)

**Interfaces:**
- Consumes: Task 4 `menu::rainbow_math::color_at`
- Produces: `menu::rainbow` with `configure(int,int,int)`, `add(color_rgba*)`, `remove(color_rgba*)`, `contains(const color_rgba*) const`, `run()`, `stop()`, public `bool m_enabled`, `int m_min`, `int m_max`, `int m_steps`; and `menu::rainbow* menu::get_rainbow()`

- [ ] **Step 1: Write the header**

Create `src/menu/base/util/rainbow.h`:

```cpp
#pragma once
#include "platform/stdafx.h"
#include "global/ui_vars.h"

namespace menu {
    // Animates registered colours through the hue cycle. Holds each colour's
    // pre-rainbow value so stop() restores exactly what was there rather than a
    // guess at what it should have been.
    class rainbow {
    public:
        void configure(int min, int max, int steps);

        void add(color_rgba* c);          // remembers the current value
        void remove(color_rgba* c);       // restores it
        bool contains(const color_rgba* c) const;

        void run();                       // one step; called per frame
        void stop();                      // restore every registered colour

        bool m_enabled = false;
        int  m_min   = 25;
        int  m_max   = 250;
        int  m_steps = 80;
    private:
        int m_step = 0;
        stl::vector<color_rgba*> m_colors;
        stl::vector<color_rgba>  m_originals;   // parallel to m_colors
    };

    rainbow* get_rainbow();
}
```

- [ ] **Step 2: Write the implementation**

Create `src/menu/base/util/rainbow.cpp`:

```cpp
#include "menu/base/util/rainbow.h"
#include "menu/base/util/rainbow_math.h"

namespace menu {
    void rainbow::configure(int min, int max, int steps) {
        m_min = min; m_max = max; m_steps = steps < 1 ? 1 : steps;
    }

    bool rainbow::contains(const color_rgba* c) const {
        for (int i = 0; i < (int)m_colors.size(); i++)
            if (m_colors[i] == c) return true;
        return false;
    }

    void rainbow::add(color_rgba* c) {
        if (!c || contains(c)) return;
        m_colors.push_back(c);
        m_originals.push_back(*c);
    }

    void rainbow::remove(color_rgba* c) {
        for (int i = 0; i < (int)m_colors.size(); i++) {
            if (m_colors[i] != c) continue;
            *m_colors[i] = m_originals[i];
            m_colors.erase(m_colors.begin() + i);
            m_originals.erase(m_originals.begin() + i);
            return;
        }
    }

    void rainbow::run() {
        if (!m_enabled || m_colors.empty()) return;

        rainbow_math::rgb c = rainbow_math::color_at(m_step, m_steps, m_min, m_max);
        m_step = (m_step + 1) % m_steps;

        for (int i = 0; i < (int)m_colors.size(); i++) {
            m_colors[i]->r = c.r;
            m_colors[i]->g = c.g;
            m_colors[i]->b = c.b;
            // Alpha is never touched: cycling opacity makes the menu flicker
            // transparent.
        }
    }

    void rainbow::stop() {
        for (int i = 0; i < (int)m_colors.size(); i++)
            *m_colors[i] = m_originals[i];
        m_enabled = false;
    }

    rainbow* get_rainbow() {
        static rainbow instance;   // function-local: no .init_array in this plugin
        return &instance;
    }
}
```

If `stl::vector` has no `erase(iterator)`, replace the two `erase` calls with a shift-down loop over the remaining elements followed by `pop_back()`; check `src/stl/vector.h` before writing.

- [ ] **Step 3: Step it from tick()**

In `src/menu/menu.cpp`, add `#include "menu/base/util/rainbow.h"`, then inside `tick()` directly before the `TICK_TRACE("notify");` line:

```cpp
        // No player_valid() gate: this reads and writes plain memory and calls
        // no natives. It cannot run during build() either, because tick is only
        // wired as the frame callback after build() returns.
        menu::get_rainbow()->run();
```

- [ ] **Step 4: Build**

Run the WSL build command.
Expected: compiles.

- [ ] **Step 5: Commit**

```bash
git add src/menu/base/util/rainbow.h src/menu/base/util/rainbow.cpp src/menu/menu.cpp
git commit -m "feat(ui): rainbow container - animate registered colours per frame

Holds each colour's pre-rainbow value alongside the pointer, so stop() and
remove() restore exactly what was there instead of a guess. Alpha is never
animated; cycling opacity makes the menu flicker transparent.

Stepped from tick without a player_valid gate, deliberately: it touches only
memory and calls no natives. The callsite says so, because every other
per-frame call in that function does need the gate."
```

---

### Task 6: Colour conversion maths, then the editor

The editor converts RGB to HSV when it opens and HSV back to RGB on every
change. If that round-trip is not stable, colours drift a little each time the
format is switched — a slow, baffling bug. The conversion comes out into a pure
header first so it can be tested, and the renderer delegates to it.

**Files:**
- Create: `src/menu/base/util/color_math.h`
- Create: `tests/color_math_test.cpp`
- Modify: `src/menu/base/renderer.cpp:381` (`rgb_to_hsv` and `hsv_to_rgb` delegate)
- Create: `src/menu/base/submenus/helper_color.h`
- Create: `src/menu/base/submenus/helper_color.cpp`
- Modify: `src/menu/menu.cpp` (register the submenu)

**Interfaces:**
- Consumes: Task 2 accessors (`menu::theme::color_count/color_name/color_ptr`); `menu::renderer::render_color_preview`
- Produces:
  - `struct menu::color_math::hsv { float h, s, v; }`
  - `hsv menu::color_math::rgb_to_hsv(int r, int g, int b)`
  - `void menu::color_math::hsv_to_rgb(float h, float s, float v, int* r, int* g, int* b)`
  - `class helper_color_menu` with `static void target(int registry_index)`, `static int current_target()`, `static color_rgba* target_color()`, `static helper_color_menu* get()`

- [ ] **Step 1: Write the failing conversion test**

Create `tests/color_math_test.cpp`:

```cpp
// Host unit tests for the colour conversion the HSVA editor round-trips through.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/color_math_test.cpp -o build/color_math_test.exe
//   ./build/color_math_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
#include "menu/base/util/color_math.h"
#include <stdio.h>

static int g_failed = 0;

static void check_near(const char* what, int got, int want, int tolerance) {
    int d = got - want; if (d < 0) d = -d;
    if (d > tolerance) { printf("FAIL %s: got %d, want %d (+-%d)\n", what, got, want, tolerance); g_failed++; }
    else               { printf("ok   %s = %d (want %d)\n", what, got, want); }
}

int main() {
    using namespace menu::color_math;

    // A round trip must land back where it started. One unit of rounding is
    // acceptable; anything more and repeated format switches walk the colour.
    const int samples[][3] = {
        { 0, 149, 255 }, { 255, 0, 0 }, { 0, 255, 0 }, { 0, 0, 255 },
        { 255, 255, 255 }, { 0, 0, 0 }, { 128, 128, 128 }, { 34, 139, 34 },
        { 255, 214, 98 }, { 52, 49, 72 },
    };

    for (int i = 0; i < (int)(sizeof(samples) / sizeof(samples[0])); i++) {
        hsv h = rgb_to_hsv(samples[i][0], samples[i][1], samples[i][2]);
        int r = 0, g = 0, b = 0;
        hsv_to_rgb(h.h, h.s, h.v, &r, &g, &b);
        check_near("round trip r", r, samples[i][0], 1);
        check_near("round trip g", g, samples[i][1], 1);
        check_near("round trip b", b, samples[i][2], 1);
    }

    // Grey has no meaningful hue; saturation must be zero rather than garbage.
    hsv grey = rgb_to_hsv(128, 128, 128);
    check_near("grey saturation", (int)(grey.s * 100.f), 0, 0);

    // Out-of-range input is clamped, not wrapped: config.json is hand-editable.
    int r = 0, g = 0, b = 0;
    hsv_to_rgb(720.f, 5.f, 5.f, &r, &g, &b);
    check_near("clamped r", r > 255 ? 999 : r, r, 0);

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed ? 1 : 0;
}
```

- [ ] **Step 2: Run it to make sure it fails**

Run: `clang++ -std=c++17 -I src tests/color_math_test.cpp -o build/color_math_test.exe`
Expected: FAIL — `'menu/base/util/color_math.h' file not found`.

- [ ] **Step 3: Extract the conversion**

Create `src/menu/base/util/color_math.h`, moving the bodies currently at
`src/menu/base/renderer.cpp:381` onward. It works on plain ints rather than
`color_rgba`, because `ui_vars.h` drags in the mini-STL and the host test cannot
have that. `<math.h>` is a C header and is fine.

```cpp
#pragma once
#include <math.h>

// RGB/HSV conversion, kept free of ui_vars and the mini-STL so the host test can
// include it. menu::renderer wraps these back up in color_rgba/color_hsv.
namespace menu::color_math {

    struct hsv { float h, s, v; };

    inline hsv rgb_to_hsv(int ri, int gi, int bi) {
        float r = ri / 255.0f, g = gi / 255.0f, b = bi / 255.0f;
        float max = fmaxf(r, fmaxf(g, b));
        float min = fminf(r, fminf(g, b));

        hsv out;
        out.v = max;

        if (max == 0.0f)             { out.s = 0.f; out.h = 0.f; return out; }
        if (max - min == 0.0f)       { out.s = 0.f; out.h = 0.f; return out; }

        out.s = (max - min) / max;

        if (max == r)      out.h =       (g - b) / (max - min);
        else if (max == g) out.h = 2.f + (b - r) / (max - min);
        else               out.h = 4.f + (r - g) / (max - min);

        out.h *= 60.f;
        if (out.h < 0.f) out.h += 360.f;
        return out;
    }

    inline void hsv_to_rgb(float h, float s, float v, int* r, int* g, int* b) {
        // Clamp rather than wrap: config.json is hand-editable and a typo should
        // produce a dull colour, not a random one.
        if (h < 0.f) h = 0.f;   if (h > 360.f) h = 360.f;
        if (s < 0.f) s = 0.f;   if (s > 1.f)   s = 1.f;
        if (v < 0.f) v = 0.f;   if (v > 1.f)   v = 1.f;

        float rf = v, gf = v, bf = v;
        if (s > 0.f) {
            float hh = (h >= 360.f ? 0.f : h) / 60.f;
            int   i  = (int)hh;
            float f  = hh - (float)i;
            float p  = v * (1.f - s);
            float q  = v * (1.f - s * f);
            float t  = v * (1.f - s * (1.f - f));
            switch (i) {
                case 0: rf = v; gf = t; bf = p; break;
                case 1: rf = q; gf = v; bf = p; break;
                case 2: rf = p; gf = v; bf = t; break;
                case 3: rf = p; gf = q; bf = v; break;
                case 4: rf = t; gf = p; bf = v; break;
                default:rf = v; gf = p; bf = q; break;
            }
        }

        *r = (int)(rf * 255.f + 0.5f);
        *g = (int)(gf * 255.f + 0.5f);
        *b = (int)(bf * 255.f + 0.5f);
    }
}
```

Then make the renderer delegate. Replace the bodies of `renderer::rgb_to_hsv`
and `renderer::hsv_to_rgb` in `src/menu/base/renderer.cpp` with:

```cpp
    color_hsv renderer::rgb_to_hsv(color_rgba in) {
        menu::color_math::hsv h = menu::color_math::rgb_to_hsv(in.r, in.g, in.b);
        color_hsv out; out.h = h.h; out.s = h.s; out.v = h.v;
        return out;
    }

    color_rgba renderer::hsv_to_rgb(float h, float s, float v, int original_alpha) {
        int r = 0, g = 0, b = 0;
        menu::color_math::hsv_to_rgb(h, s, v, &r, &g, &b);
        return color_rgba(r, g, b, original_alpha);
    }
```

with `#include "menu/base/util/color_math.h"`. Compare the moved code against
the original line by line — the point is to relocate it, not to rewrite it.

- [ ] **Step 4: Run the test to verify it passes**

Run:
```bash
clang++ -std=c++17 -I src tests/color_math_test.cpp -o build/color_math_test.exe && ./build/color_math_test.exe
```
Expected: `all passed`, exit code 0. A round-trip failure here means the
extraction changed behaviour — diff against the original renderer code before
touching the test.

- [ ] **Step 5: Write the editor header**

Create `src/menu/base/submenus/helper_color.h`:

```cpp
#pragma once
#include "menu/base/submenu.h"

// A shared colour editor, not a feature. Any caller points it at a registry
// entry and opens it; the entry index rather than a bare color_rgba* so the
// editor can title itself and reach that colour's default for a revert.
class helper_color_menu : public menu::submenu::submenu {
public:
    void load() override;
    void update() override;
    void update_once() override;

    static void        target(int registry_index);
    static int         current_target();
    static color_rgba* target_color();

    static helper_color_menu* get();
};
```

- [ ] **Step 6: Write the editor**

Create `src/menu/base/submenus/helper_color.cpp`:

```cpp
#include "menu/base/submenus/helper_color.h"
#include "menu/base/submenu_handler.h"
#include "menu/base/options/number.h"
#include "menu/base/options/scroll.h"
#include "menu/base/options/break.h"
#include "menu/base/renderer.h"
#include "menu/base/util/theme.h"

namespace {
    int  g_target = 0;      // index into the colour registry
    int  g_format = 0;      // 0 = RGBA, 1 = HSVA
    int  g_built_format = -1;
    color_hsv g_hsv;

    scroll_struct<int> g_formats[] = {
        { localization("RGBA"), 0 },
        { localization("HSVA"), 0 },
    };

    color_rgba* current() { return menu::theme::color_ptr(g_target); }

    void preview() { menu::renderer::render_color_preview(*current()); }

    // HSVA edits go through a scratch color_hsv, so push the result back after
    // every change.
    void from_hsv() {
        *current() = menu::renderer::hsv_to_rgb(g_hsv.h, g_hsv.s / 100.f,
                                                g_hsv.v / 100.f, current()->a);
    }
}

void helper_color_menu::target(int registry_index) {
    if (registry_index < 0 || registry_index >= menu::theme::color_count()) return;
    g_target = registry_index;
    g_built_format = -1;   // force a rebuild: the title and bindings changed
}

int         helper_color_menu::current_target() { return g_target; }
color_rgba* helper_color_menu::target_color()   { return current(); }

void helper_color_menu::load() {
    set_name("Color");

    add_option(scroll_option<int>(SCROLL, "Color Format")
        .add_scroll(g_format, 0, 2, g_formats)
        .add_tooltip("Edit as red/green/blue or hue/saturation/value")
        .add_hover([] { preview(); }));

    add_option(break_option("Channels").ref());
}

void helper_color_menu::update() {
    // Dirty-flag rebuild: only when the format or the target actually changed.
    // Rebuilding per frame would allocate an option set every frame.
    if (g_built_format != g_format) update_once();
}

void helper_color_menu::update_once() {
    g_built_format = g_format;
    set_name(menu::theme::color_name(g_target), false, false);
    clear_options(2);

    if (g_format == 0) {
        add_option(number_option<int>(SCROLL, "Red")
            .add_number(current()->r, "%i", 1).add_min(0).add_max(255).can_loop()
            .add_hover([] { preview(); }));
        add_option(number_option<int>(SCROLL, "Green")
            .add_number(current()->g, "%i", 1).add_min(0).add_max(255).can_loop()
            .add_hover([] { preview(); }));
        add_option(number_option<int>(SCROLL, "Blue")
            .add_number(current()->b, "%i", 1).add_min(0).add_max(255).can_loop()
            .add_hover([] { preview(); }));
        add_option(number_option<int>(SCROLL, "Alpha")
            .add_number(current()->a, "%i", 1).add_min(0).add_max(255).can_loop()
            .add_hover([] { preview(); }));
        return;
    }

    g_hsv = menu::renderer::rgb_to_hsv(*current());
    g_hsv.s *= 100.f;
    g_hsv.v *= 100.f;

    add_option(number_option<float>(SCROLL, "Hue")
        .add_number(g_hsv.h, "%.2f", 1.f).add_min(0.f).add_max(360.f).can_loop()
        .add_click([] { from_hsv(); }).add_hover([] { preview(); }));
    add_option(number_option<float>(SCROLL, "Saturation")
        .add_number(g_hsv.s, "%.2f", 1.f).add_min(0.f).add_max(100.f)
        .add_click([] { from_hsv(); }).add_hover([] { preview(); }));
    add_option(number_option<float>(SCROLL, "Value")
        .add_number(g_hsv.v, "%.2f", 1.f).add_min(0.f).add_max(100.f)
        .add_click([] { from_hsv(); }).add_hover([] { preview(); }));
    add_option(number_option<int>(SCROLL, "Alpha")
        .add_number(current()->a, "%i", 1).add_min(0).add_max(255).can_loop()
        .add_hover([] { preview(); }));
}

helper_color_menu* helper_color_menu::get() {
    static helper_color_menu instance;
    return &instance;
}
```

- [ ] **Step 7: Register it**

In `src/menu/menu.cpp` `build()`, alongside the other registrations:

```cpp
        helper_color_menu::get()->load();
        menu::submenu::handler::add_submenu(helper_color_menu::get());
```

with `#include "menu/base/submenus/helper_color.h"`.

- [ ] **Step 8: Build**

Run the WSL build command.
Expected: compiles. Confirm `scroll_struct`'s field names against `src/menu/base/options/scroll.h:15` before assuming the `{ localization(...), 0 }` initialiser shape.

- [ ] **Step 9: Verify on console**

Deploy (bump `INSULIN_BUILD_TAG` first), then: Settings → Themes → pick `Option` → change Red → the option text colour changes as you scroll. Switch the format to HSVA and back; the two views agree on the same colour.

- [ ] **Step 10: Commit**

```bash
git add src/menu/base/util/color_math.h tests/color_math_test.cpp src/menu/base/renderer.cpp src/menu/base/submenus/helper_color.h src/menu/base/submenus/helper_color.cpp src/menu/menu.cpp
git commit -m "feat(ui): shared colour editor, RGBA and HSVA

Not a feature but an editor other submenus aim: the caller sets a registry
index and opens it. An index rather than a color_rgba* so the editor can title
itself with the colour's name and reach that colour's default.

Format switching rebuilds through the dirty-flag pattern, never per frame."
```

---

### Task 7: Colour helper — presets, sync and the rainbow toggle

**Files:**
- Create: `src/menu/base/submenus/helper_color_presets.h`, `.cpp`
- Create: `src/menu/base/submenus/helper_color_sync.h`, `.cpp`
- Modify: `src/menu/base/submenus/helper_color.cpp` (three options above the break)
- Modify: `src/menu/menu.cpp` (register both children)

**Interfaces:**
- Consumes: Task 2 accessors; Task 5 `menu::get_rainbow()`; Task 6 `helper_color_menu::target_color()`
- Produces: `helper_color_presets_menu`, `helper_color_sync_menu`, each with `static <T>* get()`

- [ ] **Step 1: Write the presets submenu**

`src/menu/base/submenus/helper_color_presets.h`:

```cpp
#pragma once
#include "menu/base/submenu.h"

class helper_color_presets_menu : public menu::submenu::submenu {
public:
    void load() override;
    static helper_color_presets_menu* get();
};
```

`src/menu/base/submenus/helper_color_presets.cpp`:

```cpp
#include "menu/base/submenus/helper_color_presets.h"
#include "menu/base/submenus/helper_color.h"
#include "menu/base/submenu_handler.h"
#include "menu/base/options/button.h"
#include "menu/base/renderer.h"

namespace {
    struct preset { const char* m_name; color_rgba m_color; };

    // Constant-initialised: color_rgba's constructors are constexpr, which is
    // what lets this sit at namespace scope without a global constructor.
    const preset g_presets[] = {
        { "Ozark Blue",     color_rgba(0x00, 0x95, 0xFF, 255) },
        { "Emerald",        color_rgba(0x00, 0x9B, 0x77, 200) },
        { "Tangerine",      color_rgba(0xDD, 0x41, 0x24, 200) },
        { "Honeysuckle",    color_rgba(0xD6, 0x50, 0x76, 200) },
        { "Turquoise",      color_rgba(0x44, 0xB8, 0xAC, 200) },
        { "Mimosa",         color_rgba(0xEF, 0xC0, 0x50, 200) },
        { "Chili Pepper",   color_rgba(0x9B, 0x23, 0x35, 200) },
        { "True Red",       color_rgba(0xBC, 0x24, 0x3C, 200) },
        { "Cerulean",       color_rgba(0x98, 0xB4, 0xD4, 200) },
        { "Galaxy Blue",    color_rgba(0x2A, 0x4B, 0x7C, 200) },
        { "Orange Tiger",   color_rgba(0xF9, 0x67, 0x14, 200) },
        { "Pink Peacock",   color_rgba(0xC6, 0x21, 0x68, 200) },
        { "Aspen Gold",     color_rgba(0xFF, 0xD6, 0x62, 200) },
        { "Eclipse",        color_rgba(0x34, 0x31, 0x48, 200) },
        { "Nebulas Blue",   color_rgba(0x3F, 0x69, 0xAA, 200) },
        { "Quetzal Green",  color_rgba(0x00, 0x6E, 0x6D, 200) },
        { "Baby Blue",      color_rgba(0x6F, 0x9F, 0xD8, 200) },
        { "Ultra Violet",   color_rgba(0x6B, 0x5B, 0x95, 200) },
        { "Lime Punch",     color_rgba(0xBF, 0xD6, 0x41, 200) },
        { "Harbor Mist",    color_rgba(0xB4, 0xB7, 0xBA, 200) },
    };
    const int g_preset_count = (int)(sizeof(g_presets) / sizeof(g_presets[0]));
}

void helper_color_presets_menu::load() {
    set_name("Presets");

    for (int i = 0; i < g_preset_count; i++) {
        add_option(button_option(g_presets[i].m_name)
            .add_click([i] {
                color_rgba* t = helper_color_menu::target_color();
                if (t) { int a = t->a; *t = g_presets[i].m_color; t->a = a; }
                menu::submenu::handler::set_submenu_previous(false);
            })
            .add_hover([i] { menu::renderer::render_color_preview(g_presets[i].m_color); }));
    }
}

helper_color_presets_menu* helper_color_presets_menu::get() {
    static helper_color_presets_menu instance;
    return &instance;
}
```

Alpha is carried over rather than taken from the preset: a preset is a hue choice, and adopting its opacity would silently make a background see-through.

- [ ] **Step 2: Write the sync submenu**

`src/menu/base/submenus/helper_color_sync.h`:

```cpp
#pragma once
#include "menu/base/submenu.h"

class helper_color_sync_menu : public menu::submenu::submenu {
public:
    void load() override;
    static helper_color_sync_menu* get();
};
```

`src/menu/base/submenus/helper_color_sync.cpp`:

```cpp
#include "menu/base/submenus/helper_color_sync.h"
#include "menu/base/submenus/helper_color.h"
#include "menu/base/submenu_handler.h"
#include "menu/base/options/button.h"
#include "menu/base/renderer.h"
#include "menu/base/util/theme.h"

void helper_color_sync_menu::load() {
    set_name("Sync With...");

    // One button per registry entry - this is the surface that could not exist
    // before the registry, because nothing else could enumerate the colours.
    for (int i = 0; i < menu::theme::color_count(); i++) {
        add_option(button_option(menu::theme::color_name(i))
            .add_click([i] {
                color_rgba* t = helper_color_menu::target_color();
                color_rgba* s = menu::theme::color_ptr(i);
                if (t && s && t != s) *t = *s;
                menu::submenu::handler::set_submenu_previous(false);
            })
            .add_hover([i] {
                menu::renderer::render_color_preview(*menu::theme::color_ptr(i));
            }));
    }
}

helper_color_sync_menu* helper_color_sync_menu::get() {
    static helper_color_sync_menu instance;
    return &instance;
}
```

- [ ] **Step 3: Add the three options to the editor**

In `helper_color_menu::load()`, above the existing `Color Format` option, and add the matching includes (`helper_color_presets.h`, `helper_color_sync.h`, `menu/base/util/rainbow.h`, `menu/base/options/submenu_option.h`, `menu/base/options/toggle.h`):

```cpp
    add_option(submenu_option("Presets").add_submenu<helper_color_presets_menu>());
    add_option(submenu_option("Sync With...").add_submenu<helper_color_sync_menu>());

    add_option(toggle_option("Rainbow")
        .add_tooltip("Cycle this colour through the hue wheel")
        .add_click([] {
            color_rgba* t = current();
            menu::rainbow* rb = menu::get_rainbow();
            if (rb->contains(t)) rb->remove(t);
            else                 { rb->add(t); rb->m_enabled = true; }
        }));
```

Then change `clear_options(2)` in `update_once()` to `clear_options(5)` — three new options plus the format scroller plus the break.

- [ ] **Step 4: Stop the rainbow before a theme reset**

In `settings_themes.cpp`, as the first line of the `Reset to Default` click handler, with `#include "menu/base/util/rainbow.h"`:

```cpp
            // Before restoring: the rainbow holds pre-rainbow snapshots of the
            // colours it animates. Resetting underneath it would leave those
            // snapshots stale, and stopping later would undo the reset.
            menu::get_rainbow()->stop();
```

- [ ] **Step 5: Give the theme submenu its colour list**

Now that the editor exists, `settings_themes_menu::load()` gains the per-colour
options that open it. Append to `load()`, after the `break_option("Colours")`,
with `#include "menu/base/submenus/helper_color.h"`:

```cpp
    // Index only: stl::function caps captures at 64 bytes, and a color_entry
    // would not fit.
    for (int i = 0; i < menu::theme::color_count(); i++) {
        add_option(submenu_option(menu::theme::color_name(i))
            .add_submenu<helper_color_menu>()
            .add_click([i] { helper_color_menu::target(i); })
            .add_hover([i] {
                menu::renderer::render_color_preview(*menu::theme::color_ptr(i));
            }));
    }
```

- [ ] **Step 6: Register both children**

In `src/menu/menu.cpp` `build()`:

```cpp
        helper_color_presets_menu::get()->load();
        menu::submenu::handler::add_submenu(helper_color_presets_menu::get());
        helper_color_sync_menu::get()->load();
        menu::submenu::handler::add_submenu(helper_color_sync_menu::get());
```

- [ ] **Step 7: Build and verify on console**

Build, bump the tag, deploy. Then: a preset applies and keeps the original alpha; Sync copies one colour onto another; the Rainbow toggle cycles that colour and untoggling restores exactly the previous value; `Reset to Default` while a rainbow runs leaves every colour at its shipped value.

- [ ] **Step 8: Commit**

```bash
git add src/menu/base/submenus/helper_color_presets.* src/menu/base/submenus/helper_color_sync.* src/menu/base/submenus/helper_color.cpp src/menu/base/submenus/settings_themes.cpp src/menu/menu.cpp
git commit -m "feat(ui): colour presets, sync between colours, per-colour rainbow

Sync is the surface the registry existed for - nothing else in the codebase
could enumerate the theme colours.

Presets carry the target's existing alpha rather than the preset's: a preset is
a hue choice, and adopting its opacity would quietly make a background
see-through. Reset stops the rainbow first, because the rainbow holds
pre-animation snapshots that a reset underneath it would strand."
```

---

### Task 8: Panel scaffolding — the gate, the build tag, number formatting

Nothing renders yet after this task. It puts in place the three things all four panels need, and closes the gap that would otherwise reintroduce the boot crash.

**Files:**
- Create: `src/platform/build_tag.h`
- Create: `src/util/num_to_string.h`
- Create: `src/menu/panels/builtin_panels.h`, `.cpp`
- Modify: `src/InsulinGTAV.cpp` (use the tag macro)
- Modify: `src/menu/menu.cpp` (gate `panels::update()`, swap the demo registration)

**Interfaces:**
- Consumes: `menu::panels` framework
- Produces: `INSULIN_BUILD_TAG`; `util::itos(int, char*, int)` and `util::ftos(float, int, char*, int)`; `void menu::panels::register_builtin_panels()`

- [ ] **Step 1: Lift the build tag into a header**

Create `src/platform/build_tag.h`:

```cpp
#pragma once

// The manual build marker. module_start logs it and the debug panel shows it;
// they must agree, because this string is the project's only reliable proof of
// which .prx the console is actually running. Bump it on every deploy.
#define INSULIN_BUILD_TAG "panels-1"
```

In `src/InsulinGTAV.cpp`, add the include and replace both literals:

```cpp
    platform::klogf("BUILD=" INSULIN_BUILD_TAG " module_start base=0x%llx",
                    (unsigned long long)rage::invoker::g_eboot_base);
    platform::logf("Boot", "BUILD=" INSULIN_BUILD_TAG " " __DATE__ " " __TIME__ " base=0x%llx",
                   (unsigned long long)rage::invoker::g_eboot_base);
```

- [ ] **Step 2: Add the number formatters**

There is no `stl::to_string`, and four panels each writing their own would be four copies. Create `src/util/num_to_string.h`:

```cpp
#pragma once
#include <stdio.h>

// The mini-STL has no to_string. Panels format numbers every frame, so these
// take a caller-owned buffer rather than returning an stl::string (a 128-byte
// copy per call).
namespace util {
    inline const char* itos(int value, char* buf, int len) {
        snprintf(buf, len, "%d", value);
        return buf;
    }

    inline const char* ftos(float value, int decimals, char* buf, int len) {
        switch (decimals) {
            case 0:  snprintf(buf, len, "%.0f", value); break;
            case 1:  snprintf(buf, len, "%.1f", value); break;
            case 3:  snprintf(buf, len, "%.3f", value); break;
            default: snprintf(buf, len, "%.2f", value); break;
        }
        return buf;
    }
}
```

- [ ] **Step 3: Write the registration shell**

Create `src/menu/panels/builtin_panels.h`:

```cpp
#pragma once

namespace menu::panels {
    // Registers every built-in panel. Called once from menu::build() in place of
    // the demo panel.
    void register_builtin_panels();
}
```

Create `src/menu/panels/builtin_panels.cpp`:

```cpp
#include "menu/panels/builtin_panels.h"
#include "menu/base/util/panels.h"

namespace menu::panels {
    // Panels arrive in Tasks 9 and 10; this file owns their registration so
    // menu.cpp - already 300 lines of it - does not absorb four more render
    // callbacks.
    void register_builtin_panels() {
        panel_parent* parent = new panel_parent();
        parent->m_render = true;
        parent->m_id   = "insulin";
        parent->m_name = "Insulin";
        get_panels().push_back(parent);
    }
}
```

- [ ] **Step 4: Gate the panel update and swap the registration**

In `src/menu/menu.cpp`: delete `demo_panel_update` and `register_demo_panel`, replace the `register_demo_panel();` call in `build()` with `menu::panels::register_builtin_panels();` (include `menu/panels/builtin_panels.h`), and change the tick callsite:

```cpp
        TICK_TRACE("panels");
        // Gated like feature_update: panel callbacks call natives, and before the
        // local player exists those dereference a player that is not there. The
        // gate lives here rather than in each callback so it also covers every
        // panel written from now on.
        if (game::player_valid())
            menu::panels::update();
```

- [ ] **Step 5: Build and verify on console**

Build, deploy, boot with the kernel log attached (`nc 10.10.10.236 3232`).
Expected: `BUILD=panels-1`, `boot: build done, 75 submenus`, no fault. No panels render — correct, there are none yet.

- [ ] **Step 6: Commit**

```bash
git add src/platform/build_tag.h src/util/num_to_string.h src/menu/panels/ src/InsulinGTAV.cpp src/menu/menu.cpp
git commit -m "feat(panels): gate the panel sweep, and scaffolding for real panels

menu::panels::update() ran outside the player_valid gate. That was harmless
only because the one registered panel was a demo that calls no natives; the
four real ones do, and ungated they would be the loading-screen crash coming
back through another door. Gated at the callsite, not in the callbacks, so it
covers every panel written from now on.

The build tag moves into a header because the debug panel must show the same
string module_start logs - the tag is our only proof of which .prx is running,
and a panel disagreeing with the klog would poison it."
```

---

### Task 9: Player and vehicle panels

**Files:**
- Create: `src/menu/panels/player_panel.h`, `.cpp`
- Create: `src/menu/panels/vehicle_panel.h`, `.cpp`
- Modify: `src/menu/panels/builtin_panels.cpp`

**Interfaces:**
- Consumes: Task 8 `util::itos`, `util::ftos`, `register_builtin_panels`
- Produces: `math::vector2<float> menu::panels::player_panel_update(panel_child&)` and `vehicle_panel_update(panel_child&)` — plain function pointers, matching `panel_child::m_update`

- [ ] **Step 1: Write the player panel**

`src/menu/panels/player_panel.h`:

```cpp
#pragma once
#include "menu/base/util/panels.h"

namespace menu::panels {
    math::vector2<float> player_panel_update(panel_child& child);
}
```

`src/menu/panels/player_panel.cpp`:

```cpp
#include "menu/panels/player_panel.h"
#include "util/num_to_string.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"

namespace menu::panels {
    // m_update is a plain function pointer, so there is no capture to worry
    // about here - unlike every option handler in this codebase.
    math::vector2<float> player_panel_update(panel_child& child) {
        panel p(child, global::ui::g_panel_bar);

        Ped ped = native::get_player_ped(-1);
        if (!ped) return p.get_render_scale();   // framework draws nothing

        char buf[64];
        math::vector3<float> c = native::get_entity_coords(ped, true);

        p.item("X",       util::ftos(c.x, 2, buf, sizeof(buf)));
        p.item("Y",       util::ftos(c.y, 2, buf, sizeof(buf)));
        p.item("Z",       util::ftos(c.z, 2, buf, sizeof(buf)));
        p.item("Heading", util::ftos(native::get_entity_heading(ped), 1, buf, sizeof(buf)));
        p.item("Health",  util::itos(native::get_entity_health(ped), buf, sizeof(buf)));
        p.item("Armour",  util::itos(native::get_ped_armour(ped), buf, sizeof(buf)));
        p.item("Wanted",  util::itos(native::get_player_wanted_level(native::player_id()), buf, sizeof(buf)));
        p.item_full("Zone", native::get_name_of_zone(c.x, c.y, c.z));

        return p.get_render_scale();
    }
}
```

- [ ] **Step 2: Write the vehicle panel**

`src/menu/panels/vehicle_panel.h`:

```cpp
#pragma once
#include "menu/base/util/panels.h"

namespace menu::panels {
    math::vector2<float> vehicle_panel_update(panel_child& child);
}
```

`src/menu/panels/vehicle_panel.cpp`:

```cpp
#include "menu/panels/vehicle_panel.h"
#include "util/num_to_string.h"
#include "rage/invoker/natives.h"

namespace menu::panels {
    math::vector2<float> vehicle_panel_update(panel_child& child) {
        panel p(child, global::ui::g_panel_bar);

        Ped ped = native::get_player_ped(-1);
        if (!ped || !native::is_ped_in_any_vehicle(ped, false))
            return p.get_render_scale();

        Vehicle veh = native::get_vehicle_ped_is_in(ped, false);
        if (!veh) return p.get_render_scale();

        char buf[64];

        // The raw text label, e.g. ADDER: GET_LABEL_TEXT is in no header on this
        // build, so there is nothing to resolve it to a friendly name with.
        p.item_full("Model", native::get_display_name_from_vehicle_model(native::get_entity_model(veh)));
        p.item("Speed",  util::ftos(native::get_entity_speed(veh) * 3.6f, 1, buf, sizeof(buf)));
        p.item("Engine", util::ftos(native::get_vehicle_engine_health(veh), 0, buf, sizeof(buf)));
        p.item("Body",   util::ftos(native::get_vehicle_body_health(veh), 0, buf, sizeof(buf)));
        p.item("Handle", util::itos((int)veh, buf, sizeof(buf)));

        return p.get_render_scale();
    }
}
```

Speed is metres per second from the native, so `* 3.6f` gives km/h.

- [ ] **Step 3: Register both**

In `src/menu/panels/builtin_panels.cpp`, include both headers and append inside `register_builtin_panels()` after the parent is created and before `get_panels().push_back(parent)`:

```cpp
        panel_child player{};
        player.m_parent = parent;
        player.m_render = true;
        player.m_id     = "player";
        player.m_name   = "Player";
        player.m_double_sided = true;
        player.m_panel_option_count_left = 8;
        player.m_update = player_panel_update;
        parent->m_children_panels.push_back(player);

        panel_child vehicle{};
        vehicle.m_parent = parent;
        vehicle.m_render = true;
        vehicle.m_id     = "vehicle";
        vehicle.m_name   = "Vehicle";
        vehicle.m_double_sided = true;
        vehicle.m_panel_option_count_left = 5;
        vehicle.m_column = 0;
        vehicle.m_index  = 1;
        vehicle.m_update = vehicle_panel_update;
        parent->m_children_panels.push_back(vehicle);
```

- [ ] **Step 4: Build and verify on console**

Bump `INSULIN_BUILD_TAG` to `panels-2`, build, deploy.
Expected: on foot, the player panel shows live coordinates that change as you walk, and the vehicle panel is absent. Get in a car: the vehicle panel appears with a model label and a speed that tracks the speedometer. Restart the game and watch the loading screen with the klog attached — no fault, and the panels appear only once you are in control.

- [ ] **Step 5: Commit**

```bash
git add src/menu/panels/player_panel.* src/menu/panels/vehicle_panel.* src/menu/panels/builtin_panels.cpp src/platform/build_tag.h
git commit -m "feat(panels): live player and vehicle panels

Both bail out early with a zero height when there is nothing to show - no
player, or not in a vehicle - which the framework already treats as draw
nothing.

The vehicle panel shows the raw text label (ADDER, not Adder): GET_LABEL_TEXT
is in none of the three native headers on this build. Current gear is missing
for the same kind of reason - it lives in a CVehicle field and would need a
reverse-engineering anchor this sub-project promised not to need."
```

---

### Task 10: World and debug panels

**Files:**
- Create: `src/menu/panels/world_panel.h`, `.cpp`
- Create: `src/menu/panels/debug_panel.h`, `.cpp`
- Modify: `src/menu/panels/builtin_panels.cpp`

**Interfaces:**
- Consumes: Task 8 helpers and `INSULIN_BUILD_TAG`; `rage::hash_natives::usable()`, `entry_count()`; `rage::invoker::g_eboot_base`
- Produces: `world_panel_update(panel_child&)`, `debug_panel_update(panel_child&)`

- [ ] **Step 1: Write the world panel**

`src/menu/panels/world_panel.h`:

```cpp
#pragma once
#include "menu/base/util/panels.h"

namespace menu::panels {
    math::vector2<float> world_panel_update(panel_child& child);
}
```

`src/menu/panels/world_panel.cpp`:

```cpp
#include "menu/panels/world_panel.h"
#include "util/num_to_string.h"
#include "rage/invoker/natives.h"

namespace menu::panels {
    math::vector2<float> world_panel_update(panel_child& child) {
        panel p(child, global::ui::g_panel_bar);

        char buf[64];
        char clock[16];
        snprintf(clock, sizeof(clock), "%02d:%02d",
                 native::get_clock_hours(), native::get_clock_minutes());

        p.item("Time",    clock);
        p.item("Weather", util::itos((int)native::get_prev_weather_type_hash_name(), buf, sizeof(buf)));

        return p.get_render_scale();
    }
}
```

Weather is shown as its hash: the game returns a hash, and resolving it to a
name would need a weather-name table this sub-project does not have. A follow-up
can add one; a wrong name would be worse than an honest number.

- [ ] **Step 2: Write the debug panel**

`src/menu/panels/debug_panel.h`:

```cpp
#pragma once
#include "menu/base/util/panels.h"

namespace menu::panels {
    math::vector2<float> debug_panel_update(panel_child& child);
}
```

`src/menu/panels/debug_panel.cpp`:

```cpp
#include "menu/panels/debug_panel.h"
#include "util/num_to_string.h"
#include "platform/build_tag.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/invoker.h"
#include "rage/invoker/hash_natives.h"

namespace menu::panels {
    namespace {
        // Smoothed so the number is readable rather than a blur. m_custom_ptr is
        // the framework's per-panel storage, but this is a single float shared by
        // the one debug panel, so a file static is honest and simpler.
        float g_fps = 0.f;
    }

    math::vector2<float> debug_panel_update(panel_child& child) {
        panel p(child, global::ui::g_panel_bar);

        float dt = native::get_frame_time();
        if (dt > 0.f) {
            float instant = 1.f / dt;
            g_fps = (g_fps == 0.f) ? instant : (g_fps * 0.9f + instant * 0.1f);
        }

        char buf[64];
        char base[32];
        snprintf(base, sizeof(base), "0x%llx",
                 (unsigned long long)rage::invoker::g_eboot_base);

        p.item_full("Build", INSULIN_BUILD_TAG);
        p.item("Base",    base);
        p.item("FPS",     util::ftos(g_fps, 0, buf, sizeof(buf)));
        p.item("Natives", util::itos((int)rage::hash_natives::entry_count(), buf, sizeof(buf)));
        p.item("Table",   rage::hash_natives::usable() ? "usable" : "pending");

        return p.get_render_scale();
    }
}
```

- [ ] **Step 3: Register both**

In `src/menu/panels/builtin_panels.cpp`, add `#include "menu/panels/world_panel.h"` and `#include "menu/panels/debug_panel.h"`, then append inside `register_builtin_panels()` before `get_panels().push_back(parent)`:

```cpp
        panel_child world{};
        world.m_parent = parent;
        world.m_render = true;
        world.m_id     = "world";
        world.m_name   = "World";
        world.m_double_sided = true;
        world.m_panel_option_count_left = 2;
        world.m_column = 1;
        world.m_index  = 0;
        world.m_update = world_panel_update;
        parent->m_children_panels.push_back(world);

        panel_child debug{};
        debug.m_parent = parent;
        debug.m_render = true;
        debug.m_id     = "debug";
        debug.m_name   = "Debug";
        debug.m_double_sided = true;
        debug.m_panel_option_count_left = 5;
        debug.m_column = 1;
        debug.m_index  = 1;
        debug.m_update = debug_panel_update;
        parent->m_children_panels.push_back(debug);
```

Column 1 puts both beside the first column rather than on top of the player and
vehicle panels.

- [ ] **Step 4: Build and verify on console**

Bump the tag to `panels-3`, build, deploy.
Expected: the world panel tracks the in-game clock; the debug panel shows the tag matching the klog's `BUILD=` line, an FPS that settles rather than jitters, and `Natives 6691 / Table usable` a few seconds after the game is up.

- [ ] **Step 5: Commit**

```bash
git add src/menu/panels/world_panel.* src/menu/panels/debug_panel.* src/menu/panels/builtin_panels.cpp src/platform/build_tag.h
git commit -m "feat(panels): world and debug panels

The debug panel answers on screen what we have been reading the kernel log for:
which build is running, where the eboot is based, whether the native table came
back, and how many entries it holds. It takes the tag from the shared header, so
it cannot disagree with the line module_start logs.

Weather is shown as its hash rather than a name - the game returns a hash and we
have no weather-name table; an invented name would be worse than an honest
number. FPS is exponentially smoothed, since the raw frame time is unreadable."
```

---

### Task 11: Panels configuration submenu

**Files:**
- Create: `src/menu/base/submenus/misc_panels.h`, `.cpp`
- Modify: `src/menu/base/submenus/misc.cpp` (add the entry)
- Modify: `src/menu/menu.cpp` (register)

**Interfaces:**
- Consumes: `menu::panels::get_panels()`, `menu::panels::rearrange`; Task 8-10 panels
- Produces: `class misc_panels_menu` with `static misc_panels_menu* get()`

- [ ] **Step 1: Write the submenu**

`src/menu/base/submenus/misc_panels.h`:

```cpp
#pragma once
#include "menu/base/submenu.h"

class misc_panels_menu : public menu::submenu::submenu {
public:
    void load() override;
    void update() override;
    void update_once() override;
    static misc_panels_menu* get();
};
```

`src/menu/base/submenus/misc_panels.cpp`:

```cpp
#include "menu/base/submenus/misc_panels.h"
#include "menu/base/submenus/misc.h"
#include "menu/base/submenu_handler.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/number.h"
#include "menu/base/options/break.h"
#include "menu/base/util/panels.h"
#include "util/config.h"

namespace {
    int g_built = -1;   // how many children the option list was built for

    int child_count() {
        int n = 0;
        for (menu::panels::panel_parent* parent : menu::panels::get_panels())
            n += (int)parent->m_children_panels.size();
        return n;
    }
}

void misc_panels_menu::load() {
    set_name("Panels");
    set_parent<misc_menu>();

    // Restore each panel's saved placement. Pure config and memory, no natives,
    // so this is safe during build().
    for (menu::panels::panel_parent* parent : menu::panels::get_panels()) {
        for (menu::panels::panel_child& child : parent->m_children_panels) {
            int column = util::config::read_int(get_submenu_name_stack(), "Column",
                                                child.m_column, { parent->m_name, child.m_name });
            int index  = util::config::read_int(get_submenu_name_stack(), "Index",
                                                child.m_index, { parent->m_name, child.m_name });
            child.m_render = util::config::read_bool(get_submenu_name_stack(), "Render",
                                                     child.m_render, { parent->m_name, child.m_name });
            menu::panels::rearrange(parent, child.m_id, column, index);
        }
    }
}

void misc_panels_menu::update() {
    // The panel list is data, so rebuild only when it actually changes.
    if (g_built != child_count()) update_once();
}

void misc_panels_menu::update_once() {
    g_built = child_count();
    clear_options(0);

    for (menu::panels::panel_parent* parent : menu::panels::get_panels()) {
        add_option(break_option(parent->m_name).ref());

        for (int i = 0; i < (int)parent->m_children_panels.size(); i++) {
            menu::panels::panel_child& child = parent->m_children_panels[i];

            add_option(toggle_option(child.m_name)
                .add_toggle(child.m_render)
                .add_tooltip("Show this panel")
                .add_savable(get_submenu_name_stack()));

            // Capture the pointer and the index only - 64-byte cap.
            menu::panels::panel_parent* pp = parent;
            add_option(number_option<int>(SCROLLSELECT, "  Column")
                .add_number(child.m_column, "%i", 1).add_min(0).add_max(1)
                .add_update([pp, i](number_option<int>*, int) {
                    menu::panels::panel_child& c = pp->m_children_panels[i];
                    menu::panels::rearrange(pp, c.m_id, c.m_column, c.m_index);
                }));

            add_option(number_option<int>(SCROLLSELECT, "  Order")
                .add_number(child.m_index, "%i", 1).add_min(0).add_max(8)
                .add_update([pp, i](number_option<int>*, int) {
                    menu::panels::panel_child& c = pp->m_children_panels[i];
                    menu::panels::rearrange(pp, c.m_id, c.m_column, c.m_index);
                }));
        }
    }
}

misc_panels_menu* misc_panels_menu::get() {
    static misc_panels_menu instance;
    return &instance;
}
```

Column and order persist through the save button below, not per keystroke, to
avoid a config write on every scroll tick.

- [ ] **Step 2: Add a save button**

At the end of `update_once()`, after the loops:

```cpp
    add_option(button_option("Save Layout")
        .add_tooltip("Persist column, order and visibility for every panel")
        .add_click([] {
            util::config::begin_batch();
            for (menu::panels::panel_parent* parent : menu::panels::get_panels())
                for (menu::panels::panel_child& child : parent->m_children_panels) {
                    util::config::write_int(misc_panels_menu::get()->get_submenu_name_stack(),
                                            "Column", child.m_column, { parent->m_name, child.m_name });
                    util::config::write_int(misc_panels_menu::get()->get_submenu_name_stack(),
                                            "Index", child.m_index, { parent->m_name, child.m_name });
                    util::config::write_bool(misc_panels_menu::get()->get_submenu_name_stack(),
                                             "Render", child.m_render, { parent->m_name, child.m_name });
                }
            util::config::end_batch();
            menu::notify::stacked("Panels", "Layout saved");
        }));
```

with `#include "menu/base/options/button.h"` and `#include "menu/base/util/notify.h"`.

- [ ] **Step 3: Hang it under Miscellaneous and register it**

In `src/menu/base/submenus/misc.cpp`, add to `load()`:

```cpp
    add_option(submenu_option("Panels").add_submenu<misc_panels_menu>());
```

In `src/menu/menu.cpp` `build()`:

```cpp
        misc_panels_menu::get()->load();
        menu::submenu::handler::add_submenu(misc_panels_menu::get());
```

- [ ] **Step 4: Build and verify on console**

Bump the tag to `panels-4`, build, deploy.
Expected: Miscellaneous → Panels lists all four; toggling one hides it immediately; changing Column moves it to the second column; Save Layout persists, and a game restart brings the layout back.

- [ ] **Step 5: Commit**

```bash
git add src/menu/base/submenus/misc_panels.* src/menu/base/submenus/misc.cpp src/menu/menu.cpp
git commit -m "feat(panels): arrange and toggle panels from Miscellaneous

Column and order are applied live but written only on Save Layout - persisting
per scroll tick would be a config rewrite per frame of held input.

The option list rebuilds through the dirty-flag pattern against the panel count,
not every frame."
```

---

## Verification

After Task 11, with the kernel log attached:

- [ ] Five cold boots, no fault, `boot: build done` every time
- [ ] Change a colour, restart the game, the colour holds
- [ ] `Reset to Default` restores the shipped colours, including while a rainbow runs
- [ ] All four panels show live values on foot and in a vehicle
- [ ] Panels stay absent during the loading screen — the test the whole sub-project's risk sits on
- [ ] `clang++ -std=c++17 -I src tests/rainbow_math_test.cpp -o build/rainbow_math_test.exe && ./build/rainbow_math_test.exe` passes
