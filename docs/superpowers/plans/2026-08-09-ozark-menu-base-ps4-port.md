# Ozark Menu-Base PS4 Port — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Port Ozark's GTA V menu *base* (submenu system, option types, renderer, input, instructionals, notifications, on-screen keyboard) into the InsulinGTAV GoldHEN plugin so it opens, renders in the Ozark look, and is fully navigable on a PS4 running CUSA00411 v1.57 — features excluded, demo skeleton only.

**Architecture:** A GoldHEN `.prx` (GHPLUGIN) links SceLibcInternal (no libc++), so the port rides on a project-local **mini-STL** (`src/stl/`) with a mechanical `std:: → stl::` transform of the Ozark sources. Native calls go through the **InsulinGTA5 invoker** (runtime-base + RVA, CUSA00411 v1.57; Vector3 out-params pass as raw pointers). The menu ticks once per frame from a **detour on a per-frame script-thread native** (the InsulinGTA5 `game_thread` pattern). A thin `platform/` shim replaces Ozark's Windows `stdafx.h`.

**Tech Stack:** C++17, OpenOrbis PS4 toolchain (clang 18, lld), GoldHEN Plugin SDK (crtprx.o, libGoldHEN_Hook.a, Detour), CMake via `add_orbis_target(... TYPE GHPLUGIN)`.

## Progress (2026-08-09)

**Milestone 0 COMPLETE (compile-verified; on-console gate open).** `build/InsulinGTAV.prx`
builds. Done + committed: Task 1 (mini-STL), Task 2 (invoker; raw-pointer Vector3, no
setVectors redirect), Task 3 (platform shim), Task 4 (frame hook), Task 5 (module_start +
STL/invoker smoke). **Also done:** Task 7 (ui_vars/vars/localization/math — constexpr colours,
no init_array). Build recipe: Ninja generator required (VS/cl.exe cannot cross-compile) — see
the memory note or `build.bat`.

**Task 8a RESOLVED (2026-08-09).** The "16 missing natives" collapsed: the 6 scaleform
push/pop/param + draw_fullscreen were already covered by `scaleform.h` (v1.57-validated). The 5
genuinely-needed text/instructional natives were reversed against the v1.57 IDB
(`E:\Projects\IDA\PS4\GTA5\eboot_named.i64`, imagebase 0x0) and live in
`src/rage/invoker/missing_natives.h`: begin/end text width (0x9E0710/0x9E0720), begin/end line
count (0x9E0750/0x9E0760), get_control_instructional_button = GET_CONTROL_INSTRUCTIONAL_BUTTONS_STRING
(0xAA0FD0). Non-critical `get_text_scale_height` / `play_sound_frontend` / `is_input_disabled`
are documented fallbacks (not renamed in the IDB; sound cosmetic, height→identity, input-gate→
menu's own flag). The renderer + instructionals are now unblocked.

**Next:** Task 6 (input), Task 8 (base+renderer), then options/handler/instructionals/OSK/demo.

## Global Constraints

- **Target:** GTA V PS4 **CUSA00411 v1.57**; artifact `build/InsulinGTAV.prx` (GHPLUGIN).
- **No libc++/STL.** Use `stl::` types from `src/stl/` only. Never `std::` in ported code. Never include libc++ headers (`<string>`, `<vector>`, `<memory>`, `<functional>`, `<unordered_map>`, `<algorithm>`, …). `<stddef.h>/<stdint.h>/<string.h>/<stdio.h>/<stdarg.h>/<stdlib.h>` (C headers) are fine.
- **No `.init_array`.** GoldHEN does not run global constructors. No namespace-scope objects with non-trivial constructors. Singletons are function-local statics (`get_x()` returning `static X instance;`). POD globals only, `constexpr` where possible.
- **No Windows / no SEH.** No `<Windows.h>`, `HWND`, `WNDPROC`, `VK_*`, `GetTickCount`, `timeGetTime`, `GetAsyncKeyState`, `MessageBox`, structured exception handling. Replace per the mapping table in Task 6.
- **No hooking/MinHook, no fibers, no threads inside the menu.** The menu runs entirely on the script thread via the frame hook. `util::fiber::go_to_main()` calls in ported code are dropped (those sit in feature paths not in scope).
- **`XOR("s")` → `s`.** Provide a passthrough `XOR(x) (x)` macro; no string encryption on PS4.
- **Invoker is reference-only from InsulinGTA5.** Copy `invoker.{h,cpp}`, `natives.h`, `scaleform.h`, `types/base_types.h`. Do not pull in other InsulinGTA5 menu code.
- **Native callsites stay `native::snake_case(...)`** — already matches the generated `natives.h`.
- **Commit after every task.** Conventional commit messages. Co-author trailer:
  `Co-Authored-By: Claude Fable 5 <noreply@anthropic.com>`
- **Build command (from repo root, Git Bash on Windows):**
  `OO_PS4_TOOLCHAIN="C:/PS4/OpenOrbis/PS4Toolchain" cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=cmake/oo-ps4-toolchain.cmake && cmake --build build`
  A task's "build passes" = this command exits 0 and produces `build/InsulinGTAV.prx`.
- **On-console validation is the real gate** but is out-of-band (no console in the build environment). Each milestone lists its on-console acceptance; mark those checkboxes only once the user confirms on hardware.

## Reference Map

| Need | Source |
|---|---|
| Ozark base sources | `C:\Users\BBC\Desktop\GTA\GTA5Menus-main\ozark\GTAV\src\...` |
| Invoker + natives + scaleform | `E:\Projects\PS4\InsulinGTA5\src\rage\invoker\*`, `src\rage\types\base_types.h` |
ageinvoker*`, `src
age	ypesbase_types.h` |
| Mini-STL pattern | `E:\Projects\PS4\InsulinGTA5\src\stl\*` |
| Frame-hook pattern | `E:\Projects\PS4\InsulinGTA5\src\game\game_thread.{h,cpp}` |
| module_start wiring | `E:\Projects\PS4\InsulinGTA5\src\InsulinGTA5.cpp` |
| "Basic" invoker (PC-hash names, control-input proof) | `C:\Users\BBC\Desktop\Basic\Basic\{invoker.cpp,invoker.h,natives.h}` |

## File Structure

```
src/
  InsulinGTAV.cpp            module_start/stop, smoke tests, frame wiring
  stl/                       mini-STL (own; pattern from InsulinGTA5, extended)
    new.h string.h vector.h shared_ptr.h function.h stack.h pair.h
    unordered_map.h tuple.h algorithm.h            (new additions)
  rage/
    invoker/invoker.{h,cpp}  copied (raw-pointer Vector3 out-params; no redirect)
    invoker/natives.h        copied (CUSA00411 v1.57)
    invoker/scaleform.h      copied
    types/base_types.h       copied
  platform/
    stdafx.h                 replaces Ozark stdafx (PS4 includes, XOR passthrough)
    log.{h,cpp}              LOG* -> notify + klog
    compat.h                 GetTickCount()->sceKernelGetProcessTime ms, key constants
  game/
    game_thread.{h,cpp}      frame-hook detour (copied pattern)
  util/
    math.h                   1:1 Ozark
    localization.{h,cpp}     trimmed (no translation table dependency)
    va.{h,cpp}               if pulled in by a base module
  menu/
    menu.{h,cpp}             build()/tick() entrypoints (new, thin)
    base/
      base.{h,cpp} renderer.{h,cpp} submenu.{h,cpp} submenu_handler.{h,cpp}
      options/ option.* button.* toggle.* number.* scroll.h radio.* color_option.* break.* submenu_option.*
      util/ menu_input.* control.* fonts.* input.* instructionals.* notify.* stacked_display.* timers.* global.h
      submenus/ main.{h,cpp}   demo skeleton
  global/
    ui_vars.{h,cpp}          transformed
    vars.{h,cpp}             minimal subset the base references
```

---

## Milestone 0 — Mini-STL foundation

### Task 1: Bring in and extend the mini-STL

**Files:**
- Create: `src/stl/new.h`, `src/stl/string.h`, `src/stl/vector.h`, `src/stl/shared_ptr.h`, `src/stl/function.h`, `src/stl/stack.h`, `src/stl/pair.h` (copied verbatim from `E:\Projects\PS4\InsulinGTA5\src\stl\`)
- Create: `src/stl/unordered_map.h`, `src/stl/tuple.h`, `src/stl/algorithm.h`, `src/stl/initializer_list.h` (new)
- Modify: `src/stl/string.h` (extend — see below)
- Test: extend `src/InsulinGTAV.cpp` smoke path

**Interfaces:**
- Produces: `stl::string` (with `c_str, length, size, empty, clear, operator[], operator+=, operator==, find, substr, append, static format`), `stl::vector<T>`, `stl::shared_ptr<T>` + `stl::make_shared`, `stl::function<Sig>`, `stl::stack<T>`, `stl::pair<A,B>` + `stl::make_pair`, `stl::unordered_map<K,V>` (`operator[], find, end, erase, begin/end iteration`), `stl::tuple` (2-3 elt, `get<N>`), `stl::find_if`.
- Consumes: nothing (leaf layer).

- [ ] **Step 1: Copy the seven existing mini-STL headers verbatim** from `E:\Projects\PS4\InsulinGTA5\src\stl\` into `src/stl/`. Keep them unchanged for now.

- [ ] **Step 2: Extend `src/stl/string.h`** to cover Ozark's usage. Add to the `string` class (keep the fixed-buffer `STL_STRING_CAP` design; raise cap to 128):

```cpp
size_t size() const { return length(); }
void clear() { m_buf[0] = 0; }
char& operator[](size_t i) { return m_buf[i]; }
char operator[](size_t i) const { return m_buf[i]; }
const char* data() const { return m_buf; }
string& operator+=(const char* s) { *this = *this + s; return *this; }
string& operator+=(const string& o) { *this = *this + o.c_str(); return *this; }
string& operator+=(char c) { char t[2] = { c, 0 }; *this = *this + t; return *this; }
bool operator==(const string& o) const { return compare(o.c_str()) == 0; }
bool operator!=(const char* s) const { return compare(s) != 0; }
bool operator!=(const string& o) const { return compare(o.c_str()) != 0; }
bool operator<(const string& o) const { return compare(o.c_str()) < 0; }   // for map keys
static const size_t npos = (size_t)-1;
size_t find(const char* s) const { const char* p = strstr(m_buf, s); return p ? (size_t)(p - m_buf) : npos; }
string substr(size_t pos, size_t n = npos) const {
    string out; size_t len = length(); if (pos >= len) return out;
    if (n > len - pos) n = len - pos; if (n > STL_STRING_CAP - 1) n = STL_STRING_CAP - 1;
    memcpy(out.m_buf, m_buf + pos, n); out.m_buf[n] = 0; return out;
}
```

- [ ] **Step 3: Write `src/stl/pair.h` companion `make_pair` and `tuple`.** `src/stl/tuple.h`:

```cpp
#pragma once
namespace stl {
    template <typename A, typename B, typename C> struct tuple3 { A a; B b; C c; };
    template <typename A, typename B, typename C>
    tuple3<A,B,C> make_tuple(A a, B b, C c) { return { a, b, c }; }
}
```
(Ozark's `m_instructionals` is `vector<tuple<string,int,bool>>` — a 3-field struct is enough; keep the concrete type `stl::tuple3<stl::string,int,bool>` when transforming.)

- [ ] **Step 4: Write `src/stl/unordered_map.h`** — small vector-backed map (order-N lookup is fine for menu-sized maps):

```cpp
#pragma once
#include "stl/vector.h"
#include "stl/pair.h"
namespace stl {
    template <typename K, typename V>
    class unordered_map {
    public:
        V& operator[](const K& k) {
            for (auto& e : m_data) if (e.first == k) return e.second;
            m_data.push_back(pair<K,V>(k, V())); return m_data[m_data.size()-1].second;
        }
        pair<K,V>* find(const K& k) {
            for (auto& e : m_data) if (e.first == k) return &e;
            return nullptr;                                  // caller compares against end()
        }
        pair<K,V>* end() { return nullptr; }
        bool contains(const K& k) { return find(k) != nullptr; }
        void erase(const K& k) {
            for (size_t i = 0; i < m_data.size(); ++i) if (m_data[i].first == k) {
                for (size_t j = i; j + 1 < m_data.size(); ++j) m_data[j] = m_data[j+1];
                m_data.resize(m_data.size()-1); return; } }
        size_t size() const { return m_data.size(); }
        pair<K,V>* begin_ptr() { return m_data.begin(); }
        pair<K,V>* end_ptr() { return m_data.end(); }
        vector<pair<K,V>>& raw() { return m_data; }
    private:
        vector<pair<K,V>> m_data;
    };
}
```
Note: `find(...) != map.end()` works because `end()` returns `nullptr` and a missing key returns `nullptr`. Range-for uses `.raw()` where a transformed file iterates the map.

- [ ] **Step 5: Write `src/stl/algorithm.h`** — just `find_if`:

```cpp
#pragma once
namespace stl {
    template <typename It, typename Pred>
    It find_if(It first, It last, Pred p) { for (; first != last; ++first) if (p(*first)) return first; return last; }
}
```

- [ ] **Step 6: Write `src/stl/pair.h` `make_pair`** (append to the existing file):

```cpp
template <typename A, typename B>
pair<A,B> make_pair(A a, B b) { return pair<A,B>(a, b); }
```
(Verify the existing `pair` has a two-arg constructor; add one if missing.)

- [ ] **Step 7: Extend `STL_FUNCTION_CAP`** in `src/stl/function.h` to 64 (Ozark lambdas capture a couple of pointers/ints; 48 is tight). Change the `#define STL_FUNCTION_CAP` default to 64.

- [ ] **Step 8: Add the M0 smoke to `src/InsulinGTAV.cpp`** (full file written in Task 5). For now, just confirm the headers compile: create `src/stl_smoke.h`:

```cpp
#pragma once
#include "stl/string.h"
#include "stl/vector.h"
#include "stl/shared_ptr.h"
#include "stl/function.h"
#include "stl/stack.h"
#include "stl/unordered_map.h"
#include "stl/algorithm.h"
namespace stl_smoke {
    inline bool run() {
        stl::string s("insu"); s += "lin"; if (s != "insulin") return false;
        if (s[0] != 'i' || s.substr(4).compare("lin") != 0) return false;
        stl::vector<int> v; for (int i = 0; i < 10; ++i) v.push_back(i);
        int sum = 0; for (int x : v) sum += x; if (sum != 45) return false;
        stl::shared_ptr<int> p = stl::make_shared<int>(7); { auto q = p; if (*q != 7) return false; }
        stl::function<int(int)> f = [](int x){ return x * 2; }; if (f(21) != 42) return false;
        stl::unordered_map<stl::string,int> m; m["a"] = 1; m["b"] = 2;
        if (m["a"] != 1 || m.find("z") != m.end()) return false;
        return true;
    }
}
```

- [ ] **Step 9: Build** (see Global Constraints build command). Expected: compiles. (Wiring the smoke call into module_start happens in Task 5.)

- [ ] **Step 10: Commit**
```bash
git add src/stl src/stl_smoke.h && git commit -m "feat(stl): mini-STL foundation (copied + extended for Ozark)"
```

---

### Task 2: Copy the invoker (raw-pointer Vector3 out-params; no redirect) ✓ DONE

**Files:**
- Create: `src/rage/invoker/invoker.h`, `src/rage/invoker/invoker.cpp`, `src/rage/invoker/natives.h`, `src/rage/invoker/scaleform.h`, `src/rage/types/base_types.h` (copied from InsulinGTA5)

**Interfaces:**
- Consumes: nothing.
- Produces: `rage::invoker::invoke<R>(rva, args...)`, `rage::invoker::resolve_base()`, `rage::invoker::g_eboot_base`, `native::*` wrappers, `sf::*` scaleform wrappers, handle typedefs (`Void, Any, Ped, ...`, `math::vector3<T>`).

**Implementation finding (why no fixup):** Basic's `setVectors()` is dead code — `vectorCount`
is never incremented and `argVectors` never populated. Basic passes Vector3 out-params as raw
pointers that the native writes through, and that is exactly what our `push<T>` (T=pointer)
already does. A redirect would also be unsafe (the invoker cannot tell out- from in-params,
e.g. `create_itemset(vector3*)` is an INPUT). So the invoker is copied unchanged; the decision
is documented in `invoker.h`.

- [x] **Step 1: Copy the five files verbatim** from `E:\Projects\PS4\InsulinGTA5\src\rage\...`.
- [x] **Step 2: Document the raw-pointer decision** in `invoker.h` (no `setVectors` redirect).
- [x] **Step 3: Compile-test** a TU calling `get_hash_key` + `get_model_dimensions(&min,&max)` with the toolchain flags. Exit 0.
- [x] **Step 4: Commit** `feat(invoker): copy InsulinGTA5 invoker/natives/scaleform (CUSA00411 v1.57)`.

---

### Task 3: Platform shim (stdafx replacement, log, compat)

**Files:**
- Create: `src/platform/stdafx.h`, `src/platform/log.h`, `src/platform/log.cpp`, `src/platform/compat.h`

**Interfaces:**
- Consumes: stl/*, rage/types/base_types.h.
- Produces: the `stdafx.h` every ported file includes; `XOR(x)`, `NUMOF`, `joaat`, `LOG*` macros, `platform::now_ms()`, key-code constants used by transformed input code.

- [ ] **Step 1: Write `src/platform/compat.h`** — time + key constants:

```cpp
#pragma once
#include <stdint.h>
#include <orbis/libkernel.h>
namespace platform {
    // Millisecond monotonic clock; replaces GetTickCount()/timeGetTime().
    inline uint32_t now_ms() { return (uint32_t)(sceKernelGetProcessTime() / 1000ULL); }
    inline uint64_t now_ms64() { return sceKernelGetProcessTime() / 1000ULL; }
}
// GTA control indices used by transformed menu input (see prx.cpp Input enum).
enum { ControlFrontendDown = 187, ControlFrontendUp = 188, ControlFrontendLeft = 189,
       ControlFrontendRight = 190, ControlFrontendAccept = 201, ControlFrontendCancel = 202,
       ControlFrontendLb = 205, ControlFrontendRb = 206, ControlFrontendLt = 207, ControlFrontendRt = 208 };
```

- [ ] **Step 2: Write `src/platform/stdafx.h`** — the Ozark stdafx replacement:

```cpp
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "stl/string.h"
#include "stl/vector.h"
#include "stl/shared_ptr.h"
#include "stl/function.h"
#include "stl/stack.h"
#include "stl/pair.h"
#include "stl/tuple.h"
#include "stl/unordered_map.h"
#include "stl/algorithm.h"
#include "stl/initializer_list.h"

#include "rage/types/base_types.h"
#include "platform/compat.h"
#include "platform/log.h"

#define XOR(x) (x)              // no string encryption on PS4
#define VERSION 34

template<typename T, int N> constexpr int NUMOF(T(&)[N]) { return N; }
// joaat is provided for parity where a transformed file calls joaat("..."); the
// real hash also comes from native::get_hash_key at runtime.
```
(Copy Ozark's `CharacterMap` + `JenkinsHash32` + `joaat` macro from the original stdafx.h so compile-time `joaat("x")` keeps working.)

- [ ] **Step 3: Write `src/platform/log.h`** — Ozark-compatible LOG macros:

```cpp
#pragma once
namespace platform { void log_line(const char* tag, const char* msg); void logf(const char* tag, const char* fmt, ...); }
#define LOG(fmt, ...)                 platform::logf("Log",  fmt, ##__VA_ARGS__)
#define LOG_DEV(fmt, ...)             platform::logf("Dev",  fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)           platform::logf("Err",  fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)            platform::logf("Warn", fmt, ##__VA_ARGS__)
#define LOG_CUSTOM(tag, fmt, ...)     platform::logf(tag,    fmt, ##__VA_ARGS__)
#define LOG_CUSTOM_ERROR(tag, fmt, ...) platform::logf(tag,  fmt, ##__VA_ARGS__)
```

- [ ] **Step 4: Write `src/platform/log.cpp`** — append to `/data/insulingtav.log` (mirror InsulinGTA5's `log_line`, add tag + vsnprintf). Include the FreeBSD open flags (`ORBIS_O_WRONLY|APPEND|CREAT`).

- [ ] **Step 5: Write `src/stl/initializer_list.h`** — Ozark's `color_rgba::as_initializer_list()` uses `std::initializer_list`. clang's `<initializer_list>` is compiler-backed and works even freestanding, BUT to stay libc++-free provide a tiny shim only if `<initializer_list>` is unavailable; otherwise this header is `#include <initializer_list>` guarded. Prefer the compiler header:
```cpp
#pragma once
#include <initializer_list>   // compiler-backed, safe without libc++
```

- [ ] **Step 6: Build.** Expected: compiles.

- [ ] **Step 7: Commit**
```bash
git add src/platform src/stl/initializer_list.h && git commit -m "feat(platform): stdafx shim, log, compat for the Ozark port"
```

---

### Task 4: Frame-hook (script-thread tick)

**Files:**
- Create: `src/game/game_thread.h`, `src/game/game_thread.cpp` (copied pattern from InsulinGTA5)

**Interfaces:**
- Consumes: rage/invoker.
- Produces: `game::install_frame_hook()`, `game::set_frame_callback(fn)`, `game::run_on_game_thread(fn)`.

- [ ] **Step 1: Copy `game_thread.{h,cpp}` verbatim** from `E:\Projects\PS4\InsulinGTA5\src\game\`. It already avoids STL and uses `<GoldHEN/Detour.h>` + `__atomic` builtins.

- [ ] **Step 2: Confirm the hooked RVA.** `HOOK_RVA = 0xAA9310` (GET_PLAYER_PED impl) is validated for CUSA00411 v1.57 in InsulinGTA5. Keep it. Leave the comment noting the fallback (swap for another per-frame native if the queue never drains).

- [ ] **Step 3: Build.** Expected: compiles and links against libGoldHEN_Hook.

- [ ] **Step 4: Commit**
```bash
git add src/game && git commit -m "feat(game): per-frame script-thread hook (InsulinGTA5 pattern)"
```

---

### Task 5: module_start wiring + M0 smoke on console

**Files:**
- Modify: `src/InsulinGTAV.cpp` (replace the scaffold stub)
- Modify: `CMakeLists.txt` (add all sources + defines + include dir)

**Interfaces:**
- Consumes: everything above; `menu::build()/menu::tick()` (stubs until Task 8).
- Produces: the loadable plugin.

- [ ] **Step 1: Rewrite `CMakeLists.txt`** to compile the tree and set the freestanding define + include root:

```cmake
cmake_minimum_required(VERSION 3.20)
project(InsulinGTAV CXX)
if(NOT DEFINED ENV{OO_PS4_TOOLCHAIN})
    message(FATAL_ERROR "OO_PS4_TOOLCHAIN environment variable is not set")
endif()
file(GLOB_RECURSE INSULIN_SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_orbis_target(InsulinGTAV TYPE GHPLUGIN SOURCES ${INSULIN_SOURCES})
target_include_directories(InsulinGTAV PRIVATE src)
target_compile_definitions(InsulinGTAV PRIVATE STL_FREESTANDING)
target_compile_options(InsulinGTAV PRIVATE -std=c++17 -fno-exceptions -fno-rtti -Wno-invalid-offsetof)
```

- [ ] **Step 2: Write a minimal `src/menu/menu.h`** stub so module_start links now (real body in Task 8):
```cpp
#pragma once
namespace menu { void build(); void tick(); }
```
and `src/menu/menu.cpp` with empty bodies (temporary; replaced in Task 8).

- [ ] **Step 3: Rewrite `src/InsulinGTAV.cpp`** modeled on InsulinGTA5's, adding the STL smoke to the worker thread:

```cpp
// includes: GoldHEN/Common.h, orbis/libkernel.h, invoker, natives, game_thread, menu, stl_smoke
static void run_smoke() {
    Hash h = native::get_hash_key("insulin");
    bool stl_ok = stl_smoke::run();
    char m[192];
    snprintf(m, sizeof m, "base=0x%llx HASH=0x%08X(exp0669D57F) STL=%s",
             (unsigned long long)rage::invoker::g_eboot_base, (unsigned)h, stl_ok ? "OK":"FAIL");
    notify(m);
}
```
module_start: notify loaded → `resolve_base()` → `install_frame_hook()` → `menu::build()` → `set_frame_callback(menu::tick)` → spawn worker thread that sleeps 10s then `run_smoke()`. Mirror the InsulinGTA5 structure exactly for notify/log helpers.

- [ ] **Step 4: Build.** Expected: `build/InsulinGTAV.prx` produced.

- [ ] **Step 5: Commit**
```bash
git add -A && git commit -m "feat: module_start wiring + M0 STL/invoker smoke; CMake compiles src tree"
```

- [ ] **Step 6 (on-console, user):** deploy `InsulinGTAV.prx`, launch GTA V. Expect notifications: "loaded", "frame hook installed", `base=… HASH=0x0669D57F STL=OK`. This closes **M0**.

---

## Milestone 1 — Base opens & renders

### Task 6: Input layer (menu_input + input, control-natives; scePad fallback documented)

**Files:**
- Create: `src/menu/base/util/input.h`, `input.cpp` (transformed; Windows message pump removed)
- Create: `src/menu/base/util/menu_input.h`, `menu_input.cpp` (transformed)
- Create: `src/menu/base/util/timers.h`, `timers.cpp` (transformed)
- Create: `src/platform/pad.h`, `pad.cpp` (scePad fallback, unused by default)

**Interfaces:**
- Consumes: natives (`is_disabled_control_pressed`, `is_disabled_control_just_pressed`, `is_disabled_control_just_released`, `set_input_exclusive`, `disable_control_action`), compat control constants.
- Produces: `menu::input::is_pressed/is_just_pressed/is_just_released/is_option_pressed/is_left_pressed/is_right_pressed/is_open_bind_pressed/scroll_*`, `menu::input::mi_update()` + `push/hotkey/color/get_key`, `menu::timers::timer`.

**PC → PS4 substitution table (apply while transforming these files):**

| Ozark (PC) | PS4 replacement |
|---|---|
| `GetTickCount()`, `GetTickCount64()`, `timeGetTime()` | `platform::now_ms()` / `now_ms64()` |
| keyboard branch `is_pressed(true, VK_*, ...)` | drop the keyboard half; keep the control-native (`is_pressed(false, ControlFrontend*)`) half |
| `VK_ESCAPE/VK_RETURN/VK_BACK/VK_F12/VK_NUMPAD*` | removed (keyboard input is via the on-screen keyboard native, Task 14) |
| `input::window_process_callback` (WNDPROC) | delete — no window message pump on PS4 |
| open bind `VK_F4` | `is_open_bind_pressed()` = `native::is_disabled_control_pressed(0, ControlFrontendLb) && native::is_disabled_control_just_pressed(0, ControlFrontendCancel)` (L1 + ○) |
| `util::fiber::go_to_main()` | delete (menu runs on the script thread) |
| `menu::hotkey::*`, `localization` translation-table | keep hotkey struct calls only if referenced by base; otherwise stub `read_hotkey` to no-op |

- [ ] **Step 1: Write `src/menu/base/util/timers.{h,cpp}`** — transform: `GetTickCount64()` → `platform::now_ms64()`. Keep the `timer` class + `run_timed` API identical.

- [ ] **Step 2: Write `src/menu/base/util/input.{h,cpp}`** — transform. Remove the `window_process_callback`/`WNDPROC`/`m_windows_process` members and the whole keyboard-state machine that reads `GetAsyncKeyState` (it's already commented out in Ozark; the live path uses control natives). Keep `is_pressed/is_just_pressed/is_just_released(bool keyboard,int key,bool override)` but treat `keyboard==true` as "always false" (no PC keyboard), so the control-native branches remain. Keep `scroll_up/down/top/bottom`, `is_option_pressed`, `is_left/right[_just]_pressed`, and add `is_open_bind_pressed` per the table.

- [ ] **Step 3: Write `src/menu/base/util/menu_input.{h,cpp}`** — transform. Replace `strcpy_s` → `strncpy`, `VK_*`/keyboard branches removed, `util::fiber::go_to_main()` removed. The hotkey-capture flow (which reads raw keys) is stubbed to a no-op for now (hotkeys are a feature, not base); keep `push/color/get_key` and the `update()` queue-drain intact.

- [ ] **Step 4: Write `src/platform/pad.{h,cpp}`** — `scePadReadState` wrapper exposing `platform::pad::down(button)` for the documented fallback. Not called by default. (Include `<pad.h>`, open a handle once via `scePadOpen`.)

- [ ] **Step 5: Build.** Expected: compiles (these files have no menu-class dependencies yet beyond option.h forward use — if `menu_input.cpp` needs `base_option`/`color_option`, add forward declares; full option types land in Task 9-10, so temporarily `#if 0` the `hotkey(...)`/`color(...)` bodies that need them and restore in Task 11).

- [ ] **Step 6: Commit**
```bash
git add src/menu/base/util/input.* src/menu/base/util/menu_input.* src/menu/base/util/timers.* src/platform/pad.* && git commit -m "feat(input): transform Ozark menu_input/input/timers to control-natives"
```

---

### Task 7: ui_vars, vars subset, localization, math

**Files:**
- Create: `src/util/math.h` (1:1)
- Create: `src/util/localization.h`, `localization.cpp` (trimmed)
- Create: `src/global/ui_vars.h`, `ui_vars.cpp` (transformed)
- Create: `src/global/vars.h`, `vars.cpp` (minimal subset)
- Create: `src/menu/base/util/global.h` (script_global — keep only if referenced; else omit)

**Interfaces:**
- Consumes: stl, math.
- Produces: `color_rgba`, `color_hsv`, `menu_texture`, `radio_context`, `line_2d`, all `global::ui::*` externs; `math::vector2/3/4`, `clamp/lerp`.

- [ ] **Step 1: Copy `util/math.h` 1:1** (it's header-only, POD; verify no Windows include).

- [ ] **Step 2: Write `src/util/localization.{h,cpp}`** — trim to the two things the base needs: store an original + mapped string, `get()` returns original when untranslated. Drop `global::vars::g_localization_table.push_back(this)` registration (that global vector of pointers is a translation feature); make `register_translation()` a no-op. Signature stays `localization(stl::string, bool translate=false, bool global_register=false)`.

- [ ] **Step 3: Write `src/global/ui_vars.{h,cpp}`** — transform. `std::string`→`stl::string`, `std::unordered_map`→`stl::unordered_map`, `std::initializer_list` stays (compiler header). `color_rgba`, `color_hsv`, `radio_context`, `menu_texture`, `line_2d` structs. **Critical (no `.init_array`):** the `ui_vars.cpp` definitions that today are namespace-scope objects with constructors (`color_rgba g_success = {...}`) must be `constexpr`/POD-initialized. `color_rgba` has non-constexpr ctors in Ozark → add a `constexpr color_rgba(int,int,int,int)` constructor so the globals are constant-initialized. Any `menu_texture`/`radio_context` globals (which hold `stl::string`) cannot be constant-initialized — move those behind a `get_*()` function-local static, OR initialize them in a `menu::ui::init()` called from `menu::build()`. Prefer an explicit `init()` for the string-bearing textures.

- [ ] **Step 4: Write `src/global/vars.{h,cpp}`** — only the members the base set references. Grep the transformed base files for `global::vars::`; likely just `g_unloading` (bool) and possibly `g_game_address`. Provide those as POD globals. Drop the entire network/ROS/engine-pointer wall.

- [ ] **Step 5: Build.** Expected: compiles.

- [ ] **Step 6: Commit**
```bash
git add src/util src/global src/menu/base/util/global.h && git commit -m "feat(ui): ui_vars/vars/localization/math (constexpr globals, no init_array)"
```

---

### Task 8: base + renderer + menu entrypoints (empty submenu)

**Files:**
- Create: `src/menu/base/base.h`, `base.cpp` (transformed)
- Create: `src/menu/base/renderer.h`, `renderer.cpp` (transformed)
- Create: `src/menu/base/util/fonts.h`, `fonts.cpp` (transformed/trimmed)
- Create: `src/menu/base/util/textures.h`, `textures.cpp` (stub — Sentinel/game YTD only)
- Replace: `src/menu/menu.h`, `menu.cpp` (real `build()/tick()`)

**Interfaces:**
- Consumes: input, ui_vars, submenu_handler (Task 12 — temporarily an empty forward), natives + scaleform.
- Produces: `menu::base::update()`, `menu::base::set_open/is_open/get_*`, `menu::renderer::*`, `menu::build()`, `menu::tick()`.

- [ ] **Step 1: Write `src/menu/base/util/fonts.{h,cpp}`** — transform. Ozark `fonts::load` scans `.gfx` files from disk (`util::dirs`); on PS4 there is no custom-font pipeline in scope, so `load()` becomes a no-op and `get_font_id(name)` returns the built-in font id via `native::get_font_id`-equivalent or a fixed mapping (0 = default). Keep the API.

- [ ] **Step 2: Write `src/menu/base/util/textures.{h,cpp}`** — stub. `get_texture(name,out)` returns false (no custom textures); the renderer already falls back to game dictionaries (`commonmenu`) / sentinel quads. Keep the API so renderer compiles.

- [ ] **Step 3: Write `src/menu/base/renderer.{h,cpp}`** — transform. This is the largest base file. Native callsites already match. Points to handle:
  - `std::string`→`stl::string`, `std::pair`→`stl::pair`, `std::vector`→`stl::vector`.
  - `<random>` (`std::mt19937` for the joke tooltips) → replace with a tiny LCG (`platform::rand()`), or drop the random tooltip and use index 0. Keep it simple: a static LCG seeded from `now_ms()`.
  - `get_texture(...)` calls route through the Task-8 stub (always false → game-dict/sentinel path).
  - `draw_text` uses `native::begin_text_command_display_text`, `set_text_*`, `add_text_component_substring_player_name`, `end_text_command_display_text` — all present in natives.h.
  - `calculate_string_width`/`get_normalized_font_scale` use `begin_text_command_width`/`end_text_command_get_width`/`get_text_scale_height` — these are MISSING from natives.h (see Task 8a). Guard them behind the wrappers added there.

- [ ] **Step 3a (Task 8 sub): add the 16 missing base natives.** These Ozark-base natives are absent from the generated `natives.h`: `begin_text_command_line_count`, `begin_text_command_width`, `end_text_command_get_line_count`, `end_text_command_get_width`, `get_text_scale_height`, `get_control_instructional_button`, `get_gameplay_cam_rot`, `is_input_disabled`, `play_sound_frontend`, `draw_scaleform_movie_fullscreen`, and the 6 `*_scaleform_movie_function*` push/pop natives. Add them to `src/rage/invoker/scaleform.h` (or a new `src/rage/invoker/missing_natives.h`) as hand-wrapped RVAs. **Each RVA must be IDA-verified against the CUSA00411 v1.57 eboot before use** — do not guess. Where an RVA is not yet verified, wrap it to a safe no-op/`0` and log, so the base still renders (text width falls back to a fixed estimate). List each unverified native in the commit body.

- [ ] **Step 4: Write `src/menu/base/base.{h,cpp}`** — transform. `base::update()` disables the control actions (list already uses `native::disable_control_action`/`set_input_exclusive` — keep verbatim), then `menu::renderer::render()` + `menu::submenu::handler::update()`. Keep the `is_option_selected` scroll-lerp logic. Convert Ozark's member-variable `base` class + `get_base()` (already a function-local static — good).

- [ ] **Step 5: Write `src/menu/menu.{h,cpp}`** — real entrypoints:
```cpp
void menu::build() { menu::ui::init(); menu::submenu::handler::load(); /* register demo submenu (Task 15) */ }
void menu::tick()  { menu::base::latch_input_or_update(); }   // calls base::update() each frame
```
The open bind is polled here or in base::update: if `menu::input::is_open_bind_pressed()` toggle `set_open`.

- [ ] **Step 6: Build.** With submenu_handler not yet present, temporarily stub `menu::submenu::handler::update()`/`load()`/`get_current()` via a forward header so base/renderer link; the real handler lands in Task 12 and replaces the stub. (Order note: if cleaner, do Task 12 before wiring renderer's `get_current()->get_options()` calls — the executor may reorder Tasks 8/9/12 as long as each interim state builds.)

- [ ] **Step 7: Commit**
```bash
git add -A && git commit -m "feat(menu): base + renderer + entrypoints; hand-wrap missing base natives"
```

- [ ] **Step 8 (on-console, user):** L1+○ opens the menu; header + background render in the Ozark look; D-pad disabled behind the menu. Partial **M1** (full nav needs Task 9-13).

---

## Milestone 2 — Submenu system + all option types

### Task 9: base_option + button + toggle

**Files:** Create `src/menu/base/options/option.{h,cpp}`, `button.{h,cpp}`, `toggle.{h,cpp}` (transformed).

**Interfaces:**
- Consumes: localization, ui_vars, renderer, natives, `stl::function`, `stl::stack`, `stl::tuple3`.
- Produces: `base_option` (virtual `render/render_selected/invoke_*`), `option`/`button`/`toggle` classes; `m_requirement` as `stl::function<bool()>`, `m_instructionals` as `stl::vector<stl::tuple3<stl::string,int,bool>>`.

- [ ] **Step 1: Write `option.{h,cpp}`** — transform. `std::function`→`stl::function`, `std::vector<std::tuple<...>>`→`stl::vector<stl::tuple3<...>>`, `std::stack<std::string>*`→`stl::stack<stl::string>*`, `nlohmann::json&` params (translation) → drop those virtual methods' bodies to no-op (translation out of scope) but keep the signatures with a forward-declared empty `json` type OR remove the `write_translation/read_translation/reset_translation` virtuals entirely (verify no base caller). Prefer removing them + their callsites in the handler.

- [ ] **Step 2: Write `button.{h,cpp}`** — transform. `button` holds a `stl::function<void()>` handler; `render`/`render_selected` draw the name and invoke on select.

- [ ] **Step 3: Write `toggle.{h,cpp}`** — transform. `toggle` binds a `bool*`; renders on/off, flips on select.

- [ ] **Step 4: Build.** Expected: compiles.

- [ ] **Step 5: Commit** `git commit -m "feat(options): base_option + button + toggle"`

### Task 10: number + scroll + radio + color_option + break + submenu_option

**Files:** Create the remaining option files (transformed).

- [ ] **Step 1: `break.{h,cpp}`** — the spacer/scroll-boundary option (uses `get_total_options`/`get_scroll_offset`/`get_max_options` from the handler — verify handler API from Task 12).
- [ ] **Step 2: `number.{h,cpp}`** — transform `timeGetTime()`→`platform::now_ms()`; the `double→int` clamp UB fix noted in InsulinGTA5 history applies (clamp before cast).
- [ ] **Step 3: `scroll.h`** — header-only; transform `timeGetTime()`.
- [ ] **Step 4: `radio.{h,cpp}`** — uses `radio_context` from ui_vars.
- [ ] **Step 5: `color_option.{h,cpp}`** — HSV picker option; `render_selected` opens the color modal. Uses `menu::renderer::rgb_to_hsv/hsv_to_rgb`.
- [ ] **Step 6: `submenu_option.{h,cpp}`** — holds a target `submenu*`; on select calls `menu::submenu::handler::set_submenu(target)`.
- [ ] **Step 7: Build + Commit** `git commit -m "feat(options): number/scroll/radio/color/break/submenu_option"`

### Task 11: Restore menu_input color/hotkey bodies

- [ ] **Step 1:** Un-`#if 0` the `menu_input::color(color_rgba*)` and `hotkey(...)` bodies deferred in Task 6 now that option types exist. Wire `color(...)` to the color modal; keep `hotkey(...)` as a no-op stub (feature).
- [ ] **Step 2: Build + Commit** `git commit -m "feat(input): wire menu_input color modal now that options exist"`

### Task 12: submenu + submenu_handler

**Files:** Create `src/menu/base/submenu.{h,cpp}`, `submenu_handler.{h,cpp}` (transformed). Replace the Task-8 stub.

**Interfaces:**
- Produces: `menu::submenu::submenu` (virtual `load/update/update_once/feature_update`, `add_option<T>`, `get_options`, parent chain, name stack), `menu::submenu::handler::{load,update,add_submenu,set_submenu,set_submenu_previous,get_current,get_total_options,...}`.

- [ ] **Step 1: Write `submenu.{h,cpp}`** — transform. `std::vector<std::shared_ptr<base_option>>`→`stl::vector<stl::shared_ptr<base_option>>`, `std::make_shared`→`stl::make_shared`, `std::stack`→`stl::stack`. The `add_option<T>` template and `set_parent<T>()` template stay. Drop `hotkey::read_hotkey` call or keep against the Task-6 stub.
- [ ] **Step 2: Write `submenu_handler.{h,cpp}`** — transform. Replace `native::network_is_player_connected(native::player_id())` guard in `feature_update` with `true` (single-player; features out of scope anyway). Keep nav (`set_submenu`/`set_submenu_previous`/scroll-offset math).
- [ ] **Step 3: Remove the Task-8 handler stub;** point base/renderer at the real handler.
- [ ] **Step 4: Build + Commit** `git commit -m "feat(menu): submenu + submenu_handler (real)"`

### Task 13: Navigation end-to-end (on-console M2 core)

- [ ] **Step 1:** Ensure `menu::tick()` drives: open-bind poll → `base::update()` → `menu_input::mi_update()` → handler update. Verify scroll wrap, enter/back via `submenu_option`/`set_submenu_previous`.
- [ ] **Step 2: Build + Commit** `git commit -m "feat(menu): end-to-end navigation wiring"`
- [ ] **Step 3 (on-console, user):** navigate the demo submenu (added in Task 15) — up/down/wrap/enter/back, every option type operable. Closes **M2**.

---

## Milestone 3 — Instructionals, notify, on-screen keyboard, demo

### Task 14: instructionals + notify + stacked_display + on-screen keyboard

**Files:** Create `src/menu/base/util/instructionals.{h,cpp}`, `notify.{h,cpp}`, `stacked_display.{h,cpp}` (transformed), and wire the OSK path.

- [ ] **Step 1: `instructionals.{h,cpp}`** — transform. Uses the scaleform wrappers (`sf::*` in scaleform.h + the push/pop natives from Task 8a). Replace `localization t_*` translation-registered strings with plain `stl::string`. Button icons via `get_control_instructional_button` (Task 8a). L1+○ etc. labels.
- [ ] **Step 2: `notify.{h,cpp}` + `stacked_display.{h,cpp}`** — transform. `GetTickCount()`→`platform::now_ms()`; drop the network-event notification helpers (`t_incoming_event`/`clean_name`/protection-spam map are feature code) — keep the generic `notify(title, text)` + render path. `menu::helpers::clean_name` callsite removed.
- [ ] **Step 3: On-screen keyboard** — `menu::input` text entry uses `native::display_onscreen_keyboard`/`update_onscreen_keyboard`/`get_onscreen_keyboard_result` (all present in natives.h). Wire the `number`/text option input path to it (proven in InsulinGTA5 phase 2a).
- [ ] **Step 4: Build + Commit** `git commit -m "feat(ui): instructionals, notify/stacked_display, on-screen keyboard"`

### Task 15: Demo submenu skeleton

**Files:** Create `src/menu/base/submenus/main.{h,cpp}` (new, minimal — NOT Ozark's full main.cpp).

**Interfaces:**
- Consumes: submenu, all option types.
- Produces: `main_menu : submenu` with `get()` singleton; registered in `menu::build()`.

- [ ] **Step 1:** Write `main_menu` deriving `menu::submenu::submenu`, with `load()` adding one of each option type into a "Demo" submenu: a `button` (notify "hello"), a `toggle` (bound to a static bool), a `number` (int slider), a `radio`, a `color_option` (bound to `global::ui::g_option` color), and a `submenu_option` opening a child submenu with a couple of `button`s. Set the header title to "InsulinGTAV".
- [ ] **Step 2:** Register it in `menu::build()` via `menu::submenu::handler::add_submenu` + set as main.
- [ ] **Step 3: Build + Commit** `git commit -m "feat(menu): demo submenu skeleton exercising every option type"`
- [ ] **Step 4 (on-console, user):** instructional bar shows correct PS4 glyphs; a test notification renders; entering the number option opens the on-screen keyboard; every option type is visible and operable. Closes **M3**.

---

## Self-Review

**Spec coverage** (each spec section → task):
- Scope "portiert wird" list → Tasks 6-15 (every named module). ✓
- Invoker decision (raw-pointer Vector3 out-params, no redirect) → Task 2. ✓
- Basic natives.h as name/drift reference → Task 8a (RVA verification note). ✓
- Control-natives input + L1+○ + scePad fallback → Task 6. ✓
- On-screen keyboard → Task 14 Step 3. ✓
- STL mini-STL + std→stl transform + STL_FREESTANDING → Task 1, Global Constraints, Task 5. ✓
- Tick via script-thread hook → Task 4 + Task 8 Step 5. ✓
- Logging → notify + klog → Task 3. ✓
- No .init_array / constexpr globals → Task 7 Step 3. ✓
- Milestones M0-M3 with on-console acceptance → Task 5/8/13/15 final steps. ✓
- Sprites via sentinel/game-YTD, no WIC → Task 8 Step 2. ✓

**Placeholder scan:** No "TBD"/"handle edge cases". The one deliberate deferral (unverified RVAs in Task 8a) is explicit with a defined fallback (no-op + log + fixed width estimate) and a verification requirement — not a silent gap.

**Type consistency:** `stl::` names used consistently; `is_open_bind_pressed` defined in Task 6 and used in Task 8. `menu::ui::init()` introduced in Task 7 Step 3 and called in Task 8 Step 5. Handler API names (`get_total_options`, `set_submenu`, `set_submenu_previous`) consistent between Tasks 8/10/12.

**Ordering caveat (flagged, not a defect):** Tasks 8/9/12 have a mutual reference (renderer↔handler↔options). The plan builds through it with a Task-8 handler stub replaced in Task 12; the executor may legally reorder these three as long as each interim commit builds. Every other task is strictly ordered.
