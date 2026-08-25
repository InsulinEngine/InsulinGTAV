# Protections Tier 2 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Filter hostile network *events* — the script-event family through one hook, plus seventeen individually-filtered event types — and land the four prerequisites Tier 1 deferred.

**Architecture:** Tier 1 hooked local crash sites. Tier 2 hooks the game's own deserialised event objects: `CScriptedGameEvent::Decide` sees every incoming script event with its sender, and each of the seventeen per-event filters detours slot 8 of that event class's vtable. Filters stay pure memory reads on the network thread, reporting through the Tier 1 ring; the block lists they need are *derived on console*, never ported.

**Tech Stack:** C++17, clang via the OpenOrbis PS4 toolchain, GoldHEN `Detour`, the project's mini-STL (`src/stl/`), CMake (GLOB_RECURSE — new `.cpp` files need no build-file edit).

**Spec:** `docs/superpowers/specs/2026-08-25-ps4-protections-design.md` (§ "Tier 2 — event filters")

**Tier 1 plan, for the conventions this one inherits:** `docs/superpowers/plans/2026-08-25-protections-tier1.md`
**Tier 1 console handoff:** `docs/superpowers/plans/2026-08-25-protections-tier1-console-handoff.md`

## Global Constraints

Everything in Tier 1's Global Constraints still binds. Repeated here because they are absolute:

- **Target build is CUSA00411 v1.57 only.** Every address is an RVA against ELF base 0, made absolute at install time against `rage::invoker::g_eboot_base`.
- **No natives, no allocation, no notification, no formatting, no logging inside a hook.** Hooks run on the network thread. Breaking this crashes the game.
- **No calls into the game from a hook** unless proven side-effect-free by disassembly. Tier 1 had to redesign a filter when its intended accessor turned out not to be a pure getter.
- **No `.init_array`.** Global constructors never run. Function-local statics or constant-initialised aggregates only — and note Tier 1's finding: a struct embedding a C aggregate by value needs `Member m{}` for the enclosing type to stay constant-initialised. `= false` on the other members alone silently demotes it to dynamic init.
- **`stl::function` captures cap at 64 bytes**; `stl::string` is a fixed 128-byte buffer that truncates silently; no `stl::to_string`; no exceptions (`-fno-exceptions`); no RTTI.
- **Nothing may call a native during `menu::build()`**, including from `add_savable`.
- **Every filter ships in `Log`**, which must be **observationally inert** — detect, report, chain to the original exactly as if the hook were not there. `Enforce` is a separate deliberate flip. Tier 1 found one breach of this (a `bool` parameter clang normalised); do not introduce another.
- **Build into `build-wsl/`**, never `build/` (which holds the Windows CMake cache for host tests).
- **Bump `INSULIN_BUILD_TAG`** in `src/platform/build_tag.h` on every console deploy.
- **Derive every constant from the PS4 disassembly. Never port one.** Four PC constants failed to cross in Tier 1 — a count offset (`0x14730`→`0x14720`), two struct fields, and a vtable slot. A wrong vtable slot calls a different virtual and is invisible in a decompile.

## Build, test and deploy

Host unit test — header-only units need only the test file; a unit split into `.h`/`.cpp` must have its `.cpp` on the command line too, or the test fails to **link** rather than failing an assertion:
```bash
clang++ -std=c++17 -I src tests/<name>_test.cpp [src/<path>/<unit>.cpp] -o build/<name>_test.exe && ./build/<name>_test.exe
```

Plugin build:
```bash
wsl.exe bash -lc 'export OO_PS4_TOOLCHAIN=/home/bbc/OpenOrbis-PS4-Toolchain; \
  export PATH="$OO_PS4_TOOLCHAIN/bin/linux:$PATH"; \
  cd /mnt/e/Projects/PS4/InsulinEngine/InsulinGTAV && cmake --build build-wsl'
```
Do not delete or reconfigure `build-wsl`.

The four existing host suites must stay green throughout: `protections_ring_test`, `protections_registry_test`, `protections_coalesce_test`, plus whatever this tier adds.

## Reverse-engineering method, inherited from Tier 1

Every Phase B and C task is an RE task. The method is settled; follow it rather than re-inventing it.

- **IDA database:** `E:\Projects\IDA\PS4\GTA5\eboot_named.i64`, image base 0. Load MCP schemas with `ToolSearch`, then `idb_open`. Unscoped `search_text` times out at 60s — prefer `decompile`, `lookup_funcs`, `xrefs_to`, `find_bytes`, `disasm`, `basic_blocks`.
- **Reading a symbol NAME:** use `disasm` as the **first call in a freshly opened, normally-configured session**. `decompile()` retypes addresses in memory (Hex-Rays renders `byte_XXXX[idx]`) and later `disasm` in that session echoes the mutation — it attaches to the *address*, not the function. Separately, opening with `run_auto_analysis:false` / `build_caches:false` / `init_hexrays:false` corrupts names too. This cost Tier 1 three fix rounds.
- **Evidence goes in `E:\Projects\IDA\PS4\GTA5\analysis\PROTECTIONS_ANCHORS.md`**, which holds §1–13 from Tier 1. Append; never overwrite. Commit in that repo separately from the plugin commit.
- **The leaked `dev_ng` source** at `C:\Users\BBC\Downloads\gta v source code\GTAV Source` was decisive on four Tier 1 tasks. Check it early. `rage/framework/src/fwnet/` and `game/network/Events/` are the relevant trees here.
- **If a target cannot be identified with confidence, say so and land the rest.** Tier 1 did this twice and both were right. Do not hook a plausible candidate.

## What Tier 1 left for this tier

| Item | Where it lands |
|---|---|
| `players.h` — per-player block bitmasks | Task 3, with its first real consumer |
| In-game log view | Task 4, because Tier 2 verification happens in live sessions away from a terminal |
| Ring never exercised under concurrent producers | Task 2 — Tier 2 brings the first genuinely multi-network-thread producers |
| `install_detour` cannot detect a relative branch in the stolen prologue | Task 1 — **the hard prerequisite**; Tier 2 adds up to 18 detours and they are not pre-screened |

---

## Phase A — Prerequisites

### Task 1: Prologue safety analysis in `install_detour`

**Files:**
- Create: `src/protections/branch_check.h`
- Test: `tests/protections_branch_check_test.cpp`
- Modify: `src/protections/detour.cpp`

**Interfaces:**
- Consumes: nothing.
- Produces: `protections::prologue_verdict` (`{ bool safe; uint32_t steal_len; const char* reason; }`) and `prologue_verdict protections::check_prologue(const uint8_t* bytes, uint32_t len)`. Header-only, `<stdint.h>` only, host-testable.

This is the hard prerequisite. GoldHEN's `Detour_GetInstructionSize` accumulates whole instructions until it reaches ≥14, then `memcpy`s that range into the trampoline **without relocating operands** (`C:\PS4\GoldHEN_Plugins_SDK\source\Detour.c:127-129`). A relative branch or a RIP-relative operand inside that range therefore resolves to a wrong address in the stub.

Tier 1 caught this by hand-measuring every prologue, and it mattered: `rage::fwBasePool::New` was correctly identified and then *not hooked*, because byte 10 of its forced 15-byte steal is a `jz rel8` that would have broken the pool-empty path — the exact path its guard watched. Tier 2 adds up to eighteen more targets. Hand-screening eighteen prologues and getting all of them right is not a bet worth taking.

**Scope: detect and refuse. Do not relocate.** Relocation is a larger change that would unblock more targets, and it is recorded as a future option in `PROTECTIONS_ANCHORS.md`. This task converts a silent broken trampoline into a clean refusal with a reason — which is the safety property, and it is small enough to be certain about.

The decoder needs only enough x86-64 to walk instruction boundaries and spot two things. Keep it deliberately narrow: legacy prefixes, REX, the opcode map for the forms that actually appear in function prologues, ModRM/SIB, and displacement/immediate sizing. On any opcode it does not know, **return unsafe** — an unrecognised prologue is exactly when you want a refusal, not a guess.

- [ ] **Step 1: Write the failing test**

Create `tests/protections_branch_check_test.cpp`:

```cpp
// Host unit tests for detour prologue safety analysis.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/protections_branch_check_test.cpp -o build/protections_branch_check_test.exe
//   ./build/protections_branch_check_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
#include "protections/branch_check.h"
#include <stdio.h>
#include <string.h>

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

    // CScriptedGameEvent::Decide @ 0x16C5F10 - Tier 2's highest-value target.
    // push rbp / mov rbp,rsp / push r14 / push rbx / sub rsp,0x1d0
    // Boundaries land on exactly 14, and the RIP-relative load that follows
    // starts at offset 20 - outside the steal.
    const uint8_t decide[] = {
        0x55, 0x48,0x89,0xE5, 0x41,0x56, 0x53, 0x48,0x81,0xEC,0xD0,0x01,0x00,0x00,
        0x48,0x89,0xF1, 0x48,0x89,0xFE, 0x48,0x8B,0x05,0x45,0x17,0xA5,0x01
    };
    prologue_verdict v = check_prologue(decide, sizeof(decide));
    check_true("scripted-game-event Decide is safe", v.safe);
    check_eq("its steal is exactly 14", v.steal_len, 14);

    // rage::fwBasePool::New @ 0x1EF6A00 - Tier 1 identified it and refused to
    // hook it. Byte 10 of the forced 15-byte steal is `jz rel8`, which the stub
    // would copy unrelocated. This is the case the whole check exists for.
    const uint8_t pool_new[] = {
        0x4C,0x8B,0xD1, 0x48,0x63,0x49,0x18, 0x83,0xF9,0xFF, 0x74,0x6E,
        0x33,0xC0, 0xC3
    };
    prologue_verdict p = check_prologue(pool_new, sizeof(pool_new));
    check_true("fwBasePool::New is refused", !p.safe);
    check_true("and the reason names a relative branch",
               p.reason && strstr(p.reason, "branch") != nullptr);

    // A RIP-relative operand INSIDE the steal must also be refused: the stub
    // sits at a different address, so the displacement resolves elsewhere.
    const uint8_t rip_inside[] = {
        0x55, 0x48,0x89,0xE5, 0x48,0x8B,0x05,0x11,0x22,0x33,0x44,
        0x53, 0x41,0x56, 0x90, 0x90
    };
    prologue_verdict r = check_prologue(rip_inside, sizeof(rip_inside));
    check_true("rip-relative inside the steal is refused", !r.safe);
    check_true("and the reason says so",
               r.reason && strstr(r.reason, "rip") != nullptr);

    // A call rel32 is a relative branch too, and a common prologue opener in
    // instrumented builds.
    const uint8_t call_rel[] = {
        0x55, 0x48,0x89,0xE5, 0xE8,0x00,0x01,0x00,0x00, 0x53, 0x41,0x56,
        0x48,0x83,0xEC,0x20
    };
    check_true("call rel32 is refused", !check_prologue(call_rel, sizeof(call_rel)).safe);

    // A 14-byte boundary is fine; a steal that would have to split an
    // instruction must extend to the next whole boundary, not truncate.
    // push rbp / mov rbp,rsp / push r15..rbx (13 bytes) then a 4-byte insn.
    const uint8_t past14[] = {
        0x55, 0x48,0x89,0xE5, 0x41,0x57, 0x41,0x56, 0x41,0x55, 0x41,0x54, 0x53,
        0x49,0x89,0xFE, 0x90, 0x90
    };
    prologue_verdict q = check_prologue(past14, sizeof(past14));
    check_true("a split instruction extends the steal", q.safe);
    check_eq("to 16, not 14", q.steal_len, 16);

    // Too few bytes to reach 14 is a refusal, never a short steal.
    const uint8_t tiny[] = { 0x55, 0x48,0x89,0xE5, 0xC3 };
    check_true("a function shorter than the jump is refused",
               !check_prologue(tiny, sizeof(tiny)).safe);

    // An opcode the decoder does not know must refuse rather than guess.
    const uint8_t unknown[] = {
        0x55, 0x48,0x89,0xE5, 0x0F,0x0B, 0x53, 0x41,0x56, 0x48,0x83,0xEC,0x20, 0x90
    };
    prologue_verdict u = check_prologue(unknown, sizeof(unknown));
    if (!u.safe) check_true("an unknown opcode refuses", true);
    else         check_true("an unknown opcode refuses", false);

    // Null and zero-length are refusals, not crashes.
    check_true("null is refused", !check_prologue(nullptr, 32).safe);
    check_true("zero length is refused", !check_prologue(decide, 0).safe);

    // Every refusal carries a reason - the log line is the whole point.
    check_true("refusals always explain themselves",
               p.reason && r.reason && p.reason[0] && r.reason[0]);

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed != 0;
}
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `clang++ -std=c++17 -I src tests/protections_branch_check_test.cpp -o build/protections_branch_check_test.exe`
Expected: FAIL — `fatal error: 'protections/branch_check.h' file not found`

- [ ] **Step 3: Write the implementation**

Create `src/protections/branch_check.h`. Decode enough x86-64 to walk boundaries and refuse on the two hazards. The exact opcode coverage is yours to choose, but it must satisfy every assertion above and it must **refuse on anything it does not recognise**.

Required shape:

```cpp
#pragma once
#include <stdint.h>

// Is a function's prologue safe to detour?
//
// GoldHEN's Detour_GetInstructionSize accumulates whole instructions until it
// reaches >= 14 (the size of its absolute jump), then memcpy's that range into
// the trampoline WITHOUT relocating operands (Detour.c:127-129). Two things in
// that range therefore break:
//
//   * a relative branch (jcc rel8/rel32, jmp rel8/rel32, call rel32) - the
//     copied displacement is relative to the stub, not the original;
//   * a RIP-relative memory operand - same problem, different encoding.
//
// Tier 1 found this the expensive way: rage::fwBasePool::New was correctly
// identified and then deliberately not hooked, because byte 10 of its forced
// 15-byte steal is a `jz rel8` that would have broken the pool-empty path -
// the exact path its guard existed to watch.
//
// This decoder is deliberately narrow. It knows the instruction forms that
// actually appear in function prologues and REFUSES on anything else: an
// unrecognised prologue is precisely when a guess is worst.
namespace protections {

    struct prologue_verdict {
        bool        safe;       // false => do not detour this function
        uint32_t    steal_len;  // whole-instruction length >= 14 when safe
        const char* reason;     // why it was refused; nullptr when safe
    };

    prologue_verdict check_prologue(const uint8_t* bytes, uint32_t len);
}
```

Implement `check_prologue` inline in the header (it must stay host-compilable with no dependency beyond `<stdint.h>`).

- [ ] **Step 4: Run the test to verify it passes**

Run: `clang++ -std=c++17 -I src tests/protections_branch_check_test.cpp -o build/protections_branch_check_test.exe && ./build/protections_branch_check_test.exe`
Expected: PASS — final line `all passed`, exit code 0

- [ ] **Step 5: Wire it into `install_detour`**

In `src/protections/detour.cpp`, add `#include "protections/branch_check.h"`, and after the existing `rva` and `g_eboot_base` early-outs and before `Detour_Construct`:

```cpp
    // Read the target's first 32 bytes and refuse a prologue the SDK's
    // non-relocating memcpy would corrupt. A refusal here is a filter that
    // does not install - loud, and in the log. The alternative is a stub that
    // jumps to a wrong address on a path we cannot debug from here.
    const uint8_t* target = (const uint8_t*)(rage::invoker::g_eboot_base + rva);
    const prologue_verdict v = check_prologue(target, 32);
    if (!v.safe) {
        LOG_ERROR("protections: detour @rva 0x%llx REFUSED - %s",
                  (unsigned long long)rva, v.reason ? v.reason : "unsafe prologue");
        return false;
    }
```

- [ ] **Step 6: Build**

Run the plugin build. Expected: succeeds. Every Tier 1 filter must still install — all nine landed prologues were hand-verified safe, so a refusal here would mean the decoder is wrong, not the prologue.

- [ ] **Step 7: Commit**

```bash
git add src/protections/branch_check.h src/protections/detour.cpp tests/protections_branch_check_test.cpp
git commit -m "feat(protections): refuse a prologue the detour stub would corrupt"
```

---

### Task 2: Close the ring's concurrent-producer gap

**Files:**
- Modify: `tests/protections_ring_test.cpp`

**Interfaces:**
- Consumes: `protections::ring`, `protections::record` (Tier 1 Task 1).
- Produces: nothing new — this closes a verification gap.

Tier 1's reviewer verified the ring's acquire/release protocol by hand, twice, and both times found it correct. Nobody has ever *run* it with concurrent producers. That was acceptable for Tier 1, where every producer was the game or render thread. Tier 2 hooks the network event path, which is where genuinely concurrent producers arrive.

The ring is header-only and dependency-free, so this test is `std::thread` over plain C++ on the host — the one place in this project where the host and the target agree closely enough for a concurrency test to mean something. It will not catch a PS4-specific memory-model issue (x86-64 on both sides makes that unlikely), but it will catch a lost record, a double-claim, or a stuck slot, which are the failure modes that actually threaten the design.

- [ ] **Step 1: Add the failing test**

Append to `tests/protections_ring_test.cpp`, before `main`'s final print. Note this file has been C-headers-only so far; `<thread>` and `<atomic>` are C++ headers, which the STL1000 constraint forbids under MSVC's stdlib. Build **this** test with `-stdlib=libc++` if the installed clang supports it; if it does not, put the concurrency test in its own file `tests/protections_ring_mt_test.cpp` with its own documented build line and leave the original suite untouched. Decide which, and say which in your report.

```cpp
// Concurrent producers against a single consumer. The ring's whole reason for
// existing is that a network thread can push without blocking, and until now
// that has only ever been reasoned about, never executed.
static void test_concurrent_producers() {
    ring r;
    const int producers = 4;
    const int per_producer = 5000;

    std::atomic<int> pushed{0};
    std::atomic<bool> go{false};

    std::vector<std::thread> ts;
    for (int p = 0; p < producers; p++) {
        ts.emplace_back([&, p] {
            while (!go.load(std::memory_order_acquire)) {}
            for (int i = 0; i < per_producer; i++) {
                record rec = { (uint16_t)p, 0, 0, (uint32_t)i, 0 };
                r.push(rec);
                pushed.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    // One consumer, draining as fast as it can while the producers run.
    std::atomic<int> popped{0};
    std::thread consumer([&] {
        while (!go.load(std::memory_order_acquire)) {}
        record out;
        int spins = 0;
        while (popped.load(std::memory_order_relaxed) + (int)r.dropped()
               < producers * per_producer && spins < 100000000) {
            if (r.pop(&out)) { popped.fetch_add(1, std::memory_order_relaxed); spins = 0; }
            else             { spins++; }
        }
    });

    go.store(true, std::memory_order_release);
    for (auto& t : ts) t.join();
    consumer.join();

    record out;
    while (r.pop(&out)) popped.fetch_add(1, std::memory_order_relaxed);

    // Every push is either delivered or counted as dropped. Nothing vanishes,
    // and nothing is delivered twice.
    check_eq("every push is accounted for",
             (unsigned)(popped.load() + (int)r.dropped()),
             (unsigned)(producers * per_producer));
    check_true("nothing was delivered twice", popped.load() <= producers * per_producer);
}
```

Call it from `main` and add the includes the chosen build line needs.

- [ ] **Step 2: Run it and watch it pass**

Run the suite. Expected: PASS. **This test is expected to pass on the first run** — it is closing a verification gap, not fixing a known bug. If it fails, that is a genuine concurrency defect in code Tier 1 shipped: stop, do not "fix" the test, and report it as a Critical finding.

- [ ] **Step 3: Run it repeatedly**

Run it 20 times. A concurrency test that passes once has proven very little.
```bash
for i in $(seq 1 20); do ./build/protections_ring_mt_test.exe > /dev/null || echo "FAILED on run $i"; done; echo "20 runs complete"
```
Expected: no `FAILED` lines.

- [ ] **Step 4: Commit**

```bash
git add tests/
git commit -m "test(protections): exercise the report ring under concurrent producers"
```

---

### Task 3: Per-player block state

**Files:**
- Create: `src/protections/players.h`
- Test: `tests/protections_players_test.cpp`
- Modify: `src/menu/base/submenus/network_players.cpp`

**Interfaces:**
- Consumes: nothing.
- Produces: `protections::block_kind` (`net_events`, `clone_sync`, `clone_create`), `protections::blocks` with `bool is_blocked(block_kind, int player_index) const`, `void set_blocked(block_kind, int, bool)`, `void clear()`, `uint32_t mask(block_kind) const`, and `protections::blocks& protections::player_blocks()`. Header-only, `<stdint.h>` only.

Deferred out of Tier 1 because nothing there could use it — Tier 1's guards are local crash checks with no attributable player. Task 6 is its first real consumer.

Player index is the physical index at `CNetGamePlayer+0x31`, range 0..31. Out-of-range indices must be inert rather than undefined: a hostile or corrupt index must never shift by 32 or more, and must never report blocked, which would drop traffic from a player who does not exist.

`clone_sync` and `clone_create` are declared now and consumed in Tier 3. That is deliberate: the three kinds share one storage shape and splitting them across tiers would mean editing this file twice.

- [ ] **Step 1: Write the failing test**

Create `tests/protections_players_test.cpp`:

```cpp
// Host unit tests for per-player protection block state.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/protections_players_test.cpp -o build/protections_players_test.exe
//   ./build/protections_players_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
#include "protections/players.h"
#include <stdio.h>

static int g_failed = 0;

static void check_true(const char* what, bool ok) {
    if (!ok) { printf("FAIL %s\n", what); g_failed++; }
    else     { printf("ok   %s\n", what); }
}

int main() {
    using namespace protections;

    blocks b;

    for (int i = 0; i < 32; i++)
        check_true("starts unblocked", !b.is_blocked(block_kind::net_events, i));

    // One player's bit must not disturb its neighbours.
    b.set_blocked(block_kind::net_events, 5, true);
    check_true("player 5 blocked", b.is_blocked(block_kind::net_events, 5));
    check_true("player 4 unaffected", !b.is_blocked(block_kind::net_events, 4));
    check_true("player 6 unaffected", !b.is_blocked(block_kind::net_events, 6));

    // The three kinds are independent.
    check_true("clone_sync unaffected", !b.is_blocked(block_kind::clone_sync, 5));
    b.set_blocked(block_kind::clone_sync, 5, true);
    check_true("clone_sync now set", b.is_blocked(block_kind::clone_sync, 5));
    check_true("net_events still set", b.is_blocked(block_kind::net_events, 5));

    b.set_blocked(block_kind::net_events, 6, true);
    b.set_blocked(block_kind::net_events, 5, false);
    check_true("player 5 cleared", !b.is_blocked(block_kind::net_events, 5));
    check_true("player 6 still blocked", b.is_blocked(block_kind::net_events, 6));

    // Boundary indices are real players.
    b.set_blocked(block_kind::net_events, 0, true);
    b.set_blocked(block_kind::net_events, 31, true);
    check_true("index 0 works", b.is_blocked(block_kind::net_events, 0));
    check_true("index 31 works", b.is_blocked(block_kind::net_events, 31));

    // Out-of-range is inert, never undefined. A corrupt player index arriving
    // from the wire must not shift by 32+, and must not report blocked.
    check_true("index 32 not blocked", !b.is_blocked(block_kind::net_events, 32));
    check_true("index -1 not blocked", !b.is_blocked(block_kind::net_events, -1));
    check_true("index 255 not blocked", !b.is_blocked(block_kind::net_events, 255));
    const uint32_t before = b.mask(block_kind::net_events);
    b.set_blocked(block_kind::net_events, 32, true);
    b.set_blocked(block_kind::net_events, -1, true);
    check_true("out-of-range set is ignored", b.mask(block_kind::net_events) == before);

    b.clear();
    check_true("clear resets net_events", b.mask(block_kind::net_events) == 0);
    check_true("clear resets clone_sync", b.mask(block_kind::clone_sync) == 0);
    check_true("clear resets clone_create", b.mask(block_kind::clone_create) == 0);

    player_blocks().set_blocked(block_kind::clone_create, 2, true);
    check_true("global instance works", player_blocks().is_blocked(block_kind::clone_create, 2));
    player_blocks().clear();

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed != 0;
}
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `clang++ -std=c++17 -I src tests/protections_players_test.cpp -o build/protections_players_test.exe`
Expected: FAIL — `fatal error: 'protections/players.h' file not found`

- [ ] **Step 3: Write the implementation**

Create `src/protections/players.h`:

```cpp
#pragma once
#include <stdint.h>

// Per-player block state, indexed by the physical player index at
// CNetGamePlayer+0x31 (0..31).
//
// Three bitmasks and nothing else. This is deliberately not a player database:
// no infraction history, no reactions, no persistence. Those are a separate
// subsystem and this port does not build them.
//
// Read from hook threads with a bit test, written from the menu thread. An
// aligned 32-bit load cannot tear, so no lock is needed for a value whose worst
// race is one packet handled under the previous setting.
namespace protections {

    enum class block_kind : int {
        net_events   = 0,
        clone_sync   = 1,   // consumed in Tier 3
        clone_create = 2,   // consumed in Tier 3
        count        = 3
    };

    class blocks {
    public:
        bool is_blocked(block_kind kind, int player_index) const {
            if (!valid(kind, player_index)) return false;
            const uint32_t m = __atomic_load_n(&m_mask[(int)kind], __ATOMIC_RELAXED);
            return (m & (1u << player_index)) != 0;
        }

        void set_blocked(block_kind kind, int player_index, bool on) {
            if (!valid(kind, player_index)) return;
            const uint32_t bit = 1u << player_index;
            if (on) __atomic_fetch_or(&m_mask[(int)kind], bit, __ATOMIC_RELAXED);
            else    __atomic_fetch_and(&m_mask[(int)kind], ~bit, __ATOMIC_RELAXED);
        }

        uint32_t mask(block_kind kind) const {
            if ((int)kind < 0 || (int)kind >= (int)block_kind::count) return 0;
            return __atomic_load_n(&m_mask[(int)kind], __ATOMIC_RELAXED);
        }

        void clear() {
            for (int i = 0; i < (int)block_kind::count; i++)
                __atomic_store_n(&m_mask[i], 0u, __ATOMIC_RELAXED);
        }

    private:
        static bool valid(block_kind kind, int player_index) {
            if ((int)kind < 0 || (int)kind >= (int)block_kind::count) return false;
            return player_index >= 0 && player_index < 32;
        }

        uint32_t m_mask[(int)block_kind::count] = {};
    };

    // Function-local static: no .init_array in this plugin.
    inline blocks& player_blocks() {
        static blocks instance;
        return instance;
    }
}
```

- [ ] **Step 4: Run the test to verify it passes**

Run: `clang++ -std=c++17 -I src tests/protections_players_test.cpp -o build/protections_players_test.exe && ./build/protections_players_test.exe`
Expected: PASS — final line `all passed`

- [ ] **Step 5: Wire the toggle into the Players submenu**

Read `src/menu/base/submenus/network_players.cpp` before editing to match its existing per-player option style. Add one toggle per player row, bound through a small helper so the `stl::function` capture stays an `int` (the 64-byte capture cap forbids capturing anything larger):

```cpp
    add_option(toggle_option("Block Net Events")
        .add_tooltip("Drop network events from this player. Their sync and "
                     "your view of them are unaffected.")
        .add_click([idx] {
            using namespace protections;
            const bool now = player_blocks().is_blocked(block_kind::net_events, idx);
            player_blocks().set_blocked(block_kind::net_events, idx, !now);
        }));
```

Do **not** call `add_savable` on it: a block is a decision about the player in front of you right now, and restoring it into a session with different players would silently drop traffic from someone who was never blocked.

- [ ] **Step 6: Build and commit**

Run the plugin build. Then:
```bash
git add src/protections/players.h tests/protections_players_test.cpp src/menu/base/submenus/network_players.cpp
git commit -m "feat(protections): per-player block bitmasks, wired into the players submenu"
```

---

### Task 4: In-game report log

**Files:**
- Create: `src/menu/base/submenus/protections_log.h`, `src/menu/base/submenus/protections_log.cpp`
- Modify: `src/protections/report.h`, `src/protections/report.cpp`
- Modify: `src/menu/base/submenus/protections.cpp`, `src/menu/menu.cpp`

**Interfaces:**
- Consumes: `protections::record`, `protections::name_of`, `protections::drain_reports`.
- Produces: `protections::recent_count()`, `protections::recent_at(int, record* out, uint32_t* suppressed)` — a bounded history of the last 32 drained records; and `protections_log_menu` with the usual `load()` / `update()` / `get()`.

Tier 1's verification happened at a desk with `nc` open. Tier 2's happens inside live sessions, where the operator is holding a controller and cannot read a terminal. Without an in-game view, every Tier 2 filter's Log phase requires a second person or a trip back to the PC.

The history is a fixed 32-entry array written by `drain_reports()` on the script thread and read by the menu on the same thread — **no atomics needed**, unlike the ring. Do not reuse `ring` for this: the ring is drained destructively, and this needs the last N to stay readable.

- [ ] **Step 1: Extend `report.h`**

Add to `src/protections/report.h`, inside `namespace protections`:

```cpp
    // A bounded history of what drain_reports() has emitted, for the in-game
    // log view. Written and read on the script thread only - no atomics.
    //
    // Deliberately not the ring: the ring is drained destructively and this
    // needs the last N to stay readable while the menu is open.
    static const int recent_capacity = 32;

    int  recent_count();
    bool recent_at(int index_from_newest, record* out, uint32_t* suppressed_out);
```

- [ ] **Step 2: Record into the history when draining**

In `src/protections/report.cpp`, add a file-static history and append to it inside `drain_reports()`, at the point where the record has been popped and its suppressed count taken:

```cpp
namespace {
    struct recent_entry { record r; uint32_t suppressed; };
    recent_entry g_recent[recent_capacity] = {};
    int          g_recent_head  = 0;   // next write slot
    int          g_recent_count = 0;
}
```

and in the drain loop, after the klog line:

```cpp
        g_recent[g_recent_head].r          = r;
        g_recent[g_recent_head].suppressed = suppressed;
        g_recent_head = (g_recent_head + 1) % recent_capacity;
        if (g_recent_count < recent_capacity) g_recent_count++;
```

Then the accessors:

```cpp
int recent_count() { return g_recent_count; }

bool recent_at(int index_from_newest, record* out, uint32_t* suppressed_out) {
    if (!out || index_from_newest < 0 || index_from_newest >= g_recent_count)
        return false;
    int slot = g_recent_head - 1 - index_from_newest;
    while (slot < 0) slot += recent_capacity;
    *out = g_recent[slot].r;
    if (suppressed_out) *suppressed_out = g_recent[slot].suppressed;
    return true;
}
```

Adapt the variable names to whatever the existing drain loop actually calls them — read the function before editing.

- [ ] **Step 3: Write the submenu**

Create `src/menu/base/submenus/protections_log.h`:

```cpp
#pragma once
#include "menu/base/submenu.h"

// The last 32 drained protection reports, newest first.
//
// Tier 1 was verified at a desk with the kernel log open over TCP. Tier 2 is
// verified inside live sessions, where the operator has a controller and no
// terminal - so the log has to be reachable from the pause menu.
class protections_log_menu : public menu::submenu::submenu {
public:
    void load() override;
    void update() override;
    static protections_log_menu* get();
};
```

Create `src/menu/base/submenus/protections_log.cpp`. Rebuild the option list only when `recent_count()` or the newest record changes — use the dirty-flag pattern from `vehicle_spawner.cpp` (`clear_options(0)` then rebuild), never a rebuild every frame. Render one row per record as `<filter name>  a=<hex> b=<hex>` plus `+N` when `suppressed` is non-zero, and a player index when it is not `0xFF`.

- [ ] **Step 4: Register it**

Add a `submenu_option` on the Protections submenu pointing at `protections_log_menu`, and register it in `src/menu/menu.cpp` next to the others:
```cpp
        protections_log_menu::get()->load();
        menu::submenu::handler::add_submenu(protections_log_menu::get());
```

- [ ] **Step 5: Build, then verify the existing suites still pass**

Run the plugin build, then all five host suites (ring, registry, coalesce, branch_check, players). Expected: build clean, all suites pass.

- [ ] **Step 6: Commit**

```bash
git add src/menu/base/submenus/protections_log.h src/menu/base/submenus/protections_log.cpp src/protections/report.h src/protections/report.cpp src/menu/base/submenus/protections.cpp src/menu/menu.cpp
git commit -m "feat(protections): in-game view of the last 32 reports"
```

---

## Phase B — Script events

### Task 5: Distinct-hash learn table

**Files:**
- Create: `src/protections/learn.h`
- Test: `tests/protections_learn_test.cpp`

**Interfaces:**
- Consumes: nothing.
- Produces: `protections::learn_table` with `bool observe(uint32_t hash, uint8_t player_index)`, `int count() const`, `bool at(int, uint32_t* hash, uint32_t* hits, uint8_t* first_player) const`, `void clear()`, `uint32_t overflow() const`; capacity 128. Header-only, `<stdint.h>` only.

**This table exists because the report ring cannot do this job, and that is worth understanding before writing it.**

The coalescer keys on `filter_id` alone (`coalesce.h:51`). Learn mode reports a *distinct event hash* per observation — but every one of them would carry the same `filter_id`, so the coalescer would collapse fifty distinct hashes into one record reading `+49 more`. That would destroy precisely the data learn mode exists to collect.

The right shape is not an event stream at all. Learning wants the **set** of hashes seen, with a hit count each — not every occurrence. So: a small open-addressed table, written from the network thread inside the hook, read from the script thread by the menu.

`observe()` runs on a network thread and must obey the usual rules: no allocation, no natives, no formatting. It returns `true` when the hash was new, so the caller can decide whether it is worth a `report()` — a newly-seen hash is interesting, the ten-thousandth sighting of a known one is not.

- [ ] **Step 1: Write the failing test**

Create `tests/protections_learn_test.cpp`:

```cpp
// Host unit tests for the script-event learn table.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/protections_learn_test.cpp -o build/protections_learn_test.exe
//   ./build/protections_learn_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
#include "protections/learn.h"
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

    learn_table t;
    check_eq("starts empty", (unsigned)t.count(), 0);

    // A new hash is reported as new; a repeat is not. This is what lets the
    // hook report only the first sighting instead of flooding the ring.
    check_true("first sighting is new", t.observe(0xDEADBEEF, 3));
    check_true("second sighting is not new", !t.observe(0xDEADBEEF, 7));
    check_eq("still one distinct hash", (unsigned)t.count(), 1);

    uint32_t hash = 0, hits = 0; uint8_t first = 0xFF;
    check_true("readable", t.at(0, &hash, &hits, &first));
    check_eq("hash preserved", hash, 0xDEADBEEF);
    check_eq("hits counted", hits, 2);
    check_eq("first player is the FIRST one, not the latest", first, 3);

    // Distinct hashes stay distinct - the whole point.
    check_true("second hash is new", t.observe(0x11111111, 1));
    check_true("third hash is new", t.observe(0x22222222, 2));
    check_eq("three distinct", (unsigned)t.count(), 3);

    // Hash 0 is a legal event id and must be storable, not mistaken for an
    // empty slot.
    check_true("zero hash is new", t.observe(0, 4));
    check_eq("zero hash stored", (unsigned)t.count(), 4);
    check_true("zero hash is not new twice", !t.observe(0, 4));
    check_eq("still four", (unsigned)t.count(), 4);

    // Fill to capacity, then overflow. Overflow must be counted and must not
    // corrupt what is already there.
    t.clear();
    check_eq("cleared", (unsigned)t.count(), 0);
    check_eq("overflow cleared", t.overflow(), 0);

    for (uint32_t i = 0; i < learn_table::capacity; i++)
        check_true("fills without overflow", t.observe(0x1000 + i, 0));
    check_eq("full", (unsigned)t.count(), learn_table::capacity);
    check_eq("no overflow yet", t.overflow(), 0);

    t.observe(0xFFFFFFFF, 0);
    check_eq("overflow counted", t.overflow(), 1);
    check_eq("count did not grow past capacity", (unsigned)t.count(), learn_table::capacity);

    // An already-known hash must still register a hit when the table is full -
    // otherwise a full table stops counting the events you care about most.
    check_true("known hash still not new when full", !t.observe(0x1000, 0));
    check_eq("and did not count as overflow", t.overflow(), 1);

    // Out-of-range reads are inert.
    check_true("negative index reads nothing", !t.at(-1, &hash, &hits, &first));
    check_true("past-end index reads nothing", !t.at(t.count(), &hash, &hits, &first));
    check_true("null out is refused", !t.at(0, nullptr, &hits, &first));

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed != 0;
}
```

- [ ] **Step 2: Run the test to verify it fails**

Run: `clang++ -std=c++17 -I src tests/protections_learn_test.cpp -o build/protections_learn_test.exe`
Expected: FAIL — `fatal error: 'protections/learn.h' file not found`

- [ ] **Step 3: Write the implementation**

Create `src/protections/learn.h` with `static const int capacity = 128;`, an open-addressed table keyed by hash, and a separate `used` flag per slot so hash `0` is storable. `observe()` must be safe to call from a network thread: no allocation, no natives, no formatting. Use `__atomic_*` builtins for the slot claim the way `coalesce.h` does — a single-writer assumption is not safe here, because two network threads can deliver events concurrently.

- [ ] **Step 4: Run the test to verify it passes**

Run: `clang++ -std=c++17 -I src tests/protections_learn_test.cpp -o build/protections_learn_test.exe && ./build/protections_learn_test.exe`
Expected: PASS — final line `all passed`

- [ ] **Step 5: Commit**

```bash
git add src/protections/learn.h tests/protections_learn_test.cpp
git commit -m "feat(protections): distinct-hash learn table for script events"
```

---

### Task 6: The script-event filter

**Files:**
- Create: `src/protections/hooks_script_event.cpp`
- Modify: `src/protections/registry.h`, `src/protections/registry.cpp`, `tests/protections_registry_test.cpp`
- Modify: `src/menu/base/submenus/protections.cpp` (learn-mode surface)
- Modify: `src/platform/build_tag.h`
- Modify: `E:\Projects\IDA\PS4\GTA5\analysis\PROTECTIONS_ANCHORS.md` (append §14)

**Interfaces:**
- Consumes: `install_detour`, `PROT_CHAIN`, `should_report`, `should_block`, `report`, `protections::learn_table`, `protections::player_blocks()`.
- Produces: `bool protections::install_script_event()`; new `filter_id::script_event`.

The single highest-value hook in the whole design. One detour sees every incoming script event with its sender, covering the family YimMenu protects: bounty, CEO money, clear-wanted-level, forced mission and teleport, MC teleport, personal-vehicle destroyed, remote off-radar, rotate-cam, send-to-cutscene and -location, sound spam, spectate, give-collectible, vehicle kick, teleport-to-warehouse, start-activity, fake notifications, transaction error, and three crash events.

**The anchor is confirmed and its prologue is already checked.** `CScriptedGameEvent::Decide` @ **`0x16C5F10`**, signature `(this, CNetGamePlayer* fromPlayer)`. Args at `this+0x70`; arg **count** (not bytes) at `this+0x224`, max `0x1B0`. Prologue decoded by hand:

```
0   55                    push rbp
1   48 89 e5              mov  rbp, rsp
4   41 56                 push r14
6   53                    push rbx
7   48 81 ec d0 01 00 00  sub  rsp, 0x1d0     -> boundary at exactly 14
20  48 8b 05 45 17 a5 01  mov  rax, [rip+..]  -> RIP-relative, OUTSIDE the steal
```

Confirm this with `check_prologue` from Task 1 rather than trusting the table — that is what Task 1 is for. Also confirm the entry basic block has no predecessors.

**This task ships learn mode only. It blocks nothing by event type.** The block list is Task 14 and requires console data. What this task *does* enforce is the per-player `net_events` block from Task 3, which needs no hash knowledge.

- [ ] **Step 1: Verify the anchor and record the evidence**

Open the IDB, decompile `0x16C5F10`, and confirm: the `(this, CNetGamePlayer*)` shape; args at `+0x70`; count at `+0x224` with the `count != 0 && count <= 0x1B0` guard; and that the sender's physical player index is reachable at `fromPlayer+0x31`. Run the prologue bytes through `check_prologue`.

Append §14 to `PROTECTIONS_ANCHORS.md` with the decompiled listing. Commit in the IDA repo.

- [ ] **Step 2: Write the hook**

Create `src/protections/hooks_script_event.cpp`:

```cpp
#include "protections/detour.h"
#include "protections/registry.h"
#include "protections/report.h"
#include "protections/learn.h"
#include "protections/players.h"
#include "rage/invoker/invoker.h"

// CScriptedGameEvent::Decide - every incoming script event, with its sender.
//
// REPORT LEGEND
//   script_event: a = args[0] (the event type hash), b = arg count
//
// This filter ships in LEARN mode: it records the distinct set of event hashes
// it sees and blocks nothing by type. That is deliberate. YimMenu's eRemoteEvent
// constants are joaat hashes compiled into the freemode script, and all 42 were
// tested against the 1389 hashed entries in the leaked MP_Event_Enums.sch under
// both case conventions - ZERO matched. The leaked dev branch, the PC build
// YimMenu targets, and PS4 1.57 are three different script revisions. A block
// list built from unverified constants would either do nothing, or drop
// legitimate script events and present as random mission failures.
//
// What this filter DOES enforce is the per-player net_events block, which needs
// no hash knowledge at all.
namespace protections {
namespace {
    // CScriptedGameEvent::Decide - vtable slot 8 of 0x30A8E80.
    // Prologue: whole-instruction boundary at exactly 14, register-only, the
    // RIP-relative load at offset 20 sits outside the steal. See ANCHORS §14.
    const uint64_t RVA_SCRIPTED_GAME_EVENT_DECIDE = 0x16C5F10;

    // CScriptedGameEvent layout.
    const uint64_t EVT_ARGS       = 0x70;    // uint32_t[]
    const uint64_t EVT_ARG_COUNT  = 0x224;   // count, not bytes; max 0x1B0
    const uint32_t EVT_ARG_MAX    = 0x1B0;

    // CNetGamePlayer+0x31 is the physical player index (live-verified, Tier 1).
    const uint64_t NGP_PLAYER_IDX = 0x31;

    typedef bool (*scripted_decide_fn)(uint64_t, void*);

    detour_slot g_scripted;

    learn_table& learned() {
        static learn_table instance;   // function-local static: no .init_array
        return instance;
    }

    bool scripted_decide_hook(uint64_t self, void* from_player) {
        if (!should_report(filter_id::script_event))
            return PROT_CHAIN(g_scripted, scripted_decide_fn, self, from_player);

        const uint32_t count = *(const volatile uint32_t*)(self + EVT_ARG_COUNT);
        if (count == 0 || count > EVT_ARG_MAX)
            return PROT_CHAIN(g_scripted, scripted_decide_fn, self, from_player);

        const uint32_t hash = *(const volatile uint32_t*)(self + EVT_ARGS);

        int player = -1;
        if (from_player) {
            const uint8_t idx = *(const volatile uint8_t*)((uint64_t)from_player + NGP_PLAYER_IDX);
            if (idx < 32) player = (int)idx;
        }

        // Report only the FIRST sighting of a hash. The ten-thousandth sighting
        // of a known one is noise, and the coalescer cannot help here: it keys
        // on filter_id, so it would collapse every distinct hash into one record.
        if (learned().observe(hash, player >= 0 ? (uint8_t)player : 0xFF))
            report(filter_id::script_event, player, 0, hash, count);

        // The one thing this filter enforces today. No hash knowledge needed.
        if (player >= 0 &&
            player_blocks().is_blocked(block_kind::net_events, player) &&
            should_block(filter_id::script_event)) {
            return true;   // handled; the event never reaches the scripts
        }

        return PROT_CHAIN(g_scripted, scripted_decide_fn, self, from_player);
    }
}

bool install_script_event() {
    return install_detour(&g_scripted, RVA_SCRIPTED_GAME_EVENT_DECIDE,
                          (void*)&scripted_decide_hook);
}

int  learned_count() { return learned().count(); }
bool learned_at(int i, uint32_t* hash, uint32_t* hits, uint8_t* first) {
    return learned().at(i, hash, hits, first);
}
void learned_clear() { learned().clear(); }
}
```

**Confirm the `Decide` return convention before shipping.** The hook returns `true` on the block path. Tier 1's decompile showed `Decide` returning a bool, but whether `true` means "handled, stop" or "not handled, continue" must be read out of `netEventMgr`'s dispatch, not assumed. If it is inverted, the block path silently does nothing — which in `Log` is harmless and in `Enforce` is a filter that appears to work and does not. State what you found in your report.

- [ ] **Step 3: Wire the registry**

Add to `registry.h`, near the other filter ids: `script_event = 30,`. Declare:
```cpp
    bool install_script_event();

    // Learn-mode accessors for the menu. Script thread only.
    int  learned_count();
    bool learned_at(int index, uint32_t* hash, uint32_t* hits, uint8_t* first_player);
    void learned_clear();
```

Add the table row in `registry.cpp` — `can_block` is **true**, because the per-player path genuinely refuses events:
```cpp
            { filter_id::script_event, "Script Events", 2, (int)mode::log, (int)mode::log, false, &install_script_event, true },
```

Append a `{ return false; }` stub for `install_script_event` to the existing block in `tests/protections_registry_test.cpp`, plus host stubs for the three learn accessors.

- [ ] **Step 4: Add the learn-mode menu surface**

In `src/menu/base/submenus/protections.cpp`, under the Diagnostics break, add a button that reports how many distinct hashes have been seen, and one that clears the table:

```cpp
    add_option(button_option("Script Events Learned")
        .add_tooltip("Distinct script-event hashes seen this session")
        .add_click([] {
            char msg[64];
            snprintf(msg, sizeof(msg), "%d distinct event hashes", protections::learned_count());
            menu::notify::stacked("Protections", msg);
        }));

    add_option(button_option("Clear Learned Events")
        .add_tooltip("Start a fresh baseline - do this before a known-attack session")
        .add_click([] { protections::learned_clear(); }));
```

The full list is read from the kernel log and the Task 4 in-game log; a scrolling hash list in the menu is not worth building until the console phase shows it is needed.

- [ ] **Step 5: Re-run the host suites and build**

Run all six host suites (ring, registry, coalesce, branch_check, players, learn) and the plugin build. Set `INSULIN_BUILD_TAG` to `"prot-t2-scriptevt-1"`.

- [ ] **Step 6: Commit**

```bash
git add src/protections/hooks_script_event.cpp src/protections/registry.h src/protections/registry.cpp tests/protections_registry_test.cpp src/menu/base/submenus/protections.cpp src/platform/build_tag.h
git commit -m "feat(protections): script-event learn mode and per-player event blocking"
cd /e/Projects/IDA/PS4/GTA5 && git add analysis/PROTECTIONS_ANCHORS.md && \
  git commit -m "re(gtav-ps4): CScriptedGameEvent::Decide anchor and layout"
```

---

## Phase C — Per-event filters

Seven tasks, each following one shape. The shape is stated once here rather than seven times.

**Finding an event's `Decide`.** `analysis/NET_EVENT_CATALOG.md` lists all 81 events with their id and registered function. The registered function stamps the event class's vtable as its **first member write**; the vtable's slot 6 is `Prepare`, slot 7 is `Handle` (deserialise), slot 8 is `Decide` (apply). Worked example, verified:

```
WEAPON_DAMAGE_EVENT (id 6) -> registered fn 0x16B91F0
  0x16B91F0 stamps off_30A7EC0        <- CWeaponDamageEvent vtable
  0x30A7EC0 + 6*8 = Prepare 0x16B9AD0
  0x30A7EC0 + 7*8 = Handle  0x16B9AE0
  0x30A7EC0 + 8*8 = Decide  0x16B9AF0
```

**Hook `Decide`, not `Handle`.** By `Decide` the event is a fully-constructed C++ object with typed fields, which is why this design was chosen over re-parsing the bit buffer. `Handle` is the deserialiser; hooking it means re-deriving wire layouts that drift between title updates.

**Every task must:**
1. Resolve the event's vtable and `Decide` RVA from the catalogue's registered function, and record both in `PROTECTIONS_ANCHORS.md` with the decompiled evidence.
2. **Run the prologue through `check_prologue` (Task 1).** If it refuses, do not hook that event — report it identified-but-unhookable, exactly as Tier 1 did for `fwBasePool::New`, and move on.
3. Derive every field offset from the PS4 disassembly. Never port one.
4. State which dereference retail itself leaves unchecked — that determines what a `Log` report *means*, and belongs in the console notes.
5. Add a `REPORT LEGEND` comment block naming what `detail_a` and `detail_b` carry for each filter in the file.
6. Honour the per-player `net_events` block, the same way Task 6 does.
7. Add registry rows, `registry.h` declarations, and host-test stubs appended to the existing block.

**Filter ids** are assigned per group below so the tasks do not collide: they are stable config keys and log identifiers, and must not be renumbered later.

### The Phase C task template

Every Phase C task runs these seven steps. They are written out once here; each task below states its own RVAs, fields and checks, and its steps refer to this template by number. Read both.

- [ ] **Step 1: Resolve each event's vtable and `Decide`.** For every event in your group, take the registered function from `analysis/NET_EVENT_CATALOG.md`, decompile it, and read the vtable it stamps as its first member write. Then `get_bytes` that vtable and take slot 8 (`vtable + 64`) for `Decide`. Record every RVA. If an event's registered function does not stamp a vtable as its first write, stop and say so — the catalogue's "factory" column is really the registered function, and one of them may not be the shape this method assumes.

- [ ] **Step 2: Check every prologue with `check_prologue`.** Read the target's first 32 bytes with `get_bytes` and run them through the Task 1 decoder's rules by hand (or build a small host harness). Any target it refuses is **not hooked** — report it identified-but-unhookable, exactly as Tier 1 did for `fwBasePool::New`, and carry on with the rest. Also confirm each entry basic block has no predecessors via `basic_blocks`.

- [ ] **Step 3: Derive the field offsets from the PS4 disassembly.** Never port one from YimMenu. Four PC constants failed to cross in Tier 1, including a vtable slot that is invisible in a decompile. For each field you read, state in your report whether it happened to match PC or had to be re-derived.

- [ ] **Step 4: Write the hook file.** Structure it exactly like the Tier 1 hook files — a header comment carrying the evidence trail and its `PROTECTIONS_ANCHORS.md` section pointer, a `REPORT LEGEND` block, then an anonymous namespace holding `RVA_*` constants → field-offset constants → `*_fn` typedefs → `detour_slot g_*`, then the hooks, then one-line `install_*()` at namespace scope. Each hook has this skeleton:

```cpp
    bool <event>_decide_hook(uint64_t self, void* from_player) {
        if (!should_report(filter_id::<id>))
            return PROT_CHAIN(g_<slot>, <event>_fn, self, from_player);

        int player = -1;
        if (from_player) {
            const uint8_t idx = *(const volatile uint8_t*)((uint64_t)from_player + NGP_PLAYER_IDX);
            if (idx < 32) player = (int)idx;
        }

        // Per-player block first: it needs no field knowledge and is the one
        // refusal that is correct for every event type.
        if (player >= 0 &&
            player_blocks().is_blocked(block_kind::net_events, player) &&
            should_block(filter_id::<id>)) {
            return true;
        }

        // <the event-specific validity check, reading typed fields off `self`>
        if (<hostile condition>) {
            report(filter_id::<id>, player, 0, <detail_a>, <detail_b>);
            if (should_block(filter_id::<id>))
                return true;
        }

        return PROT_CHAIN(g_<slot>, <event>_fn, self, from_player);
    }
```

`NGP_PLAYER_IDX` is `0x31` (`CNetGamePlayer`'s physical player index, live-verified in Tier 1). Confirm `Decide`'s return convention the same way Task 6 does — whether `true` means "handled, stop" or "not handled, continue" must be read out of `netEventMgr`'s dispatch, not assumed. If it is inverted, the block path silently does nothing.

- [ ] **Step 5: Wire the registry.** Declare each `install_*()` in `src/protections/registry.h`, add a table row per filter in `src/protections/registry.cpp` (tier `2`, `default_mode` and `current` both `(int)mode::log`, `can_block` `true` unless the event has nothing refusable), and append a `{ return false; }` stub per installer to the **existing** block in `tests/protections_registry_test.cpp`. Do not start a new stub block.

- [ ] **Step 6: Record the evidence and build.** Append a section per event to `E:\Projects\IDA\PS4\GTA5nalysis\PROTECTIONS_ANCHORS.md` — vtable, `Decide` RVA, prologue measurement, every field offset with the disassembly that justifies it, and which dereference retail leaves unchecked. Commit that in the IDA repo separately. Then set `INSULIN_BUILD_TAG` as each task states, run the plugin build, and re-run all six host suites (ring, registry, coalesce, branch_check, players, learn).

- [ ] **Step 7: Commit, and write the console notes.** Commit the plugin change. In your report, state for each filter what a `Log` report *means* — whether it indicates a real attack or a state retail itself tolerates. That distinction is the most valuable thing you produce: Tier 1 found two guards that fire legitimately, and without that written down the obvious reading of the log would have backed out two correctly-verified anchors.

### Task 7: Weapon events

**Files:** create `src/protections/hooks_events_weapon.cpp`; modify registry, test stubs, `build_tag.h`, anchors doc.
**Filter ids:** `weapon_damage = 31`, `give_weapon = 32`, `remove_weapon = 33`.

`WEAPON_DAMAGE_EVENT` (id 6, registered `0x16B91F0`, vtable `0x30A7EC0`, `Decide` `0x16B9AF0` — verified), `GIVE_WEAPON_EVENT` (id 12, `0x16BC8F0`), `REMOVE_WEAPON_EVENT` (id 13, `0x16BCD70`).

The valuable check is weapon-hash validity: `g_WeaponInfoArray` is already resolved at RVA `0x385E850`. YimMenu walks it comparing `info->m_name == hash` and `info->GetClassId() == "cweaponinfo"_J`. Derive the array's element stride and the name field offset from the PS4 disassembly — **do not port YimMenu's**. An invalid weapon hash in a damage event is the classic remote-crash vector.

Report `detail_a = weapon hash`, `detail_b = damage type`.

**Steps** (mechanics are in the Phase C template above; these are your targets):

- [ ] **Step 1 — resolve vtables and `Decide` RVAs** (template step 1) for:
  - `WEAPON_DAMAGE_EVENT` — id 6, registered `0x16B91F0`. Vtable `0x30A7EC0` and `Decide` `0x16B9AF0` are already verified; confirm rather than re-derive.
  - `GIVE_WEAPON_EVENT` — id 12, registered `0x16BC8F0`.
  - `REMOVE_WEAPON_EVENT` — id 13, registered `0x16BCD70`.
- [ ] **Step 2 — run every prologue through `check_prologue`** (template step 2). Any refusal means that event is not hooked; report it and continue with the rest.
- [ ] **Step 3 — derive the field offsets** (template step 3). Includes `g_WeaponInfoArray` (`0x385E850`): derive its element stride and the name-field offset from the PS4 disassembly, and the class-id check YimMenu uses.
- [ ] **Step 4 — write the hook file** (template step 4), one hook per event, each honouring the per-player `net_events` block.
- [ ] **Step 5 — wire the registry** (template step 5) with the filter ids named above.
- [ ] **Step 6 — record evidence and build** (template step 6). Set `INSULIN_BUILD_TAG` to `"prot-t2-weapon-1"`.
- [ ] **Step 7 — commit and write the console notes** (template step 7), stating per filter what a `Log` report means.


### Task 8: Control events

**Files:** create `src/protections/hooks_events_control.cpp`; modify registry, test stubs, `build_tag.h`, anchors doc.
**Filter ids:** `give_control = 34`, `request_control = 35`.

`GIVE_CONTROL_EVENT` (id 5, registered `0x16B6E90`, vtable **`0x30A7D40`** already recorded, `Prepare 0x16B62F0`, `Handle 0x16B6340` — resolve `Decide` from slot 8), `REQUEST_CONTROL_EVENT` (id 4, `0x16B6120`).

These are how an attacker takes ownership of your vehicle or ped before acting on it. The check is whether the object being handed over is one you own and are currently using.

**Steps** (mechanics are in the Phase C template above; these are your targets):

- [ ] **Step 1 — resolve vtables and `Decide` RVAs** (template step 1) for:
  - `GIVE_CONTROL_EVENT` — id 5, registered `0x16B6E90`. Vtable `0x30A7D40`, `Prepare 0x16B62F0`, `Handle 0x16B6340` already recorded; resolve `Decide` from slot 8.
  - `REQUEST_CONTROL_EVENT` — id 4, registered `0x16B6120`.
- [ ] **Step 2 — run every prologue through `check_prologue`** (template step 2). Any refusal means that event is not hooked; report it and continue with the rest.
- [ ] **Step 3 — derive the field offsets** (template step 3). You need the event's object-id field and a way to compare it against the local player's ped and current vehicle net objects (`CPed+0xD0` is `m_pNetObj`; `netObject+0x0A` is the object id — both live-verified in Tier 1).
- [ ] **Step 4 — write the hook file** (template step 4), one hook per event, each honouring the per-player `net_events` block.
- [ ] **Step 5 — wire the registry** (template step 5) with the filter ids named above.
- [ ] **Step 6 — record evidence and build** (template step 6). Set `INSULIN_BUILD_TAG` to `"prot-t2-control-1"`.
- [ ] **Step 7 — commit and write the console notes** (template step 7), stating per filter what a `Log` report means.


### Task 9: Ped task events

**Files:** create `src/protections/hooks_events_pedtask.cpp`; modify registry, test stubs, `build_tag.h`, anchors doc.
**Filter ids:** `clear_ped_tasks = 36`, `ragdoll_request = 37`.

`NETWORK_CLEAR_PED_TASKS_EVENT` (id 43, `0x16CB090`), `RAGDOLL_REQUEST_EVENT` (id 24, registered `CRagdollRequestEvent__Factory` `0x16C3D40` — already named in the IDB).

Both are direct griefing tools: clear-tasks interrupts whatever you are doing, ragdoll-request knocks you down remotely. The check is whether the target ped is the local player and whether the sender has any legitimate reason to be acting on it.

**Steps** (mechanics are in the Phase C template above; these are your targets):

- [ ] **Step 1 — resolve vtables and `Decide` RVAs** (template step 1) for:
  - `NETWORK_CLEAR_PED_TASKS_EVENT` — id 43, registered `0x16CB090`.
  - `RAGDOLL_REQUEST_EVENT` — id 24, registered `CRagdollRequestEvent__Factory` `0x16C3D40` (already named in the IDB).
- [ ] **Step 2 — run every prologue through `check_prologue`** (template step 2). Any refusal means that event is not hooked; report it and continue with the rest.
- [ ] **Step 3 — derive the field offsets** (template step 3). You need each event's target ped/object-id field, compared against the local player's net object id.
- [ ] **Step 4 — write the hook file** (template step 4), one hook per event, each honouring the per-player `net_events` block.
- [ ] **Step 5 — wire the registry** (template step 5) with the filter ids named above.
- [ ] **Step 6 — record evidence and build** (template step 6). Set `INSULIN_BUILD_TAG` to `"prot-t2-pedtask-1"`.
- [ ] **Step 7 — commit and write the console notes** (template step 7), stating per filter what a `Log` report means.


### Task 10: Script state events

**Files:** create `src/protections/hooks_events_scriptstate.cpp`; modify registry, test stubs, `build_tag.h`, anchors doc.
**Filter ids:** `script_world_state = 38`, `script_entity_state = 39`.

`SCRIPT_WORLD_STATE_EVENT` (id 33, `0x16C7C80`), `SCRIPT_ENTITY_STATE_CHANGE_EVENT` (id 50, `0x16CD990`).

`SCRIPT_WORLD_STATE_EVENT` carries a state-type discriminator; YimMenu rejects out-of-range types and specific hostile ones. Derive the valid range from the game's own switch, the way Tier 1 took the draw-list capacity from the game's own bound check — that is the only kind of range worth trusting.

**Steps** (mechanics are in the Phase C template above; these are your targets):

- [ ] **Step 1 — resolve vtables and `Decide` RVAs** (template step 1) for:
  - `SCRIPT_WORLD_STATE_EVENT` — id 33, registered `0x16C7C80`.
  - `SCRIPT_ENTITY_STATE_CHANGE_EVENT` — id 50, registered `0x16CD990`.
- [ ] **Step 2 — run every prologue through `check_prologue`** (template step 2). Any refusal means that event is not hooked; report it and continue with the rest.
- [ ] **Step 3 — derive the field offsets** (template step 3). The state-type discriminator's valid range must come from the game's own switch or bound check, not from YimMenu.
- [ ] **Step 4 — write the hook file** (template step 4), one hook per event, each honouring the per-player `net_events` block.
- [ ] **Step 5 — wire the registry** (template step 5) with the filter ids named above.
- [ ] **Step 6 — record evidence and build** (template step 6). Set `INSULIN_BUILD_TAG` to `"prot-t2-scriptstate-1"`.
- [ ] **Step 7 — commit and write the console notes** (template step 7), stating per filter what a `Log` report means.


### Task 11: Nuisance events

**Files:** create `src/protections/hooks_events_nuisance.cpp`; modify registry, test stubs, `build_tag.h`, anchors doc.
**Filter ids:** `play_sound = 40`, `change_radio = 41`, `door_break = 42`.

`NETWORK_PLAY_SOUND_EVENT` (id 51, `0x16CE290`), `CHANGE_RADIO_STATION_EVENT` (id 23, `0x16C3A50`), `DOOR_BREAK_EVENT` (id 27, `0x16C5310`).

Sound spam is the highest-volume attack in the set, which makes it the best test of the coalescer under real load — expect the log to show a large `+N` rather than a flood. Radio-station changes are only legitimate from a player in your vehicle; YimMenu also rate-limits them per sender.

**Steps** (mechanics are in the Phase C template above; these are your targets):

- [ ] **Step 1 — resolve vtables and `Decide` RVAs** (template step 1) for:
  - `NETWORK_PLAY_SOUND_EVENT` — id 51, registered `0x16CE290`.
  - `CHANGE_RADIO_STATION_EVENT` — id 23, registered `0x16C3A50`.
  - `DOOR_BREAK_EVENT` — id 27, registered `0x16C5310`.
- [ ] **Step 2 — run every prologue through `check_prologue`** (template step 2). Any refusal means that event is not hooked; report it and continue with the rest.
- [ ] **Step 3 — derive the field offsets** (template step 3). For sound: the sound-name and soundset hashes plus the entity flag. For radio: the vehicle net id, compared against the local player's current vehicle.
- [ ] **Step 4 — write the hook file** (template step 4), one hook per event, each honouring the per-player `net_events` block.
- [ ] **Step 5 — wire the registry** (template step 5) with the filter ids named above.
- [ ] **Step 6 — record evidence and build** (template step 6). Set `INSULIN_BUILD_TAG` to `"prot-t2-nuisance-1"`.
- [ ] **Step 7 — commit and write the console notes** (template step 7), stating per filter what a `Log` report means.


### Task 12: Report events

**Files:** create `src/protections/hooks_events_report.cpp`; modify registry, test stubs, `build_tag.h`, anchors doc.
**Filter ids:** `report_cash_spawn = 43`, `report_myself = 44`.

`REPORT_CASH_SPAWN_EVENT` (id 83, `0x16D94C0`), `REPORT_MYSELF_EVENT` (id 82, `0x16D9210`).

These make *you* report yourself to R\* telemetry. On an LSO private server the telemetry goes nowhere, so the practical value is lower than on PC — say so in the console notes rather than overstating it. They are cheap to add while the surrounding work is being done, and they matter if the client is ever pointed at a real backend.

**Steps** (mechanics are in the Phase C template above; these are your targets):

- [ ] **Step 1 — resolve vtables and `Decide` RVAs** (template step 1) for:
  - `REPORT_CASH_SPAWN_EVENT` — id 83, registered `0x16D94C0`.
  - `REPORT_MYSELF_EVENT` — id 82, registered `0x16D9210`.
- [ ] **Step 2 — run every prologue through `check_prologue`** (template step 2). Any refusal means that event is not hooked; report it and continue with the rest.
- [ ] **Step 3 — derive the field offsets** (template step 3). Minimal: these are blocked outright rather than validated, so you mainly need to confirm the hook shape and that blocking is inert on an LSO server.
- [ ] **Step 4 — write the hook file** (template step 4), one hook per event, each honouring the per-player `net_events` block.
- [ ] **Step 5 — wire the registry** (template step 5) with the filter ids named above.
- [ ] **Step 6 — record evidence and build** (template step 6). Set `INSULIN_BUILD_TAG` to `"prot-t2-report-1"`.
- [ ] **Step 7 — commit and write the console notes** (template step 7), stating per filter what a `Log` report means.


### Task 13: Blast events

**Files:** create `src/protections/hooks_events_blast.cpp`; modify registry, test stubs, `build_tag.h`, anchors doc.
**Filter ids:** `explosion = 45`, `vehicle_special_ability = 46`, `kick_votes = 47`.

`EXPLOSION_EVENT` (id 17, `0x16BF8B0`), `ACTIVATE_VEHICLE_SPECIAL_ABILITY_EVENT` (id 84, `0x16D2F80`), `KICK_VOTES_EVENT` (id 64, `0x16D2300`).

`EXPLOSION_EVENT` is the most-used remote kill. The check is explosion type in range and the owner/position being plausible. `KICK_VOTES_EVENT` carries a player bitfield; a vote naming you is worth reporting even when you do not block it, because it tells you a kick attempt is in progress.

**Steps** (mechanics are in the Phase C template above; these are your targets):

- [ ] **Step 1 — resolve vtables and `Decide` RVAs** (template step 1) for:
  - `EXPLOSION_EVENT` — id 17, registered `0x16BF8B0`.
  - `ACTIVATE_VEHICLE_SPECIAL_ABILITY_EVENT` — id 84, registered `0x16D2F80`.
  - `KICK_VOTES_EVENT` — id 64, registered `0x16D2300`.
- [ ] **Step 2 — run every prologue through `check_prologue`** (template step 2). Any refusal means that event is not hooked; report it and continue with the rest.
- [ ] **Step 3 — derive the field offsets** (template step 3). For explosions: the type field's valid range from the game's own bound check, plus the owner and position fields. For kick votes: the player bitfield, tested against the local physical player index.
- [ ] **Step 4 — write the hook file** (template step 4), one hook per event, each honouring the per-player `net_events` block.
- [ ] **Step 5 — wire the registry** (template step 5) with the filter ids named above.
- [ ] **Step 6 — record evidence and build** (template step 6). Set `INSULIN_BUILD_TAG` to `"prot-t2-blast-1"`.
- [ ] **Step 7 — commit and write the console notes** (template step 7), stating per filter what a `Log` report means.


---

## The console gate

**Tasks 1–13 can all be built and reviewed without a console. Task 14 cannot.**

Before Task 14, the operator must run the Tier 2 build in a live LSO session with `script_event` in `Log`, and collect the learned hash set:

1. Deploy, join a session, press **Clear Learned Events** to start a clean baseline.
2. Play normally for a full session — missions, shops, vehicles, other players present. Every hash seen here is **benign** and must never be blocked.
3. Record the set from the kernel log (`prot would-block Script Events a=<hash> b=<count>`) or the in-game log from Task 4.
4. If a known attack tool is available, run a second session against it with a cleared table. Hashes appearing only in that session are the candidates.
5. Write both sets into `docs/superpowers/plans/2026-08-25-protections-tier2-learned-events.md`, with the session conditions for each.

A hash that appears in the benign baseline must not go on the block list regardless of what any PC menu calls it. That is the entire reason this tier derives instead of porting.

---

## Phase D — Blocked, pending console data

### Task 14: The script-event block list

**Files:** modify `src/protections/hooks_script_event.cpp`, `src/menu/base/submenus/protections.cpp`.

**Do not start this task until the console gate above has produced a hash set.** There is nothing to write without it, and writing it speculatively is exactly the failure the spec warns about — a block list of unverified constants either does nothing or drops legitimate script events, and the second presents as random mission failures nobody attributes to the menu.

When the data exists: add a compile-time table of `{ hash, name, evidence }` to `hooks_script_event.cpp`, each entry carrying the session in which it was observed hostile. Add per-event toggles to the Protections submenu grouped by effect (money, teleport, crash, nuisance), and block only when the filter is in `Enforce` **and** that entry's toggle is on.

Keep learn mode running alongside the block list permanently. It is how the next event gets identified.

---

## Tier 2 completion criteria

1. Every landed filter installs and the game runs normally with all of them in `Log`. No crash, no measurable frame cost.
2. `check_prologue` refuses at least one real target — if it never refuses anything across eighteen prologues, verify it is actually running rather than assuming every target happened to be safe.
3. Ten minutes of normal play produces **zero** reports from the per-event filters in `Log`. The script-event filter is exempt: it reports every newly-seen hash by design, and that is the data being collected.
4. The learned hash set is written down with its session conditions.
5. Per-player `net_events` blocking demonstrably drops one player's script events while leaving others unaffected.
6. Every anchor is recorded in `analysis/PROTECTIONS_ANCHORS.md` with decompiled evidence.

Filters that could not be anchored, or whose prologue `check_prologue` refuses, are reported as such — not guessed, not force-hooked.
