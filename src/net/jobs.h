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
