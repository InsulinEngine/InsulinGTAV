# Companion Map and Click-to-Teleport Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Click a point on a map of Los Santos in the phone browser and have the game fade out, move the player there, resolve the ground under them, and fade back in.

**Architecture:** The browser owns the map: it draws six static tiles, converts between pixels and world coordinates, and posts a world position. The plugin owns the move: `POST /api/teleport` already parses and queues a POD job (built in the previous plan), and the companion submenu's `feature_update()` drains it into a frame-driven state machine that fades, warps, waits for collision to stream, resolves ground height and fades back. The state machine is pure logic with its engine calls injected, so it is unit-tested on the host exactly as `net::serve_one` is.

**Tech Stack:** C++17, `-fno-exceptions -fno-rtti`, project-local mini-STL, OpenOrbis toolchain, host tests built with clang++ against C headers only. Browser side is plain HTML/CSS/JS, no framework and no CDN. Map tiles come from the PC copy of GTA V through the CodeWalker MCP server.

**Spec:** `docs/superpowers/specs/2026-09-06-companion-server-design.md`, sections "Map and teleport" and "Build order" step 3.

## Global Constraints

- Target build is **CUSA00411 v1.57** only. Every RVA is an offset from ELF base 0.
- **Natives only run on the game's script thread.** The HTTP thread must never call one or read an engine structure.
- **Nothing may call a native during `menu::build()`.** Option construction sets state; applying belongs in `feature_update()`.
- **No `.init_array`.** Global constructors do not run. Use function-local statics or explicit init.
- `stl::function` captures cap at **64 bytes**; `stl::string` is a fixed **128-byte** buffer that truncates silently.
- Files intended for host tests must not include `platform/log.h` or any PS4 header. `src/menu/base/util/frame_clock.h` and `src/net/conn.h` are the two reference shapes.
- Host tests build with: `clang++ -std=c++17 -I src tests/<name>.cpp [deps] -o build/<name>.exe`. C headers only; MSVC's C++ stdlib rejects the installed clang (STL1000).
- Source files are picked up by `file(GLOB_RECURSE src/*.cpp CONFIGURE_DEPENDS)` — adding a `.cpp` needs no CMake edit.
- Console FTP is `10.10.10.236:2121`. The plugin's own log is `/data/Ozark/insulingtav.log` — **not** `/data/insulingtav.log`, which does not exist. The kernel log on port 3232 is currently silent and is not a usable channel.
- Bump `INSULIN_BUILD_TAG` in `src/platform/build_tag.h` on every deploy and confirm it in the log; `__TIME__` only changes for the TU that recompiled.

## The map projection, taken from the game rather than guessed

The spec proposed deriving the world-to-map transform by checking two known points against the live player position. That is unnecessary: `x64a.rpf\data\tune\minimap.ymt` ships the exact numbers for the pause-map bitmap.

```xml
<Bitmap>
  <iBitmapTilesX value="2" />
  <iBitmapTilesY value="3" />
  <vBitmapTileSize x="4500" y="4500" />
  <vBitmapStart x="-4140" y="8400" />
  <eBitmapForPause>MM_BITMAP_VERSION_SEA</eBitmapForPause>
</Bitmap>
```

So the pause map is a **2-wide, 3-tall** grid of tiles, each covering **4500 x 4500** world units, with the top-left corner at world **(-4140, 8400)** and Y decreasing downward. The full bitmap therefore covers:

| Axis | From | To | Span |
|---|---|---|---|
| World X | -4140 | 4860 | 9000 |
| World Y | 8400 | -5100 | 13500 |

Every landmark in `src/menu/base/submenus/teleport.cpp`'s `g_places[]` falls inside that box, Mount Chiliad at `(501.5, 5604.4)` and the military base at `(-2047.4, 3132.1)` included, which is the sanity check that the reading is right.

`eBitmapForPause` is `MM_BITMAP_VERSION_SEA`, so the pause map uses the `minimap_sea_*` tiles, not the plain `minimap_*` ones. Those are the tiles to extract.

Note the separate `<Tiles>` block in the same file (`vMiniMapWorldSize 9400 x 12492`, `vMiniMapWorldStart -4500, 8000`). Those numbers describe the *vector minimap*, not the pause bitmap. Using them for the bitmap puts every marker slightly off. The `<Bitmap>` block is the one this plan uses.

Tile file naming is `minimap_sea_<row>_<column>.ytd`, rows 0..2 top to bottom, columns 0..1 left to right. This follows by elimination: the files run `0_0, 0_1, 1_0, 1_1, 2_0, 2_1`, so the first index takes three values and the second takes two, and `iBitmapTilesX` is 2. Task 4 verifies it visually rather than trusting the deduction.

## What this plan corrects in the previous one

The spec claims the move itself can lean on the existing teleport submenu: "Ground height and the move itself go through the existing teleport submenu — fade, resolve ground, wait for collision. That path is built and tested; this only triggers it."

It is not built. `teleport.cpp:28`'s `warp()` is `set_entity_coords_no_offset` plus a notification, with no fade, no ground resolution and no collision wait. The waypoint teleport works around the gap by arriving 100 m above the target and letting the player fall in. The natives needed to do it properly all exist (`do_screen_fade_out`/`is_screen_faded_out`/`do_screen_fade_in` and `request_collision_at_coord` as direct RVAs, `get_ground_z_for_3d_coord` through the hash table), so this plan builds that path in Task 2 rather than pretending it is already there.

---

### Task 1: Move the companion web root under OZARK_DIR

`src/menu/base/submenus/companion.cpp` hardcodes `"/data/GoldHEN/insulin/web"`. Every other path the plugin owns lives under `OZARK_DIR` in `src/platform/paths.h`, whose own header comment warns that "a path spelled out at a callsite is a path that drifts from the one the loader creates". This is that drift, introduced by the previous plan. Fix it before adding a second directory of assets under the wrong root.

**Files:**
- Modify: `src/platform/paths.h`
- Modify: `src/menu/base/submenus/companion.cpp`

**Interfaces:**
- Consumes: `OZARK_DIR` from `src/platform/paths.h`.
- Produces: `OZARK_WEB` (`"/data/Ozark/web"`) and `OZARK_MAP` (`"/data/Ozark/web/map"`).

- [ ] **Step 1: Add the two path constants**

In `src/platform/paths.h`, after the `OZARK_IMGCACHE` line:

```c
#define OZARK_WEB       OZARK_DIR "/web"
#define OZARK_MAP       OZARK_WEB "/map"
```

- [ ] **Step 2: Use them in the submenu**

In `src/menu/base/submenus/companion.cpp`, add the include next to the others:

```cpp
#include "platform/paths.h"
```

and replace the literal in `feature_update()`:

```cpp
        if (net::server_start((unsigned short)g_port,
                              OZARK_WEB,
                              g_pin, companion_state_json)) {
```

- [ ] **Step 3: Build**

```bash
wsl.exe bash -lc 'export OO_PS4_TOOLCHAIN=/home/bbc/OpenOrbis-PS4-Toolchain; \
  export PATH="$OO_PS4_TOOLCHAIN/bin/linux:$PATH"; \
  cd /mnt/e/Projects/PS4/InsulinEngine/InsulinGTAV && cmake --build build-wsl'
```

Expected: builds clean.

- [ ] **Step 4: Move the page on the console and verify**

Bump `INSULIN_BUILD_TAG` to `map-1`, rebuild, then deploy the plugin and put the page at the new root:

```python
import ftplib, io
f = ftplib.FTP(); f.connect('10.10.10.236', 2121, timeout=20); f.login()
prx = open(r'E:\Projects\PS4\InsulinEngine\InsulinGTAV\build-wsl\InsulinGTAV.prx','rb').read()
f.storbinary('STOR /data/GoldHEN/plugins/InsulinGTAV.prx', io.BytesIO(prx))
for d in ('/data/Ozark/web', '/data/Ozark/web/map'):
    try: f.mkd(d)
    except Exception as e: print('mkd', d, e)
page = open(r'E:\Projects\PS4\InsulinEngine\InsulinGTAV\assets\web\index.html','rb').read()
f.storbinary('STOR /data/Ozark/web/index.html', io.BytesIO(page))
f.retrlines('LIST /data/Ozark/web')
f.quit()
```

Restart the game, switch the server on, and confirm the page still comes back:

```bash
curl -i "http://10.10.10.236:8080/?pin=<PIN>"
```

Expected: `200`, `Content-Type: text/html; charset=utf-8`. The old `/data/GoldHEN/insulin` directory can be deleted by hand; nothing reads it any more.

- [ ] **Step 5: Commit**

```bash
git add src/platform/paths.h src/menu/base/submenus/companion.cpp src/platform/build_tag.h
git commit -m "fix(companion): serve the page from OZARK_WEB, not a hardcoded path"
```

---

### Task 2: The teleport state machine, host-tested

A teleport spans frames: fade out, move, wait for the world to stream in, ask for ground height, fade back. That is a state machine, and making it a pure one with its engine calls injected means the interesting part — the ordering and the give-up paths — is testable on the PC, exactly as `net::serve_one` is testable without a socket.

**Files:**
- Create: `src/game/teleport_fsm.h`
- Test: `tests/teleport_fsm_test.cpp`

**Interfaces:**
- Consumes: nothing.
- Produces:
  - `enum class game::tp::phase : unsigned char { idle, fading_out, streaming, resolving }`
  - `struct game::tp::request { float x, y, z; bool ground; }`
  - `struct game::tp::actions { void (*fade_out)(); bool (*faded_out)(); void (*fade_in)(); void (*move)(float,float,float); void (*stream)(float,float,float); bool (*ground_z)(float,float,float,float*); }`
  - `class game::tp::machine` with `bool submit(const request&)`, `void tick(const actions&)`, `phase state() const`, `bool busy() const`
  - `const float game::tp::probe_z` (`1000.0f`), `const int game::tp::stream_frames` (`30`), `const int game::tp::resolve_frames` (`60`), `const float game::tp::ground_clearance` (`1.0f`)

- [ ] **Step 1: Write the failing test**

Create `tests/teleport_fsm_test.cpp`:

```cpp
// Host unit tests for the companion teleport state machine.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/teleport_fsm_test.cpp -o build/teleport_fsm_test.exe
//   ./build/teleport_fsm_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
#include "game/teleport_fsm.h"
#include <stdio.h>
#include <math.h>

static int g_failed = 0;

static void check_true(const char* what, bool ok) {
    if (!ok) { printf("FAIL %s\n", what); g_failed++; }
    else     { printf("ok   %s\n", what); }
}

static void check_near(const char* what, float got, float want) {
    if (fabsf(got - want) > 0.01f) {
        printf("FAIL %s: got %.3f, want %.3f\n", what, got, want); g_failed++;
    } else {
        printf("ok   %s = %.3f\n", what, got);
    }
}

// --- the fake engine -------------------------------------------------------

static int   g_fade_out_calls = 0, g_fade_in_calls = 0, g_move_calls = 0, g_stream_calls = 0;
static float g_last_move_x, g_last_move_y, g_last_move_z;
static bool  g_is_faded = false;      // what faded_out() reports
static bool  g_ground_ok = true;      // whether ground_z() answers
static float g_ground_value = 42.0f;

static void fake_fade_out()  { g_fade_out_calls++; }
static bool fake_faded_out() { return g_is_faded; }
static void fake_fade_in()   { g_fade_in_calls++; }
static void fake_move(float x, float y, float z) {
    g_move_calls++; g_last_move_x = x; g_last_move_y = y; g_last_move_z = z;
}
static void fake_stream(float, float, float) { g_stream_calls++; }
static bool fake_ground_z(float, float, float, float* out) {
    if (!g_ground_ok) return false;
    *out = g_ground_value;
    return true;
}

static game::tp::actions fakes() {
    game::tp::actions a;
    a.fade_out  = fake_fade_out;
    a.faded_out = fake_faded_out;
    a.fade_in   = fake_fade_in;
    a.move      = fake_move;
    a.stream    = fake_stream;
    a.ground_z  = fake_ground_z;
    return a;
}

static void reset() {
    g_fade_out_calls = g_fade_in_calls = g_move_calls = g_stream_calls = 0;
    g_is_faded = false; g_ground_ok = true; g_ground_value = 42.0f;
}

int main() {
    const game::tp::actions a = fakes();

    // A fresh machine is idle and does nothing when ticked.
    {
        reset();
        game::tp::machine m;
        check_true("fresh is idle", m.state() == game::tp::phase::idle);
        check_true("fresh is not busy", !m.busy());
        m.tick(a);
        check_true("idle tick calls nothing", g_fade_out_calls == 0 && g_move_calls == 0);
    }

    // submit() starts the fade and nothing else; the move waits for black.
    {
        reset();
        game::tp::machine m;
        game::tp::request r = { 100.0f, 200.0f, 0.0f, true };
        check_true("submit accepted", m.submit(r));
        check_true("submit is busy", m.busy());
        m.tick(a);
        check_true("fade requested once", g_fade_out_calls == 1);
        check_true("no move before black", g_move_calls == 0);
        check_true("phase is fading_out", m.state() == game::tp::phase::fading_out);

        // Ticking while the screen is still fading must not re-request the fade.
        m.tick(a);
        check_true("fade not re-requested", g_fade_out_calls == 1);

        // Once black, the machine moves to the probe altitude and asks the
        // world to stream in there.
        g_is_faded = true;
        m.tick(a);
        check_true("moved once black", g_move_calls == 1);
        check_near("probe x", g_last_move_x, 100.0f);
        check_near("probe y", g_last_move_y, 200.0f);
        check_near("probe z", g_last_move_z, game::tp::probe_z);
        check_true("collision requested", g_stream_calls == 1);
        check_true("phase is streaming", m.state() == game::tp::phase::streaming);
    }

    // A second submit while busy is refused, so sixteen queued map clicks
    // cannot interleave into sixteen overlapping teleports.
    {
        reset();
        game::tp::machine m;
        game::tp::request r = { 1.0f, 2.0f, 0.0f, true };
        check_true("first submit ok", m.submit(r));
        check_true("second submit refused", !m.submit(r));
    }

    // The full happy path: ground resolves, the player lands just above it.
    {
        reset();
        game::tp::machine m;
        game::tp::request r = { 10.0f, 20.0f, 0.0f, true };
        m.submit(r);
        g_is_faded = true;
        m.tick(a);                                   // fade seen, move to probe
        for (int i = 0; i < game::tp::stream_frames; i++) m.tick(a);
        check_true("phase is resolving", m.state() == game::tp::phase::resolving);

        g_ground_value = 55.0f;
        m.tick(a);
        check_true("moved twice", g_move_calls == 2);
        check_near("landed just above ground", g_last_move_z, 56.0f);
        check_true("faded back in", g_fade_in_calls == 1);
        check_true("back to idle", m.state() == game::tp::phase::idle);
        check_true("not busy again", !m.busy());
    }

    // ground == false trusts the z it was given and never calls ground_z.
    {
        reset();
        game::tp::machine m;
        game::tp::request r = { 10.0f, 20.0f, 77.0f, false };
        m.submit(r);
        g_is_faded = true;
        m.tick(a);
        for (int i = 0; i < game::tp::stream_frames; i++) m.tick(a);
        m.tick(a);
        check_near("used the given z", g_last_move_z, 77.0f);
        check_true("faded back in", g_fade_in_calls == 1);
        check_true("idle", m.state() == game::tp::phase::idle);
    }

    // The failure that matters: ground never resolves. The machine must give
    // up and fade back in regardless, because a teleport that leaves the
    // screen black forever is worse than one that drops the player.
    {
        reset();
        game::tp::machine m;
        game::tp::request r = { 10.0f, 20.0f, 0.0f, true };
        m.submit(r);
        g_is_faded = true;
        m.tick(a);
        for (int i = 0; i < game::tp::stream_frames; i++) m.tick(a);

        g_ground_ok = false;
        for (int i = 0; i < game::tp::resolve_frames + 2; i++) m.tick(a);
        check_true("gave up and faded in", g_fade_in_calls == 1);
        check_true("idle after giving up", m.state() == game::tp::phase::idle);
        check_true("left the player at probe height to fall",
                   g_last_move_z == game::tp::probe_z);
        check_true("accepts work again", m.submit(r));
    }

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed ? 1 : 0;
}
```

- [ ] **Step 2: Run it to make sure it fails**

Run:
```bash
clang++ -std=c++17 -I src tests/teleport_fsm_test.cpp -o build/teleport_fsm_test.exe
```
Expected: FAIL — `'game/teleport_fsm.h' file not found`.

- [ ] **Step 3: Write the state machine**

Create `src/game/teleport_fsm.h`:

```cpp
#pragma once

// Frame-driven teleport: fade out, move, let the world stream in, resolve the
// ground, fade back. It spans frames, so it is a state machine rather than a
// function, and it is driven one step per call from the game thread.
//
// Every engine call is injected. That keeps this file free of natives, PS4
// headers and STL so it compiles for the host and is unit-tested on the PC
// (tests/teleport_fsm_test.cpp) - the same split net::serve_one uses for its
// byte transport, and for the same reason: the ordering and the give-up paths
// are the parts with real edge cases, and they are untestable on console.

namespace game::tp {

    // Arrive from high above and drop in. The world below a coordinate is not
    // loaded until something asks for it, so ground height is not answerable
    // until the player is already there.
    const float probe_z = 1000.0f;

    // Frames spent waiting for collision to stream in before the first ground
    // query, and frames spent retrying that query before giving up. At 30 fps
    // that is one second of streaming and two seconds of asking.
    const int stream_frames  = 30;
    const int resolve_frames = 60;

    // How far above the resolved ground to place the player, so they settle
    // onto it rather than starting inside it.
    const float ground_clearance = 1.0f;

    enum class phase : unsigned char {
        idle,          // nothing in flight
        fading_out,    // fade requested, waiting for the screen to go black
        streaming,     // at probe altitude, waiting for the world to load
        resolving      // asking for ground height until it answers or we stop
    };

    struct request {
        float x, y, z;
        bool  ground;      // resolve ground height instead of trusting z
    };

    // The engine, injected. Mirrors net::read_fn/write_fn in net/conn.h.
    struct actions {
        void (*fade_out)();
        bool (*faded_out)();
        void (*fade_in)();
        void (*move)(float x, float y, float z);
        void (*stream)(float x, float y, float z);
        // Ground height under (x, y) probed from z. False when the answer is
        // not available yet, which is normal for the first frames after a move.
        bool (*ground_z)(float x, float y, float probe, float* out);
    };

    class machine {
    public:
        // Takes the request, or refuses it while one is already in flight. The
        // job ring holds sixteen entries, so a phone with an impatient finger
        // can hand over sixteen clicks; running them concurrently would fight
        // over the same player.
        bool submit(const request& r) {
            if (m_phase != phase::idle) return false;
            m_req    = r;
            m_phase  = phase::fading_out;
            m_frames = 0;
            m_asked  = false;
            return true;
        }

        void tick(const actions& a) {
            switch (m_phase) {
            case phase::idle:
                return;

            case phase::fading_out:
                // Request the fade once, then wait for it to finish.
                if (!m_asked) { a.fade_out(); m_asked = true; }
                if (!a.faded_out()) return;
                a.move(m_req.x, m_req.y, probe_z);
                a.stream(m_req.x, m_req.y, probe_z);
                m_phase  = phase::streaming;
                m_frames = 0;
                return;

            case phase::streaming:
                if (++m_frames < stream_frames) return;
                m_phase  = phase::resolving;
                m_frames = 0;
                return;

            case phase::resolving: {
                if (!m_req.ground) { land(a, m_req.z); return; }

                float z = 0.0f;
                if (a.ground_z(m_req.x, m_req.y, probe_z, &z)) {
                    land(a, z + ground_clearance);
                    return;
                }
                // Give up rather than hold a black screen forever. The player
                // stays at probe altitude and falls, which is what the existing
                // waypoint teleport does on every jump.
                if (++m_frames >= resolve_frames) release(a);
                return;
            }
            }
        }

        phase state() const { return m_phase; }
        bool  busy()  const { return m_phase != phase::idle; }

    private:
        // Put the player down, then hand the screen back.
        void land(const actions& a, float z) {
            a.move(m_req.x, m_req.y, z);
            release(a);
        }

        // Hand the screen back without moving again: the player is already at
        // probe altitude and falls the rest of the way.
        //
        // Two functions rather than one with a "should I move" float compare.
        // Deciding by `z != probe_z` would read as clever and then silently
        // skip the move for a ground:false request that legitimately asked for
        // z = 1000.
        void release(const actions& a) {
            a.fade_in();
            m_phase = phase::idle;
        }

        request m_req    = { 0.0f, 0.0f, 0.0f, false };
        phase   m_phase  = phase::idle;
        int     m_frames = 0;
        bool    m_asked  = false;
    };
}
```

- [ ] **Step 4: Run the test and make sure it passes**

Run:
```bash
clang++ -std=c++17 -I src tests/teleport_fsm_test.cpp -o build/teleport_fsm_test.exe && ./build/teleport_fsm_test.exe
```
Expected: every line `ok`, ending in `all passed`.

- [ ] **Step 5: Commit**

```bash
git add src/game/teleport_fsm.h tests/teleport_fsm_test.cpp
git commit -m "feat(game): frame-driven teleport state machine, host-tested"
```

---

### Task 3: Drive the machine from the companion drain

The state machine has no engine behind it yet. This task supplies one and connects the job ring to it, which makes `POST /api/teleport` actually move the player — testable with `curl` alone, before any map exists.

**Files:**
- Create: `src/game/teleport_actions.h`
- Create: `src/game/teleport_actions.cpp`
- Modify: `src/menu/base/submenus/companion.cpp`
- Modify: `src/menu/base/submenus/teleport.cpp`

**Interfaces:**
- Consumes: `game::tp::actions`, `game::tp::machine`, `game::tp::request` (Task 2); `net::jobs()`, `net::job`, `net::job_kind` (previous plan).
- Produces:
  - `Entity game::teleport_subject()` — the vehicle when seated, else the ped
  - `const game::tp::actions& game::live_actions()` — the real engine, filled once

- [ ] **Step 1: Write the engine side**

Create `src/game/teleport_actions.h`:

```cpp
#pragma once

#include "game/teleport_fsm.h"
#include "rage/types/base_types.h"

// The engine behind game::tp::machine. Everything here calls natives, so it is
// game-thread only and deliberately kept out of teleport_fsm.h, which compiles
// for the host.

namespace game {

    // Move the vehicle when seated, not the ped: pulling the ped out from under
    // a car leaves the car behind and drops the player through the world.
    Entity teleport_subject();

    // The real actions table. Safe to call before the hash natives are up; the
    // ground query simply reports "not yet", which the machine already handles.
    const tp::actions& live_actions();
}
```

Create `src/game/teleport_actions.cpp`:

```cpp
#include "game/teleport_actions.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/natives_hash.h"

namespace {

    void act_fade_out()  { native::do_screen_fade_out(400); }
    bool act_faded_out() { return native::is_screen_faded_out(); }
    void act_fade_in()   { native::do_screen_fade_in(400); }

    void act_move(float x, float y, float z) {
        Entity e = game::teleport_subject();
        if (!e) return;
        native::set_entity_coords_no_offset(e, x, y, z, false, false, false);
    }

    void act_stream(float x, float y, float z) {
        native::request_collision_at_coord(x, y, z);
    }

    bool act_ground_z(float x, float y, float probe, float* out) {
        // A hash native: inert until the command table has been recovered and
        // verified, which takes a few seconds after boot. Reporting false then
        // is exactly right - the machine retries and eventually gives up.
        //
        // It lives in `native`, not `native::hash`: natives.h and
        // natives_hash.h both fill the same namespace, which is why the
        // generator refuses to emit a name that already exists in the other.
        return native::get_ground_z_for_3d_coord(x, y, probe, out, false, false);
    }
}

namespace game {

    Entity teleport_subject() {
        Ped ped = native::get_player_ped(-1);
        if (ped && native::is_ped_in_any_vehicle(ped, false)) {
            Vehicle veh = native::get_vehicle_ped_is_in(ped, false);
            if (veh)
                return veh;
        }
        return ped;
    }

    const tp::actions& live_actions() {
        // Function-local static: .init_array does not run in this plugin, so a
        // namespace-scope object with an initialiser would stay zeroed.
        static tp::actions a = {
            act_fade_out, act_faded_out, act_fade_in,
            act_move, act_stream, act_ground_z
        };
        return a;
    }
}
```

The signature was read off the generated header and is `(float x, float y, float z, float* groundZ, bool ignoreWater, bool p5)` at `src/rage/invoker/natives_hash.h:694`. If a regenerated header ever disagrees, that file wins over this plan.

- [ ] **Step 2: Remove the duplicate subject helper from the teleport submenu**

`src/menu/base/submenus/teleport.cpp` has its own `self_ped()` and `teleport_subject()` in an anonymous namespace — the same rule written twice is how two copies drift apart. Add

```cpp
#include "game/teleport_actions.h"
```

with the other includes, delete both functions from the anonymous namespace, and change their three callers (inside `warp()` and the two Nudge buttons) to `game::teleport_subject()`.

Leave `warp()` itself alone. The landmark buttons are instant jumps to known-good coordinates and need no fade.

- [ ] **Step 3: Drive the machine from the drain**

In `src/menu/base/submenus/companion.cpp`, add the includes:

```cpp
#include "game/teleport_fsm.h"
#include "game/teleport_actions.h"
```

add the machine to the anonymous namespace:

```cpp
    game::tp::machine g_teleporter;
```

and replace the drain block at the end of `feature_update()` with:

```cpp
    // Step whatever is in flight first, then take one new job. Ticking before
    // popping means a submit on an idle machine can never be refused.
    g_teleporter.tick(game::live_actions());

    if (!g_teleporter.busy()) {
        net::job j;
        if (net::jobs().pop(&j) && j.kind == net::job_kind::teleport) {
            game::tp::request r = { j.x, j.y, j.z, j.ground };
            g_teleporter.submit(r);
            platform::logf("net", "teleport to %.1f %.1f ground=%d",
                           j.x, j.y, (int)j.ground);
        }
    }
```

Note the deliberate change in shape: the old code drained the whole ring every frame. It now takes at most one job per frame and only while nothing is in flight, so queued clicks run one after another instead of fighting over the player.

- [ ] **Step 4: Build and re-run the host tests**

```bash
wsl.exe bash -lc 'export OO_PS4_TOOLCHAIN=/home/bbc/OpenOrbis-PS4-Toolchain; \
  export PATH="$OO_PS4_TOOLCHAIN/bin/linux:$PATH"; \
  cd /mnt/e/Projects/PS4/InsulinEngine/InsulinGTAV && cmake --build build-wsl'
```

Expected: builds clean.

```bash
clang++ -std=c++17 -I src tests/teleport_fsm_test.cpp -o build/teleport_fsm_test.exe && ./build/teleport_fsm_test.exe
```

Expected: `all passed`.

- [ ] **Step 5: Verify on console**

Bump `INSULIN_BUILD_TAG` to `map-2`, deploy, restart the game, switch the server on, then from the PC:

```bash
curl -i -X POST "http://10.10.10.236:8080/api/teleport?pin=<PIN>" \
     -d '{"x":-1037.7,"y":-2737.8,"ground":true}'
```

Expected, in order:
1. `200` from the request itself.
2. The screen fades to black, the player arrives at the airport, the screen fades back.
3. `/data/Ozark/insulingtav.log` carries `[net] teleport to -1037.7 -2737.8 ground=1`.
4. `GET /api/state?pin=<PIN>` reports coordinates at the airport with a sane `z` — around 20, not 1000.

Then the three cases that matter more than the happy path:

- **Teleport while driving.** Get in a car first. The car must come along; if the player arrives on foot and the car stays behind, `teleport_subject()` is wrong.
- **Six requests in a row.** Fire the `curl` six times as fast as possible. The teleports must run one after another and the game must stay up. If the screen ends up stuck black, the give-up path is not firing and the log says how far it got.
- **A teleport into the ocean**, e.g. `{"x":3000,"y":-4000,"ground":true}`. Ground will not resolve out there; after two seconds the machine must fade back in anyway and drop the player into the water rather than holding a black screen.

- [ ] **Step 6: Commit**

```bash
git add src/game/teleport_actions.h src/game/teleport_actions.cpp \
        src/menu/base/submenus/companion.cpp src/menu/base/submenus/teleport.cpp \
        src/platform/build_tag.h
git commit -m "feat(companion): teleport jobs drive the game thread for real"
```

---

### Task 4: Extract the pause-map tiles

Six PNGs, taken from the PC copy of GTA V through the CodeWalker MCP server. Nothing here runs on the console and nothing depends on the PS4 RPF reader — the artwork is the same world either way, and this side-steps a large piece of tooling the map does not need.

**Files:**
- Create: `tools/extract_map_tiles.md` (what was run, so it is repeatable)
- Create: `assets/web/map/tile_<row>_<col>.png` (6 files)

**Interfaces:**
- Consumes: the CodeWalker MCP server (`binary_to_xml`), the PC game directory it reports in `server_status`.
- Produces: six PNG tiles laid out 2 wide by 3 tall, plus the note file recording the projection constants.

- [ ] **Step 1: Export the six texture dictionaries to DDS**

For each of `0_0, 0_1, 1_0, 1_1, 2_0, 2_1`, call the CodeWalker MCP tool:

```
binary_to_xml(
  inputPath     = "x64b.rpf\\data\\cdimages\\scaleform_generic.rpf\\minimap_sea_<row>_<col>.ytd",
  outputXmlPath = "E:\\Projects\\PS4\\InsulinEngine\\InsulinGTAV\\build\\maptiles\\minimap_sea_<row>_<col>.xml",
  textureFolder = "E:\\Projects\\PS4\\InsulinEngine\\InsulinGTAV\\build\\maptiles\\dds"
)
```

`minimap_sea_*` and not `minimap_*`: `minimap.ymt` sets `eBitmapForPause` to `MM_BITMAP_VERSION_SEA`, so the sea variants are what the pause map actually draws, and they are the higher-detail set (~190 KB against ~35 KB).

- [ ] **Step 2: Convert DDS to PNG**

```python
from PIL import Image
import glob, os, pathlib
out = pathlib.Path(r'E:\Projects\PS4\InsulinEngine\InsulinGTAV\assets\web\map')
out.mkdir(parents=True, exist_ok=True)
for f in sorted(glob.glob(r'E:\Projects\PS4\InsulinEngine\InsulinGTAV\build\maptiles\dds\*.dds')):
    name = os.path.basename(f).lower()
    # minimap_sea_<row>_<col>.dds -> tile_<row>_<col>.png
    parts = name.replace('.dds', '').split('_')
    row, col = parts[-2], parts[-1]
    im = Image.open(f).convert('RGB')
    print(name, im.size, im.mode)
    im.save(out / f'tile_{row}_{col}.png', optimize=True)
```

If Pillow refuses the DDS — it decodes DXT1/3/5 but not BC7 — read the four-character code at byte 84 of the file to see what it actually is, and convert with `texconv -ft png` from DirectXTex instead. Do not guess the format; the header states it.

- [ ] **Step 3: Verify the tiles are what you think they are**

Stitch them and look at the result before trusting the row/column deduction:

```python
from PIL import Image
import pathlib
d = pathlib.Path(r'E:\Projects\PS4\InsulinEngine\InsulinGTAV\assets\web\map')
t = Image.open(d / 'tile_0_0.png')
w, h = t.size
sheet = Image.new('RGB', (w * 2, h * 3))
for r in range(3):
    for c in range(2):
        sheet.paste(Image.open(d / f'tile_{r}_{c}.png'), (c * w, r * h))
sheet.save(d.parent.parent / 'map_check.png')
print('tile', w, 'x', h, '-> sheet', sheet.size)
```

Open `map_check.png`. It must read as Los Santos with the city in the **south** (bottom) and Mount Chiliad's landmass in the **north** (top), the coastline running down the left. If it comes out mirrored or rotated, the first index is the column rather than the row: swap `row, col` in Step 2 and redo this check. Do not proceed on a sheet that looks wrong — every marker position downstream depends on this orientation.

Delete `map_check.png` afterwards; it is a check, not an asset.

- [ ] **Step 4: Write down how it was made**

Create `tools/extract_map_tiles.md` recording: the source paths inside `x64b.rpf`, why `minimap_sea_*` rather than `minimap_*`, the projection constants from `minimap.ymt` (`iBitmapTilesX 2`, `iBitmapTilesY 3`, `vBitmapTileSize 4500x4500`, `vBitmapStart -4140, 8400`), the resulting world bounds (X `-4140..4860`, Y `8400..-5100`), and the tile pixel size the export produced. Anyone regenerating these later needs the constants, not just the commands.

- [ ] **Step 5: Deploy the tiles**

```python
import ftplib, io, glob, os
f = ftplib.FTP(); f.connect('10.10.10.236', 2121, timeout=20); f.login()
try: f.mkd('/data/Ozark/web/map')
except Exception as e: print('mkd', e)
for p in sorted(glob.glob(r'E:\Projects\PS4\InsulinEngine\InsulinGTAV\assets\web\map\*.png')):
    data = open(p, 'rb').read()
    f.storbinary('STOR /data/Ozark/web/map/' + os.path.basename(p), io.BytesIO(data))
    print(os.path.basename(p), len(data))
f.retrlines('LIST /data/Ozark/web/map')
f.quit()
```

Then confirm the server hands one back with the right type:

```bash
curl -sI "http://10.10.10.236:8080/map/tile_0_0.png?pin=<PIN>"
```

Expected: `200` and `Content-Type: image/png` — `content_type_for()` in `src/net/conn.cpp:53` already knows `.png`. Check `Content-Length` against the local file size. A static file larger than the 256 KB response buffer is refused rather than streamed, so if a tile trips that, downscale it before the map page can use it.

- [ ] **Step 6: Commit**

```bash
git add assets/web/map tools/extract_map_tiles.md
git commit -m "assets(map): pause-map tiles extracted from the PC build"
```

---

### Task 5: The map page

The page grows a map: six tiles in a grid, the player as a marker that follows the live position, and a click that asks for a teleport. All the projection maths lives here, because the plugin has no use for it.

**Files:**
- Modify: `assets/web/index.html`

**Interfaces:**
- Consumes: `GET /api/state` (`{x, y, z, heading}`), `POST /api/teleport` (`{"x":..,"y":..,"ground":true}`), the six tiles under `/map/`.
- Produces: nothing other tasks consume.

- [ ] **Step 1: Rewrite the page**

Replace `assets/web/index.html` entirely:

```html
<!doctype html>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Insulin Companion</title>
<style>
  :root { color-scheme: dark; }
  body { font: 15px system-ui, sans-serif; margin: 0; padding: 12px;
         background: #14161a; color: #e8e8e8; }
  h1 { font-size: 16px; margin: 0 0 10px; font-weight: 600; }
  .bar { display: flex; gap: 8px; align-items: center; margin-bottom: 10px;
         flex-wrap: wrap; }
  input { font: inherit; padding: 7px 8px; border-radius: 6px;
          border: 1px solid #333; background: #14161a; color: inherit; width: 5em; }
  .pill { background: #1e2128; border-radius: 999px; padding: 5px 11px;
          font-variant-numeric: tabular-nums; color: #b6bec9; }
  #wrap { position: relative; width: 100%; max-width: 720px;
          aspect-ratio: 2 / 3; background: #0e1013; border-radius: 10px;
          overflow: hidden; touch-action: manipulation; }
  #tiles { display: grid; grid-template-columns: 1fr 1fr;
           grid-template-rows: 1fr 1fr 1fr; width: 100%; height: 100%; }
  #tiles img { width: 100%; height: 100%; display: block; }
  #me { position: absolute; width: 16px; height: 16px; margin: -8px 0 0 -8px;
        pointer-events: none; transition: left .15s linear, top .15s linear; }
  #me svg { display: block; }
  #ping { position: absolute; width: 22px; height: 22px; margin: -11px 0 0 -11px;
          border: 2px solid #ffd24a; border-radius: 50%; opacity: 0;
          pointer-events: none; }
  #ping.on { animation: pop .6s ease-out; }
  @keyframes pop { from { opacity: 1; transform: scale(.3); }
                   to   { opacity: 0; transform: scale(1.6); } }
</style>

<h1>Insulin Companion</h1>
<div class="bar">
  <label>PIN <input id="pin" maxlength="4" inputmode="numeric"></label>
  <span class="pill" id="pos">-</span>
  <span class="pill" id="status">waiting</span>
</div>

<div id="wrap">
  <div id="tiles"></div>
  <div id="me" hidden>
    <svg width="16" height="16" viewBox="0 0 16 16">
      <circle cx="8" cy="8" r="7" fill="#4aa3ff" stroke="#0b0d10" stroke-width="2"/>
      <path d="M8 1.5 L11 7 H5 Z" fill="#eaf4ff"/>
    </svg>
  </div>
  <div id="ping"></div>
</div>

<script>
// Projection straight out of x64a.rpf\data\tune\minimap.ymt, <Bitmap> block:
//   iBitmapTilesX 2, iBitmapTilesY 3, vBitmapTileSize 4500x4500,
//   vBitmapStart (-4140, 8400), Y decreasing downward.
// Do NOT use the <Tiles> block in the same file - those numbers describe the
// vector minimap, not this bitmap, and every marker would land slightly off.
const COLS = 2, ROWS = 3, TILE = 4500;
const WORLD = {
  x0: -4140, y0: 8400,
  w:  COLS * TILE,          //  9000
  h:  ROWS * TILE           // 13500
};

const $ = id => document.getElementById(id);
const pin = $('pin');
pin.value = new URLSearchParams(location.search).get('pin')
         || localStorage.getItem('pin') || '';
pin.addEventListener('change', () => localStorage.setItem('pin', pin.value));

// Lay the tiles out. Row 0 is north, column 0 is west. The PIN rides in the
// query string because an <img> request cannot set a header - which is exactly
// why pin_ok() in src/net/conn.cpp accepts both forms.
const tiles = $('tiles');
for (let r = 0; r < ROWS; r++)
  for (let c = 0; c < COLS; c++) {
    const img = document.createElement('img');
    img.src = `/map/tile_${r}_${c}.png?pin=${encodeURIComponent(pin.value)}`;
    img.alt = '';
    tiles.appendChild(img);
  }

const worldToFrac = (x, y) => ({
  u: (x - WORLD.x0) / WORLD.w,
  v: (WORLD.y0 - y) / WORLD.h
});
const fracToWorld = (u, v) => ({
  x: WORLD.x0 + u * WORLD.w,
  y: WORLD.y0 - v * WORLD.h
});

const headers = () => ({ 'X-Insulin-Pin': pin.value });

async function poll() {
  try {
    const r = await fetch('/api/state', { headers: headers() });
    if (r.status === 401) { $('status').textContent = 'wrong PIN'; return; }
    if (!r.ok)            { $('status').textContent = 'no state yet'; return; }
    const j = await r.json();

    const f = worldToFrac(j.x, j.y);
    const me = $('me');
    if (f.u < 0 || f.u > 1 || f.v < 0 || f.v > 1) {
      // Off the bitmap: interiors and the far ocean sit outside it.
      me.hidden = true;
      $('status').textContent = 'off map';
    } else {
      me.hidden = false;
      me.style.left = (f.u * 100) + '%';
      me.style.top  = (f.v * 100) + '%';
      // The heading is degrees clockwise from north, which is what CSS
      // rotate() means too, so it goes straight in.
      me.style.transform = `rotate(${j.heading}deg)`;
      $('status').textContent = 'connected';
    }
    $('pos').textContent =
      `${j.x.toFixed(0)}, ${j.y.toFixed(0)}, ${j.z.toFixed(0)}  ${j.heading.toFixed(0)}\u00b0`;
  } catch (e) {
    $('status').textContent = 'offline';
  }
}

$('wrap').addEventListener('click', async ev => {
  const box = ev.currentTarget.getBoundingClientRect();
  const u = (ev.clientX - box.left) / box.width;
  const v = (ev.clientY - box.top)  / box.height;
  const w = fracToWorld(u, v);

  const ping = $('ping');
  ping.style.left = (u * 100) + '%';
  ping.style.top  = (v * 100) + '%';
  ping.classList.remove('on');
  void ping.offsetWidth;            // restart the animation
  ping.classList.add('on');

  $('status').textContent = 'teleporting\u2026';
  try {
    const r = await fetch('/api/teleport', {
      method: 'POST',
      headers: headers(),
      body: JSON.stringify({ x: w.x, y: w.y, ground: true })
    });
    $('status').textContent = r.ok ? 'sent' : ('refused ' + r.status);
  } catch (e) {
    $('status').textContent = 'offline';
  }
});

setInterval(poll, 500);
poll();
</script>
```

- [ ] **Step 2: Deploy and verify the projection against known ground truth**

```python
import ftplib, io
f = ftplib.FTP(); f.connect('10.10.10.236', 2121, timeout=20); f.login()
page = open(r'E:\Projects\PS4\InsulinEngine\InsulinGTAV\assets\web\index.html','rb').read()
f.storbinary('STOR /data/Ozark/web/index.html', io.BytesIO(page))
print('page', len(page))
f.quit()
```

No plugin rebuild is needed — the page is a static file.

Open `http://10.10.10.236:8080/?pin=<PIN>` and check, in this order:

1. **Six tiles load** and form one continuous map with no seams or gaps.
2. **The marker sits where the player is.** Use the menu's own landmarks as ground truth: teleport in-game to Franklin's House (`-14, -1440`) and the marker must land on the southern city; to Mount Chiliad (`501, 5604`) and it must land on the northern mountain. A marker mirrored top-to-bottom means `vBitmapStart.y` is being applied with the wrong sign; a marker off by exactly one tile means the row/column order from Task 4 is swapped.
3. **The marker turns with the player.** Face north, then east; the arrow must follow.
4. **A click teleports.** Click the airport; the game fades, moves and fades back, and the marker arrives where the click was.
5. **Click accuracy.** Click a distinctive spot, wait for the arrival, and read `/api/state`. The arrival should be within roughly 50 m of the clicked world coordinate. The bitmap is 9000 world units across a few hundred CSS pixels, so one pixel is tens of metres — that is the honest resolution limit of a whole-map view, not a bug.
6. **On a phone**, on the same network, the whole loop works with a finger.

- [ ] **Step 3: Commit**

```bash
git add assets/web/index.html
git commit -m "feat(companion): map page with live marker and click-to-teleport"
```

---

## What this plan deliberately leaves out

- **Pan and zoom.** The whole map is one screen. Clicking at this scale is accurate to a few tens of metres, which is enough to arrive at a district; reaching an exact doorway is what the existing landmark buttons and the waypoint teleport are for. Zoom is worth adding once the map has proven useful, not before.
- **Blips.** Other players, vehicles and mission markers would each need an engine read and a snapshot field. The single player marker proves the projection; a second pass can add more once it is trusted.
- **Rewiring the landmark teleports through the state machine.** `warp()` stays an instant jump. Those coordinates are known-good and need no ground resolution, and changing a working feature to share a code path is a separate change with its own testing.
- **Texture swapping and the catalogue.** Spec stage 4, its own plan. That is the piece which genuinely needs the PS4 RPF reader, since a texture catalogue has to match the console's own archives — unlike map tiles, where the PC artwork is the same picture.
