# Animated Textures Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A reusable `animated_texture` block that plays a frame sequence anywhere the menu draws a sprite, with the menu header banner as its first consumer.

**Architecture:** Frames are converted from a GIF on the PC into numbered PNGs plus a `frames.json` manifest, loaded into the existing custom texture dictionary (one texture per frame), and played by swapping the drawn texture name each tick. The frame-selection maths lives in a dependency-free header so it can be unit-tested on the host; everything else is gated on the PS4 build plus on-console acceptance.

**Tech Stack:** C++17 freestanding (OpenOrbis PS4 toolchain, clang 18, lld), GoldHEN GHPLUGIN, project mini-STL (`src/stl/`), `tj` mini-JSON (`src/util/json.h`), PowerShell 7 + .NET `System.Drawing` for the converter.

**Design spec:** `docs/superpowers/specs/2026-08-11-animated-textures-design.md` — read it first; this plan implements it and does not restate its reasoning.

## Global Constraints

- **Target:** GTA V PS4 **CUSA00411 v1.57**; artifact `build/InsulinGTAV.prx` (GHPLUGIN).
- **No libc++/STL.** Use `stl::` types from `src/stl/` only. Never `std::` in ported code. Never include libc++ headers (`<string>`, `<vector>`, `<memory>`, `<functional>`, `<algorithm>`, …). C headers (`<stdint.h>`, `<stdio.h>`, `<string.h>`, `<stdarg.h>`, `<stdlib.h>`) are fine.
- **No `.init_array`.** GoldHEN does not run global constructors. No namespace-scope objects with non-trivial constructors. Singletons are function-local statics; POD globals only, `constexpr` where possible.
- **No Windows / no SEH** in plugin code: no `<Windows.h>`, `VK_*`, `GetTickCount`, `timeGetTime`, `GetAsyncKeyState`.
- **No hooking/fibers/threads inside the menu.** Everything runs on the script thread via the frame hook.
- **`stl::vector` has no `data()`** — take `&v[0]`, and only after checking `size() > 0`.
- **PS4 build command (from repo root):** `./build.bat`
  A task's "build passes" = that command exits 0 and produces `build/InsulinGTAV.prx`. (`build.bat` locates Ninja inside the Visual Studio install; plain `cmake -G Ninja` fails here because Ninja is not on PATH.)
- **Host test build:** `clang++ -std=c++17 -I src <test.cpp> -o build/<name>.exe`
  Host test sources may include **only C headers** (`<stdio.h>`, `<stdint.h>`). MSVC's C++ standard library rejects the installed Clang (`error STL1000: expected Clang 20 or newer`), so `<cstdio>` and friends will not compile. This is a constraint on test files, not on plugin code.
- **Values from the spec, verbatim:** max **16** frames per animation; default frame delay **66 ms**; animation directory `/data/insulin/anim/<name>/`; manifest `frames.json`; texture names `<anim>_000`…`<anim>_015`; the header animation is the one registered as `banner`.
- **Commit after every task.** Conventional commit messages, trailer:
  `Co-Authored-By: Claude Opus 5 (1M context) <noreply@anthropic.com>`

---

## File Structure

| File | Responsibility |
|---|---|
| `src/menu/base/util/frame_clock.h` (create) | Pure frame-selection maths. No STL, no PS4 headers, no state. Host-testable. |
| `tests/frame_clock_test.cpp` (create) | Host unit tests for the above. |
| `src/menu/base/util/animated_texture.h` (create) | The `animated_texture` type and the `menu::animation` registry API. |
| `src/menu/base/util/animated_texture.cpp` (create) | Timing/playback, the registry, and the directory/manifest loader. |
| `tools/gif2frames.ps1` (create) | GIF → frame directory + `frames.json` on the PC. |
| `src/menu/menu.cpp` (modify) | Call `menu::animation::update()` once per tick. |
| `src/menu/base/renderer.cpp` (modify) | Header draws `menu::animation::header_asset()`. |
| `src/menu/base/submenus/main.cpp` (modify) | Button that loads the banner animation. |

The loader lives with the type rather than in `rage::gfx` because it depends on `rage::gfx` — dependency direction stays `menu → rage`, as everywhere else in the tree.

---

## Task 1: frame_clock — the timing core

**Files:**
- Create: `src/menu/base/util/frame_clock.h`
- Test: `tests/frame_clock_test.cpp`

**Interfaces:**
- Consumes: nothing.
- Produces: `menu::frame_clock::total_ms(const uint16_t* delays, int count) -> int` and `menu::frame_clock::frame_at(const uint16_t* delays, int count, int elapsed_ms, bool loop) -> int`. Task 3 calls both.

- [ ] **Step 1: Write the failing test**

Create `tests/frame_clock_test.cpp`:

```cpp
// Host unit tests for the animated-texture timing core.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/frame_clock_test.cpp -o build/frame_clock_test.exe
//   ./build/frame_clock_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
#include "menu/base/util/frame_clock.h"
#include <stdio.h>
#include <stdint.h>

static int g_failed = 0;

static void check(const char* what, int got, int want) {
    if (got != want) { printf("FAIL %s: got %d, want %d\n", what, got, want); g_failed++; }
    else             { printf("ok   %s = %d\n", what, got); }
}

int main() {
    using namespace menu::frame_clock;

    // A three-frame sequence: 100 ms, 100 ms, 200 ms -> 400 ms per pass.
    const uint16_t d[3] = { 100, 100, 200 };
    check("total", total_ms(d, 3), 400);

    // Boundaries: a frame owns [start, start + delay).
    check("t=0",   frame_at(d, 3, 0,   true), 0);
    check("t=99",  frame_at(d, 3, 99,  true), 0);
    check("t=100", frame_at(d, 3, 100, true), 1);
    check("t=199", frame_at(d, 3, 199, true), 1);
    check("t=200", frame_at(d, 3, 200, true), 2);
    check("t=399", frame_at(d, 3, 399, true), 2);

    // Looping wraps modulo the total; a long gap must not run off the end.
    check("wrap t=400",    frame_at(d, 3, 400,    true), 0);
    check("wrap t=450",    frame_at(d, 3, 450,    true), 0);
    check("wrap t=500",    frame_at(d, 3, 500,    true), 1);
    check("wrap t=100000", frame_at(d, 3, 100000, true), 0);

    // One-shot stops on the last frame and stays there.
    check("oneshot t=400",   frame_at(d, 3, 400,   false), 2);
    check("oneshot t=99999", frame_at(d, 3, 99999, false), 2);

    // Degenerate inputs must not divide by zero, loop forever, or go out of range.
    const uint16_t zero[2] = { 0, 0 };
    check("all-zero delays", frame_at(zero, 2, 500, true), 0);
    check("zero total",      total_ms(zero, 2), 0);
    check("empty count",     frame_at(d, 0, 500, true), 0);
    check("negative elapsed", frame_at(d, 3, -5, true), 0);

    const uint16_t one[1] = { 66 };
    check("single frame t=0",    frame_at(one, 1, 0,    true), 0);
    check("single frame t=1000", frame_at(one, 1, 1000, true), 0);

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed ? 1 : 0;
}
```

- [ ] **Step 2: Run the test to verify it fails**

```bash
clang++ -std=c++17 -I src tests/frame_clock_test.cpp -o build/frame_clock_test.exe
```

Expected: FAIL to compile — `'menu/base/util/frame_clock.h' file not found`.

- [ ] **Step 3: Write the implementation**

Create `src/menu/base/util/frame_clock.h`:

```cpp
#pragma once
#include <stdint.h>

// Frame-selection maths for animated textures: which frame of a delay sequence
// is showing after N milliseconds.
//
// Deliberately free of STL, PS4 headers and engine state so it compiles for the
// host and is unit-tested on the PC (tests/frame_clock_test.cpp). It is the only
// part of the animation feature with real edge cases -- wrapping, one-shot
// endings, degenerate delay tables -- so it is the part worth testing off-target.
namespace menu::frame_clock {

    // Duration of one pass through `delays`, in ms. 0 if there is nothing to play.
    inline int total_ms(const uint16_t* delays, int count) {
        int total = 0;
        for (int i = 0; i < count; i++) total += (int)delays[i];
        return total;
    }

    // Index of the frame showing `elapsed_ms` into playback. A frame owns the
    // half-open span [start, start + delay).
    //   loop == true  -> the sequence repeats; elapsed wraps modulo the total
    //   loop == false -> playback stops on the last frame and stays there
    // Returns 0 for an empty sequence or an all-zero delay table (which would
    // otherwise divide by zero), and never returns an out-of-range index.
    inline int frame_at(const uint16_t* delays, int count, int elapsed_ms, bool loop) {
        if (count <= 0) return 0;
        const int total = total_ms(delays, count);
        if (total <= 0) return 0;
        if (elapsed_ms <= 0) return 0;

        if (elapsed_ms >= total) {
            if (!loop) return count - 1;
            elapsed_ms %= total;
        }

        int t = elapsed_ms;
        for (int i = 0; i < count; i++) {
            if (t < (int)delays[i]) return i;
            t -= (int)delays[i];
        }
        return count - 1;   // not reachable while total > 0; a guard, not a path
    }
}
```

- [ ] **Step 4: Run the tests to verify they pass**

```bash
clang++ -std=c++17 -I src tests/frame_clock_test.cpp -o build/frame_clock_test.exe && ./build/frame_clock_test.exe
```

Expected: every line prefixed `ok`, final line `all passed`, exit code 0.

- [ ] **Step 5: Confirm the PS4 build still succeeds**

```bash
./build.bat
```

Expected: exit 0, `build/InsulinGTAV.prx` produced. (The header is not referenced yet; this only proves it does not break the tree.)

- [ ] **Step 6: Commit**

```bash
git add src/menu/base/util/frame_clock.h tests/frame_clock_test.cpp
git commit -m "feat(ui): frame-selection maths for animated textures, host-tested"
```

---

## Task 2: gif2frames.ps1 — the converter

**Files:**
- Create: `tools/gif2frames.ps1`
- Test: `tests/gif2frames_test.ps1`

**Interfaces:**
- Consumes: nothing.
- Produces: a directory of `000.png…NNN.png` plus `frames.json` in the shape Task 4 parses:
  `{ "loop": bool, "default_delay": int, "frames": [ { "file": string, "delay": int } ] }`

- [ ] **Step 1: Write the failing test**

The fixture is built byte by byte rather than shipped as a binary: an 85-byte GIF89a with two 1×1 frames whose stored delays are 10 and 20 hundredths of a second. Those exact bytes were written and read back with `System.Drawing` while this plan was being written — a correct reader reports `frames=2` and property `0x5100` = `10,0,0,0,20,0,0,0` (a packed array of 4-byte little-endian ints, one per frame, in 1/100 s), i.e. 100 ms and 200 ms.

Create `tests/gif2frames_test.ps1`:

```powershell
<#
    Tests tools/gif2frames.ps1 against a hand-built fixture GIF.
    Run:  pwsh -File tests/gif2frames_test.ps1
    Exit: 0 all passed, 1 something failed.
#>
$ErrorActionPreference = 'Stop'
$root    = Split-Path -Parent $PSScriptRoot
$script  = Join-Path $root 'tools\gif2frames.ps1'
$work    = Join-Path $root 'build\fixture'
$failed  = 0

function Check([string]$what, $got, $want) {
    if ("$got" -ne "$want") { Write-Host "FAIL $what : got '$got', want '$want'"; $script:failed++ }
    else                    { Write-Host "ok   $what = $got" }
}

# --- fixture: GIF89a, 1x1, two frames, delays 10 and 20 (1/100 s) ------------
$bytes = [byte[]]@(0x47,0x49,0x46,0x38,0x39,0x61, 0x01,0x00, 0x01,0x00, 0x80,0x00,0x00,
          0x00,0x00,0x00, 0xFF,0xFF,0xFF, 0x21,0xFF,0x0B) +
         [System.Text.Encoding]::ASCII.GetBytes("NETSCAPE2.0") +
         [byte[]]@(0x03,0x01,0x00,0x00,0x00,
          0x21,0xF9,0x04,0x00,0x0A,0x00,0x00,0x00,
          0x2C,0x00,0x00,0x00,0x00,0x01,0x00,0x01,0x00,0x00, 0x02,0x02,0x44,0x01,0x00,
          0x21,0xF9,0x04,0x00,0x14,0x00,0x00,0x00,
          0x2C,0x00,0x00,0x00,0x00,0x01,0x00,0x01,0x00,0x00, 0x02,0x02,0x44,0x01,0x00,
          0x3B)

New-Item -ItemType Directory -Force -Path $work | Out-Null
$gif = Join-Path $work 'two.gif'
[System.IO.File]::WriteAllBytes($gif, $bytes)

# The fixture must be readable, or every assertion below tests nothing.
Add-Type -AssemblyName System.Drawing
$img = [System.Drawing.Image]::FromFile($gif)
try {
    $dim = New-Object System.Drawing.Imaging.FrameDimension $img.FrameDimensionsList[0]
    Check 'fixture frame count' $img.GetFrameCount($dim) 2
    Check 'fixture delay table' ($img.GetPropertyItem(0x5100).Value -join ',') '10,0,0,0,20,0,0,0'
} finally { $img.Dispose() }

# --- full conversion --------------------------------------------------------
$out = Join-Path $work 'out'
if (Test-Path $out) { Remove-Item $out -Recurse -Force }
& pwsh -File $script -Gif $gif -OutDir $out | Out-Null

Check 'wrote 000.png'  (Test-Path (Join-Path $out '000.png')) 'True'
Check 'wrote 001.png'  (Test-Path (Join-Path $out '001.png')) 'True'
Check 'wrote manifest' (Test-Path (Join-Path $out 'frames.json')) 'True'

$m = Get-Content (Join-Path $out 'frames.json') -Raw | ConvertFrom-Json
Check 'manifest frame count' $m.frames.Count 2
Check 'frame 0 file'         $m.frames[0].file '000.png'
Check 'frame 0 delay (ms)'   $m.frames[0].delay 100
Check 'frame 1 delay (ms)'   $m.frames[1].delay 200
Check 'loop flag'            $m.loop 'True'
Check 'default delay'        $m.default_delay 66

# --- down-sampling keeps total duration and says so -------------------------
$out1 = Join-Path $work 'out1'
if (Test-Path $out1) { Remove-Item $out1 -Recurse -Force }
& pwsh -File $script -Gif $gif -OutDir $out1 -MaxFrames 1 3>$null | Out-Null

$m1 = Get-Content (Join-Path $out1 'frames.json') -Raw | ConvertFrom-Json
Check 'sampled frame count'   $m1.frames.Count 1
Check 'folded delay (100+200)' $m1.frames[0].delay 300

if ($failed) { Write-Host "`n$failed FAILED"; exit 1 }
Write-Host "`nall passed"; exit 0
```

- [ ] **Step 2: Run the test to verify it fails**

```powershell
pwsh -File tests\gif2frames_test.ps1
```

Expected: the two fixture checks pass (proving the fixture itself is sound), then failure — `tools/gif2frames.ps1` does not exist yet, so the run throws and no output files appear.

- [ ] **Step 3: Write the converter**

Create `tools/gif2frames.ps1`:

```powershell
<#
.SYNOPSIS
    Converts an animated GIF into the frame directory the InsulinGTAV menu loads.

.DESCRIPTION
    Writes 000.png..NNN.png plus frames.json (per-frame delays in ms) into -OutDir.
    Copy that directory to /data/insulin/anim/<name>/ on the console.

    Uses .NET System.Drawing rather than ffmpeg or ImageMagick: neither is installed
    on the dev machine and neither ships with Windows, so depending on one would make
    this script fail on first use. Consequences, by design: PNG output only (DDS would
    need a block compressor) and frames keep the GIF's own resolution.

    The menu plays at most 16 frames, so longer GIFs are sampled down evenly and the
    delays of dropped frames are added to the frame that is kept -- total duration and
    playback speed are preserved, smoothness is not.

.EXAMPLE
    pwsh -File tools/gif2frames.ps1 -Gif banner.gif -OutDir out/banner
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)] [string] $Gif,
    [Parameter(Mandatory = $true)] [string] $OutDir,
    [int]  $MaxFrames    = 16,
    [int]  $DefaultDelay = 66,
    [bool] $Loop         = $true
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path -LiteralPath $Gif)) { throw "GIF not found: $Gif" }
Add-Type -AssemblyName System.Drawing

$img = [System.Drawing.Image]::FromFile((Resolve-Path -LiteralPath $Gif))
try {
    $dim   = New-Object System.Drawing.Imaging.FrameDimension $img.FrameDimensionsList[0]
    $count = $img.GetFrameCount($dim)
    if ($count -lt 1) { throw "No frames in $Gif" }

    # Property 0x5100 (FrameDelay) is a packed array of 4-byte LE ints in 1/100 s.
    # A still GIF has no such property at all.
    $delays = @()
    try {
        $raw = $img.GetPropertyItem(0x5100).Value
        for ($i = 0; $i -lt $count; $i++) {
            $ms = [BitConverter]::ToInt32($raw, $i * 4) * 10
            $delays += $(if ($ms -le 0) { $DefaultDelay } else { $ms })
        }
    } catch {
        Write-Warning "No frame-delay data in the GIF; using ${DefaultDelay} ms for every frame."
        $delays = @($DefaultDelay) * $count
    }

    # Even sampling down to $MaxFrames, folding dropped delays into the kept frame.
    $keep = @(0..($count - 1))
    if ($count -gt $MaxFrames) {
        $keep = @(0..($MaxFrames - 1) | ForEach-Object { [int][Math]::Floor($_ * $count / $MaxFrames) })
        Write-Warning "GIF has $count frames; sampling down to $MaxFrames (the menu caps there). Total duration is preserved."
    }

    $kept = @()
    for ($k = 0; $k -lt $keep.Count; $k++) {
        $from = $keep[$k]
        $to   = if ($k + 1 -lt $keep.Count) { $keep[$k + 1] } else { $count }
        $sum  = 0
        for ($j = $from; $j -lt $to; $j++) { $sum += $delays[$j] }
        $kept += [pscustomobject]@{ Index = $from; Delay = $sum }
    }

    New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
    Get-ChildItem -LiteralPath $OutDir -Filter '*.png' -ErrorAction SilentlyContinue | Remove-Item -Force

    $manifest = @()
    for ($k = 0; $k -lt $kept.Count; $k++) {
        $img.SelectActiveFrame($dim, $kept[$k].Index) | Out-Null
        $name = '{0:d3}.png' -f $k
        $bmp  = New-Object System.Drawing.Bitmap $img.Width, $img.Height
        try {
            $g = [System.Drawing.Graphics]::FromImage($bmp)
            try { $g.DrawImage($img, 0, 0, $img.Width, $img.Height) } finally { $g.Dispose() }
            $bmp.Save((Join-Path $OutDir $name), [System.Drawing.Imaging.ImageFormat]::Png)
        } finally { $bmp.Dispose() }
        $manifest += [pscustomobject]@{ file = $name; delay = $kept[$k].Delay }
    }

    [pscustomobject]@{
        loop          = $Loop
        default_delay = $DefaultDelay
        frames        = $manifest
    } | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $OutDir 'frames.json') -Encoding utf8

    $total = ($manifest | Measure-Object -Property delay -Sum).Sum
    Write-Host "$($manifest.Count) frame(s), $($img.Width)x$($img.Height), ${total} ms total -> $OutDir"
} finally {
    $img.Dispose()
}
```

Note on the redraw: `SelectActiveFrame` mutates the shared `Image`, so each frame is copied into its own `Bitmap` before saving — saving `$img` directly writes the same frame every time in some GDI+ versions.

- [ ] **Step 4: Run the test to verify it passes**

```powershell
pwsh -File tests\gif2frames_test.ps1
```

Expected: every line prefixed `ok`, final line `all passed`, exit code 0. In particular `frame 0 delay (ms) = 100`, `frame 1 delay (ms) = 200` (the GIF's hundredths converted to milliseconds) and `folded delay (100+200) = 300` (down-sampling preserves total duration).

If the run reports `sampled frame count` correct but `folded delay` wrong, the delay-folding loop is summing the wrong span — check that the last kept frame folds in every remaining frame up to `$count`, not just up to the next kept index.

- [ ] **Step 5: Commit**

```bash
git add tools/gif2frames.ps1 tests/gif2frames_test.ps1
git commit -m "feat(tools): GIF to frame-directory converter via System.Drawing"
```

---

## Task 3: animated_texture — playback

**Files:**
- Create: `src/menu/base/util/animated_texture.h`
- Create: `src/menu/base/util/animated_texture.cpp`

**Interfaces:**
- Consumes: `menu::frame_clock::frame_at/total_ms` (Task 1).
- Produces: class `menu::animated_texture` with `set_dictionary(const char*)`, `add_frame(const char*, int)`, `set_loop(bool)`, `advance(float dt_seconds)`, `reset()`, `current() -> stl::pair<stl::string, stl::string>`, `current_index() -> int`, `frame_count() -> int`, `ready() -> bool`. Task 4 fills it; Task 5 draws `current()`.

There is no host test for this task: the type depends on the mini-STL and `platform/stdafx.h`, which do not build for the host. Its only non-trivial logic is delegated to the Task 1 header, which is tested. The gate here is the PS4 build.

- [ ] **Step 1: Write the header**

Create `src/menu/base/util/animated_texture.h`:

```cpp
#pragma once
#include "platform/stdafx.h"
#include "stl/string.h"
#include "stl/vector.h"
#include "stl/pair.h"
#include <stdint.h>

// A frame sequence living in one custom texture dictionary, played by handing
// the renderer a different texture name each tick.
//
// This type knows nothing about files or image formats: it holds names and
// delays. That is the seam a future runtime GIF decoder plugs into -- it would
// call add_frame() with textures it created and nothing here would change.
namespace menu {
    class animated_texture {
    public:
        animated_texture() : m_elapsed_ms(0), m_loop(true) {}

        void set_dictionary(const char* dict) { m_dict = dict ? dict : ""; }
        void set_loop(bool loop)              { m_loop = loop; }

        // delay_ms <= 0 is replaced by the 66 ms default (GIFs commonly store 0).
        void add_frame(const char* texture_name, int delay_ms);

        // dt in seconds -- pass global::ui::g_delta. Frames are never stepped one
        // at a time: the accumulator is absolute, so a dropped frame catches up
        // instead of putting the animation into slow motion.
        void advance(float dt_seconds);
        void reset() { m_elapsed_ms = 0; }

        // Drop all frames and rewind. Used when an animation is reloaded.
        void clear();

        // {dictionary, texture name} of the current frame, ready for draw_sprite.
        // Both strings are empty when the animation holds no frames.
        stl::pair<stl::string, stl::string> current() const;
        int  current_index() const;

        int  frame_count() const { return (int)m_names.size(); }
        bool ready() const       { return m_names.size() > 0; }

    private:
        stl::string              m_dict;
        stl::vector<stl::string> m_names;
        stl::vector<uint16_t>    m_delays;
        int                      m_elapsed_ms;
        bool                     m_loop;
    };

    // The registry of loaded animations. Registration is a function-local static
    // (no .init_array on GoldHEN); animations live for the session.
    namespace animation {
        // Default frame delay in ms when a manifest omits or zeroes one.
        static const int k_default_delay_ms = 66;
        // Frames beyond this are refused, with a log line -- never silently.
        static const int k_max_frames = 16;

        // The animation registered under `name`, or nullptr.
        animated_texture* get(const char* name);

        // Register (or replace) an empty animation under `name` and return it.
        animated_texture* create(const char* name);

        // Advance every registered animation. Called once per tick.
        void update(float dt_seconds);

        // Banner frame if the animation named "banner" is loaded and ready,
        // otherwise the static {"insulin", "logo"} pair.
        stl::pair<stl::string, stl::string> header_asset();
    }
}
```

- [ ] **Step 2: Write the implementation**

Create `src/menu/base/util/animated_texture.cpp`:

```cpp
#include "menu/base/util/animated_texture.h"
#include "menu/base/util/frame_clock.h"
#include "platform/log.h"

namespace menu {
    void animated_texture::add_frame(const char* texture_name, int delay_ms) {
        if (!texture_name || !texture_name[0]) return;
        if ((int)m_names.size() >= animation::k_max_frames) {
            LOG_WARN("anim: \"%s\" already holds %d frames, refusing \"%s\"",
                     m_dict.c_str(), animation::k_max_frames, texture_name);
            return;
        }
        if (delay_ms <= 0) delay_ms = animation::k_default_delay_ms;
        if (delay_ms > 65535) delay_ms = 65535;          // uint16_t storage
        m_names.push_back(stl::string(texture_name));
        m_delays.push_back((uint16_t)delay_ms);
    }

    void animated_texture::clear() {
        m_names.clear();
        m_delays.clear();
        m_elapsed_ms = 0;
    }

    void animated_texture::advance(float dt_seconds) {
        if (m_names.size() == 0 || dt_seconds <= 0.f) return;
        m_elapsed_ms += (int)(dt_seconds * 1000.f + 0.5f);

        // Bound the accumulator either way, so it cannot overflow across a long
        // session (signed overflow is UB, and int milliseconds run out after ~24
        // days of uptime). Looping wraps; one-shot saturates at the end, which
        // frame_clock already reads as "stay on the last frame".
        const int total = menu::frame_clock::total_ms(&m_delays[0], (int)m_delays.size());
        if (total > 0) {
            if (m_loop)                     m_elapsed_ms %= total;
            else if (m_elapsed_ms > total)  m_elapsed_ms = total;
        }
    }

    int animated_texture::current_index() const {
        if (m_names.size() == 0) return 0;
        return menu::frame_clock::frame_at(&m_delays[0], (int)m_delays.size(), m_elapsed_ms, m_loop);
    }

    stl::pair<stl::string, stl::string> animated_texture::current() const {
        if (m_names.size() == 0) return stl::make_pair(stl::string(""), stl::string(""));
        return stl::make_pair(m_dict, m_names[current_index()]);
    }

    namespace animation {
        struct slot { stl::string name; animated_texture anim; };

        static stl::vector<slot>& registry() {
            static stl::vector<slot> instance;   // function-local: no .init_array
            return instance;
        }

        animated_texture* get(const char* name) {
            if (!name) return nullptr;
            stl::vector<slot>& r = registry();
            for (size_t i = 0; i < r.size(); i++)
                if (r[i].name == name) return &r[i].anim;
            return nullptr;
        }

        animated_texture* create(const char* name) {
            if (!name || !name[0]) return nullptr;
            animated_texture* existing = get(name);
            if (existing) { existing->clear(); return existing; }

            slot s;
            s.name = name;
            registry().push_back(s);
            return &registry()[registry().size() - 1].anim;
        }

        void update(float dt_seconds) {
            stl::vector<slot>& r = registry();
            for (size_t i = 0; i < r.size(); i++) r[i].anim.advance(dt_seconds);
        }

        stl::pair<stl::string, stl::string> header_asset() {
            animated_texture* banner = get("banner");
            if (banner && banner->ready()) return banner->current();
            return stl::make_pair(stl::string("insulin"), stl::string("logo"));
        }
    }
}
```

Notes:
- `create()` returns a pointer into the registry vector. Callers must not hold it across another `create()` — a `push_back` can reallocate. Task 4 uses it immediately and drops it; nothing else stores one.
- `r[i].name == name` is valid: `stl::string` defines `operator==(const char*)` and `operator==(const string&)` (`src/stl/string.h:43-44`). No `strcmp` needed.
- `s.name = name` is valid: `string(const char* s)` is a non-explicit constructor (`src/stl/string.h:17`).

- [ ] **Step 3: Build**

```bash
./build.bat
```

Expected: exit 0, `build/InsulinGTAV.prx` produced.

- [ ] **Step 4: Commit**

```bash
git add src/menu/base/util/animated_texture.h src/menu/base/util/animated_texture.cpp
git commit -m "feat(ui): animated_texture playback + animation registry"
```

---

## Task 4: The loader — directory and manifest

**Files:**
- Modify: `src/menu/base/util/animated_texture.h` (add the two loader declarations to `namespace animation`)
- Modify: `src/menu/base/util/animated_texture.cpp` (implement them)

**Interfaces:**
- Consumes: `menu::animation::create` (Task 3); `rage::gfx::menu_textures()`, `texture_dictionary::add(name, path)`, `texture_dictionary::commit()` (existing, `src/rage/gfx.h`).
- Produces: `menu::animation::load_from_dir(const char* name, const char* dir) -> animated_texture*` (nullptr when nothing loadable was found) and `menu::animation::load_banner() -> animated_texture*`. Task 5 calls `load_banner()`.

- [ ] **Step 1: Declare the loader**

In `src/menu/base/util/animated_texture.h`, inside `namespace animation`, after `create(...)`:

```cpp
        // Load frames from `dir` into the shared "insulin" dictionary and register
        // them under `name`. Reads <dir>/frames.json when present; otherwise scans
        // the directory for *.png / *.dds in name order at the default delay.
        // Textures are added as "<name>_000", "<name>_001", ... -- explicit names,
        // because two animations whose files both start at 000 would collide in the
        // dictionary and commit() drops colliding codes.
        // Returns nullptr if the directory is missing, empty, or nothing loaded.
        animated_texture* load_from_dir(const char* name, const char* dir);

        // load_from_dir("banner", "/data/insulin/anim/banner").
        animated_texture* load_banner();
```

- [ ] **Step 2: Implement it**

In `src/menu/base/util/animated_texture.cpp`, extend the includes:

```cpp
#include "rage/gfx.h"
#include "util/json.h"
#include <orbis/libkernel.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
```

and add inside `namespace animation`, after `create(...)`:

```cpp
        static bool ext_is(const char* name, const char* dotext) {
            size_t ln = strlen(name), le = strlen(dotext);
            if (ln < le) return false;
            const char* e = name + ln - le;
            for (size_t i = 0; i < le; i++) {
                char a = e[i], b = dotext[i];
                if (a >= 'A' && a <= 'Z') a = (char)(a + 32);
                if (b >= 'A' && b <= 'Z') b = (char)(b + 32);
                if (a != b) return false;
            }
            return true;
        }

        // Insertion-sorted directory listing of image files, so 000.png..015.png
        // play in order regardless of what the filesystem hands back.
        static stl::vector<stl::string> image_files(const char* dir) {
            stl::vector<stl::string> out;
            int fd = sceKernelOpen(dir, 0 /* O_RDONLY */, 0);
            if (fd < 0) return out;

            char buf[4096];
            int n;
            while ((n = sceKernelGetdents(fd, buf, sizeof(buf))) > 0) {
                int pos = 0;
                while (pos < n) {
                    struct dirent* de = (struct dirent*)(buf + pos);
                    if (de->d_reclen == 0) break;
                    if (de->d_namlen > 0 && (ext_is(de->d_name, ".png") || ext_is(de->d_name, ".dds"))) {
                        stl::string name(de->d_name);
                        size_t at = out.size();
                        out.push_back(name);
                        while (at > 0 && strcmp(out[at - 1].c_str(), out[at].c_str()) > 0) {
                            stl::string tmp = out[at - 1];
                            out[at - 1] = out[at];
                            out[at] = tmp;
                            at--;
                        }
                    }
                    pos += de->d_reclen;
                }
            }
            sceKernelClose(fd);
            return out;
        }

        animated_texture* load_from_dir(const char* name, const char* dir) {
            if (!name || !name[0] || !dir || !dir[0]) return nullptr;

            // Frame list: the manifest if there is one, else the sorted directory.
            struct pending { stl::string file; int delay; };
            stl::vector<pending> frames;
            bool loop = true;

            char manifest_path[320];
            snprintf(manifest_path, sizeof(manifest_path), "%s/frames.json", dir);
            tj::json root = tj::json::load_from_file(manifest_path);

            const tj::json* list = root.try_get("frames");
            if (list && list->is_array() && list->size() > 0) {
                loop = root.value_bool("loop", true);
                int fallback = (int)root.value_int("default_delay", k_default_delay_ms);
                if (fallback <= 0) fallback = k_default_delay_ms;

                for (size_t i = 0; i < list->size(); i++) {
                    const tj::json* item = &(*(tj::json*)list)[i];
                    const tj::json* file = item->try_get("file");
                    if (!file || !file->is_string() || !file->get_string()[0]) continue;
                    int delay = (int)item->value_int("delay", fallback);
                    pending p;
                    p.file  = file->get_string();
                    p.delay = delay > 0 ? delay : fallback;
                    frames.push_back(p);
                }
            } else {
                stl::vector<stl::string> found = image_files(dir);
                for (size_t i = 0; i < found.size(); i++) {
                    pending p;
                    p.file  = found[i];
                    p.delay = k_default_delay_ms;
                    frames.push_back(p);
                }
                if (frames.size() > 0)
                    platform::logf("anim", "\"%s\": no frames.json, scanned %d file(s) at %d ms",
                                   name, (int)frames.size(), k_default_delay_ms);
            }

            if (frames.size() == 0) {
                LOG_WARN("anim: \"%s\": nothing loadable in %s", name, dir);
                return nullptr;
            }
            if ((int)frames.size() > k_max_frames)
                LOG_WARN("anim: \"%s\": %d frames found, only the first %d are loaded",
                         name, (int)frames.size(), k_max_frames);

            // Load the images into the shared dictionary under explicit names.
            rage::gfx::texture_dictionary& dict = rage::gfx::menu_textures();
            animated_texture* anim = create(name);
            if (!anim) return nullptr;
            anim->set_dictionary(dict.name());
            anim->set_loop(loop);

            int loaded = 0;
            for (size_t i = 0; i < frames.size() && loaded < k_max_frames; i++) {
                char tex_name[64];
                snprintf(tex_name, sizeof(tex_name), "%s_%03d", name, loaded);
                char full[320];
                snprintf(full, sizeof(full), "%s/%s", dir, frames[i].file.c_str());

                if (!dict.add(tex_name, full)) {
                    LOG_WARN("anim: \"%s\": frame %s failed to load, skipping", name, full);
                    continue;   // a bad frame drops out; the rest still plays
                }
                anim->add_frame(tex_name, frames[i].delay);
                loaded++;
            }

            if (loaded == 0) {
                LOG_ERROR("anim: \"%s\": no frame loaded from %s", name, dir);
                return nullptr;
            }

            dict.commit();   // one commit for the whole sequence
            platform::logf("anim", "\"%s\": %d frame(s) from %s, loop=%d", name, loaded, dir, (int)loop);
            return anim;
        }

        animated_texture* load_banner() {
            return load_from_dir("banner", "/data/insulin/anim/banner");
        }
```

Note on `&(*(tj::json*)list)[i]`: `tj::json::operator[](size_t)` exists only as a non-const overload (it auto-vivifies), so indexing a `const json*` needs the cast. The element is known to exist because the loop is bounded by `list->size()`.

- [ ] **Step 3: Build**

```bash
./build.bat
```

Expected: exit 0, `build/InsulinGTAV.prx` produced.

- [ ] **Step 4: Commit**

```bash
git add src/menu/base/util/animated_texture.h src/menu/base/util/animated_texture.cpp
git commit -m "feat(ui): load animation frames from a directory + frames.json"
```

---

## Task 5: Wire it into the menu

**Files:**
- Modify: `src/menu/menu.cpp` (tick)
- Modify: `src/menu/base/renderer.cpp` (header draw)
- Modify: `src/menu/base/submenus/main.cpp` (load button)

**Interfaces:**
- Consumes: `menu::animation::update`, `menu::animation::header_asset`, `menu::animation::load_banner` (Tasks 3-4).
- Produces: nothing further.

- [ ] **Step 1: Advance animations once per tick**

In `src/menu/menu.cpp`, add the include next to the other menu utils:

```cpp
#include "menu/base/util/animated_texture.h"
```

and in `tick()`, immediately after `global::ui::g_delta = native::get_frame_time();`:

```cpp
        // Step every loaded animation before anything draws, so update and render
        // stay separate and the renderer keeps no side effects.
        menu::animation::update(global::ui::g_delta);
```

- [ ] **Step 2: Draw the animated header**

In `src/menu/base/renderer.cpp`, add the include:

```cpp
#include "menu/base/util/animated_texture.h"
```

and replace the header block that currently reads:

```cpp
        if (rage::gfx::banner_ready()) {
            draw_sprite_aligned({ "insulin", "logo" }, { global::ui::g_position.x, global::ui::g_position.y - 0.08f }, { global::ui::g_scale.x, 0.08f }, 0.f, { 255, 255, 255, 255 });
        } else {
```

with:

```cpp
        // Header source, in order: the "banner" animation's current frame, the
        // static custom logo, then the game/sentinel texture below.
        menu::animated_texture* banner_anim = menu::animation::get("banner");
        if ((banner_anim && banner_anim->ready()) || rage::gfx::banner_ready()) {
            draw_sprite_aligned(menu::animation::header_asset(), { global::ui::g_position.x, global::ui::g_position.y - 0.08f }, { global::ui::g_scale.x, 0.08f }, 0.f, { 255, 255, 255, 255 });
        } else {
```

- [ ] **Step 3: Keep the øZARK title suppressed when an animation is showing**

Further down the same file, the title text is drawn only when no banner is loaded. Change:

```cpp
            if (!rage::gfx::banner_ready()) {
```

to:

```cpp
            menu::animated_texture* title_anim = menu::animation::get("banner");
            if (!rage::gfx::banner_ready() && !(title_anim && title_anim->ready())) {
```

- [ ] **Step 4: Add the load button**

In `src/menu/base/submenus/main.cpp`, add the include:

```cpp
#include "menu/base/util/animated_texture.h"
```

and add this option directly after the existing `"Load Custom Textures"` button:

```cpp
    add_option(button_option("Load Banner Animation")
        .add_tooltip("Plays /data/insulin/anim/banner as the menu header")
        .add_click([] {
            menu::animated_texture* a = menu::animation::load_banner();
            if (a) menu::notify::stacked("Animation", stl::string::format("%i frames", a->frame_count()), global::ui::g_success);
            else   menu::notify::stacked("Animation", "Nothing loadable in /data/insulin/anim/banner", global::ui::g_error);
        }));
```

The signature is `stacked(stl::string title, stl::string text, color_rgba color = global::ui::g_notify_bar, uint32_t timeout = 6000)` (`src/menu/base/util/notify.h:44`), so the three-argument form above is correct.

- [ ] **Step 5: Build**

```bash
./build.bat
```

Expected: exit 0, `build/InsulinGTAV.prx` produced.

- [ ] **Step 6: Re-run the host tests (nothing in Tasks 3-5 should have changed the timing core)**

```bash
clang++ -std=c++17 -I src tests/frame_clock_test.cpp -o build/frame_clock_test.exe && ./build/frame_clock_test.exe
```

Expected: `all passed`, exit 0.

- [ ] **Step 7: Commit**

```bash
git add src/menu/menu.cpp src/menu/base/renderer.cpp src/menu/base/submenus/main.cpp
git commit -m "feat(ui): play the banner animation in the menu header"
```

---

## Task 6: On-console acceptance

**Files:** none — this task is hardware validation, performed by the user.

- [ ] **Step 1: Convert a real GIF**

```powershell
pwsh -File tools\gif2frames.ps1 -Gif <your.gif> -OutDir build\banner
```

- [ ] **Step 2: Deploy**

Copy `build/banner/*` to `/data/insulin/anim/banner/` on the console (FTP), and deploy `build/InsulinGTAV.prx` as usual.

- [ ] **Step 3 (user, on console): confirm each acceptance criterion**

1. With no animation directory present, the header shows the static logo (or the sentinel) exactly as before — the feature is invisible until used.
2. "Load Banner Animation" reports the frame count; the header then animates at the GIF's own speed.
3. Press the PS button, wait, and return: no crash, and the animation resumes. (The extra textures pass through the same suspend-time accounting the heap guard covers.)
4. Leave the menu closed for a minute, reopen it: the animation is mid-loop and smooth, not stuck or racing.

- [ ] **Step 4: Record the result**

If all four hold, tick this task and note the confirmation in the commit for any follow-up fix. If one fails, capture `/data/insulingtav.log` (the `anim` and `gfx` lines) plus a klog capture (`nc <ip> 3232`) before changing code.

---

## Notes for the implementer

- **Do not build a decoder seam.** The spec defers runtime `.gif` decoding deliberately. `add_frame()` is the entire extension point; no format registry, loader interface or stub belongs in this work.
- **Never cap silently.** Both the converter and the loader report what they dropped. Keep that property in any change.
- **The 64-entry dictionary.** 16 animation frames plus the existing custom textures share the `"insulin"` dictionary (`gfx.cpp` warns at 64). If a second animation is ever loaded alongside the first, this is the limit that bites — give it its own `texture_dictionary` at that point rather than raising the cap.
