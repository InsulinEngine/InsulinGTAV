#pragma once

#include "net/conn.h"

// The socket half of the companion server: a listener thread that accepts one
// connection at a time and hands each to net::serve_one.
//
// This is the only file in the feature that touches sockets, and the only one
// that cannot be exercised by a host test. Everything it calls is already
// covered by tests/net_conn_test.cpp.
//
// One connection at a time is a requirement, not a simplification: serve_one
// works out of shared static buffers and is documented as not reentrant.

namespace net {

    // Starts the listener. Safe to call twice; the second call is a no-op while
    // the first is still running. web_root and pin are copied.
    bool server_start(unsigned short port, const char* web_root,
                      const char* pin, state_fn state);

    // Blocks until the listener thread has exited. Call from the game thread;
    // it will not sit longer than the abort takes to unblock the thread.
    void server_stop();

    bool           server_running();
    unsigned short server_port();
}
