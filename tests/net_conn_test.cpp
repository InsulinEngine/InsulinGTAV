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
#include <stdlib.h>
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

// A provider that misbehaves the way snprintf does when its output does not
// fit: it reports how much it *would* have written, not how much fit in cap.
// serve_one must clamp this itself - it fills every byte of the buffer it
// was actually given (cap, whatever that is) with a known pattern and claims
// a length far past that, so a response body longer than cap would prove the
// clamp is missing rather than merely echoing whatever happened to follow
// the buffer in memory.
static unsigned g_overflow_cap = 0;

static unsigned fake_state_overflow(char* out, unsigned cap) {
    g_overflow_cap = cap;
    for (unsigned i = 0; i < cap; i++) out[i] = (char)('A' + (i % 26));
    return cap + 999;
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

    // A provider that reports a length past its own buffer must not turn
    // into an out-of-bounds read: the response body is clamped to the
    // buffer's actual capacity, never to the provider's over-length claim.
    cfg.state = fake_state_overflow;
    run(cfg, "GET /api/state?pin=4711 HTTP/1.1\r\n\r\n", &f);
    check_true("overflow state is 200", strstr(f.out, "HTTP/1.1 200") == f.out);
    check_true("overflow cap was recorded", g_overflow_cap != 0);
    char want_len[32];
    snprintf(want_len, sizeof(want_len), "Content-Length: %u", g_overflow_cap);
    check_true("overflow content-length clamped to buffer cap", strstr(f.out, want_len) != 0);
    const char* overflow_body = strstr(f.out, "\r\n\r\n");
    check_true("overflow body not longer than buffer cap",
               overflow_body != 0 && strlen(overflow_body + 4) == g_overflow_cap);
    cfg.state = fake_state;

    // /api/teleport pushes exactly one job and does not touch the engine.
    net::jobs().reset();
    run(cfg, "POST /api/teleport?pin=4711 HTTP/1.1\r\nContent-Length: 35\r\n\r\n"
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
    run(cfg, "POST /api/teleport?pin=4711 HTTP/1.1\r\nContent-Length: 35\r\n\r\n"
             "{\"x\":10.5,\"y\":-20.25,\"ground\":true}", &f);
    check_true("full ring is 503", strstr(f.out, "HTTP/1.1 503") == f.out);

    // An unknown API path is 404, not a silent 200.
    run(cfg, "GET /api/nothing?pin=4711 HTTP/1.1\r\n\r\n", &f);
    check_true("unknown api is 404", strstr(f.out, "HTTP/1.1 404") == f.out);

    printf(g_failed ? "\n%d FAILED\n" : "\nall passed\n", g_failed);
    return g_failed != 0;
}
