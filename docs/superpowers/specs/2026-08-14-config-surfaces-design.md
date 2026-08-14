# Config surfaces: colour registry, theme editing, rainbow, panels

Sub-project A of the remaining Ozark port. Delivers the surfaces that let a user
change how the menu looks and see what the game is doing, and fills two holes
found while scoping it: there is no way to edit any theme colour, and the panel
framework has been carrying a demo panel and nothing else.

Sub-projects B (swaps) and C (ESP) are out of scope here and get their own specs.
They each need a reverse-engineering anchor that does not exist yet; nothing in
this document does.

## What exists today

`global/ui_vars` defines 31 `color_rgba` globals that the whole renderer draws
from. None of them is reachable from the menu: `settings_themes.cpp` is a 31-line
scaffold whose "Save Theme" button emits a `Theme saved` notification and writes
nothing, and whose "Reset to Default" resets nothing.

`menu/base/util/panels.{h,cpp}` is a complete side-panel framework — columns,
ordering, height accumulation, `panel::item()` rendering. `menu.cpp` registers
exactly one panel against it, `register_demo_panel()`, and Ozark's own panels
(player info, friends, sessions) were deliberately not ported because they are
network features.

`menu::input::color()` gives a modal colour picker. There is no numeric editor,
no presets, no way to copy one colour to another.

## Scope

Five pieces, built and verified in this order. Each is usable on its own.

1. Colour registry — the name-to-pointer table everything else addresses colours through
2. Theme submenu — real save, load and reset over that registry
3. Colour helper — shared editor with presets and sync
4. Rainbow — animated colours
5. Panels — four real panels plus Ozark's arrange/toggle submenu

## Constraints that shape the design

These are the project's boot and platform rules, not preferences. Each one
below has already cost this project a crash or a day.

- **No `.init_array`.** Global constructors do not run. Namespace-scope data must
  be constant-initialised or live behind a function-local static.
- **`stl::function` captures cap at 64 bytes.** A per-colour option must capture
  an index, never an entry or a struct.
- **Nothing may call a native during `menu::build()`**, and per-frame work that
  calls natives must be gated on `game::player_valid()`.
- **Host tests take C headers only.** MSVC's C++ stdlib rejects the installed
  clang (`STL1000`), so anything under test must be includable without the C++
  standard library — the pattern `menu/base/util/frame_clock.h` already follows.

## 1. Colour registry

New: `src/global/color_registry.{h,cpp}`

```cpp
namespace global::ui {
    struct color_entry {
        const char*  m_name;      // display name AND config key
        color_rgba*  m_color;
    };

    extern const color_entry g_color_registry[];
    extern const int         g_color_registry_count;   // 31

    // Snapshot the compiled-in values so "Reset to Default" has something true
    // to restore. MUST run after global::ui::init() and BEFORE the config is
    // applied — see the ordering note below.
    void capture_color_defaults();
    const color_rgba& color_default(int index);
}
```

The table is constant-initialised: every element is a string literal plus the
address of a namespace-scope object, both address constants, so no global
constructor is involved.

Defaults are captured at runtime rather than repeated in the table. Repeating
them would duplicate every value already in `ui_vars.cpp` and the two copies
would drift the first time someone tunes a colour.

**The ordering requirement.** `menu::build()` must call, in this order:

```
global::ui::init();                     // colours hold their compiled-in values
global::ui::capture_color_defaults();   // snapshot those
util::config::load();                   // config becomes available
settings_themes_menu::apply_colors();   // user's colours overwrite the globals
```

`apply_colors()` is a static on the theme submenu rather than free-standing,
because it reads through the same name stack the submenu writes with — one place
owns the key layout.

If `capture_color_defaults()` runs after the config is applied, it snapshots the
user's colours as the factory defaults and "Reset to Default" silently becomes a
no-op that looks like it worked. This is the one genuinely fragile point in the
sub-project and belongs in a comment at the callsite.

## 2. Theme submenu

Rewrite: `src/menu/base/submenus/settings_themes.cpp`

One `submenu_option` per registry entry. Selecting it points the colour helper at
that entry and opens it:

```cpp
for (int i = 0; i < global::ui::g_color_registry_count; i++) {
    add_option(submenu_option(global::ui::g_color_registry[i].m_name)
        .add_click([i] { helper_color_menu::target(i); })     // index only: 64-byte cap
        .add_submenu<helper_color_menu>()
        .add_hover([i] { menu::renderer::render_color_preview(
                             *global::ui::g_color_registry[i].m_color); }));
}
```

Above them, the two buttons that currently lie:

- **Save Theme** — writes all 31 entries, then `util::config::save()`.
- **Reset to Default** — copies `color_default(i)` back into every entry and writes.

**Persistence format.** Four ints per colour, under the theme submenu's name
stack with additional stacks `{"Colors", <entry name>}` and keys `R`, `G`, `B`,
`A`. Not a packed `0xRRGGBBAA` int: `util::config::write_int` takes a signed
`int`, and any colour with red above 0x7F would pack to a negative number that
reads back correctly only by accident. Four ints are also legible when someone
edits `config.json` by hand.

`apply_colors()` reads the same keys in `build()`. It touches only memory,
so it is safe inside the boot window.

## 3. Colour helper

New: `src/menu/base/submenus/helper_color.{h,cpp}`,
`helper_color_presets.{h,cpp}`, `helper_color_sync.{h,cpp}`

The helper is a *shared editor*, not a feature. It holds a target in an anonymous
namespace and any caller can aim it before opening:

```cpp
class helper_color_menu : public menu::submenu::submenu {
public:
    static void target(int registry_index);   // what the editor edits
    ...
};
```

Keeping the target as a registry index rather than a raw `color_rgba*` means the
helper can name what it is editing in its own title, and reach the default for a
per-colour revert, without a second lookup.

**Editor.** A `scroll_option` picks RGBA or HSVA; changing it calls
`update_once()`, which does `clear_options(n)` and rebuilds — the dirty-flag
pattern the port already uses for changing lists. Never rebuild per frame.

- RGBA: four `number_option<int>`, 0-255, `can_loop()`, live preview on hover.
- HSVA: `number_option<float>` for hue 0-360 and saturation/value 0-100, plus the
  int alpha. Each writes back through `menu::renderer::hsv_to_rgb`.

**Presets.** `helper_color_presets` — a static table of named colours as a
`scroll_option`; selecting one assigns to the target. The table is
constant-initialised `color_rgba`, which its `constexpr` constructors already
allow.

**Sync With…** `helper_color_sync` — one button per registry entry; clicking
copies that entry's *current* value into the target and returns. This is the
piece that could not exist before the registry: it needs to enumerate colours,
and nothing else in the codebase could.

Every mutation runs an `on_change` callback so a colour that is mid-edit
redraws immediately rather than at the next submenu rebuild.

## 4. Rainbow

New: `src/menu/base/util/rainbow_math.h` (pure, host-testable),
`src/menu/base/util/rainbow.{h,cpp}` (stateful)

The split mirrors `frame_clock.h` against `animated_texture.cpp`: the maths must
be includable from a host test that has no C++ standard library, so the stateful
container that owns `stl::vector` lives in a separate file.

```cpp
// rainbow_math.h - C headers only, no STL, no PS4 headers.
namespace menu::rainbow_math {
    struct rgb { int r, g, b; };

    // Hue at `step` of `steps`, rendered into [min,max] per channel.
    rgb color_at(int step, int steps, int min, int max);
}
```

Behaviour, defined here rather than inherited by guesswork: hue advances
`360/steps` degrees per call and wraps; saturation and value are full; the
resulting channels are remapped into `[min, max]` so the cycle never reaches
black or full saturation. Ozark's defaults — min 25, max 250, steps 80 — carry
over as ours.

The stateful half holds the registered colour pointers with their pre-rainbow
values, so `stop()` restores exactly what was there:

```cpp
namespace menu {
    class rainbow {
    public:
        void configure(int min, int max, int steps);
        void add(color_rgba* c);       // remembers the current value
        void remove(color_rgba* c);    // restores it
        bool contains(const color_rgba* c) const;
        void run();                    // one step; per frame from tick()
        void stop();                   // restore every registered colour
        bool m_enabled = false;
    };
    rainbow* get_rainbow();            // function-local static
}
```

`run()` is called from `menu::tick()`. It needs **no** `player_valid()` gate — it
reads and writes plain memory and calls no natives — but it must not run during
`build()`, which it does not, because `tick` is only wired as the frame callback
after `build()` returns.

Alpha is never touched: a rainbow that also cycles opacity makes the menu
flicker transparent.

The helper gains a per-colour "Rainbow" toggle that calls `add`/`remove`. Config
persists one bool per registry entry plus the three parameters.

## 5. Panels

New: `src/menu/panels/{player,vehicle,world,debug}_panel.{h,cpp}` and a
`register_builtin_panels()` that `menu.cpp` calls in place of
`register_demo_panel()`. Keeping them out of `menu.cpp` stops that file — already
300 lines of registration — from absorbing four more render callbacks.

Each panel is a `panel_child` with an `m_update` callback returning the total
height, per the existing framework signature. `m_custom_ptr[0x150]` carries
per-panel state; the debug panel uses it for the FPS accumulator.

| Panel | Contents |
|---|---|
| Player | position X/Y/Z/heading, health, armour, wanted level, zone name |
| Vehicle | model name, speed, gear, engine and body health, handle |
| World | game time, weather, nearby ped and vehicle counts |
| Debug | build tag, eboot base, hash-table status and count, FPS |

**The gate.** `menu::panels::update()` is called from `menu::tick()` outside any
`player_valid()` check. That has been harmless only because the demo panel calls
no natives. All four of these do. The call gets the same gate `feature_update`
has:

```cpp
if (game::player_valid())
    menu::panels::update();
```

At the callsite, not inside the four callbacks — one place that covers every
panel written from now on, instead of a rule each new panel can forget. Panels
disappear during loading, which is correct.

**Build tag.** The debug panel and `module_start` must show the same string, so
the literal moves out of `InsulinGTAV.cpp` into `src/platform/build_tag.h` as
`INSULIN_BUILD_TAG`. The tag is the project's only reliable proof of which `.prx`
is running; a panel that disagrees with the klog would poison that.

**Panels submenu.** Ozark's `misc_panels` under Miscellaneous: per panel, column,
index and visibility, persisted, applied through `menu::panels::rearrange`. Built
with the dirty-flag pattern, since the panel list is data.

## Failure modes

- **A registry entry whose pointer goes stale.** Cannot happen by construction:
  every pointer targets a namespace-scope global with static storage duration.
- **A config file with a colour out of range.** Values are clamped to 0-255 on
  read. A hand-edited `config.json` should not be able to produce an
  undrawable colour.
- **Rainbow left enabled with a colour that was later reset.** `remove()` and
  `stop()` restore from the value captured at `add()` time, so a reset while the
  rainbow runs leaves the rainbow's own snapshot stale. `Reset to Default` calls
  `stop()` first.
- **Panels with no vehicle / no player.** Each callback returns early with a
  zero height, which the framework already treats as "draw nothing".

## Testing

Host tests, built the way `tests/frame_clock_test.cpp` documents
(`clang++ -std=c++17 -I src tests/<name>.cpp -o build/<name>.exe`):

- `tests/rainbow_math_test.cpp` — hue returns to its start after `steps` calls;
  every channel stays inside `[min, max]`; `steps` of 0 or 1 does not divide by
  zero.
- `tests/color_pack_test.cpp` — HSV to RGB and back is stable across the 31
  default colours; out-of-range config values clamp.

On console, in build order:

1. Change a colour, reopen the menu — the change holds. Restart the game — it
   still holds. `Reset to Default` restores the original.
2. Presets apply; Sync copies; the HSVA editor and the RGBA editor agree on the
   same colour.
3. Rainbow cycles smoothly, `stop()` restores exactly the pre-rainbow colour.
4. All four panels show live values on foot and in a vehicle.
5. **The gate:** start the game with panels enabled and watch the loading screen.
   Panels must stay absent until the player exists, and the game must not close
   itself. This is the test the whole sub-project's risk sits on.

## Out of scope

- Ozark's network panels (player info, friends, sessions) — network features.
- Theme import/export as files. Colours persist in `config.json`; a theme file
  format is a separate decision.
- Localisation of the new option names. The port's `localization` is a
  pass-through; translation is not in this sub-project.
