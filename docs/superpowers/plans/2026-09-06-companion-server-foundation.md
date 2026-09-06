# Companion Server Foundation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Run an HTTP server inside the plugin that serves a page from `/data` and reports the player's live position, so a phone on the same network can reach the console.

**Architecture:** A dedicated socket thread accepts one connection at a time and never touches an engine structure. It parses requests on raw byte buffers, serves static files, answers `/api/state` from a snapshot the game thread publishes, and hands anything that needs the engine to a POD job ring drained by a submenu's `feature_update()`. Everything except the socket layer and the position read is pure logic and unit-tested on the host.

**Tech Stack:** C++17, `-fno-exceptions -fno-rtti`, project-local mini-STL, OpenOrbis toolchain, `<orbis/Net.h>` sceNet sockets, host tests built with clang++ against C headers only.

**Spec:** `docs/superpowers/specs/2026-09-06-companion-server-design.md`

## Global Constraints

- Target build is **CUSA00411 v1.57** only. Every RVA is an offset from ELF base 0.
- **No `stl::string` anywhere in the request path.** It is a fixed 128-byte buffer that truncates silently; header blocks are routinely longer. Raw `const char*` + length throughout.
- **No exceptions, no RTTI.** `-fno-exceptions -fno-rtti` are set project-wide.
- **No `.init_array`.** Global constructors do not run. Use function-local statics (`static T x; return &x;`) or explicit init.
- **The HTTP thread must never call a native or read an engine structure.** Natives are only safe on the game script thread.
- **Nothing may call a native during `menu::build()`.** Option construction sets state; applying belongs in `feature_update()`.
- Files intended for host tests must not include `platform/log.h` or any PS4 header — `src/util/image/decode.cpp` is the reference for this.
- Host tests build with: `clang++ -std=c++17 -I src tests/<name>.cpp [deps] -o build/<name>.exe`. C headers only; MSVC's C++ stdlib rejects the installed clang (STL1000).
- Header block cap **8192** bytes. Default port **8080**. PIN is exactly 4 ASCII digits.
- Source files are picked up by `file(GLOB_RECURSE src/*.cpp CONFIGURE_DEPENDS)` — adding a `.cpp` needs no CMake edit.
- Console FTP is `10.10.10.236:2121`, kernel log `10.10.10.236:3232`.

---

### Task 1: HTTP request parsing and response writing

Pure byte-buffer parsing. No sockets, no files, no engine.

**Files:**
- Create: `src/net/http.h`
- Create: `src/net/http.cpp`
- Test: `tests/net_http_test.cpp`

**Interfaces:**
- Consumes: nothing.
- Produces:
  - `enum class net::method { get, put, post, other }`
  - `struct net::request { method m; const char* path; unsigned path_len; const char* query; unsigned query_len; unsigned content_length; const char* pin; unsigned pin_len; unsigned header_bytes; }`
  - `enum class net::parse_result { need_more, ok, bad }`
  - `net::parse_result net::parse_request(const char* buf, unsigned len, request* out)`
  - `unsigned net::write_response(char* out, unsigned cap, int status, const char* content_type, const char* body, unsigned body_len)`
  - `const unsigned net::header_cap = 8192;`

- [ ] **Step 1: Write the failing test**

Create `tests/net_http_test.cpp`:

```cpp
// Host unit tests for the companion server's HTTP parsing.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/net_http_test.cpp src/net/http.cpp -o build/net_http_test.exe
//   ./build/net_http_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
#include "net/http.h"
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

static bool span_is(const char* p, unsigned n, const char* want) {
    return p && n == (unsigned)strlen(want) && memcmp(p, want, n) == 0;
}

int main() {
    net::request r;

    // A minimal GET parses, and path excludes the query string.
    const char* g = "GET /index.html?pin=1234 HTTP/1.1\r\nHost: x\r\n\r\n";
    check_true("get parses", net::parse_request(g, (unsigned)strlen(g), &r) == net::parse_result::ok);
    check_true("get method", r.m == net::method::get);
    check_true("get path", span_is(r.path, r.path_len, "/index.html"));
    check_true("get query", span_is(r.query, r.query_len, "pin=1234"));
    check_eq("get header_bytes", r.header_bytes, (unsigned)strlen(g));

    // No query at all leaves an empty span, not a null path.
    const char* nq = "GET / HTTP/1.1\r\n\r\n";
    check_true("noquery parses", net::parse_request(nq, (unsigned)strlen(nq), &r) == net::parse_result::ok);
    check_true("noquery path", span_is(r.path, r.path_len, "/"));
    check_eq("noquery query len", r.query_len, 0);

    // An incomplete header block asks for more rather than failing.
    const char* part = "GET / HTTP/1.1\r\nHost: x\r\n";
    check_true("partial needs more",
               net::parse_request(part, (unsigned)strlen(part), &r) == net::parse_result::need_more);

    // Content-Length is read; header name matching is case-insensitive.
    const char* put = "PUT /api/upload/a HTTP/1.1\r\ncontent-length: 42\r\n\r\n";
    check_true("put parses", net::parse_request(put, (unsigned)strlen(put), &r) == net::parse_result::ok);
    check_true("put method", r.m == net::method::put);
    check_eq("put content_length", r.content_length, 42);

    // The PIN header is extracted, whatever its case.
    const char* pin = "GET /api/state HTTP/1.1\r\nX-Insulin-Pin: 4711\r\n\r\n";
    check_true("pin parses", net::parse_request(pin, (unsigned)strlen(pin), &r) == net::parse_result::ok);
    check_true("pin value", span_is(r.pin, r.pin_len, "4711"));

    // Absent PIN is null, not an empty span pointing at random memory.
    check_true("no pin parses", net::parse_request(g, (unsigned)strlen(g), &r) == net::parse_result::ok);
    check_true("no pin is null", r.pin == 0);

    // A header block larger than the cap is rejected, not buffered.
    static char big[net::header_cap + 64];
    memset(big, 'a', sizeof(big));
    memcpy(big, "GET / HTTP/1.1\r\nX: ", 19);
    check_true("oversized rejected",
               net::parse_request(big, (unsigned)sizeof(big), &r) == net::parse_result::bad);

    // A garbage first line is rejected.
    const char* junk = "nonsense\r\n\r\n";
    check_true("junk rejected",
               net::parse_request(junk, (unsigned)strlen(junk), &r) == net::parse_result::bad);

    // Responses carry status, type, length and an explicit close.
    char out[256];
    unsigned n = net::write_response(out, sizeof(out), 200, "text/plain", "hi", 2);
    check_true("response written", n > 0);
    out[n] = 0;
    check_true("response status", strstr(out, "HTTP/1.1 200 OK\r\n") == out);
    check_true("response type", strstr(out, "Content-Type: text/plain\r\n") != 0);
    check_true("response length", strstr(out, "Content-Length: 2\r\n") != 0);
    check_true("response close", strstr(out, "Connection: close\r\n") != 0);
    check_true("response body", strstr(out, "\r\n\r\nhi") != 0);

    // 401 is reachable with no body.
    n = net::write_response(out, sizeof(out), 401, "text/plain", 0, 0);
    out[n] = 0;
    check_true("401 status", strstr(out, "HTTP/1.1 401 Unauthorized\r\n") == out);
    check_true("401 zero length", strstr(out, "Content-Length: 0\r\n") != 0);

    // A buffer that cannot hold the response returns 0 rather than truncating.
    check_eq("tight buffer refuses", net::write_response(out, 8, 200, "text/plain", "hi", 2), 0);

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed != 0;
}
```

- [ ] **Step 2: Run test to verify it fails**

Run:
```bash
clang++ -std=c++17 -I src tests/net_http_test.cpp src/net/http.cpp -o build/net_http_test.exe
```
Expected: FAIL — `net/http.h` does not exist yet, so the compile errors out.

- [ ] **Step 3: Write the header**

Create `src/net/http.h`:

```cpp
#pragma once

// HTTP/1.1 request parsing for the companion server, on raw byte buffers.
//
// stl::string is a fixed 128-byte buffer that truncates silently, and a header
// block is routinely longer than that, so nothing here uses it. Every field is
// a pointer into the caller's buffer plus a length; the buffer must outlive the
// request struct, and no field is nul-terminated.
//
// No PS4 headers, no logging: this compiles on the host so the parser can be
// unit-tested (see tests/net_http_test.cpp).

namespace net {

    // Anything larger than this is refused rather than buffered.
    const unsigned header_cap = 8192;

    enum class method { get, put, post, other };

    enum class parse_result {
        need_more,  // no CRLFCRLF yet - read more and call again
        ok,         // request filled in
        bad         // malformed, or the header block exceeds header_cap
    };

    struct request {
        method      m;
        const char* path;            // "/index.html", query stripped
        unsigned    path_len;
        const char* query;           // "pin=1234", empty span if absent
        unsigned    query_len;
        unsigned    content_length;  // 0 when the header is absent
        const char* pin;             // X-Insulin-Pin value, null when absent
        unsigned    pin_len;
        unsigned    header_bytes;    // bytes consumed; the body starts here
    };

    parse_result parse_request(const char* buf, unsigned len, request* out);

    // Writes a complete response into out. Returns the byte count, or 0 if it
    // does not fit - callers must treat 0 as "drop the connection", never as
    // "send nothing".
    unsigned write_response(char* out, unsigned cap,
                            int status, const char* content_type,
                            const char* body, unsigned body_len);
}
```

- [ ] **Step 4: Write the implementation**

Create `src/net/http.cpp`:

```cpp
#include "net/http.h"

#include <string.h>
#include <stdio.h>

namespace {

    bool ci_equal(const char* a, unsigned a_len, const char* b) {
        unsigned n = 0;
        while (b[n]) n++;
        if (a_len != n) return false;
        for (unsigned i = 0; i < n; i++) {
            char x = a[i], y = b[i];
            if (x >= 'A' && x <= 'Z') x = (char)(x - 'A' + 'a');
            if (y >= 'A' && y <= 'Z') y = (char)(y - 'A' + 'a');
            if (x != y) return false;
        }
        return true;
    }

    unsigned to_uint(const char* p, unsigned len) {
        unsigned v = 0;
        for (unsigned i = 0; i < len; i++) {
            if (p[i] < '0' || p[i] > '9') break;
            v = v * 10 + (unsigned)(p[i] - '0');
        }
        return v;
    }

    const char* reason_for(int status) {
        switch (status) {
            case 200: return "OK";
            case 400: return "Bad Request";
            case 401: return "Unauthorized";
            case 403: return "Forbidden";
            case 404: return "Not Found";
            case 413: return "Payload Too Large";
            case 503: return "Service Unavailable";
            default:  return "Error";
        }
    }
}

namespace net {

    parse_result parse_request(const char* buf, unsigned len, request* out) {
        // Find the end of the header block first. Refusing oversized blocks
        // before any parsing keeps the work bounded no matter what arrives.
        unsigned end = 0;
        bool found = false;
        unsigned scan_to = len < header_cap ? len : header_cap;
        for (unsigned i = 0; i + 3 < scan_to; i++) {
            if (buf[i] == '\r' && buf[i + 1] == '\n' && buf[i + 2] == '\r' && buf[i + 3] == '\n') {
                end = i + 4;
                found = true;
                break;
            }
        }
        if (!found) return len >= header_cap ? parse_result::bad : parse_result::need_more;

        out->m              = method::other;
        out->path           = 0;
        out->path_len       = 0;
        out->query          = buf;   // empty span, never null
        out->query_len      = 0;
        out->content_length = 0;
        out->pin            = 0;
        out->pin_len        = 0;
        out->header_bytes   = end;

        // Request line: METHOD SP TARGET SP VERSION CRLF
        unsigned sp1 = 0;
        while (sp1 < end && buf[sp1] != ' ' && buf[sp1] != '\r') sp1++;
        if (sp1 >= end || buf[sp1] != ' ') return parse_result::bad;

        if      (ci_equal(buf, sp1, "GET"))  out->m = method::get;
        else if (ci_equal(buf, sp1, "PUT"))  out->m = method::put;
        else if (ci_equal(buf, sp1, "POST")) out->m = method::post;
        else                                 return parse_result::bad;

        unsigned target = sp1 + 1;
        unsigned sp2 = target;
        while (sp2 < end && buf[sp2] != ' ' && buf[sp2] != '\r') sp2++;
        if (sp2 >= end || buf[sp2] != ' ') return parse_result::bad;
        if (sp2 == target) return parse_result::bad;

        unsigned q = target;
        while (q < sp2 && buf[q] != '?') q++;
        out->path     = buf + target;
        out->path_len = q - target;
        if (q < sp2) {
            out->query     = buf + q + 1;
            out->query_len = sp2 - (q + 1);
        }

        // Headers. Each line is NAME ":" OWS VALUE CRLF.
        unsigned i = target;
        while (i + 1 < end && !(buf[i] == '\r' && buf[i + 1] == '\n')) i++;
        i += 2;

        while (i + 1 < end && !(buf[i] == '\r' && buf[i + 1] == '\n')) {
            unsigned line_end = i;
            while (line_end + 1 < end && !(buf[line_end] == '\r' && buf[line_end + 1] == '\n')) line_end++;

            unsigned colon = i;
            while (colon < line_end && buf[colon] != ':') colon++;
            if (colon < line_end) {
                unsigned name_len = colon - i;
                unsigned v = colon + 1;
                while (v < line_end && (buf[v] == ' ' || buf[v] == '\t')) v++;
                unsigned v_len = line_end - v;

                if (ci_equal(buf + i, name_len, "content-length")) {
                    out->content_length = to_uint(buf + v, v_len);
                } else if (ci_equal(buf + i, name_len, "x-insulin-pin")) {
                    out->pin     = buf + v;
                    out->pin_len = v_len;
                }
            }
            i = line_end + 2;
        }

        return parse_result::ok;
    }

    unsigned write_response(char* out, unsigned cap,
                            int status, const char* content_type,
                            const char* body, unsigned body_len) {
        if (!body) body_len = 0;

        // snprintf reports what it *would* have written, so an overlong header
        // is caught before any of the body is copied.
        int n = snprintf(out, cap,
                         "HTTP/1.1 %d %s\r\n"
                         "Content-Type: %s\r\n"
                         "Content-Length: %u\r\n"
                         "Connection: close\r\n"
                         "\r\n",
                         status, reason_for(status),
                         content_type ? content_type : "application/octet-stream",
                         body_len);
        if (n < 0 || (unsigned)n >= cap) return 0;
        if ((unsigned)n + body_len > cap) return 0;

        if (body_len) memcpy(out + n, body, body_len);
        return (unsigned)n + body_len;
    }
}
```

- [ ] **Step 5: Run test to verify it passes**

Run:
```bash
clang++ -std=c++17 -I src tests/net_http_test.cpp src/net/http.cpp -o build/net_http_test.exe && ./build/net_http_test.exe
```
Expected: every line `ok`, final line `all passed`, exit code 0.

- [ ] **Step 6: Commit**

```bash
git add src/net/http.h src/net/http.cpp tests/net_http_test.cpp
git commit -m "feat(net): HTTP request parsing and response writing on raw buffers"
```

---

### Task 2: POD job ring for the thread handover

The HTTP thread cannot call natives, and `game::run_on_game_thread` takes a bare `void(*)()` with no capture, so a job cannot ride in a lambda. This is the explicit queue that replaces it.

**Files:**
- Create: `src/net/jobs.h`
- Create: `src/net/jobs.cpp`
- Test: `tests/net_jobs_test.cpp`
- Test: `tests/net_jobs_mt_test.cpp`

**Interfaces:**
- Consumes: nothing.
- Produces:
  - `enum class net::job_kind : unsigned { none = 0, teleport = 1 }`
  - `struct net::job { job_kind kind; float x, y, z; bool ground; }`
  - `class net::job_ring` with `static const unsigned capacity = 16;`, `void reset()`, `bool push(const job&)`, `bool pop(job*)`, `unsigned dropped() const`
  - `net::job_ring& net::jobs()` — the single shared ring

- [ ] **Step 1: Write the failing single-threaded test**

Create `tests/net_jobs_test.cpp`:

```cpp
// Host unit tests for the companion server's job ring.
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/net_jobs_test.cpp src/net/jobs.cpp -o build/net_jobs_test.exe
//   ./build/net_jobs_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
#include "net/jobs.h"
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

static net::job tp(float x) {
    net::job j;
    j.kind = net::job_kind::teleport;
    j.x = x; j.y = 0.0f; j.z = 0.0f; j.ground = true;
    return j;
}

int main() {
    net::job_ring r;
    r.reset();
    net::job out;

    check_true("empty pops nothing", !r.pop(&out));

    check_true("push accepted", r.push(tp(1.0f)));
    check_true("pop returns it", r.pop(&out));
    check_true("pop kind", out.kind == net::job_kind::teleport);
    check_true("pop payload", out.x == 1.0f);
    check_true("empty again", !r.pop(&out));

    // FIFO order, not LIFO.
    r.push(tp(1.0f)); r.push(tp(2.0f)); r.push(tp(3.0f));
    r.pop(&out); check_true("fifo first", out.x == 1.0f);
    r.pop(&out); check_true("fifo second", out.x == 2.0f);
    r.pop(&out); check_true("fifo third", out.x == 3.0f);

    // Filling to capacity works; one more is dropped, not written over a live slot.
    r.reset();
    for (unsigned i = 0; i < net::job_ring::capacity; i++) {
        check_true("fill accepted", r.push(tp((float)i)));
    }
    check_true("overflow refused", !r.push(tp(999.0f)));
    check_eq("dropped counted", r.dropped(), 1);

    // The oldest entry survived the refused push.
    check_true("oldest intact", r.pop(&out) && out.x == 0.0f);

    // Draining frees slots again - wrap-around is correct.
    while (r.pop(&out)) {}
    for (unsigned i = 0; i < net::job_ring::capacity; i++) {
        check_true("refill accepted", r.push(tp(100.0f + i)));
    }
    check_true("first after wrap", r.pop(&out) && out.x == 100.0f);

    // reset() clears the drop counter too.
    r.reset();
    check_eq("dropped cleared by reset", r.dropped(), 0);
    check_true("empty after reset", !r.pop(&out));

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed != 0;
}
```

- [ ] **Step 2: Run test to verify it fails**

Run:
```bash
clang++ -std=c++17 -I src tests/net_jobs_test.cpp src/net/jobs.cpp -o build/net_jobs_test.exe
```
Expected: FAIL — `net/jobs.h` does not exist.

- [ ] **Step 3: Write the implementation**

Create `src/net/jobs.h`:

```cpp
#pragma once

// Work handed from the HTTP thread to the game thread.
//
// The HTTP thread may not call a native or read an engine structure, so
// anything that needs the engine is pushed here and drained from the companion
// submenu's feature_update(), which already runs once per frame behind the
// player_valid() gate.
//
// game::run_on_game_thread takes a bare void(*)() with no capture, and
// stl::function caps captures at 64 bytes - smaller than a path - so the
// payload cannot ride in a callable. Jobs are POD and copied by value.
//
// No PS4 headers: this compiles on the host for tests/net_jobs_test.cpp.

namespace net {

    enum class job_kind : unsigned {
        none     = 0,
        teleport = 1
    };

    struct job {
        job_kind kind;
        float    x, y, z;
        bool     ground;   // resolve ground height instead of using z
    };

    class job_ring {
    public:
        static const unsigned capacity = 16;

        void     reset();
        bool     push(const job& j);   // any thread; false when full
        bool     pop(job* out);        // game thread only
        unsigned dropped() const;

    private:
        volatile int m_lock  = 0;
        unsigned     m_head  = 0;      // next write
        unsigned     m_tail  = 0;      // next read
        unsigned     m_count = 0;
        unsigned     m_dropped = 0;
        job          m_slots[capacity] = {};
    };

    // The one ring the server and the submenu share. Function-local static:
    // .init_array does not run in this plugin.
    job_ring& jobs();
}
```

Create `src/net/jobs.cpp`:

```cpp
#include "net/jobs.h"

namespace {
    // Same primitive the protections ring and game_thread.cpp use. The critical
    // section is a handful of field writes, so a spin is cheaper than anything
    // that could put the HTTP thread to sleep holding the game thread up.
    struct guard {
        volatile int* lock;
        explicit guard(volatile int* l) : lock(l) {
            while (__sync_lock_test_and_set(lock, 1)) { }
        }
        ~guard() { __sync_lock_release(lock); }
    };
}

namespace net {

    void job_ring::reset() {
        guard g(&m_lock);
        m_head = m_tail = m_count = m_dropped = 0;
    }

    bool job_ring::push(const job& j) {
        guard g(&m_lock);
        if (m_count == capacity) {
            m_dropped++;
            return false;
        }
        m_slots[m_head] = j;
        m_head = (m_head + 1) % capacity;
        m_count++;
        return true;
    }

    bool job_ring::pop(job* out) {
        guard g(&m_lock);
        if (m_count == 0) return false;
        *out = m_slots[m_tail];
        m_tail = (m_tail + 1) % capacity;
        m_count--;
        return true;
    }

    unsigned job_ring::dropped() const {
        return m_dropped;
    }

    job_ring& jobs() {
        static job_ring instance;
        return instance;
    }
}
```

- [ ] **Step 4: Run the single-threaded test to verify it passes**

Run:
```bash
clang++ -std=c++17 -I src tests/net_jobs_test.cpp src/net/jobs.cpp -o build/net_jobs_test.exe && ./build/net_jobs_test.exe
```
Expected: every line `ok`, final line `all passed`, exit code 0.

- [ ] **Step 5: Write the concurrent test**

Create `tests/net_jobs_mt_test.cpp`:

```cpp
// Host unit test: the job ring under a concurrent producer and consumer.
//
// This lives in its own file, separate from tests/net_jobs_test.cpp, because it
// needs <thread>/<atomic> - C++ headers - and the installed clang has no libc++
// on this box, so it falls through to the real MSVC STL. Follow the same build
// line tests/protections_ring_mt_test.cpp documents for that situation; the
// plain "-I src" line used by the rest of the suite is not enough here.
//
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/net_jobs_mt_test.cpp src/net/jobs.cpp -o build/net_jobs_mt_test.exe
//   ./build/net_jobs_mt_test.exe
#include "net/jobs.h"
#include <stdio.h>
#include <thread>
#include <atomic>

static int g_failed = 0;

static void check_true(const char* what, bool ok) {
    if (!ok) { printf("FAIL %s\n", what); g_failed++; }
    else     { printf("ok   %s\n", what); }
}

int main() {
    net::job_ring r;
    r.reset();

    const unsigned per_thread = 5000;
    const unsigned producers  = 4;

    std::atomic<unsigned> pushed(0);
    std::atomic<bool>     stop(false);

    std::thread ts[producers];
    for (unsigned t = 0; t < producers; t++) {
        ts[t] = std::thread([&r, &pushed, per_thread]() {
            for (unsigned i = 0; i < per_thread; i++) {
                net::job j;
                j.kind = net::job_kind::teleport;
                j.x = 1.0f; j.y = 0.0f; j.z = 0.0f; j.ground = true;
                if (r.push(j)) pushed++;
            }
        });
    }

    // One consumer, standing in for the game thread.
    unsigned popped = 0;
    std::thread consumer([&r, &popped, &stop]() {
        net::job out;
        while (!stop.load()) {
            while (r.pop(&out)) popped++;
        }
        while (r.pop(&out)) popped++;
    });

    for (unsigned t = 0; t < producers; t++) ts[t].join();
    stop.store(true);
    consumer.join();

    // Nothing is invented and nothing vanishes: everything the ring accepted
    // comes back out exactly once, and the rest is accounted for as dropped.
    check_true("no jobs lost or duplicated", popped == pushed.load());
    check_true("all attempts accounted for",
               pushed.load() + r.dropped() == producers * per_thread);
    check_true("every popped job is well-formed", popped > 0);

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed != 0;
}
```

- [ ] **Step 6: Run the concurrent test to verify it passes**

Run:
```bash
clang++ -std=c++17 -I src tests/net_jobs_mt_test.cpp src/net/jobs.cpp -o build/net_jobs_mt_test.exe && ./build/net_jobs_mt_test.exe
```
Expected: `all passed`, exit code 0. If the link fails on STL headers, apply the extra flags documented at the top of `tests/protections_ring_mt_test.cpp`.

- [ ] **Step 7: Commit**

```bash
git add src/net/jobs.h src/net/jobs.cpp tests/net_jobs_test.cpp tests/net_jobs_mt_test.cpp
git commit -m "feat(net): POD job ring for the HTTP-to-game-thread handover"
```

---

### Task 3: Connection handling, routing, static files and the PIN gate

Everything a connection does, over an injectable byte transport so it is fully host-testable with no socket.

**Files:**
- Create: `src/net/conn.h`
- Create: `src/net/conn.cpp`
- Test: `tests/net_conn_test.cpp`

**Interfaces:**
- Consumes: `net::parse_request`, `net::write_response`, `net::request`, `net::method`, `net::parse_result`, `net::header_cap` (Task 1); `net::jobs()`, `net::job`, `net::job_kind` (Task 2).
- Produces:
  - `typedef int (*net::read_fn)(void* ctx, char* buf, unsigned cap)` — bytes read, 0 on clean close, -1 on error
  - `typedef int (*net::write_fn)(void* ctx, const char* buf, unsigned len)` — bytes written, -1 on error
  - `typedef unsigned (*net::state_fn)(char* out, unsigned cap)` — writes JSON, returns length
  - `struct net::config { const char* web_root; char pin[5]; state_fn state; }`
  - `void net::serve_one(const config& cfg, void* ctx, read_fn rd, write_fn wr)`

- [ ] **Step 1: Write the failing test**

Create `tests/net_conn_test.cpp`:

```cpp
// Host unit tests for the companion server's connection handling: routing, the
// PIN gate, static files and path-traversal rejection. The byte transport is
// injected, so no socket is involved.
//
// Build + run (from repo root):
//   clang++ -std=c++17 -I src tests/net_conn_test.cpp src/net/conn.cpp src/net/http.cpp src/net/jobs.cpp -o build/net_conn_test.exe
//   ./build/net_conn_test.exe
// C headers only: MSVC's C++ stdlib rejects the installed clang (STL1000).
#include "net/conn.h"
#include "net/jobs.h"
#include <stdio.h>
#include <string.h>

static int g_failed = 0;

static void check_true(const char* what, bool ok) {
    if (!ok) { printf("FAIL %s\n", what); g_failed++; }
    else     { printf("ok   %s\n", what); }
}

// A fake transport: hands the request out in one chunk, collects the reply.
struct fake {
    const char* in;
    unsigned    in_len;
    unsigned    in_pos;
    char        out[8192];
    unsigned    out_len;
};

static int fake_read(void* ctx, char* buf, unsigned cap) {
    fake* f = (fake*)ctx;
    unsigned left = f->in_len - f->in_pos;
    if (left == 0) return 0;
    unsigned n = left < cap ? left : cap;
    memcpy(buf, f->in + f->in_pos, n);
    f->in_pos += n;
    return (int)n;
}

static int fake_write(void* ctx, const char* buf, unsigned len) {
    fake* f = (fake*)ctx;
    if (f->out_len + len >= sizeof(f->out)) return -1;
    memcpy(f->out + f->out_len, buf, len);
    f->out_len += len;
    f->out[f->out_len] = 0;
    return (int)len;
}

static unsigned fake_state(char* out, unsigned cap) {
    int n = snprintf(out, cap, "{\"x\":1.5,\"y\":2.5,\"z\":3.5,\"heading\":90.0}");
    return n < 0 ? 0u : (unsigned)n;
}

static void run(net::config& cfg, const char* request, fake* f) {
    f->in = request;
    f->in_len = (unsigned)strlen(request);
    f->in_pos = 0;
    f->out_len = 0;
    f->out[0] = 0;
    net::serve_one(cfg, f, fake_read, fake_write);
}

int main() {
    // A web root with one file in it, written next to the test binary.
    const char* root = "build/net_conn_root";
#ifdef _WIN32
    system("if not exist build\\net_conn_root mkdir build\\net_conn_root");
#else
    system("mkdir -p build/net_conn_root");
#endif
    FILE* fp = fopen("build/net_conn_root/index.html", "wb");
    check_true("fixture written", fp != 0);
    if (fp) { fputs("<h1>insulin</h1>", fp); fclose(fp); }

    net::config cfg;
    cfg.web_root = root;
    memcpy(cfg.pin, "4711", 5);
    cfg.state = fake_state;

    fake f;

    // No PIN at all is rejected before anything is served.
    run(cfg, "GET /index.html HTTP/1.1\r\n\r\n", &f);
    check_true("missing pin is 401", strstr(f.out, "HTTP/1.1 401") == f.out);

    // A wrong PIN is rejected the same way.
    run(cfg, "GET /index.html HTTP/1.1\r\nX-Insulin-Pin: 0000\r\n\r\n", &f);
    check_true("wrong pin is 401", strstr(f.out, "HTTP/1.1 401") == f.out);

    // The PIN is accepted from the header.
    run(cfg, "GET /index.html HTTP/1.1\r\nX-Insulin-Pin: 4711\r\n\r\n", &f);
    check_true("header pin serves file", strstr(f.out, "HTTP/1.1 200") == f.out);
    check_true("file body served", strstr(f.out, "<h1>insulin</h1>") != 0);
    check_true("html content type", strstr(f.out, "Content-Type: text/html") != 0);

    // ...and from the query string, so the first navigation can carry it.
    run(cfg, "GET /index.html?pin=4711 HTTP/1.1\r\n\r\n", &f);
    check_true("query pin serves file", strstr(f.out, "HTTP/1.1 200") == f.out);

    // "/" resolves to index.html.
    run(cfg, "GET /?pin=4711 HTTP/1.1\r\n\r\n", &f);
    check_true("root serves index", strstr(f.out, "<h1>insulin</h1>") != 0);

    // A missing file is 404, not a crash or an empty 200.
    run(cfg, "GET /nope.html?pin=4711 HTTP/1.1\r\n\r\n", &f);
    check_true("missing file is 404", strstr(f.out, "HTTP/1.1 404") == f.out);

    // Path traversal is refused even though the PIN is right.
    run(cfg, "GET /../CMakeLists.txt?pin=4711 HTTP/1.1\r\n\r\n", &f);
    check_true("traversal is 403", strstr(f.out, "HTTP/1.1 403") == f.out);

    // /api/state is answered from the injected provider, as JSON.
    run(cfg, "GET /api/state?pin=4711 HTTP/1.1\r\n\r\n", &f);
    check_true("state is 200", strstr(f.out, "HTTP/1.1 200") == f.out);
    check_true("state is json", strstr(f.out, "Content-Type: application/json") != 0);
    check_true("state body", strstr(f.out, "\"heading\":90.0") != 0);

    // /api/teleport pushes exactly one job and does not touch the engine.
    net::jobs().reset();
    run(cfg, "POST /api/teleport?pin=4711 HTTP/1.1\r\nContent-Length: 38\r\n\r\n"
             "{\"x\":10.5,\"y\":-20.25,\"ground\":true}", &f);
    check_true("teleport is 200", strstr(f.out, "HTTP/1.1 200") == f.out);
    net::job j;
    check_true("teleport queued", net::jobs().pop(&j));
    check_true("teleport kind", j.kind == net::job_kind::teleport);
    check_true("teleport x", j.x == 10.5f);
    check_true("teleport y", j.y == -20.25f);
    check_true("teleport ground", j.ground);
    check_true("exactly one job", !net::jobs().pop(&j));

    // A full ring answers busy instead of blocking.
    net::jobs().reset();
    for (unsigned i = 0; i < net::job_ring::capacity; i++) {
        net::job fill; fill.kind = net::job_kind::teleport;
        fill.x = fill.y = fill.z = 0.0f; fill.ground = false;
        net::jobs().push(fill);
    }
    run(cfg, "POST /api/teleport?pin=4711 HTTP/1.1\r\nContent-Length: 38\r\n\r\n"
             "{\"x\":10.5,\"y\":-20.25,\"ground\":true}", &f);
    check_true("full ring is 503", strstr(f.out, "HTTP/1.1 503") == f.out);

    // An unknown API path is 404, not a silent 200.
    run(cfg, "GET /api/nothing?pin=4711 HTTP/1.1\r\n\r\n", &f);
    check_true("unknown api is 404", strstr(f.out, "HTTP/1.1 404") == f.out);

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed != 0;
}
```

- [ ] **Step 2: Run test to verify it fails**

Run:
```bash
clang++ -std=c++17 -I src tests/net_conn_test.cpp src/net/conn.cpp src/net/http.cpp src/net/jobs.cpp -o build/net_conn_test.exe
```
Expected: FAIL — `net/conn.h` does not exist.

- [ ] **Step 3: Write the header**

Create `src/net/conn.h`:

```cpp
#pragma once

#include "net/http.h"

// One request on one connection: read, route, reply.
//
// The byte transport is injected rather than opened here, for two reasons: the
// same code then runs under a host test with no socket (tests/net_conn_test.cpp),
// and the socket layer stays in one file that never has to be unit-tested.
//
// Nothing in this file touches an engine structure or calls a native. Work that
// needs the engine is pushed to net::jobs() and drained on the game thread.
//
// No PS4 headers: this compiles on the host.

namespace net {

    // Bytes read into buf, 0 on clean close, -1 on error.
    typedef int (*read_fn)(void* ctx, char* buf, unsigned cap);
    // Bytes written, -1 on error. Partial writes are the caller's problem.
    typedef int (*write_fn)(void* ctx, const char* buf, unsigned len);
    // Writes a JSON object into out, returns its length (0 on failure).
    typedef unsigned (*state_fn)(char* out, unsigned cap);

    struct config {
        const char* web_root;   // e.g. "/data/GoldHEN/insulin/web"
        char        pin[5];     // 4 digits + nul
        state_fn    state;      // published by the game thread; never null
    };

    // Serves exactly one request, then returns. The caller closes the socket.
    void serve_one(const config& cfg, void* ctx, read_fn rd, write_fn wr);
}
```

- [ ] **Step 4: Write the implementation**

Create `src/net/conn.cpp`:

```cpp
#include "net/conn.h"
#include "net/jobs.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

namespace {

    // Big enough for any reply this server produces: the largest is a static
    // file, and files above this are refused rather than streamed. Keeping it
    // static avoids an allocation on the request path, and the buffer lives in
    // .bss rather than on the server thread's stack - a large frame there
    // faults on entry with no useful log.
    const unsigned k_io_cap = 256 * 1024;
    static char g_in[net::header_cap + 4096];
    static char g_out[k_io_cap];

    void reply(void* ctx, net::write_fn wr, int status,
               const char* type, const char* body, unsigned body_len) {
        unsigned n = net::write_response(g_out, sizeof(g_out), status, type, body, body_len);
        if (n) wr(ctx, g_out, n);
    }

    bool span_eq(const char* p, unsigned n, const char* want) {
        return n == (unsigned)strlen(want) && memcmp(p, want, n) == 0;
    }

    // The PIN may arrive as a header or as ?pin=NNNN, because the very first
    // navigation cannot set a header. Both are checked against the same value.
    bool pin_ok(const net::config& cfg, const net::request& r) {
        if (r.pin && r.pin_len == 4 && memcmp(r.pin, cfg.pin, 4) == 0) return true;

        const char* q = r.query;
        unsigned    n = r.query_len;
        for (unsigned i = 0; i + 4 <= n; i++) {
            bool at_start = (i == 0) || q[i - 1] == '&';
            if (at_start && memcmp(q + i, "pin=", 4) == 0) {
                unsigned v = i + 4, len = 0;
                while (v + len < n && q[v + len] != '&') len++;
                return len == 4 && memcmp(q + v, cfg.pin, 4) == 0;
            }
        }
        return false;
    }

    const char* content_type_for(const char* path, unsigned len) {
        struct { const char* ext; const char* type; } table[] = {
            { ".html", "text/html; charset=utf-8" },
            { ".js",   "text/javascript" },
            { ".css",  "text/css" },
            { ".json", "application/json" },
            { ".png",  "image/png" },
            { ".jpg",  "image/jpeg" },
        };
        for (unsigned i = 0; i < sizeof(table) / sizeof(table[0]); i++) {
            unsigned e = (unsigned)strlen(table[i].ext);
            if (len >= e && memcmp(path + len - e, table[i].ext, e) == 0) return table[i].type;
        }
        return "application/octet-stream";
    }

    // Rejects "..", backslashes and absolute paths outright rather than trying
    // to normalise them. A companion server has no legitimate use for any of
    // them, and refusing is easier to get right than canonicalising.
    bool path_is_safe(const char* p, unsigned n) {
        if (n == 0 || p[0] != '/') return false;
        for (unsigned i = 0; i < n; i++) {
            if (p[i] == '\\') return false;
            if (p[i] == '.' && i + 1 < n && p[i + 1] == '.') return false;
        }
        return true;
    }

    // Reads a bare JSON number for "key". Good enough for the two flat objects
    // this server accepts, and it cannot run off the end of the body.
    bool json_number(const char* body, unsigned len, const char* key, float* out) {
        char pattern[32];
        int pn = snprintf(pattern, sizeof(pattern), "\"%s\"", key);
        if (pn <= 0) return false;

        for (unsigned i = 0; i + (unsigned)pn <= len; i++) {
            if (memcmp(body + i, pattern, (unsigned)pn) != 0) continue;
            unsigned j = i + (unsigned)pn;
            while (j < len && (body[j] == ' ' || body[j] == ':')) j++;

            char num[32];
            unsigned k = 0;
            while (j < len && k + 1 < sizeof(num) &&
                   ((body[j] >= '0' && body[j] <= '9') || body[j] == '-' ||
                    body[j] == '+' || body[j] == '.' || body[j] == 'e' || body[j] == 'E')) {
                num[k++] = body[j++];
            }
            if (k == 0) return false;
            num[k] = 0;
            *out = (float)atof(num);
            return true;
        }
        return false;
    }

    bool json_true(const char* body, unsigned len, const char* key) {
        char pattern[32];
        int pn = snprintf(pattern, sizeof(pattern), "\"%s\"", key);
        if (pn <= 0) return false;
        for (unsigned i = 0; i + (unsigned)pn <= len; i++) {
            if (memcmp(body + i, pattern, (unsigned)pn) != 0) continue;
            unsigned j = i + (unsigned)pn;
            while (j < len && (body[j] == ' ' || body[j] == ':')) j++;
            return j + 4 <= len && memcmp(body + j, "true", 4) == 0;
        }
        return false;
    }

    void serve_static(const net::config& cfg, void* ctx, net::write_fn wr,
                      const char* path, unsigned path_len) {
        if (!path_is_safe(path, path_len)) {
            reply(ctx, wr, 403, "text/plain", "forbidden", 9);
            return;
        }

        // "/" means the page itself.
        const char* rel = path;
        unsigned    rel_len = path_len;
        if (rel_len == 1) { rel = "/index.html"; rel_len = 11; }

        char full[512];
        int n = snprintf(full, sizeof(full), "%s%.*s", cfg.web_root, (int)rel_len, rel);
        if (n <= 0 || (unsigned)n >= sizeof(full)) {
            reply(ctx, wr, 404, "text/plain", "not found", 9);
            return;
        }

        FILE* fp = fopen(full, "rb");
        if (!fp) {
            reply(ctx, wr, 404, "text/plain", "not found", 9);
            return;
        }

        // Read straight into the response buffer, after the space the header
        // will need. write_response copies it into place; that one extra copy
        // buys a single code path for every reply.
        static char file_buf[k_io_cap - 1024];
        size_t got = fread(file_buf, 1, sizeof(file_buf), fp);
        int    more = fgetc(fp);
        fclose(fp);

        if (more != EOF) {
            reply(ctx, wr, 413, "text/plain", "file too large", 14);
            return;
        }

        reply(ctx, wr, 200, content_type_for(rel, rel_len), file_buf, (unsigned)got);
    }
}

namespace net {

    void serve_one(const config& cfg, void* ctx, read_fn rd, write_fn wr) {
        // Read until the header block is complete or the cap is hit.
        unsigned len = 0;
        request  r;
        parse_result pr = parse_result::need_more;

        while (len < sizeof(g_in)) {
            int got = rd(ctx, g_in + len, (unsigned)sizeof(g_in) - len);
            if (got <= 0) return;              // closed or errored: no reply
            len += (unsigned)got;

            pr = parse_request(g_in, len, &r);
            if (pr != parse_result::need_more) break;
        }

        if (pr != parse_result::ok) {
            reply(ctx, wr, 400, "text/plain", "bad request", 11);
            return;
        }

        if (!pin_ok(cfg, r)) {
            reply(ctx, wr, 401, "text/plain", "pin required", 12);
            return;
        }

        // Pull in whatever body was promised, up to what the buffer holds.
        unsigned body_len = r.content_length;
        if (body_len > sizeof(g_in) - r.header_bytes) {
            reply(ctx, wr, 413, "text/plain", "body too large", 14);
            return;
        }
        while (len < r.header_bytes + body_len) {
            int got = rd(ctx, g_in + len, (unsigned)sizeof(g_in) - len);
            if (got <= 0) return;
            len += (unsigned)got;
        }
        const char* body = g_in + r.header_bytes;

        if (span_eq(r.path, r.path_len, "/api/state") && r.m == method::get) {
            static char json[512];
            unsigned n = cfg.state(json, sizeof(json));
            if (!n) {
                reply(ctx, wr, 503, "text/plain", "no state yet", 12);
                return;
            }
            reply(ctx, wr, 200, "application/json", json, n);
            return;
        }

        if (span_eq(r.path, r.path_len, "/api/teleport") && r.m == method::post) {
            job j;
            j.kind   = job_kind::teleport;
            j.z      = 0.0f;
            j.ground = json_true(body, body_len, "ground");
            if (!json_number(body, body_len, "x", &j.x) ||
                !json_number(body, body_len, "y", &j.y)) {
                reply(ctx, wr, 400, "text/plain", "x and y required", 16);
                return;
            }
            json_number(body, body_len, "z", &j.z);

            if (!jobs().push(j)) {
                reply(ctx, wr, 503, "text/plain", "busy", 4);
                return;
            }
            reply(ctx, wr, 200, "application/json", "{\"ok\":true}", 11);
            return;
        }

        // Anything else under /api/ is a mistake, not a file.
        if (r.path_len >= 5 && memcmp(r.path, "/api/", 5) == 0) {
            reply(ctx, wr, 404, "text/plain", "not found", 9);
            return;
        }

        serve_static(cfg, ctx, wr, r.path, r.path_len);
    }
}
```

- [ ] **Step 5: Run test to verify it passes**

Run:
```bash
clang++ -std=c++17 -I src tests/net_conn_test.cpp src/net/conn.cpp src/net/http.cpp src/net/jobs.cpp -o build/net_conn_test.exe && ./build/net_conn_test.exe
```
Expected: every line `ok`, final line `all passed`, exit code 0.

- [ ] **Step 6: Commit**

```bash
git add src/net/conn.h src/net/conn.cpp tests/net_conn_test.cpp
git commit -m "feat(net): connection routing, static files and the PIN gate"
```

---

### Task 4: Socket listener thread

The only file that touches sockets, and the only one in this plan that cannot be host-tested. Verified by talking to the console.

**Files:**
- Create: `src/net/server.h`
- Create: `src/net/server.cpp`
- Modify: `CMakeLists.txt` — add `SceNet` to the link libraries

**Interfaces:**
- Consumes: `net::serve_one`, `net::config`, `net::read_fn`, `net::write_fn`, `net::state_fn` (Task 3).
- Produces:
  - `bool net::server_start(unsigned short port, const char* web_root, const char* pin, state_fn state)`
  - `void net::server_stop()`
  - `bool net::server_running()`
  - `unsigned short net::server_port()`

- [ ] **Step 1: Add SceNet to the link libraries**

`cmake/OrbisTargets.cmake` only sets `ORBIS_LIBS` when it is not already defined, so the top-level file can pre-set it. Add this to `CMakeLists.txt` immediately **before** the `add_orbis_target(...)` call:

```cmake
# Default GHPLUGIN libs plus SceNet: the companion server opens a socket.
# OrbisTargets.cmake only fills ORBIS_LIBS in when it is not already set.
set(ORBIS_LIBS SceLibcInternal kernel SceSysmodule GoldHEN_Hook SceNet)
```

- [ ] **Step 2: Write the header**

Create `src/net/server.h`:

```cpp
#pragma once

#include "net/conn.h"

// The socket half of the companion server: a listener thread that accepts one
// connection at a time and hands each to net::serve_one.
//
// This is the only file in the feature that touches sockets, and the only one
// that cannot be exercised by a host test. Everything it calls is already
// covered by tests/net_conn_test.cpp.

namespace net {

    // Starts the listener. Safe to call twice; the second call is a no-op while
    // the first is still running. web_root and pin are copied.
    bool server_start(unsigned short port, const char* web_root,
                      const char* pin, state_fn state);

    void           server_stop();
    bool           server_running();
    unsigned short server_port();
}
```

- [ ] **Step 3: Write the implementation**

Create `src/net/server.cpp`:

```cpp
#include "net/server.h"
#include "platform/log.h"

#include <orbis/Net.h>
#include <pthread.h>
#include <string.h>
#include <stdint.h>

// The toolchain declares BSD socket names in sys/socket.h, but libxnet.a is an
// empty 8-byte archive - the names are declared and never provided, so a plain
// socket()/bind() build links clean here and fails to resolve on console. The
// sceNet entry points in orbis/Net.h are the real ones.
//
// orbis/_types/net.h carries OrbisNetId, OrbisNetSockaddr and
// ORBIS_NET_AF_INET / ORBIS_NET_SOCK_STREAM, but stops there: there is no
// sockaddr_in equivalent and no protocol or option constants. Those are defined
// here, with the standard SCE values, and the struct mirrors the 16-byte layout
// of OrbisNetSockaddr (1 + 1 + 14).
#define ORBIS_NET_IPPROTO_TCP   6
#define ORBIS_NET_SOL_SOCKET    0xffff
#define ORBIS_NET_SO_REUSEADDR  0x00000004

typedef struct {
    uint8_t  sin_len;
    uint8_t  sin_family;
    uint16_t sin_port;
    uint32_t sin_addr;
    uint16_t sin_vport;
    char     sin_zero[6];
} orbis_sockaddr_in;

namespace {

    volatile bool  g_running  = false;
    volatile bool  g_stopping = false;
    OrbisNetId     g_listen   = -1;
    pthread_t      g_thread   = 0;
    unsigned short g_port     = 0;

    char        g_root[256];
    char        g_pin[5];
    net::config g_cfg;

    int sock_read(void* ctx, char* buf, unsigned cap) {
        OrbisNetId s = (OrbisNetId)(long)ctx;
        return (int)sceNetRecv(s, buf, cap, 0);
    }

    int sock_write(void* ctx, const char* buf, unsigned len) {
        OrbisNetId s = (OrbisNetId)(long)ctx;
        unsigned sent = 0;
        while (sent < len) {
            int n = (int)sceNetSend(s, buf + sent, len - sent, 0);
            if (n <= 0) return -1;
            sent += (unsigned)n;
        }
        return (int)sent;
    }

    void* listener(void*) {
        platform::klogf("net: listener up on port %u", (unsigned)g_port);

        while (!g_stopping) {
            OrbisNetSockaddr peer;
            OrbisNetSocklen_t peer_len = sizeof(peer);
            OrbisNetId c = sceNetAccept(g_listen, &peer, &peer_len);
            if (c < 0) {
                if (g_stopping) break;
                continue;
            }
            net::serve_one(g_cfg, (void*)(long)c, sock_read, sock_write);
            sceNetSocketClose(c);
        }

        platform::klogf("net: listener down");
        g_running = false;
        return 0;
    }
}

namespace net {

    bool server_start(unsigned short port, const char* web_root,
                      const char* pin, state_fn state) {
        if (g_running) return true;

        g_stopping = false;
        g_port     = port;

        snprintf(g_root, sizeof(g_root), "%s", web_root);
        memcpy(g_pin, pin, 4);
        g_pin[4] = 0;

        g_cfg.web_root = g_root;
        memcpy(g_cfg.pin, g_pin, 5);
        g_cfg.state = state;

        sceNetInit();

        g_listen = sceNetSocket("insulin", ORBIS_NET_AF_INET,
                                ORBIS_NET_SOCK_STREAM, ORBIS_NET_IPPROTO_TCP);
        if (g_listen < 0) {
            platform::klogf("net: socket failed (%d)", (int)g_listen);
            return false;
        }

        int on = 1;
        sceNetSetsockopt(g_listen, ORBIS_NET_SOL_SOCKET, ORBIS_NET_SO_REUSEADDR,
                         &on, sizeof(on));

        orbis_sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_len    = sizeof(addr);
        addr.sin_family = ORBIS_NET_AF_INET;
        addr.sin_addr   = 0;                           // INADDR_ANY
        addr.sin_port   = sceNetHtons(port);

        if (sceNetBind(g_listen, (OrbisNetSockaddr*)&addr, sizeof(addr)) < 0) {
            platform::klogf("net: bind to %u failed", (unsigned)port);
            sceNetSocketClose(g_listen);
            g_listen = -1;
            return false;
        }
        if (sceNetListen(g_listen, 2) < 0) {
            platform::klogf("net: listen failed");
            sceNetSocketClose(g_listen);
            g_listen = -1;
            return false;
        }

        // An explicit stack size: pthread_create with a NULL attr gives a tiny
        // stack on this platform, and a frame that overruns it faults on entry
        // with no useful log. 128 KB is far more than this thread's frames need
        // (its buffers are static, not local).
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setstacksize(&attr, 128 * 1024);

        g_running = true;
        if (pthread_create(&g_thread, &attr, listener, 0) != 0) {
            platform::klogf("net: thread create failed");
            g_running = false;
            sceNetSocketClose(g_listen);
            g_listen = -1;
            pthread_attr_destroy(&attr);
            return false;
        }
        pthread_attr_destroy(&attr);

        platform::logf("net", "server started on port %u", (unsigned)port);
        return true;
    }

    void server_stop() {
        if (!g_running) return;
        g_stopping = true;

        // Abort unblocks the accept() the listener is parked in; without it the
        // thread would sit there until the next connection arrived.
        if (g_listen >= 0) {
            sceNetSocketAbort(g_listen, 0);
            sceNetSocketClose(g_listen);
            g_listen = -1;
        }
        pthread_join(g_thread, 0);
        g_running = false;
        platform::logf("net", "server stopped");
    }

    bool           server_running() { return g_running; }
    unsigned short server_port()    { return g_port; }
}
```

- [ ] **Step 4: Verify it compiles for the console**

Run:
```bash
wsl.exe bash -lc 'export OO_PS4_TOOLCHAIN=/home/bbc/OpenOrbis-PS4-Toolchain; \
  export PATH="$OO_PS4_TOOLCHAIN/bin/linux:$PATH"; \
  cd /mnt/e/Projects/PS4/InsulinEngine/InsulinGTAV && cmake --build build-wsl'
```
Expected: builds clean. If a `sceNet*` symbol or an `ORBIS_NET_*` constant does not resolve, check its spelling against `$OO_PS4_TOOLCHAIN/include/orbis/Net.h` and fix it there — that header is the authority, not this plan.

- [ ] **Step 5: Commit**

```bash
git add src/net/server.h src/net/server.cpp CMakeLists.txt
git commit -m "feat(net): sceNet listener thread for the companion server"
```

---

### Task 5: Companion submenu, wiring, and the first console milestone

The switch, the visible status, and the drain that turns queued jobs into engine work. This is the step that proves the thread split on real hardware.

**Files:**
- Create: `src/menu/base/submenus/companion.h`
- Create: `src/menu/base/submenus/companion.cpp`
- Modify: `src/menu/menu.cpp` — register the submenu next to the others
- Create: `assets/web/index.html` — deployed to `/data/GoldHEN/insulin/web/index.html`

**Interfaces:**
- Consumes: `net::server_start`, `net::server_stop`, `net::server_running`, `net::server_port` (Task 4); `net::jobs()`, `net::job`, `net::job_kind` (Task 2).
- Produces:
  - `class companion_menu : public menu::submenu::submenu` with `load()`, `feature_update()`, `static companion_menu* get()`
  - `unsigned companion_state_json(char* out, unsigned cap)` — the `state_fn` handed to the server; returns 0 until the game thread has published a snapshot

- [ ] **Step 1: Write the submenu header**

Create `src/menu/base/submenus/companion.h`:

```cpp
#pragma once

#include "menu/base/submenu.h"

// The companion server's switch and status, plus the per-frame drain that turns
// queued HTTP work into engine work.
//
// Boot rules this file obeys, both of them load-bearing:
//   - load() only sets state. It must never call a native or start the server,
//     because build() can run while the game is still on its loading screen.
//   - feature_update() is where everything happens; menu::tick already gates it
//     on game::player_valid().

class companion_menu : public menu::submenu::submenu {
public:
    void load() override;
    void feature_update() override;

    static companion_menu* get();
};

// No update() override: the status line is a break option that rewrites its own
// name through add_update, so there is nothing left for the submenu to do while
// it is open. Declaring update() here without defining it would be a link error.

// Handed to the server as its state provider. Runs on the HTTP thread and reads
// only a snapshot the game thread published - it never touches the engine.
unsigned companion_state_json(char* out, unsigned cap);
```

- [ ] **Step 2: Write the submenu**

Create `src/menu/base/submenus/companion.cpp`:

```cpp
#include "menu/base/submenus/companion.h"
#include "menu/base/submenus/misc.h"
// submenu.h deliberately pulls in only options/option.h, so each submenu
// includes the concrete option types it actually uses.
#include "menu/base/options/toggle.h"
#include "menu/base/options/break.h"
#include "net/server.h"
#include "net/jobs.h"
#include "game/player_valid.h"
#include "platform/log.h"

#include <stdio.h>
#include <string.h>

namespace {

    bool     g_enabled = false;      // what the toggle says
    bool     g_started = false;      // what the server actually is
    char     g_pin[5]  = "0000";
    unsigned g_port    = 8080;

    // The snapshot the HTTP thread reads. Written by the game thread once per
    // frame, read by the HTTP thread at any moment. Torn reads are acceptable
    // here - the worst case is a marker one frame stale - and the alternative
    // (a lock on the render path) is not worth it.
    struct snapshot {
        volatile bool  valid;
        volatile float x, y, z, heading;
    };
    snapshot g_snap = { false, 0.0f, 0.0f, 0.0f, 0.0f };

    void make_pin() {
        // No RNG dependency: the frame counter at enable time is unpredictable
        // enough for the job, which is stopping accidents, not attackers.
        unsigned seed = (unsigned)(unsigned long long)&g_snap;
        for (int i = 0; i < 4; i++) {
            seed = seed * 1103515245u + 12345u;
            g_pin[i] = (char)('0' + (seed >> 16) % 10);
        }
        g_pin[4] = 0;
    }
}

unsigned companion_state_json(char* out, unsigned cap) {
    if (!g_snap.valid) return 0;
    int n = snprintf(out, cap,
                     "{\"x\":%.2f,\"y\":%.2f,\"z\":%.2f,\"heading\":%.1f}",
                     g_snap.x, g_snap.y, g_snap.z, g_snap.heading);
    return (n < 0 || (unsigned)n >= cap) ? 0u : (unsigned)n;
}

void companion_menu::load() {
    set_name("Companion Server");
    set_parent<misc_menu>();

    // State only. Starting the server here would run during build(), which can
    // happen while the game is still loading.
    add_option(toggle_option("Enable Server")
        .add_toggle(g_enabled)
        .add_tooltip("Serves a page on this console over the local network. "
                     "Open http://<console ip>:8080 and enter the PIN below.")
        .add_savable(get_submenu_name_stack()));

    // The status line. There is no set_status on the submenu base; a break
    // option that rewrites its own name is how this codebase shows live text
    // (see the number options in misc_panels.cpp for the same add_update
    // pattern). The lambda captures nothing, which keeps it far inside
    // stl::function's 64-byte cap.
    add_option(break_option("Stopped")
        .add_update([](break_option* o) {
            char line[96];
            if (net::server_running()) {
                snprintf(line, sizeof(line), "Running on port %u - PIN %s",
                         (unsigned)net::server_port(), g_pin);
            } else {
                snprintf(line, sizeof(line), "Stopped");
            }
            o->set_name(line);
        }));
}

void companion_menu::feature_update() {
    // Start and stop follow the toggle, but only from here - never from load().
    if (g_enabled && !g_started) {
        make_pin();
        if (net::server_start((unsigned short)g_port,
                              "/data/GoldHEN/insulin/web",
                              g_pin, companion_state_json)) {
            g_started = true;
            char msg[96];
            snprintf(msg, sizeof(msg), "Companion server on :%u, PIN %s", g_port, g_pin);
            platform::notify(msg);
        } else {
            g_enabled = false;         // do not retry every frame
            platform::notify("Companion server failed to start");
        }
    } else if (!g_enabled && g_started) {
        net::server_stop();
        g_started = false;
    }

    if (!g_started) return;

    // Publish the snapshot the HTTP thread serves. Pure memory reads through
    // the same chain player_valid() uses; no native is involved.
    if (game::player_valid()) {
        const float* m = game::local_player_matrix();
        if (m) {
            g_snap.x = m[12];
            g_snap.y = m[13];
            g_snap.z = m[14];
            g_snap.heading = game::local_player_heading();
            g_snap.valid = true;
        }
    }

    // Drain whatever the HTTP thread queued. Natives are safe here.
    net::job j;
    while (net::jobs().pop(&j)) {
        if (j.kind == net::job_kind::teleport) {
            platform::logf("net", "teleport job %.1f %.1f ground=%d",
                           j.x, j.y, (int)j.ground);
            // Wired to the real teleport in the next plan; logging it here keeps
            // this task's deliverable to "the queue drains on the game thread".
        }
    }
}

companion_menu* companion_menu::get() {
    static companion_menu instance;
    return &instance;
}
```

- [ ] **Step 3: Add the position helpers the submenu needs**

`src/game/player_valid.h` currently exposes only `player_valid()`; it walks the pointer chain inline and throws the ped away. Add the ped accessor and the two readers, and let `player_valid()` use the accessor so the chain exists once.

Append inside `namespace game`, and add `#include <math.h>` at the top of the file:

```cpp
    // The local player CPed*, or 0. Pure memory reads - the same chain
    // player_valid() checks, kept in one place.
    inline uintptr_t local_player_ped() {
        uintptr_t base = rage::invoker::g_eboot_base;
        if (!base) return 0;
        uintptr_t factory = *(uintptr_t*)(base + RVA_PED_FACTORY);
        if (!factory) return 0;
        return *(uintptr_t*)(factory + FACTORY_LOCAL_PED_OFF);
    }

    // Matrix34 on CEntity: +0x60 right row, +0x70 forward row, +0x90 position
    // row (RE catalog section 7, proven from GET_ENTITY_HEADING and the thin
    // accessor natives). As floats from +0x60 that is m[0..2], m[4..6] and
    // m[12..14].
    //
    // The indirection matters: GET_ENTITY_HEADING takes the matrix from the
    // entity at +0x14B0 instead when the ped carries config flag bit 0x40 at
    // +0x13CB (InVehicle). Without this the marker freezes at the point you got
    // into the car, which reads exactly like a broken map.
    inline const float* local_player_matrix() {
        uintptr_t ped = local_player_ped();
        if (!ped) return 0;

        uintptr_t src = ped;
        if (*(const volatile unsigned char*)(ped + 0x13CB) & 0x40) {
            uintptr_t veh = *(const volatile uintptr_t*)(ped + 0x14B0);
            if (veh) src = veh;
        }
        return (const float*)(src + 0x60);
    }

    // Degrees, matching GET_ENTITY_HEADING: atan2 of the forward row.
    inline float local_player_heading() {
        const float* m = local_player_matrix();
        if (!m) return 0.0f;
        return atan2f(-m[4], m[5]) * 57.2957795f;
    }
```

Then rewrite the body of `player_valid()` to `return local_player_ped() != 0;`, leaving its comment block intact.

- [ ] **Step 4: Register the submenu**

In `src/menu/menu.cpp`, next to the existing registrations (the block around `misc_camera_menu`), add:

```cpp
        companion_menu::get()->load();
        menu::submenu::handler::add_submenu(companion_menu::get());
```

and add `#include "menu/base/submenus/companion.h"` with the other submenu includes.

- [ ] **Step 5: Write the page**

Create `assets/web/index.html`:

```html
<!doctype html>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Insulin Companion</title>
<style>
  body { font: 16px system-ui, sans-serif; margin: 0; padding: 24px;
         background: #14161a; color: #e8e8e8; }
  h1 { font-size: 18px; margin: 0 0 16px; }
  .card { background: #1e2128; border-radius: 10px; padding: 16px; max-width: 420px; }
  .row { display: flex; justify-content: space-between; padding: 6px 0; }
  .k { color: #8b94a3; }
  .v { font-variant-numeric: tabular-nums; }
  input { font: inherit; padding: 8px; border-radius: 6px; border: 1px solid #333;
          background: #14161a; color: inherit; width: 6em; }
</style>
<h1>Insulin Companion</h1>
<div class="card">
  <div class="row"><span class="k">PIN</span><input id="pin" maxlength="4" inputmode="numeric"></div>
  <div class="row"><span class="k">X</span><span class="v" id="x">-</span></div>
  <div class="row"><span class="k">Y</span><span class="v" id="y">-</span></div>
  <div class="row"><span class="k">Z</span><span class="v" id="z">-</span></div>
  <div class="row"><span class="k">Heading</span><span class="v" id="h">-</span></div>
  <div class="row"><span class="k">Status</span><span class="v" id="s">waiting</span></div>
</div>
<script>
  const pin = document.getElementById('pin');
  pin.value = new URLSearchParams(location.search).get('pin') || '';

  async function poll() {
    try {
      const r = await fetch('/api/state', { headers: { 'X-Insulin-Pin': pin.value } });
      if (r.status === 401) { document.getElementById('s').textContent = 'wrong PIN'; return; }
      if (!r.ok) { document.getElementById('s').textContent = 'no state yet'; return; }
      const j = await r.json();
      for (const [id, key] of [['x','x'],['y','y'],['z','z'],['h','heading']]) {
        document.getElementById(id).textContent = j[key].toFixed(2);
      }
      document.getElementById('s').textContent = 'connected';
    } catch (e) {
      document.getElementById('s').textContent = 'offline';
    }
  }
  setInterval(poll, 500);
  poll();
</script>
```

- [ ] **Step 6: Bump the build tag**

In `src/platform/build_tag.h`, change the tag string to `companion-1`. `__TIME__` alone is unreliable — it only changes for the translation unit that gets recompiled, so it can stay stale while everything else changed. The startup log line is how you prove which build is running.

- [ ] **Step 7: Build, deploy and verify on the console**

Build:
```bash
wsl.exe bash -lc 'export OO_PS4_TOOLCHAIN=/home/bbc/OpenOrbis-PS4-Toolchain; \
  export PATH="$OO_PS4_TOOLCHAIN/bin/linux:$PATH"; \
  cd /mnt/e/Projects/PS4/InsulinEngine/InsulinGTAV && cmake --build build-wsl'
```

Deploy the plugin and the page:
```python
import ftplib, io
f = ftplib.FTP(); f.connect('10.10.10.236', 2121, timeout=20); f.login()
prx = open(r'E:\Projects\PS4\InsulinEngine\InsulinGTAV\build-wsl\InsulinGTAV.prx','rb').read()
f.storbinary('STOR /data/GoldHEN/plugins/InsulinGTAV.prx', io.BytesIO(prx))
try: f.mkd('/data/GoldHEN/insulin')
except Exception: pass
try: f.mkd('/data/GoldHEN/insulin/web')
except Exception: pass
page = open(r'E:\Projects\PS4\InsulinEngine\InsulinGTAV\assets\web\index.html','rb').read()
f.storbinary('STOR /data/GoldHEN/insulin/web/index.html', io.BytesIO(page))
f.quit()
```

Then, with the game restarted:

1. Watch the kernel log in a second terminal: `nc 10.10.10.236 3232`. Confirm the startup line shows `BUILD=companion-1`.
2. Open the menu, go to Misc → Companion Server, switch it on. The on-screen notification names the port and PIN, and the kernel log shows `IGV net: listener up on port 8080`.
3. From the PC: `curl -i "http://10.10.10.236:8080/?pin=<PIN>"` returns `200` and the page.
4. `curl -i "http://10.10.10.236:8080/api/state?pin=<PIN>"` returns coordinates that change when you move.
5. `curl -i "http://10.10.10.236:8080/api/state"` (no PIN) returns `401`.
6. Open `http://10.10.10.236:8080/?pin=<PIN>` on a phone; the numbers update twice a second.
7. Leave the page polling for at least 30 minutes while playing. Nothing from `IGV` should appear in the kernel log beyond the startup lines. **This is the real test of this task** — the thread split is what could take the game down, and it fails by crashing, not by returning an error.
8. Switch the toggle off. The log shows `IGV net: listener down`, and `curl` refuses to connect.

If the game closes itself at any point, do not redeploy and retry: a crash that disappears on a rebuild with no code change is timing-dependent, which means a boot-order rule is being broken. Map the crash RIP to an RVA (`RVA = RIP - base`, base is in the startup log line) and look it up in `eboot_named.i64`.

- [ ] **Step 8: Commit**

```bash
git add src/menu/base/submenus/companion.h src/menu/base/submenus/companion.cpp \
        src/menu/menu.cpp src/game/player_valid.h src/platform/build_tag.h \
        assets/web/index.html
git commit -m "feat(menu): companion server submenu, live state page, job drain"
```

---

## What this plan deliberately leaves out

- **The map and click-to-teleport.** The drain logs teleport jobs rather than performing them; wiring them to the existing teleport path needs map tiles, and tiles need the PS4 RPF reader. That is the next plan.
- **Texture swapping and the catalogue.** Spec stage 4, its own plan, and it depends on the same RPF reader.
- **Uploads.** `PUT /api/upload/<id>` is specified but unused until textures land, so it is not built here.
