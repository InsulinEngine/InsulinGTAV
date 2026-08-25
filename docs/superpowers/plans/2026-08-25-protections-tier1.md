# Protections Tier 1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the protections framework — filter registry, lossy report ring, per-player block state, detour installer, menu — and land the eleven local crash guards on top of it.

**Architecture:** Filters are pure memory reads that run on the game's network thread and must never call a native, allocate, or notify. Each writes a fixed-size record into a lock-free bounded ring; a drain on the script thread turns records into kernel-log lines and rate-limited on-screen notifications. Every filter carries a tri-state mode (Off / Log / Enforce) persisted to `config.json`; `Log` runs the detection and then chains to the original anyway, so a wrong offset is a bad log line rather than a crash.

**Tech Stack:** C++17, clang via the OpenOrbis PS4 toolchain, GoldHEN `Detour` API, the project's mini-STL (`src/stl/`), CMake (GLOB_RECURSE — new `.cpp` files need no build-file edit).

**Spec:** `docs/superpowers/specs/2026-08-25-ps4-protections-design.md`

## Global Constraints

- **Target build is CUSA00411 v1.57 only.** Every address is an RVA against ELF base 0, made absolute at install time against `rage::invoker::g_eboot_base`.
- **No natives, no allocation, no notification inside a hook.** Hooks run on the network thread. Breaking this crashes the game (boot-rule #1).
- **No `.init_array`.** Global constructors do not run. Use function-local statics (`static T x; return &x;`) or explicit init.
- **`stl::function` captures cap at 64 bytes** (`STL_FUNCTION_CAP`). Capture an index or id, never an object.
- **`stl::string` is a fixed 128-byte buffer** that truncates silently.
- **No `stl::to_string`, no exceptions (`-fno-exceptions`), no RTTI.**
- **Nothing may call a native during `menu::build()`** — including from `add_savable`. Option construction sets state; applying it belongs in `feature_update()`.
- **Build directory is `build-wsl/`**, never `build/` (which holds a Windows CMake cache; mixing them fails confusingly).
- **Host unit tests use C headers only** (`<stdio.h>`, not `<cstdio>`): MSVC's C++ stdlib rejects the installed clang with STL1000.
- **Bump `INSULIN_BUILD_TAG` in `src/platform/build_tag.h` on every console deploy.** It is the only reliable proof of which `.prx` is running; `__TIME__` goes stale for TUs that were not recompiled.

## Build, test and deploy commands

Host unit test (from repo root). Header-only units need only the test file;
a unit split into `.h`/`.cpp` must have its `.cpp` on the command line too, or
the test fails to **link** rather than failing an assertion — which breaks the
red/green cycle:
```bash
# header-only unit
clang++ -std=c++17 -I src tests/<name>_test.cpp -o build/<name>_test.exe && ./build/<name>_test.exe
# unit with a .cpp
clang++ -std=c++17 -I src tests/<name>_test.cpp src/<path>/<unit>.cpp -o build/<name>_test.exe && ./build/<name>_test.exe
```
These units are host-compilable because they depend on nothing from the PS4
toolchain — `ring.h` includes only `<stdint.h>`, and `registry.cpp` includes
only `registry.h` plus clang's `__atomic_*` builtins. Keep it that way: an
include of anything under `platform/`, `rage/` or `stl/` makes them
untestable on the host.

Plugin build:
```bash
wsl.exe bash -lc 'export OO_PS4_TOOLCHAIN=/home/bbc/OpenOrbis-PS4-Toolchain; \
  export PATH="$OO_PS4_TOOLCHAIN/bin/linux:$PATH"; \
  cd /mnt/e/Projects/PS4/InsulinEngine/InsulinGTAV && cmake --build build-wsl'
```

Deploy over FTP:
```bash
python -c "
import ftplib, io
data = open(r'E:\Projects\PS4\InsulinEngine\InsulinGTAV\build-wsl\InsulinGTAV.prx','rb').read()
f = ftplib.FTP(); f.connect('10.10.10.236', 2121, timeout=20); f.login()
f.storbinary('STOR /data/GoldHEN/plugins/InsulinGTAV.prx', io.BytesIO(data)); f.quit()
print('deployed', len(data), 'bytes')"
```

Watch the live log: `nc 10.10.10.236 3232` — plugin lines are prefixed `IGV`.

FTP `RETR` is unreliable on GoldHEN (returns a fixed-size blob regardless of file). Never treat a download as proof of what is deployed; confirm from the build-tag log line.

## Deferred to Tier 2

Two components the spec lists under `src/protections/` are **not** built here,
because nothing in Tier 1 would consume them and shipping unused code is how
untested code reaches production:

- **`players.h` — per-player block bitmasks.** Tier 1's guards are local crash
  checks that no player is attributable for; the first real consumer is the
  net-event filter in Tier 2, which is where it belongs.
- **`protections_log.cpp` — an in-game view of recent blocks.** During Tier 1 the
  kernel log over `nc 10.10.10.236 3232` covers this need completely, and every
  Tier 1 guard is verified at a desk with a terminal open. It starts mattering in
  Tier 2, when verification moves into live sessions away from the PC.

---

## Phase A — Framework

### Task 1: Bounded lossy report ring

**Files:**
- Create: `src/protections/ring.h`
- Test: `tests/protections_ring_test.cpp`

**Interfaces:**
- Consumes: nothing.
- Produces: `protections::record` (POD: `uint16_t filter_id; uint8_t player_index; uint8_t flags; uint32_t detail_a; uint32_t detail_b;`), `protections::ring` with `void push(const record&)`, `bool pop(record* out)`, `uint32_t dropped() const`, `void reset()`. Header-only, no dependencies beyond `<stdint.h>` so it is host-testable.

Multiple network threads may push; exactly one thread (the frame callback) pops. Overflow drops the newest record and increments a counter — a report must never block a network thread.

- [ ] **Step 1: Write the failing test**

Create `tests/protections_ring_test.cpp`:

```cpp
// Host unit tests for the protections report ring.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/protections_ring_test.cpp -o build/protections_ring_test.exe
//   ./build/protections_ring_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
#include "protections/ring.h"
#include <stdio.h>

static int g_failed = 0;

static void check_true(const char* what, bool ok) {
    if (!ok) { printf("FAIL %s\n", what); g_failed++; }
    else     { printf("ok   %s\n", what); }
}

static void check_eq(const char* what, unsigned got, unsigned want) {
    if (got != want) { printf("FAIL %s: got %u, want %u\n", what, got, want); g_failed++; }
    else             { printf("ok   %s = %u\n", what, got); }
}

int main() {
    using namespace protections;

    ring r;
    record out;

    // Empty ring pops nothing.
    check_true("empty pop fails", !r.pop(&out));

    // One in, one out, fields intact.
    record a = { 7, 3, 1, 0xDEADBEEF, 42 };
    r.push(a);
    check_true("pop after push", r.pop(&out));
    check_eq("filter_id", out.filter_id, 7);
    check_eq("player_index", out.player_index, 3);
    check_eq("flags", out.flags, 1);
    check_eq("detail_a", out.detail_a, 0xDEADBEEF);
    check_eq("detail_b", out.detail_b, 42);
    check_true("ring is empty again", !r.pop(&out));

    // FIFO order across a wrap: fill, drain, refill past the wrap point.
    r.reset();
    for (unsigned i = 0; i < ring::capacity; i++) {
        record x = { (unsigned short)i, 0, 0, i, 0 };
        r.push(x);
    }
    check_eq("no drops when exactly full", r.dropped(), 0);

    // One more than capacity is dropped, not overwritten.
    record overflow = { 999, 0, 0, 999, 0 };
    r.push(overflow);
    check_eq("one drop past capacity", r.dropped(), 1);

    // The original contents survived the overflow attempt, in order.
    for (unsigned i = 0; i < ring::capacity; i++) {
        check_true("pop in fifo order", r.pop(&out) && out.detail_a == i);
    }
    check_true("drained", !r.pop(&out));

    // After draining, the ring accepts pushes again (wrap-around correctness).
    for (unsigned i = 0; i < ring::capacity; i++) {
        record x = { 1, 0, 0, 1000 + i, 0 };
        r.push(x);
    }
    check_true("first after wrap", r.pop(&out) && out.detail_a == 1000);

    // reset() clears the drop counter too.
    r.reset();
    check_eq("dropped cleared by reset", r.dropped(), 0);
    check_true("empty after reset", !r.pop(&out));

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed != 0;
}
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `clang++ -std=c++17 -I src tests/protections_ring_test.cpp -o build/protections_ring_test.exe`
Expected: FAIL — `fatal error: 'protections/ring.h' file not found`

- [ ] **Step 3: Write the implementation**

Create `src/protections/ring.h`:

```cpp
#pragma once
#include <stdint.h>

// Fixed-size lossy report ring. Producers are network threads inside protection
// hooks; the single consumer is the frame callback on the script thread.
//
// Two properties are non-negotiable and drive the design:
//   * A push must never block. A sound-spam attack produces hundreds of blocks
//     per second, and stalling a network thread to report them would be a worse
//     denial of service than the attack.
//   * A record must stay valid after the producing thread has moved on, so it
//     holds no pointers and no strings - only ids the consumer can resolve
//     against the registry.
//
// Overflow drops the NEWEST record rather than overwriting the oldest: during a
// burst the first few reports identify the attack, and the thousandth does not.
namespace protections {

    struct record {
        uint16_t filter_id;
        uint8_t  player_index;   // 0xFF when not attributable to a player
        uint8_t  flags;
        uint32_t detail_a;       // filter-defined: event hash, object id, node id
        uint32_t detail_b;
    };

    class ring {
    public:
        static const uint32_t capacity = 64;   // must stay a power of two

        void push(const record& r) {
            uint32_t claim;
            for (;;) {
                claim = __atomic_load_n(&m_tail, __ATOMIC_RELAXED);
                uint32_t head = __atomic_load_n(&m_head, __ATOMIC_ACQUIRE);
                if (claim - head >= capacity) {          // unsigned wrap is intended
                    __atomic_fetch_add(&m_dropped, 1, __ATOMIC_RELAXED);
                    return;
                }
                // Claim the slot before writing it. On failure another producer
                // took this index and `claim` has been reloaded for us.
                if (__atomic_compare_exchange_n(&m_tail, &claim, claim + 1, false,
                                                __ATOMIC_ACQ_REL, __ATOMIC_RELAXED))
                    break;
            }

            const uint32_t idx = claim & (capacity - 1);
            m_slots[idx] = r;
            // Release: the consumer must not see ready=1 before the payload.
            __atomic_store_n(&m_ready[idx], (uint8_t)1, __ATOMIC_RELEASE);
        }

        bool pop(record* out) {
            const uint32_t head = m_head;                 // consumer-owned, plain read
            if (head == __atomic_load_n(&m_tail, __ATOMIC_ACQUIRE))
                return false;                             // nothing claimed

            const uint32_t idx = head & (capacity - 1);
            if (!__atomic_load_n(&m_ready[idx], __ATOMIC_ACQUIRE))
                return false;                             // claimed but still being written

            *out = m_slots[idx];
            __atomic_store_n(&m_ready[idx], (uint8_t)0, __ATOMIC_RELAXED);
            __atomic_store_n(&m_head, head + 1, __ATOMIC_RELEASE);
            return true;
        }

        uint32_t dropped() const { return __atomic_load_n(&m_dropped, __ATOMIC_RELAXED); }

        // Not thread-safe. Test-only, and for a deliberate clear from the menu.
        void reset() {
            m_head = 0;
            m_tail = 0;
            m_dropped = 0;
            for (uint32_t i = 0; i < capacity; i++) m_ready[i] = 0;
        }

    private:
        record   m_slots[capacity] = {};
        uint8_t  m_ready[capacity] = {};
        uint32_t m_head    = 0;
        uint32_t m_tail    = 0;
        uint32_t m_dropped = 0;
    };
}
```

- [ ] **Step 4: Run the test to verify it passes**

Run: `clang++ -std=c++17 -I src tests/protections_ring_test.cpp -o build/protections_ring_test.exe && ./build/protections_ring_test.exe`
Expected: PASS — every line `ok`, final line `all passed`, exit code 0

- [ ] **Step 5: Commit**

```bash
git add src/protections/ring.h tests/protections_ring_test.cpp
git commit -m "feat(protections): lossy report ring that never blocks a network thread"
```

---

### Task 2: Filter registry

**Files:**
- Create: `src/protections/registry.h`, `src/protections/registry.cpp`
- Test: `tests/protections_registry_test.cpp`

**Interfaces:**
- Consumes: nothing from Task 1.
- Produces: `protections::mode` (`off = 0`, `log = 1`, `enforce = 2`), `protections::filter_id` enum, `protections::filter` struct, and the free functions `int count()`, `filter* at(int)`, `filter* find(filter_id)`, `const char* name_of(filter_id)`, `mode mode_of(filter_id)`, `bool should_report(filter_id)`, `bool should_block(filter_id)`, `void set_mode(filter_id, mode)`, `bool ensure_installed(filter_id)`.

`should_report` is true for `log` and `enforce`; `should_block` only for `enforce`. Every hook's first line is `if (!should_report(id)) return chain(...)`, so a disabled filter costs one predictable branch.

The mode is a plain `int` so a `dropdown_option` can bind to it by reference. It is read from hook threads and written from the menu thread without a lock: an aligned 32-bit read cannot tear, and the worst case of a race is one packet handled under the previous mode.

- [ ] **Step 1: Write the failing test**

Create `tests/protections_registry_test.cpp`:

```cpp
// Host unit tests for the protections filter registry.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/protections_registry_test.cpp src/protections/registry.cpp -o build/protections_registry_test.exe
//   ./build/protections_registry_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
#include "protections/registry.h"
#include <stdio.h>
#include <string.h>

static int g_failed = 0;

static void check_true(const char* what, bool ok) {
    if (!ok) { printf("FAIL %s\n", what); g_failed++; }
    else     { printf("ok   %s\n", what); }
}

int main() {
    using namespace protections;

    // Every filter has a non-empty, unique name, and names are stable config
    // keys - a duplicate would make two filters share a persisted setting.
    check_true("registry is non-empty", count() > 0);
    for (int i = 0; i < count(); i++) {
        filter* f = at(i);
        check_true("filter has a name", f->name && f->name[0]);
        for (int j = i + 1; j < count(); j++)
            check_true("names are unique", strcmp(f->name, at(j)->name) != 0);
    }

    // find() round-trips against at().
    for (int i = 0; i < count(); i++)
        check_true("find matches at", find(at(i)->id) == at(i));

    // An id outside the table resolves to nothing rather than reading past it.
    check_true("unknown id finds nothing", find((filter_id)0x7FFF) == nullptr);
    check_true("unknown id names safely", name_of((filter_id)0x7FFF) != nullptr);

    // Mode gating: off reports nothing, log reports without blocking,
    // enforce does both. This is the whole log-before-enforce contract.
    const filter_id probe = at(0)->id;

    set_mode(probe, mode::off);
    check_true("off does not report", !should_report(probe));
    check_true("off does not block", !should_block(probe));

    set_mode(probe, mode::log);
    check_true("log reports", should_report(probe));
    check_true("log does not block", !should_block(probe));

    set_mode(probe, mode::enforce);
    check_true("enforce reports", should_report(probe));
    check_true("enforce blocks", should_block(probe));

    // An out-of-range mode value must not enable blocking.
    set_mode(probe, (mode)99);
    check_true("bogus mode does not block", !should_block(probe));

    // An unknown id is inert rather than a crash - hooks call these directly.
    check_true("unknown id does not report", !should_report((filter_id)0x7FFF));
    check_true("unknown id does not block", !should_block((filter_id)0x7FFF));

    // Every filter defaults to log, never enforce. New detections must prove
    // themselves against real traffic before they are allowed to drop it.
    for (int i = 0; i < count(); i++)
        check_true("defaults to log", at(i)->default_mode == (int)mode::log);

    // install_enabled_filters() covers the gap left by add_savable restoring a
    // mode without firing its change handler. With no install function set it
    // must be a safe no-op, and it must be idempotent - it runs once per boot
    // but nothing should break if it runs twice.
    set_mode(probe, mode::enforce);
    install_enabled_filters();
    install_enabled_filters();
    check_true("install_enabled_filters leaves mode alone", should_block(probe));
    for (int i = 0; i < count(); i++)
        check_true("no install fn means not installed", at(i)->install || !at(i)->installed);

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed != 0;
}
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `clang++ -std=c++17 -I src tests/protections_registry_test.cpp src/protections/registry.cpp -o build/protections_registry_test.exe`
Expected: FAIL — `no such file or directory: 'src/protections/registry.cpp'`
(and, if you drop the `.cpp` from the line, `fatal error: 'protections/registry.h' file not found`). Either way the step is red before the implementation exists.

- [ ] **Step 3: Write the implementation**

Create `src/protections/registry.h`:

```cpp
#pragma once
#include <stdint.h>

// The filter table. One entry per protection, carrying its persisted mode and
// its detour installer.
//
// Detours install once, on first enable, and are never removed: unhooking while
// another thread sits inside the trampoline is a crash worth not risking. `off`
// therefore means "chain straight through", not "unhooked".
namespace protections {

    enum class mode : int { off = 0, log = 1, enforce = 2 };

    // Stable numeric ids. These are written into report records and must not be
    // renumbered - a saved config and a log line both refer to them.
    enum class filter_id : uint16_t {
        self_test           = 0,   // no detour; proves the report path end to end

        skeleton_extension  = 10,
        fragment_physics    = 11,
        invalid_decal       = 12,
        searchlight         = 13,
        task_ambient_clips  = 14,
        task_parachute      = 15,
        render_ped          = 16,
        render_entity       = 17,
        render_big_ped      = 18,
        pool_exhaustion     = 19,
        reliable_alloc      = 20,
    };

    struct filter {
        filter_id   id;
        const char* name;          // stable; also the config key
        uint8_t     tier;
        int         default_mode;  // always (int)mode::log
        int         current;       // bound to a dropdown_option by reference
        bool        installed;
        bool      (*install)();    // nullptr for filters with no detour
    };

    int     count();
    filter* at(int index);
    filter* find(filter_id id);

    const char* name_of(filter_id id);
    mode        mode_of(filter_id id);
    void        set_mode(filter_id id, mode m);

    // The two gates every hook uses.
    bool should_report(filter_id id);   // log or enforce
    bool should_block(filter_id id);    // enforce only

    // Installs the detour if it is not installed yet. Idempotent; returns true
    // when the filter is live (or needs no detour).
    bool ensure_installed(filter_id id);

    // Installs every filter whose persisted mode is not Off.
    //
    // This exists because dropdown_option::add_savable restores the saved value
    // but deliberately does NOT fire add_change - the same boot rule that stops
    // toggle_option from invoking click handlers during menu::build(). Without
    // this call a filter saved as Enforce comes back showing Enforce with its
    // detour never installed: a protection that reads as on and does nothing.
    // Idempotent; call it once the game is up, never from build().
    void install_enabled_filters();
}
```

Create `src/protections/registry.cpp`:

```cpp
#include "protections/registry.h"

namespace protections {
namespace {
    // Function-local static, not a namespace-scope object: this plugin has no
    // .init_array, so a global constructor would never run.
    filter* table(int* out_count) {
        static filter t[] = {
            { filter_id::self_test,          "Self Test",           1, (int)mode::log, (int)mode::log, false, nullptr },
            { filter_id::skeleton_extension, "Skeleton Extension",  1, (int)mode::log, (int)mode::log, false, nullptr },
            { filter_id::fragment_physics,   "Fragment Physics",    1, (int)mode::log, (int)mode::log, false, nullptr },
            { filter_id::invalid_decal,      "Invalid Decal",       1, (int)mode::log, (int)mode::log, false, nullptr },
            { filter_id::searchlight,        "Searchlight",         1, (int)mode::log, (int)mode::log, false, nullptr },
            { filter_id::task_ambient_clips, "Task Ambient Clips",  1, (int)mode::log, (int)mode::log, false, nullptr },
            { filter_id::task_parachute,     "Task Parachute",      1, (int)mode::log, (int)mode::log, false, nullptr },
            { filter_id::render_ped,         "Render Ped",          1, (int)mode::log, (int)mode::log, false, nullptr },
            { filter_id::render_entity,      "Render Entity",       1, (int)mode::log, (int)mode::log, false, nullptr },
            { filter_id::render_big_ped,     "Render Big Ped",      1, (int)mode::log, (int)mode::log, false, nullptr },
            { filter_id::pool_exhaustion,    "Pool Exhaustion",     1, (int)mode::log, (int)mode::log, false, nullptr },
            { filter_id::reliable_alloc,     "Reliable Allocator",  1, (int)mode::log, (int)mode::log, false, nullptr },
        };
        *out_count = (int)(sizeof(t) / sizeof(t[0]));
        return t;
    }
}

int count() {
    int n = 0;
    table(&n);
    return n;
}

filter* at(int index) {
    int n = 0;
    filter* t = table(&n);
    if (index < 0 || index >= n) return nullptr;
    return &t[index];
}

filter* find(filter_id id) {
    int n = 0;
    filter* t = table(&n);
    for (int i = 0; i < n; i++)
        if (t[i].id == id) return &t[i];
    return nullptr;
}

const char* name_of(filter_id id) {
    filter* f = find(id);
    return f ? f->name : "<unknown>";
}

mode mode_of(filter_id id) {
    filter* f = find(id);
    if (!f) return mode::off;
    const int m = __atomic_load_n(&f->current, __ATOMIC_RELAXED);
    if (m != (int)mode::log && m != (int)mode::enforce) return mode::off;
    return (mode)m;
}

void set_mode(filter_id id, mode m) {
    filter* f = find(id);
    if (!f) return;
    __atomic_store_n(&f->current, (int)m, __ATOMIC_RELAXED);
}

bool should_report(filter_id id) {
    const mode m = mode_of(id);
    return m == mode::log || m == mode::enforce;
}

bool should_block(filter_id id) {
    return mode_of(id) == mode::enforce;
}

bool ensure_installed(filter_id id) {
    filter* f = find(id);
    if (!f) return false;
    if (!f->install) return true;      // nothing to hook
    if (f->installed) return true;
    f->installed = f->install();
    return f->installed;
}

void install_enabled_filters() {
    int n = 0;
    filter* t = table(&n);
    for (int i = 0; i < n; i++)
        if (mode_of(t[i].id) != mode::off)
            ensure_installed(t[i].id);
}
}
```

- [ ] **Step 4: Run the test to verify it passes**

Run: `clang++ -std=c++17 -I src tests/protections_registry_test.cpp src/protections/registry.cpp -o build/protections_registry_test.exe && ./build/protections_registry_test.exe`
Expected: PASS — final line `all passed`, exit code 0

- [ ] **Step 5: Commit**

```bash
git add src/protections/registry.h src/protections/registry.cpp tests/protections_registry_test.cpp
git commit -m "feat(protections): filter registry with off/log/enforce gating"
```

---

### Task 3: Report drain and detour installer

**Files:**
- Create: `src/protections/report.h`, `src/protections/report.cpp`
- Create: `src/protections/detour.h`, `src/protections/detour.cpp`

**Interfaces:**
- Consumes: `protections::ring`, `protections::record` (Task 1); `protections::filter_id`, `protections::name_of` (Task 2).
- Produces:
  - `void protections::report(filter_id, int player_index, uint8_t flags, uint32_t detail_a, uint32_t detail_b)` — safe to call from any thread, never allocates.
  - `void protections::drain_reports()` — call once per frame from the script thread.
  - `uint32_t protections::total_reports()`, `uint32_t protections::total_dropped()` — counters for the menu.
  - `bool protections::install_detour(detour_slot* slot, uint64_t rva, void* hook)` and `#define PROT_CHAIN(slot, fn_type, ...)` — the detour helper. `detour_slot` wraps a GoldHEN `Detour`.

There is no `run_on_game_thread` call here: that API takes a bare `void(*)()` with no payload, which is exactly why reports go through a ring rather than a queued closure. `drain_reports` is called directly from the existing frame callback.

Notifications are rate-limited to one per second per filter. During a spam attack the log keeps every record it can hold, while the screen stays usable.

- [ ] **Step 1: Write `report.h`**

Create `src/protections/report.h`:

```cpp
#pragma once
#include <stdint.h>
#include "protections/registry.h"

// Reporting for protection filters.
//
// report() is callable from a network thread: it writes one POD record into the
// ring and returns. It does not allocate, format, log, or notify - all of that
// happens in drain_reports() on the script thread, where it is legal.
namespace protections {

    void report(filter_id id, int player_index, uint8_t flags,
                uint32_t detail_a, uint32_t detail_b);

    // Convenience for the common case: no player, no details.
    inline void report(filter_id id) { report(id, -1, 0, 0, 0); }

    // Drains the ring to the kernel log and to rate-limited notifications.
    // Call once per frame from the script thread.
    void drain_reports();

    uint32_t total_reports();
    uint32_t total_dropped();
}
```

- [ ] **Step 2: Write `report.cpp`**

Create `src/protections/report.cpp`:

```cpp
#include "protections/report.h"
#include "protections/ring.h"
#include "menu/base/util/notify.h"
#include "platform/log.h"
#include "rage/invoker/natives.h"

#include <stdio.h>

namespace protections {
namespace {
    ring& reports() {
        static ring instance;      // function-local static: no .init_array
        return instance;
    }

    uint32_t g_total   = 0;
    uint32_t g_last_notify_ms[64] = {};   // indexed by filter table position

    const uint32_t NOTIFY_INTERVAL_MS = 1000;
}

void report(filter_id id, int player_index, uint8_t flags,
            uint32_t detail_a, uint32_t detail_b)
{
    record r;
    r.filter_id    = (uint16_t)id;
    r.player_index = (player_index >= 0 && player_index < 32) ? (uint8_t)player_index : 0xFF;
    r.flags        = flags;
    r.detail_a     = detail_a;
    r.detail_b     = detail_b;

    reports().push(r);
    __atomic_fetch_add(&g_total, 1, __ATOMIC_RELAXED);
}

void drain_reports()
{
    record r;
    while (reports().pop(&r)) {
        const filter_id id   = (filter_id)r.filter_id;
        const char*     name = name_of(id);
        const bool      blocked = (mode_of(id) == mode::enforce);

        // Kernel log first: it is the channel that survives a crash, and this
        // is the record that matters when a filter takes the game down.
        if (r.player_index == 0xFF) {
            platform::klogf("prot %s %s a=%08x b=%08x",
                            blocked ? "BLOCK" : "would-block",
                            name, (unsigned)r.detail_a, (unsigned)r.detail_b);
        } else {
            platform::klogf("prot %s %s player=%u a=%08x b=%08x",
                            blocked ? "BLOCK" : "would-block",
                            name, (unsigned)r.player_index,
                            (unsigned)r.detail_a, (unsigned)r.detail_b);
        }

        // Then the screen, rate-limited. A sound-spam attack produces hundreds
        // of these per second; a notification each would be its own denial of
        // service. Find the table slot so the rate limit is per filter.
        int slot = -1;
        for (int i = 0; i < count() && i < 64; i++)
            if (at(i)->id == id) { slot = i; break; }

        if (slot < 0) continue;

        const uint32_t now = native::get_game_timer();
        if (now - g_last_notify_ms[slot] < NOTIFY_INTERVAL_MS) continue;
        g_last_notify_ms[slot] = now;

        char body[96];
        if (r.player_index == 0xFF)
            snprintf(body, sizeof(body), "%s %s", blocked ? "Blocked" : "Detected", name);
        else
            snprintf(body, sizeof(body), "%s %s from player %u",
                     blocked ? "Blocked" : "Detected", name, (unsigned)r.player_index);

        menu::notify::stacked("Protections", body);
    }
}

uint32_t total_reports() { return __atomic_load_n(&g_total, __ATOMIC_RELAXED); }
uint32_t total_dropped() { return reports().dropped(); }
}
```

`native::get_game_timer` is verified present at `src/rage/invoker/natives.h:524`
(`static int get_game_timer() { return _i<int>(0x9F63E0); }`) — a direct-RVA
wrapper, so it works from the first frame rather than waiting on the hash table.
It is the only native this file calls, and it is called from the script thread
inside `drain_reports`, never from a hook.

- [ ] **Step 3: Write the detour helper**

Create `src/protections/detour.h`:

```cpp
#pragma once
#include <stdint.h>
#include <GoldHEN/Detour.h>

// Thin wrapper over the GoldHEN Detour API, matching the pattern proven in
// rage/heap_guard.cpp. Each filter owns one slot; a slot is installed once and
// never removed.
namespace protections {

    struct detour_slot {
        Detour d;
        bool   constructed;
        bool   installed;
    };

    // Resolves `rva` against g_eboot_base and detours it to `hook`.
    // Idempotent: a second call on an installed slot returns true and does
    // nothing. Returns false if the base is unresolved or the detour failed.
    bool install_detour(detour_slot* slot, uint64_t rva, void* hook);
}

// Call the original from inside a hook. Pass a TYPEDEF'd function-pointer type,
// never an inline one: an inline `void(*)(void*, int)` contains commas, which
// the preprocessor splits into separate macro arguments.
//   typedef void (*my_fn)(void*, int);
//   PROT_CHAIN(g_slot, my_fn, a, b)
#define PROT_CHAIN(slot, ...) Detour_Stub(&(slot).d, __VA_ARGS__)
```

Create `src/protections/detour.cpp`:

```cpp
#include "protections/detour.h"
#include "rage/invoker/invoker.h"
#include "platform/log.h"

namespace protections {

bool install_detour(detour_slot* slot, uint64_t rva, void* hook)
{
    if (!slot || !hook) return false;
    if (slot->installed) return true;
    if (!rage::invoker::g_eboot_base) {
        LOG_ERROR("protections: detour @rva 0x%llx skipped - base unresolved",
                  (unsigned long long)rva);
        return false;
    }

    if (!slot->constructed) {
        Detour_Construct(&slot->d, DetourMode_x64);
        slot->constructed = true;
    }

    void* stub = Detour_DetourFunction(&slot->d,
                                       rage::invoker::g_eboot_base + rva,
                                       hook);
    slot->installed = (stub != nullptr);

    if (slot->installed)
        platform::klogf("prot detour installed @ base+0x%llx", (unsigned long long)rva);
    else
        LOG_ERROR("protections: detour @rva 0x%llx FAILED", (unsigned long long)rva);

    return slot->installed;
}
}
```

- [ ] **Step 4: Verify it compiles**

Run the plugin build (see "Build, test and deploy commands").
Expected: build succeeds. These files are compiled but not yet called by anything.

- [ ] **Step 5: Commit**

```bash
git add src/protections/report.h src/protections/report.cpp src/protections/detour.h src/protections/detour.cpp
git commit -m "feat(protections): report drain and detour installer"
```

---

### Task 4: Protections submenu, self-test filter, and frame wiring

**Files:**
- Modify: `src/menu/base/submenus/protections.h` (rewrite)
- Modify: `src/menu/base/submenus/protections.cpp` (rewrite)
- Modify: `src/menu/menu.cpp` (find `menu::tick`; add the drain call)
- Modify: `src/platform/build_tag.h` (bump the tag)

**Interfaces:**
- Consumes: everything from Tasks 1–3.
- Produces: a working Protections submenu, and `protections::drain_reports()` called once per frame.

This is the task that proves the whole report path on console with no reverse engineering: the `self_test` filter has no detour, and a menu button fires a report through it. If the notification appears and the klog line reads `prot would-block Self Test`, then ring, registry, drain, rate limit, log and notification all work — and every guard that follows is only a matter of finding its anchor.

The existing submenu's three local toggles (anti-fire, anti-ragdoll, explosion proof) are deleted here. They are self-care, not protections, and the spec moves them to `Player > Proofs`. Preserving them is out of scope for this task; if that submenu does not already offer equivalents, note it and raise it rather than silently dropping the behaviour.

- [ ] **Step 1: Rewrite the submenu header**

Replace `src/menu/base/submenus/protections.h` with:

```cpp
#pragma once
#include "menu/base/submenu.h"

// One row per protection filter, each a three-way Off / Log / Enforce.
//
// New filters default to Log: they run the detection and report it, then chain
// to the original anyway. Enforce is a separate, deliberate flip per filter once
// the log shows the detection firing only when it should.
class protections_menu : public menu::submenu::submenu {
public:
    void load() override;
    void update() override;
    static protections_menu* get();
};
```

- [ ] **Step 2: Rewrite the submenu implementation**

Replace `src/menu/base/submenus/protections.cpp` with:

```cpp
#include "menu/base/submenus/protections.h"
#include "menu/base/submenus/main.h"
#include "menu/base/options/button.h"
#include "menu/base/options/dropdown.h"
#include "menu/base/options/break.h"
#include "menu/base/util/notify.h"
#include "protections/registry.h"
#include "protections/report.h"

#include <stdio.h>

namespace {
    // Backing ints for the dropdowns. A dropdown binds to an int by reference,
    // and registry entries hold their mode as an int for exactly this reason -
    // so the menu writes the value the hooks read, with no copy to keep in sync.
    int* mode_ref(int index) { return &protections::at(index)->current; }
}

void protections_menu::load() {
    set_name("Protections");
    set_parent<main_menu>();

    add_option(break_option("Filters").ref());

    for (int i = 0; i < protections::count(); i++) {
        protections::filter* f = protections::at(i);

        add_option(dropdown_option(f->name)
            .add_index(*mode_ref(i))
            .add_item("Off")
            .add_item("Log")
            .add_item("Enforce")
            .add_tooltip("Off ignores. Log detects and reports without blocking. Enforce blocks.")
            .add_change([i](int value) {
                // Installing on demand keeps a filter's detour out of the
                // process until it is actually wanted. Idempotent.
                if (value != (int)protections::mode::off)
                    protections::ensure_installed(protections::at(i)->id);
            })
            .add_savable(get_submenu_name_stack()));
    }

    add_option(break_option("Diagnostics").ref());

    add_option(button_option("Fire Self Test")
        .add_tooltip("Pushes one report through the whole path: ring, drain, log, notification")
        .add_click([] {
            protections::report(protections::filter_id::self_test, 3, 0, 0xABCDEF01, 7);
        }));

    add_option(button_option("Report Counters")
        .add_tooltip("Total reports and how many were dropped by ring overflow")
        .add_click([] {
            char msg[96];
            snprintf(msg, sizeof(msg), "%u reported, %u dropped",
                     (unsigned)protections::total_reports(),
                     (unsigned)protections::total_dropped());
            menu::notify::stacked("Protections", msg);
        }));
}

void protections_menu::update() {
    // Nothing per-frame here. Reports are drained from menu::tick so they keep
    // flowing with the menu closed, which is when an attack actually arrives.
}

protections_menu* protections_menu::get() {
    static protections_menu instance;
    return &instance;
}
```

- [ ] **Step 3: Wire the drain into the frame callback**

In `src/menu/menu.cpp`, add the include near the other project includes:

```cpp
#include "protections/report.h"
```

Then find this line in `menu::tick` (`src/menu/menu.cpp`, around line 375):

```cpp
        global::ui::g_delta = native::get_frame_time();
```

and add the drain immediately **after** it:

```cpp
        // Placement matters twice over. It sits BELOW the overlay guard because
        // it calls a native (the timer) and the whole point of that guard is
        // that native work crashes the game while the ShellUI overlay is up.
        // It sits ABOVE the player_valid() gate because it touches nothing that
        // needs a player - and a protection can fire during loading, when the
        // gate is still closed. Records simply wait in the ring until here.
        protections::drain_reports();
```

Read the surrounding function before editing to confirm the ordering; do not
guess the insertion point. In particular, do **not** put the drain in the
ungated prologue above `if (ov) { ... return; }` — that region is explicitly
documented as memory-reads-only.

Then find the `if (game::player_valid() && ...)` block further down the same
function, and add this one-shot immediately **above** it:

```cpp
        // Detours for filters restored from config. add_savable puts the saved
        // mode back but does not fire the change handler that installs the
        // detour - so without this, a filter saved as Enforce comes back
        // reading Enforce and doing nothing. Deferred to here rather than
        // menu::build() so no detour lands while the game is still loading.
        static bool s_filters_installed = false;
        if (!s_filters_installed && game::player_valid()) {
            protections::install_enabled_filters();
            s_filters_installed = true;
        }
```

A function-local `static bool` is the right tool here and does not break the
no-`.init_array` rule: it is zero-initialised, so it needs no dynamic
initialiser and no guard variable.

- [ ] **Step 4: Bump the build tag**

In `src/platform/build_tag.h`, change the tag to:

```cpp
#define INSULIN_BUILD_TAG "prot-1"
```

- [ ] **Step 5: Build**

Run the plugin build.
Expected: build succeeds with no warnings about `protections/`.

- [ ] **Step 6: Verify on console**

1. Deploy the `.prx` (see commands above).
2. Start `nc 10.10.10.236 3232` before launching the game.
3. Launch GTA V. Confirm the klog shows `BUILD=prot-1 module_start base=0x...` — if it shows a different tag, the console is running an older build and nothing below is meaningful.
4. Open the menu, go to Protections. Expect twelve dropdown rows, each reading `Log [2/3]`, plus the two diagnostic buttons.
5. Press **Fire Self Test**.
   - Expect an on-screen notification: `Detected Self Test from player 3`.
   - Expect a klog line: `IGV prot would-block Self Test player=3 a=abcdef01 b=00000007`.
6. Set **Self Test** to `Enforce`, fire again. The notification and klog line must now read `Blocked` / `prot BLOCK`.
7. Press Fire Self Test rapidly ten times. Expect ten klog lines but roughly one notification per second — that is the rate limit working.
8. Press **Report Counters**; the reported total must match the number of times you fired.
9. Set Self Test to `Off`, fire again: nothing should be reported. (The button calls `report()` directly, so `Off` is verified at drain time by the absence of a `BLOCK`; the record still logs as `would-block`. If that reads confusingly, that is a finding worth raising, not a bug to silently fix.)
10. Quit the game and relaunch. The Protections rows must come back with the modes you left them on — that is `add_savable` round-tripping through `config.json`.

If the game closes itself at any point, capture the klog crash dump, compute `RVA = RIP - base` from the startup line, and look the RVA up in the IDB before changing anything.

- [ ] **Step 7: Commit**

```bash
git add src/menu/base/submenus/protections.h src/menu/base/submenus/protections.cpp src/menu/menu.cpp src/platform/build_tag.h
git commit -m "feat(protections): submenu, self-test filter, per-frame report drain"
```

---

## Phase B — The guards

Every guard task has the same shape, and it is worth stating once rather than eleven times:

1. **Find the PS4 anchor.** The spec's "Verified anchors" table does not cover Phase B — none of these eleven functions is located yet, and finding each one is the substance of its task. Method: open `E:\Projects\IDA\PS4\GTA5\eboot_named.i64`, then match the *shape* of the YimMenu target rather than any byte pattern. Byte signatures do not cross compilers (PC is MSVC, PS4 is clang), so the port is by meaning — the same rule `analysis/PC_SIG_PORT.md` records for the earlier signature port.
2. **Write down the evidence.** Add the RVA and the decompiled shape that justifies it to `E:\Projects\IDA\PS4\GTA5\analysis\PROTECTIONS_ANCHORS.md` (create it in the first Phase B task). An anchor with no recorded evidence is a guess that will be re-derived later at full cost.
3. **Write the hook**, gated on `should_report` first and `should_block` second.
4. **Deploy in `Log`, play, read the log.** A guard that fires during normal play is wrong, and that is the failure this phase is designed to catch cheaply.
5. **Flip to `Enforce`** only after the log is clean.

If an anchor cannot be found with confidence, **stop and report it** rather than hooking a plausible-looking function. A detour on the wrong function in the render or network path is a hard crash, and a guessed anchor is the single most likely way to cause one.

Where a guard needs a global whose offset came from the PC build (`+0x14730` for the draw-handler count, the skeleton-extension count), that constant is **not portable** and must be re-derived from the PS4 disassembly.

### Task 5: Render guards (`render_ped`, `render_entity`, `render_big_ped`)

**Files:**
- Create: `src/protections/hooks_render.cpp`
- Create: `E:\Projects\IDA\PS4\GTA5\analysis\PROTECTIONS_ANCHORS.md`
- Modify: `src/protections/registry.cpp` (fill in three `install` pointers)
- Modify: `src/platform/build_tag.h`

**Interfaces:**
- Consumes: `protections::install_detour`, `PROT_CHAIN` (Task 3); `should_report`, `should_block`, `ensure_installed` (Task 2); `protections::report` (Task 3).
- Produces: `bool protections::install_render_ped()`, `install_render_entity()`, `install_render_big_ped()` — each matching the `bool(*)()` in the registry's `install` field.

These three share one dependency — the draw-handler manager global and the count offset within it — which is why they are one task. YimMenu reads `*(int*)((__int64)(*m_draw_handler_mgr) + 0x14730)` and bails when it reaches 499 (ped) or 512 (entity, big ped): the crash is a draw-list overflow, and the guard refuses to add the entry that would overflow it.

- [ ] **Step 1: Locate the draw-handler manager and count offset**

In the IDB, find the per-frame draw-list add path. Start from the render entity list: search for the function that increments a counter and stores an entry into a large fixed array indexed by that counter, called once per rendered entity. Confirm the array capacity is 512 from the bound check in the game's own code — that number is the guard's threshold and reading it out of the disassembly is what makes the constant trustworthy.

Record in `PROTECTIONS_ANCHORS.md`:
- the manager global's RVA,
- the count field's offset within it,
- the capacity the game itself checks against,
- the three function RVAs, each with the decompiled shape that identifies it.

Acceptance for this step: three RVAs and one global, each with a decompiled listing pasted into the anchors file showing why it is that function. If any cannot be identified with confidence, stop and report.

- [ ] **Step 2: Write the hooks**

Create `src/protections/hooks_render.cpp`. Substitute the RVAs found in Step 1
for the three constants; the `COUNT_OFFSET` and `CAPACITY` values likewise come
from Step 1, **not** from YimMenu's PC constants.

```cpp
#include "protections/detour.h"
#include "protections/registry.h"
#include "protections/report.h"
#include "rage/invoker/invoker.h"

// Draw-list overflow guards. A remote player can push the render entity list
// past its capacity; the game does not bound-check the add, and the overflow
// corrupts whatever follows the array. Each guard refuses the add that would
// cross the line, which costs one missing entity for one frame.
//
// The count offset and capacity below are read out of the PS4 disassembly, not
// carried over from the PC build - see analysis/PROTECTIONS_ANCHORS.md.
namespace protections {
namespace {
    // TODO-ANCHOR: replace with the RVAs recorded in PROTECTIONS_ANCHORS.md.
    const uint64_t RVA_RENDER_PED      = 0;
    const uint64_t RVA_RENDER_ENTITY   = 0;
    const uint64_t RVA_RENDER_BIG_PED  = 0;
    const uint64_t RVA_DRAW_HANDLER_MGR = 0;
    const uint64_t COUNT_OFFSET        = 0;
    const int      CAPACITY            = 512;
    const int      PED_HEADROOM        = 13;   // YimMenu bails at 499 of 512

    typedef void* (*render_ped_fn)(void*, void*, void*, void*);
    typedef void  (*render_entity_fn)(void*, void*, int, bool);
    typedef void* (*render_big_ped_fn)(void*, void*, void*, void*);

    detour_slot g_ped;
    detour_slot g_entity;
    detour_slot g_big_ped;

    int draw_list_count() {
        if (!RVA_DRAW_HANDLER_MGR) return 0;
        void** mgr_ptr = (void**)(rage::invoker::g_eboot_base + RVA_DRAW_HANDLER_MGR);
        void*  mgr     = *mgr_ptr;
        if (!mgr) return 0;
        return *(int*)((uint8_t*)mgr + COUNT_OFFSET);
    }

    void* render_ped_hook(void* renderer, void* ped, void* a3, void* a4) {
        if (should_report(filter_id::render_ped) &&
            draw_list_count() >= CAPACITY - PED_HEADROOM) {
            report(filter_id::render_ped, -1, 0, (uint32_t)draw_list_count(), 0);
            if (should_block(filter_id::render_ped))
                return nullptr;
        }
        return PROT_CHAIN(g_ped, render_ped_fn, renderer, ped, a3, a4);
    }

    void render_entity_hook(void* renderer, void* entity, int unk, bool a4) {
        if (should_report(filter_id::render_entity) && draw_list_count() >= CAPACITY) {
            report(filter_id::render_entity, -1, 0, (uint32_t)draw_list_count(), 0);
            if (should_block(filter_id::render_entity)) {
                // The sentinel the caller expects for "no entry produced".
                int* flags = (int*)((uint8_t*)renderer + 4);
                *flags &= ~0x80000000;
                *flags &= ~0x40000000;
                *flags |= (a4 & 1) << 30;
                *(int*)renderer = -2;
                return;
            }
        }
        PROT_CHAIN(g_entity, render_entity_fn, renderer, entity, unk, a4);
    }

    void* render_big_ped_hook(void* renderer, void* ped, void* a3, void* a4) {
        if (should_report(filter_id::render_big_ped) && draw_list_count() >= CAPACITY) {
            report(filter_id::render_big_ped, -1, 0, (uint32_t)draw_list_count(), 0);
            if (should_block(filter_id::render_big_ped)) {
                *(int*)((uint8_t*)a4 + 4) = -2;
                return (uint8_t*)a4 + 0x14;
            }
        }
        return PROT_CHAIN(g_big_ped, render_big_ped_fn, renderer, ped, a3, a4);
    }
}

bool install_render_ped()     { return install_detour(&g_ped,     RVA_RENDER_PED,     (void*)&render_ped_hook); }
bool install_render_entity()  { return install_detour(&g_entity,  RVA_RENDER_ENTITY,  (void*)&render_entity_hook); }
bool install_render_big_ped() { return install_detour(&g_big_ped, RVA_RENDER_BIG_PED, (void*)&render_big_ped_hook); }
}
```

The `TODO-ANCHOR` markers must all be replaced with real values in this same
task. A commit that still contains a zero RVA is not complete — `install_detour`
would refuse it, and the filter would silently never install.

- [ ] **Step 3: Declare the installers and wire them into the registry**

Add to `src/protections/registry.h`, inside `namespace protections`, after
`ensure_installed`:

```cpp
    // Guard installers. Each matches the registry's install field.
    bool install_render_ped();
    bool install_render_entity();
    bool install_render_big_ped();
```

Then in `src/protections/registry.cpp`, replace the three `nullptr` install
pointers in the table with the matching function:

```cpp
            { filter_id::render_ped,     "Render Ped",     1, (int)mode::log, (int)mode::log, false, &install_render_ped },
            { filter_id::render_entity,  "Render Entity",  1, (int)mode::log, (int)mode::log, false, &install_render_entity },
            { filter_id::render_big_ped, "Render Big Ped", 1, (int)mode::log, (int)mode::log, false, &install_render_big_ped },
```

- [ ] **Step 4: Re-run the host tests**

Run:
```bash
clang++ -std=c++17 -I src tests/protections_registry_test.cpp src/protections/registry.cpp -o build/protections_registry_test.exe && ./build/protections_registry_test.exe
```
Expected: PASS. The registry test asserts every filter still defaults to `log`
and that names stay unique, which is exactly what a table edit can break.

- [ ] **Step 5: Build and bump the tag**

Set `INSULIN_BUILD_TAG` to `"prot-render-1"`, then run the plugin build.
Expected: build succeeds.

- [ ] **Step 6: Verify on console in Log mode**

1. Deploy, watch klog, launch the game.
2. Confirm the build tag line reads `prot-render-1`.
3. Set all three render filters to `Log`. Confirm klog shows three
   `prot detour installed @ base+0x...` lines with the RVAs from Step 1.
4. Play normally for ten minutes — drive through the city, spawn a crowd of
   peds, enter and leave interiors.
5. **Expect zero `prot would-block Render *` lines.** The draw list should not
   approach capacity in ordinary play. Any report here means the threshold or
   the count offset is wrong; do not proceed to Enforce, and re-check Step 1.
6. Confirm the game's framerate is unchanged. These are per-entity hooks on the
   render path and are the most performance-sensitive code in Tier 1.

- [ ] **Step 7: Commit**

```bash
git add src/protections/hooks_render.cpp src/protections/registry.h src/protections/registry.cpp src/platform/build_tag.h
git commit -m "feat(protections): draw-list overflow guards for ped and entity rendering"
```

Also commit the anchors file in the IDA repo:

```bash
cd /e/Projects/IDA/PS4/GTA5 && git add analysis/PROTECTIONS_ANCHORS.md && \
  git commit -m "re(gtav-ps4): draw-handler manager and the three render entry points"
```

---

### Task 6: Task guards (`task_ambient_clips`, `task_parachute`)

**Files:**
- Create: `src/protections/hooks_tasks.cpp`
- Modify: `src/protections/registry.h`, `src/protections/registry.cpp`
- Modify: `E:\Projects\IDA\PS4\GTA5\analysis\PROTECTIONS_ANCHORS.md`
- Modify: `src/platform/build_tag.h`

**Interfaces:**
- Consumes: `install_detour`, `PROT_CHAIN`, `should_report`, `should_block`, `report`.
- Produces: `bool protections::install_task_ambient_clips()`, `bool protections::install_task_parachute()`.

Both are `CTask` update functions that dereference a pointer a remote sync can leave null. They are one task because they share a shape and a search method.

- [ ] **Step 1: Locate both functions**

`task_parachute` has a usable lead: the string `ProcessCloneParachuteObjectCollision`
is referenced by functions at `0xE3A3B0` and `0xE3B7A0`. The target is the clone
task update near those; YimMenu's version keys on `a2 == 1 && a3 == 1` and walks
`this+0x10 → +0x50 → +0x40`. Confirm the same three-step chain exists in the PS4
function before accepting it.

`task_ambient_clips` has no string anchor. Find it through the task-type table:
`CTaskAmbientClips` is a concrete `CTask` subclass, so locate its vtable via the
task factory or the task-type enum, then take the update slot. YimMenu's version
requires `this+0x100` non-null. Its own comment says the guard is incomplete
("this doesn't block the crash completely") — port it as-is and do not try to
improve it here; a partial guard whose limits are known beats an invented one.

Record both RVAs with evidence in `PROTECTIONS_ANCHORS.md`. If either resists
identification, land the one you found and report the other rather than guessing.

- [ ] **Step 2: Write the hooks**

Create `src/protections/hooks_tasks.cpp`:

```cpp
#include "protections/detour.h"
#include "protections/registry.h"
#include "protections/report.h"

// Two CTask update functions that dereference pointers a remote sync can leave
// null. Both guards are null checks on the exact chain the original walks.
namespace protections {
namespace {
    // TODO-ANCHOR: replace with the RVAs recorded in PROTECTIONS_ANCHORS.md.
    const uint64_t RVA_TASK_AMBIENT_CLIPS = 0;
    const uint64_t RVA_TASK_PARACHUTE     = 0;

    typedef int (*task_update_fn)(uint64_t, int, int);

    detour_slot g_ambient;
    detour_slot g_parachute;

    int task_ambient_clips_hook(uint64_t self, int a2, int a3) {
        if (should_report(filter_id::task_ambient_clips) &&
            *(uint64_t*)(self + 0x100) == 0) {
            report(filter_id::task_ambient_clips, -1, 0, (uint32_t)a2, (uint32_t)a3);
            if (should_block(filter_id::task_ambient_clips))
                return 0;
        }
        return PROT_CHAIN(g_ambient, task_update_fn, self, a2, a3);
    }

    int task_parachute_hook(uint64_t self, int a2, int a3) {
        // Only the (1,1) path reaches the crashing code.
        if (a2 == 1 && a3 == 1 && should_report(filter_id::task_parachute)) {
            uint64_t p1 = *(uint64_t*)(self + 0x10);
            uint64_t p2 = p1 ? *(uint64_t*)(p1 + 0x50) : 0;
            uint64_t p3 = p2 ? *(uint64_t*)(p2 + 0x40) : 0;
            if (!p3) {
                report(filter_id::task_parachute, -1, 0, (uint32_t)(p1 != 0), (uint32_t)(p2 != 0));
                if (should_block(filter_id::task_parachute))
                    return 0;
            }
        }
        return PROT_CHAIN(g_parachute, task_update_fn, self, a2, a3);
    }
}

bool install_task_ambient_clips() { return install_detour(&g_ambient,   RVA_TASK_AMBIENT_CLIPS, (void*)&task_ambient_clips_hook); }
bool install_task_parachute()     { return install_detour(&g_parachute, RVA_TASK_PARACHUTE,     (void*)&task_parachute_hook); }
}
```

- [ ] **Step 3: Wire into the registry**

Add to `src/protections/registry.h`:

```cpp
    bool install_task_ambient_clips();
    bool install_task_parachute();
```

And in `src/protections/registry.cpp`, set the two install pointers:

```cpp
            { filter_id::task_ambient_clips, "Task Ambient Clips", 1, (int)mode::log, (int)mode::log, false, &install_task_ambient_clips },
            { filter_id::task_parachute,     "Task Parachute",     1, (int)mode::log, (int)mode::log, false, &install_task_parachute },
```

- [ ] **Step 4: Re-run the host tests**

Run:
```bash
clang++ -std=c++17 -I src tests/protections_registry_test.cpp src/protections/registry.cpp -o build/protections_registry_test.exe && ./build/protections_registry_test.exe
```
Expected: PASS.

- [ ] **Step 5: Build, bump the tag, verify on console**

Set `INSULIN_BUILD_TAG` to `"prot-tasks-1"`, build, deploy.

1. Set both filters to `Log`; confirm two `prot detour installed` klog lines.
2. Play for ten minutes including a parachute jump and normal ped ambient
   behaviour in a crowd.
3. **Reports are expected here — this is the corrected expectation.** An earlier
   draft of this step said "expect zero reports; a report means the offset is
   wrong". That is wrong for both filters, and acting on it would back out a
   verified anchor:

   - **Task Ambient Clips WILL fire in ordinary play.** The game treats a null
     anims group as legal — `CTaskAmbientClips::Start_OnUpdate` null-checks the
     very field the guard tests. This is YimMenu's known imprecision, not a bad
     offset. Keep it on `Log`; do **not** promote it to `Enforce` without a
     deliberate test watching ped idle animations, because returning 0 from
     `UpdateFSM` skips the whole switch, `TaskSetState` is never called, and the
     ped is pinned in `State_Start` with no ambient clip ever chosen.
   - **Task Parachute may fire benignly.** The guard trips when any of the three
     links is null, but retail already null-checks links 2 and 3, so a null there
     is a legal transient. Read the klog detail field: `a=00000000` means link 1
     — the one retail never checks — was null, which is the fatal case the guard
     exists to stop. `a=00000001` is the game's own transient and is **not**
     grounds to back the anchor out.
4. Flip both to `Enforce`, repeat the parachute jump, and confirm the jump still
   works normally — a guard that breaks the legitimate path is worse than the
   crash it prevents.

- [ ] **Step 6: Commit**

```bash
git add src/protections/hooks_tasks.cpp src/protections/registry.h src/protections/registry.cpp src/platform/build_tag.h
git commit -m "feat(protections): null guards for ambient-clip and parachute clone tasks"
cd /e/Projects/IDA/PS4/GTA5 && git add analysis/PROTECTIONS_ANCHORS.md && \
  git commit -m "re(gtav-ps4): ambient-clip and parachute clone task update anchors"
```

---

### Task 7: Pointer-chain guards (`fragment_physics`, `invalid_decal`, `searchlight`)

**Files:**
- Create: `src/protections/hooks_chains.cpp`
- Modify: `src/protections/registry.h`, `src/protections/registry.cpp`
- Modify: `E:\Projects\IDA\PS4\GTA5\analysis\PROTECTIONS_ANCHORS.md`
- Modify: `src/platform/build_tag.h`

**Interfaces:**
- Consumes: `install_detour`, `PROT_CHAIN`, `should_report`, `should_block`, `report`.
- Produces: `bool protections::install_fragment_physics()`, `install_invalid_decal()`, `install_searchlight()`.

Three unrelated crash sites that share one shape: the original walks a pointer chain that a crafted sync can break, and the guard refuses the call when the chain does not hold.

- [ ] **Step 1: Locate all three**

`searchlight` has the strongest lead: the strings `defaultsearchlight`,
`helisearchlight` and `boatsearchlight` are all referenced from `0x136C7D0`,
which is the searchlight config/factory. The target takes `(void*, CPed*)` and
YimMenu guards it with a "does this ped have a searchlight" accessor — find that
accessor near `0x136C7D0` and then its caller that matches the two-argument shape.

`invalid_decal` takes `(uintptr_t, int)` and crashes when `a2 == 2` and the chain
`+0x48 → +0x30 → +0x2C8` ends in null. Find it via the decal system: the strings
`ptxu_Decal` and `ptxu_DecalPool` are referenced from `0x1429E80`.

`fragment_physics_crash_2(float*, float*)` has no string anchor and is the
hardest of the three. It is a small leaf function taking two float pointers,
called from the fragment physics update. If it cannot be identified with
confidence, **land the other two and report this one as unfound** — do not hook
a plausible candidate.

Record every RVA with evidence in `PROTECTIONS_ANCHORS.md`.

- [ ] **Step 2: Write the hooks**

Create `src/protections/hooks_chains.cpp`:

```cpp
#include "protections/detour.h"
#include "protections/registry.h"
#include "protections/report.h"
#include "rage/invoker/invoker.h"

// Three crash sites that share a shape: the original walks a pointer chain a
// crafted sync can break, and the guard declines the call when it does not hold.
namespace protections {
namespace {
    // TODO-ANCHOR: replace with the RVAs recorded in PROTECTIONS_ANCHORS.md.
    const uint64_t RVA_FRAGMENT_PHYSICS = 0;
    const uint64_t RVA_INVALID_DECAL    = 0;
    const uint64_t RVA_SEARCHLIGHT      = 0;
    const uint64_t RVA_GET_SEARCHLIGHT  = 0;   // the accessor found in Step 1

    typedef bool (*fragment_physics_fn)(float*, float*);
    typedef void (*invalid_decal_fn)(uintptr_t, int);
    typedef void (*searchlight_fn)(void*, void*);
    typedef void* (*get_searchlight_fn)(void*);

    detour_slot g_fragment;
    detour_slot g_decal;
    detour_slot g_searchlight;

    bool fragment_physics_hook(float* a1, float* a2) {
        if (should_report(filter_id::fragment_physics) && (!a1 || !a2)) {
            report(filter_id::fragment_physics, -1, 0, (uint32_t)(a1 != nullptr), (uint32_t)(a2 != nullptr));
            if (should_block(filter_id::fragment_physics))
                return false;
        }
        return PROT_CHAIN(g_fragment, fragment_physics_fn, a1, a2);
    }

    void invalid_decal_hook(uintptr_t a1, int a2) {
        if (a1 && a2 == 2 && should_report(filter_id::invalid_decal)) {
            uintptr_t p1 = *(uintptr_t*)(a1 + 0x48);
            uintptr_t p2 = p1 ? *(uintptr_t*)(p1 + 0x30) : 0;
            if (p2 && *(uintptr_t*)(p2 + 0x2C8) == 0) {
                report(filter_id::invalid_decal, -1, 0, (uint32_t)a2, 0);
                if (should_block(filter_id::invalid_decal))
                    return;
            }
        }
        PROT_CHAIN(g_decal, invalid_decal_fn, a1, a2);
    }

    void searchlight_hook(void* a1, void* ped) {
        if (should_report(filter_id::searchlight)) {
            get_searchlight_fn get = (get_searchlight_fn)(rage::invoker::g_eboot_base + RVA_GET_SEARCHLIGHT);
            if (!ped || !get(ped)) {
                report(filter_id::searchlight, -1, 0, (uint32_t)(ped != nullptr), 0);
                if (should_block(filter_id::searchlight))
                    return;
            }
        }
        PROT_CHAIN(g_searchlight, searchlight_fn, a1, ped);
    }
}

bool install_fragment_physics() { return install_detour(&g_fragment,    RVA_FRAGMENT_PHYSICS, (void*)&fragment_physics_hook); }
bool install_invalid_decal()    { return install_detour(&g_decal,       RVA_INVALID_DECAL,    (void*)&invalid_decal_hook); }
bool install_searchlight()      { return install_detour(&g_searchlight, RVA_SEARCHLIGHT,      (void*)&searchlight_hook); }
}
```

Note the deliberate deviation from YimMenu in `searchlight_hook`: calling the
accessor is calling into the game from a hook, which the skill's probe discipline
warns against. It is accepted here because the accessor is a pure getter with no
side effects — confirm that from its disassembly in Step 1 before shipping. If it
turns out to do more than read a field, read the field directly instead.

- [ ] **Step 3: Wire into the registry**

Add to `src/protections/registry.h`:

```cpp
    bool install_fragment_physics();
    bool install_invalid_decal();
    bool install_searchlight();
```

And set the three install pointers in `src/protections/registry.cpp`:

```cpp
            { filter_id::fragment_physics, "Fragment Physics", 1, (int)mode::log, (int)mode::log, false, &install_fragment_physics },
            { filter_id::invalid_decal,    "Invalid Decal",    1, (int)mode::log, (int)mode::log, false, &install_invalid_decal },
            { filter_id::searchlight,      "Searchlight",      1, (int)mode::log, (int)mode::log, false, &install_searchlight },
```

- [ ] **Step 4: Re-run the host tests**

Run:
```bash
clang++ -std=c++17 -I src tests/protections_registry_test.cpp src/protections/registry.cpp -o build/protections_registry_test.exe && ./build/protections_registry_test.exe
```
Expected: PASS.

- [ ] **Step 5: Build, bump the tag, verify on console**

Set `INSULIN_BUILD_TAG` to `"prot-chains-1"`, build, deploy.

1. Set the landed filters to `Log`; confirm one `prot detour installed` line each.
2. Play for ten minutes. Include a police helicopter chase at night (searchlight),
   shooting walls and vehicles (decals), and a heavy vehicle crash (fragment
   physics).
3. **Expect zero reports.** Each guard fires only on a state the game should not
   reach locally.
4. Flip to `Enforce` and repeat the same three activities. Searchlights must still
   track, bullet holes must still appear, and crashes must still deform vehicles.

- [ ] **Step 6: Commit**

```bash
git add src/protections/hooks_chains.cpp src/protections/registry.h src/protections/registry.cpp src/platform/build_tag.h
git commit -m "feat(protections): pointer-chain guards for fragment physics, decals and searchlights"
cd /e/Projects/IDA/PS4/GTA5 && git add analysis/PROTECTIONS_ANCHORS.md && \
  git commit -m "re(gtav-ps4): fragment physics, decal and searchlight crash-site anchors"
```

---

### Task 8: Counter guards (`skeleton_extension`, `pool_exhaustion`)

**Files:**
- Create: `src/protections/hooks_counters.cpp`
- Modify: `src/protections/registry.h`, `src/protections/registry.cpp`
- Modify: `E:\Projects\IDA\PS4\GTA5\analysis\PROTECTIONS_ANCHORS.md`
- Modify: `src/platform/build_tag.h`

**Interfaces:**
- Consumes: `install_detour`, `PROT_CHAIN`, `should_report`, `should_block`, `report`.
- Produces: `bool protections::install_skeleton_extension()`, `bool protections::install_pool_exhaustion()`.

Both watch a counter rather than a pointer chain. `pool_exhaustion` is
**log-only by design** — it never blocks. YimMenu's version calls `LOGF(FATAL)`
on a failed pool allocation; killing the process on pool exhaustion is a worse
outcome than the exhaustion, so this port reports the caller and returns whatever
the original returned.

- [ ] **Step 1: Locate both functions**

`skeleton_extension`: find the function that adds a skeleton extension to an
entity and the global counting how many exist. YimMenu refuses at 32. Do not
assume 32 — find the array's real capacity in the PS4 code, the same way Task 5
takes its capacity from the game's own bound check.

`pool_exhaustion`: find the generic pool-allocate function — it takes a pool
pointer, pops a free-list entry, and returns null when the pool is empty. It is
heavily called, so confirm it by its callers being pool constructors across many
unrelated systems.

Record both, plus the counter global and the real capacity, in
`PROTECTIONS_ANCHORS.md`.

- [ ] **Step 2: Write the hooks**

Create `src/protections/hooks_counters.cpp`:

```cpp
#include "protections/detour.h"
#include "protections/registry.h"
#include "protections/report.h"
#include "rage/invoker/invoker.h"

// Two counter watchers.
//
// pool_exhaustion never blocks: there is nothing useful to return when a pool is
// empty, and YimMenu's LOGF(FATAL) trades a recoverable degradation for a
// guaranteed process kill. Reporting the caller is the whole value - it names
// the pool that ran dry, which is the thing you actually need to know.
namespace protections {
namespace {
    // TODO-ANCHOR: replace with the RVAs recorded in PROTECTIONS_ANCHORS.md.
    const uint64_t RVA_ADD_SKELETON_EXTENSION = 0;
    const uint64_t RVA_SKELETON_EXT_COUNT     = 0;
    const int      SKELETON_EXT_CAPACITY      = 32;   // confirm from PS4 code
    const uint64_t RVA_CREATE_POOL_ITEM       = 0;

    typedef void* (*add_skeleton_extension_fn)(void*);
    typedef void* (*create_pool_item_fn)(void*);

    detour_slot g_skeleton;
    detour_slot g_pool;

    void* add_skeleton_extension_hook(void* entity) {
        if (should_report(filter_id::skeleton_extension)) {
            const int n = *(int*)(rage::invoker::g_eboot_base + RVA_SKELETON_EXT_COUNT);
            if (n >= SKELETON_EXT_CAPACITY) {
                report(filter_id::skeleton_extension, -1, 0, (uint32_t)n, 0);
                if (should_block(filter_id::skeleton_extension))
                    return nullptr;
            }
        }
        return PROT_CHAIN(g_skeleton, add_skeleton_extension_fn, entity);
    }

    void* create_pool_item_hook(void* pool) {
        void* item = PROT_CHAIN(g_pool, create_pool_item_fn, pool);

        // Log-only, always. Never blocks - see the note above.
        if (!item && should_report(filter_id::pool_exhaustion)) {
            const uint64_t ra  = (uint64_t)__builtin_return_address(0);
            const uint64_t rva = ra - rage::invoker::g_eboot_base;
            report(filter_id::pool_exhaustion, -1, 0,
                   (uint32_t)(rva & 0xFFFFFFFF), (uint32_t)(rva >> 32));
        }
        return item;
    }
}

bool install_skeleton_extension() { return install_detour(&g_skeleton, RVA_ADD_SKELETON_EXTENSION, (void*)&add_skeleton_extension_hook); }
bool install_pool_exhaustion()    { return install_detour(&g_pool,     RVA_CREATE_POOL_ITEM,       (void*)&create_pool_item_hook); }
}
```

`__builtin_return_address(0)` gives the caller because the detour replaces the
prologue and the original call's return address is still on the stack — the same
technique `rage/heap_guard.cpp` documents. It is logged as an RVA so it drops
straight into the IDB.

- [ ] **Step 3: Wire into the registry**

Add to `src/protections/registry.h`:

```cpp
    bool install_skeleton_extension();
    bool install_pool_exhaustion();
```

And set the two install pointers in `src/protections/registry.cpp`:

```cpp
            { filter_id::skeleton_extension, "Skeleton Extension", 1, (int)mode::log, (int)mode::log, false, &install_skeleton_extension },
            { filter_id::pool_exhaustion,    "Pool Exhaustion",    1, (int)mode::log, (int)mode::log, false, &install_pool_exhaustion },
```

- [ ] **Step 4: Re-run the host tests**

Run:
```bash
clang++ -std=c++17 -I src tests/protections_registry_test.cpp src/protections/registry.cpp -o build/protections_registry_test.exe && ./build/protections_registry_test.exe
```
Expected: PASS.

- [ ] **Step 5: Build, bump the tag, verify on console**

Set `INSULIN_BUILD_TAG` to `"prot-counters-1"`, build, deploy.

1. Set both to `Log`. `create_pool_item` is a hot function — watch the framerate
   closely for the first minute. If it drops measurably, say so and stop: a hot
   hook is exactly the case the skill warns about, and the fix is to narrow the
   hook, not to accept the cost.
2. Play for ten minutes with heavy traffic and many spawned vehicles.
3. **Expect zero skeleton reports.** Pool reports may legitimately appear under
   heavy load; each names the caller RVA, which is the diagnostic. Note any that
   appear.
4. Flip `skeleton_extension` to `Enforce`. Leave `pool_exhaustion` on `Log` — it
   has no enforce behaviour, which is intentional and documented in the source.

- [ ] **Step 6: Commit**

```bash
git add src/protections/hooks_counters.cpp src/protections/registry.h src/protections/registry.cpp src/platform/build_tag.h
git commit -m "feat(protections): skeleton-extension cap and pool-exhaustion reporting"
cd /e/Projects/IDA/PS4/GTA5 && git add analysis/PROTECTIONS_ANCHORS.md && \
  git commit -m "re(gtav-ps4): skeleton extension counter and generic pool allocator anchors"
```

---

### Task 9: Reliable-message allocator recovery (`reliable_alloc`)

**Files:**
- Create: `src/protections/hooks_reliable_alloc.cpp`
- Modify: `src/protections/registry.h`, `src/protections/registry.cpp`
- Modify: `E:\Projects\IDA\PS4\GTA5\analysis\PROTECTIONS_ANCHORS.md`
- Modify: `src/platform/build_tag.h`

**Interfaces:**
- Consumes: `install_detour`, `PROT_CHAIN`, `should_report`, `should_block`, `report`.
- Produces: `bool protections::install_reliable_alloc()`.

This is the only Tier 1 filter that is a recovery path rather than a guard, and
the most intricate. It is last for that reason. When the network message
allocator cannot satisfy a request, YimMenu asks the connection manager to free
memory, retries, and if that fails walks both message queues freeing everything
including unacked reliables.

**This one is genuinely risky.** It frees live network messages on a network
thread. Treat `Enforce` here as a separate decision from every other filter in
this tier, and do not flip it on the same session you first install it.

- [ ] **Step 1: Map the allocator and connection structures**

Needed, all currently unmapped:
- the allocate-for-reliable-message function (the detour target),
- `rage::sysMemAllocator::Allocate` and `GetMemoryAvailable` — `g_sysMemAllocator`
  is already known at `0x311BD80`, so start from its vtable,
- the connection's allocator field offset,
- the connection-manager "try free memory" function,
- the normal message queue and reliables resend queue offsets and their counts,
- the remove-from-queue and remove-from-unacked-reliables functions.

Record every one in `PROTECTIONS_ANCHORS.md` with evidence. This is the largest
RE surface in Tier 1. If it stalls, **land Tier 1 without this filter and report
it** — the other ten are independently valuable, and Tier 2 does not depend on
this one.

- [ ] **Step 2: Write the hook, recovery disabled**

Create `src/protections/hooks_reliable_alloc.cpp` with the detection only: on a
failed allocation, report the requested size and available space, then return
what the original returned. Do **not** write the queue-walking recovery yet.

```cpp
#include "protections/detour.h"
#include "protections/registry.h"
#include "protections/report.h"

// Recovery for network-message allocator exhaustion.
//
// Staged deliberately: this version only detects. The recovery walks live
// message queues and frees unacked reliables on a network thread, which is the
// most dangerous thing anywhere in Tier 1, and it should not be written until
// the detection has been seen to fire correctly on a real session.
namespace protections {
namespace {
    // TODO-ANCHOR: replace with the RVAs recorded in PROTECTIONS_ANCHORS.md.
    const uint64_t RVA_ALLOCATE_RELIABLE = 0;

    typedef void* (*allocate_reliable_fn)(void*, int);

    detour_slot g_alloc;

    void* allocate_reliable_hook(void* cxn, int required) {
        void* mem = PROT_CHAIN(g_alloc, allocate_reliable_fn, cxn, required);

        if (!mem && should_report(filter_id::reliable_alloc))
            report(filter_id::reliable_alloc, -1, 0, (uint32_t)required, 0);

        return mem;
    }
}

bool install_reliable_alloc() { return install_detour(&g_alloc, RVA_ALLOCATE_RELIABLE, (void*)&allocate_reliable_hook); }
}
```

- [ ] **Step 3: Wire into the registry**

Add to `src/protections/registry.h`:

```cpp
    bool install_reliable_alloc();
```

And set the install pointer in `src/protections/registry.cpp`:

```cpp
            { filter_id::reliable_alloc, "Reliable Allocator", 1, (int)mode::log, (int)mode::log, false, &install_reliable_alloc },
```

- [ ] **Step 4: Re-run the host tests**

Run:
```bash
clang++ -std=c++17 -I src tests/protections_registry_test.cpp src/protections/registry.cpp -o build/protections_registry_test.exe && ./build/protections_registry_test.exe
```
Expected: PASS.

- [ ] **Step 5: Build, bump the tag, verify detection on console**

Set `INSULIN_BUILD_TAG` to `"prot-relalloc-1"`, build, deploy.

1. Set to `Log`. Confirm the detour installs.
2. Join an LSO session and play for twenty minutes.
3. **Expect zero reports** in a healthy session. A report means the allocator is
   genuinely running dry, which is itself the finding this filter exists to
   surface.

- [ ] **Step 6: Report and stop**

Write up what the detection showed. **Do not implement the recovery path in this
task.** If the detection never fired, the recovery has no observed problem to
solve, and writing it would be speculative work in the most dangerous place in
the tier — raise it as a decision rather than proceeding.

- [ ] **Step 7: Commit**

```bash
git add src/protections/hooks_reliable_alloc.cpp src/protections/registry.h src/protections/registry.cpp src/platform/build_tag.h
git commit -m "feat(protections): detect network message allocator exhaustion"
cd /e/Projects/IDA/PS4/GTA5 && git add analysis/PROTECTIONS_ANCHORS.md && \
  git commit -m "re(gtav-ps4): reliable message allocator anchor"
```

---

## Tier 1 completion criteria

From the spec's success criteria, the ones this tier owns:

1. The framework installs and the game runs normally with the **Enforce-eligible**
   guards in `Enforce`. No crash, no measurable frame cost.

   Amended after the final whole-branch review. The original wording was "with
   all guards in `Enforce`", which the landed subsystem cannot satisfy - not
   because it fell short, but because four of the twelve rows were never going
   to have a meaningful `Enforce`, and saying otherwise would make the criterion
   unpassable-by-construction and therefore useless as a gate.

   **Enforce-eligible (7)** - these are what criterion 1 covers:

   | filter | why it can enforce |
   |---|---|
   | `skeleton_extension` | the sole caller already null-tests the return |
   | `invalid_decal` | retail's own null check skips the body on that branch anyway |
   | `searchlight` | a report means the call would have faulted |
   | `task_parachute` | returns `FSM_Continue`, the original's own value on that path |
   | `render_ped` | bails 13 slots early, costs one entity for one frame |
   | `render_entity` | writes the sentinel the caller expects for "no entry produced" |
   | `render_big_ped` | same shape - but see below |

   **Not Enforce-eligible (5)**, each for a different reason:

   - `self_test` - no detour. It is the report path's own end-to-end probe;
     there is no call to refuse.
   - `fragment_physics` - the PS4 twin of `fragment_physics_crash_2` was not
     identified, so there is nothing to hook (anchors §10).
   - `pool_exhaustion` - `rage::fwBasePool::New` is identified at `0x1EF6A00`
     but cannot be detoured safely: the 15-byte steal contains a `jz rel8` that
     GoldHEN's stub does not relocate (anchors §12).
   - `reliable_alloc` - has a detour, but retail null-checks every
     `AllocCritical` return, so there is nothing to block. The hook never calls
     `should_block()`; the recovery is deliberately unwritten.
   - `task_ambient_clips` - hookable and blocking, but it **must stay on `Log`**.
     The game itself treats a null anims group as legal (`Start_OnUpdate`
     null-checks the same field), so the filter is expected to fire in ordinary
     play. YimMenu says of its own version that it does not block the crash
     completely. Enforcing it would drop legitimate task updates.

   The first four carry `can_block = false` in the registry, which is what makes
   the menu offer them `Off`/`Log` only and stops the drain printing `BLOCK` for
   something nothing blocked. `task_ambient_clips` is the one row where the
   restraint is a documented judgement rather than a table field: it *can*
   block, and must not be asked to.

   One caveat inside the eligible set: **`render_big_ped`'s `Enforce` should be
   the last one flipped.** Its block path writes only the index sentinel
   (`*(a4 + 4) = -2`) and returns `a4 + 0x14`, leaving the rest of the
   out-buffer (`+0`, `+2`, `+12`, `+16`, `+17`) as the caller left it, whereas
   `render_entity` reproduces its target's `!v8` exit field for field. Nothing
   says that is wrong - the caller is documented as reading the index at `+4` -
   but it is the one enforcement path on the branch that was not matched
   store-for-store against the original's own early exit.
2. The self-test filter demonstrates the full report path: ring → drain → klog →
   rate-limited notification.
3. Every filter's mode persists across a game restart.
4. Ten minutes of normal play produces **zero** reports from any guard in `Log`
   mode — the false-positive check, and the precondition for `Enforce`.
5. Every anchor used is recorded in `analysis/PROTECTIONS_ANCHORS.md` with the
   decompiled evidence that justifies it.

Filters that could not be anchored are reported as unfound, not guessed. Tier 2
does not depend on any of them.
