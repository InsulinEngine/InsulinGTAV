#pragma once

// Frame-driven teleport: fade out, move, let the world stream in, resolve the
// ground, fade back. It spans frames, so it is a state machine rather than a
// function, and it is driven one step per call from the game thread.
//
// Every engine call is injected. That keeps this file free of natives, PS4
// headers and STL so it compiles for the host and is unit-tested on the PC
// (tests/teleport_fsm_test.cpp) - the same split net::serve_one uses for its
// byte transport, and for the same reason: the ordering and the give-up paths
// are the parts with real edge cases, and they are untestable on console.

namespace game::tp {

    // Arrive from high above and drop in. The world below a coordinate is not
    // loaded until something asks for it, so ground height is not answerable
    // until the player is already there.
    const float probe_z = 1000.0f;

    // Frames spent waiting for collision to stream in before the first ground
    // query, and frames spent retrying that query before giving up. At 30 fps
    // that is one second of streaming and two seconds of asking.
    const int stream_frames  = 30;
    const int resolve_frames = 60;

    // How far above the resolved ground to place the player, so they settle
    // onto it rather than starting inside it.
    const float ground_clearance = 1.0f;

    enum class phase : unsigned char {
        idle,          // nothing in flight
        fading_out,    // fade requested, waiting for the screen to go black
        streaming,     // at probe altitude, waiting for the world to load
        resolving      // asking for ground height until it answers or we stop
    };

    struct request {
        float x, y, z;
        bool  ground;      // resolve ground height instead of trusting z
    };

    // The engine, injected. Mirrors net::read_fn/write_fn in net/conn.h.
    struct actions {
        void (*fade_out)();
        bool (*faded_out)();
        void (*fade_in)();
        void (*move)(float x, float y, float z);
        void (*stream)(float x, float y, float z);
        // Ground height under (x, y) probed from z. False when the answer is
        // not available yet, which is normal for the first frames after a move.
        bool (*ground_z)(float x, float y, float probe, float* out);
    };

    class machine {
    public:
        // Takes the request, or refuses it while one is already in flight. The
        // job ring holds sixteen entries, so a phone with an impatient finger
        // can hand over sixteen clicks; running them concurrently would fight
        // over the same player.
        bool submit(const request& r) {
            if (m_phase != phase::idle) return false;
            m_req    = r;
            m_phase  = phase::fading_out;
            m_frames = 0;
            m_asked  = false;
            return true;
        }

        void tick(const actions& a) {
            switch (m_phase) {
            case phase::idle:
                return;

            case phase::fading_out:
                // Request the fade once, then wait for it to finish.
                if (!m_asked) { a.fade_out(); m_asked = true; }
                if (!a.faded_out()) return;
                a.move(m_req.x, m_req.y, probe_z);
                a.stream(m_req.x, m_req.y, probe_z);
                m_phase  = phase::streaming;
                m_frames = 0;
                return;

            case phase::streaming:
                if (++m_frames < stream_frames) return;
                m_phase  = phase::resolving;
                m_frames = 0;
                return;

            case phase::resolving: {
                if (!m_req.ground) { land(a, m_req.z); return; }

                float z = 0.0f;
                if (a.ground_z(m_req.x, m_req.y, probe_z, &z)) {
                    land(a, z + ground_clearance);
                    return;
                }
                // Give up rather than hold a black screen forever. The player
                // stays at probe altitude and falls, which is what the existing
                // waypoint teleport does on every jump.
                if (++m_frames >= resolve_frames) release(a);
                return;
            }
            }
        }

        phase state() const { return m_phase; }
        bool  busy()  const { return m_phase != phase::idle; }

    private:
        // Put the player down, then hand the screen back.
        void land(const actions& a, float z) {
            a.move(m_req.x, m_req.y, z);
            release(a);
        }

        // Hand the screen back without moving again: the player is already at
        // probe altitude and falls the rest of the way.
        //
        // Two functions rather than one with a "should I move" float compare.
        // Deciding by `z != probe_z` would read as clever and then silently
        // skip the move for a ground:false request that legitimately asked for
        // z = 1000.
        void release(const actions& a) {
            a.fade_in();
            m_phase = phase::idle;
        }

        request m_req    = { 0.0f, 0.0f, 0.0f, false };
        phase   m_phase  = phase::idle;
        int     m_frames = 0;
        bool    m_asked  = false;
    };
}
