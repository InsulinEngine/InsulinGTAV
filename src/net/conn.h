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
