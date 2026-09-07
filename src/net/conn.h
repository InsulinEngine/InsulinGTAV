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
    // Writes a JSON object into out, returns its length (0 on failure). The
    // return value must be the number of bytes actually written, and never
    // more than cap - unlike snprintf, which reports what it *would* have
    // written. The caller still clamps defensively, but an implementation
    // must not rely on that.
    typedef unsigned (*state_fn)(char* out, unsigned cap);

    struct config {
        const char* web_root;   // e.g. "/data/GoldHEN/insulin/web"
        char        pin[5];     // 4 digits + nul
        state_fn    state;      // published by the game thread; never null
    };

    // Serves exactly one request, then returns. The caller closes the socket.
    //
    // Not reentrant: the request buffer, response buffer and static file
    // buffer are shared statics with no synchronisation, traded for keeping
    // a 256 KB frame off the server thread's stack. Call this from one
    // thread at a time, one connection at a time - a thread-per-connection
    // accept loop would corrupt responses across clients.
    void serve_one(const config& cfg, void* ctx, read_fn rd, write_fn wr);
}
