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
