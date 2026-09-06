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
