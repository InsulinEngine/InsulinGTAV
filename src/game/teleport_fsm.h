#pragma once

// Frame-driven teleport: fade out, walk the subject down a ladder of altitudes
// until the ground answers, put them on it, fade back. It spans frames, so it
// is a state machine rather than a function, and it is driven one step per call
// from the game thread.
//
// Every engine call is injected. That keeps this file free of natives, PS4
// headers and STL so it compiles for the host and is unit-tested on the PC
// (tests/teleport_fsm_test.cpp) - the same split net::serve_one uses for its
// byte transport, and for the same reason: the ordering and the give-up paths
// are the parts with real edge cases, and they are untestable on console.
//
// TWO INVARIANTS. Both have their own tests. Do not break either.
//
//   1. EVERY PHASE HAS A BOUNDED EXIT. No phase may wait on an engine signal
//      without a frame budget that ends the flight regardless of what the
//      engine says. A phase that can wait forever does not merely stall one
//      teleport: busy() stays true, so the companion drain never pops another
//      job, the sixteen-entry ring fills, and every later teleport is refused
//      for the rest of the session - from a black screen, with the fade never
//      handed back. This bit us in fading_out, where IS_SCREEN_FADED_OUT never
//      reads true if anything else (a mission script, a wasted/busted screen,
//      an interior transition) issues its own DO_SCREEN_FADE_IN inside our
//      400ms window.
//
//   2. EVERY PATH THAT REACHES idle PASSES THROUGH finish(), which calls
//      freeze(false) before fade_in(). A player left frozen in mid-air hangs
//      there permanently, which is strictly worse than the fall the freeze
//      exists to prevent. finish() is the only place m_phase becomes idle;
//      keep it that way.

namespace game::tp {

    // --- the ladder ---------------------------------------------------------
    //
    // GET_GROUND_Z_FOR_3D_COORD cannot answer for a column the world has not
    // streamed, and the world streams around ENTITIES. Parking the subject at
    // 1000m and waiting was the previous design; it always gave up, because at
    // 1000m the collision that loads is the empty air around the subject while
    // the ground a kilometre below is never asked for at all. Waiting longer
    // cannot fix that - nothing is ever going to stream ground collision for a
    // point the entity is nowhere near.
    //
    // So the subject is walked DOWN instead, and the ground is queried at each
    // step. This is robust to both explanations of the old failure at once: it
    // fixes the streaming (the collision request now follows the subject down
    // to where the ground actually is) and it fixes the probe distance (each
    // query is fired from progressively closer above the ground), and we do
    // not need to know which of the two was the culprit.
    //
    // Descending rather than ascending, and taking the FIRST hit: from above,
    // the first surface found is the top one, which is the one you want on
    // Chiliad. Climbing from the bottom would query from inside the mountain
    // and answer with whatever is under the rock.
    //
    // Bottom-weighted, because that is where the map is. Every landmark in the
    // menu's own table sits under 40m (LSIA 20.2, LS Customs 39.0, Franklins
    // house 31.1, Trevors trailer 32.2, Military Base 32.8); the tallest roof
    // anyone teleports onto is Maze Bank at 326.2; the single highest ground
    // on the map is Mount Chiliad at 797.9. Three rungs cover everything above
    // 250m and the other five crowd into the band where the answer nearly
    // always is, so the common teleport converges in about a second and a half
    // while Chiliad still works on the first pass.
    const float rungs[] = {
        825.0f,   // clears Mount Chiliad's summit (797.9)
        500.0f,   // empty band - no terrain and no structure lives up here
        325.0f,   // Maze Bank roof (326.2), the tallest thing in the table
        225.0f,
        150.0f,   // ~100m over typical ground: what the menu's own warp uses
        100.0f,
        65.0f,
        35.0f     // typical ground is a few metres under this rung
    };
    const int rung_count = (int)(sizeof(rungs) / sizeof(rungs[0]));

    // Frames spent at each rung before dropping to the next. The ground query
    // runs on EVERY one of them, so this is not a fixed wait before asking -
    // it is how long the streamer is given to answer before we move on. Ten
    // frames is a third of a second at the console's 30fps, and the lower
    // rungs all request roughly the same collision column, so by the time the
    // subject reaches 100m that column has been requested for over a second
    // without interruption.
    const int rung_frames = 10;

    // The bottom rung is the one where the ground is genuinely within reach,
    // so it gets a full second rather than a third of one before we conclude
    // that nothing is ever going to load.
    const int last_rung_frames = 30;

    // Worst case for the whole sweep is 7*10 + 30 = 100 frames, deliberately
    // MORE than the 90 frames (30 streaming + 60 resolving) the build that
    // failed acceptance spent. A redesign that quietly gave the engine less
    // time than the known-failing one would make the next run unreadable: a
    // failure would not distinguish "wrong approach" from "not enough time".

    // Upper bound on waiting for the screen to go black. Seven times the 400ms
    // fade. See invariant 1: without this the machine waits forever.
    const int fade_frames = 90;

    // ground:false asks for an exact z, so there is nothing to resolve - but
    // the subject is still held there for a second first, so collision has a
    // chance to load under it before it is released.
    const int hold_frames = 30;

    // Where an exhausted ladder leaves the player. NOT the bottom rung: if no
    // rung answered then no collision ever loaded for that column, and 35m is
    // below most of the map's terrain, so releasing there can leave the player
    // inside a hill. 150m is above essentially all playable ground and is the
    // same order of drop as the menu's Waypoint button, which has arrived at
    // blip z + 100m for as long as the menu has existed and is the behaviour
    // this project's users already accept. (The old give-up left them at
    // 1000m, which is not the same thing at all and is not survivable.)
    const float fallback_z = 150.0f;

    // How far above the resolved ground to place the player, so they settle
    // onto it rather than starting inside it.
    const float ground_clearance = 1.0f;

    enum class phase : unsigned char {
        idle,          // nothing in flight
        fading_out,    // fade requested, waiting for the screen to go black
        sweeping,      // walking down the ladder, asking for ground at each rung
        holding        // ground:false - parked on the requested z while it loads
    };

    // Latched at finish(), cleared at submit(). The completion log reads this
    // instead of inferring the outcome from the player's position, which it
    // used to do and which the sweep makes impossible anyway: a give-up and a
    // successful low landing now end at similar altitudes.
    enum class outcome : unsigned char {
        none,          // nothing has run yet
        landed,        // a rung answered; the player is standing on the ground
        placed,        // ground:false - the requested z was used as given
        no_ground,     // ladder exhausted; left at fallback_z to fall
        no_fade        // the screen never went black; abandoned without moving
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
        // not available yet, which is normal until the subject is low enough
        // for the column to have streamed in. This is the machine's readiness
        // signal, and the only honest one available: it answers exactly the
        // question being asked. There used to be a collision_ready() action
        // wrapping HAS_COLLISION_LOADED_AROUND_ENTITY here; it was removed,
        // because around an entity at 1000m it reports "loaded" for the empty
        // air the entity is sitting in and says nothing about the ground.
        bool (*ground_z)(float x, float y, float probe, float* out);
        // Holds the subject at the rung it was moved to. Without it the
        // subject falls away from the point being probed between queries, and
        // the sweep's altitudes stop meaning anything. Set once per flight, at
        // the first move, and cleared exactly once in finish().
        void (*freeze)(bool on);
    };

    class machine {
    public:
        // Takes the request, or refuses it while one is already in flight. The
        // job ring holds sixteen entries, so a phone with an impatient finger
        // can hand over sixteen clicks; running them concurrently would fight
        // over the same player.
        bool submit(const request& r) {
            if (m_phase != phase::idle) return false;
            m_req     = r;
            m_phase   = phase::fading_out;
            m_frames  = 0;
            m_rung    = 0;
            m_asked   = false;
            m_outcome = outcome::none;
            m_hit     = -1;
            m_ground  = 0.0f;
            return true;
        }

        void tick(const actions& a) {
            switch (m_phase) {
            case phase::idle:
                return;

            case phase::fading_out: {
                // Request the fade once, then wait for it to finish - but not
                // forever (invariant 1). Nothing has been moved or frozen at
                // this point, so giving up here costs the player nothing but
                // the fade they already saw.
                if (!m_asked) { a.fade_out(); m_asked = true; }
                if (!a.faded_out()) {
                    if (++m_frames >= fade_frames) finish(a, outcome::no_fade);
                    return;
                }
                // Place first, then hold: freezing before the move would
                // freeze the subject at its old position instead of the rung.
                const float top = m_req.ground ? rungs[0] : m_req.z;
                a.move(m_req.x, m_req.y, top);
                a.freeze(true);
                a.stream(m_req.x, m_req.y, top);
                m_rung   = 0;
                m_frames = 0;
                m_phase  = m_req.ground ? phase::sweeping : phase::holding;
                return;
            }

            case phase::holding:
                // Ask every frame: request_collision_at_coord is a hint the
                // streamer is free to drop, so one call is not a guarantee.
                a.stream(m_req.x, m_req.y, m_req.z);
                if (++m_frames >= hold_frames) land(a, m_req.z, outcome::placed);
                return;

            case phase::sweeping: {
                const float at = rungs[m_rung];
                a.stream(m_req.x, m_req.y, at);

                float z = 0.0f;
                if (a.ground_z(m_req.x, m_req.y, at, &z)) {
                    m_hit    = m_rung;
                    m_ground = z;
                    land(a, z + ground_clearance, outcome::landed);
                    return;
                }

                const bool last  = (m_rung + 1 >= rung_count);
                const int  dwell = last ? last_rung_frames : rung_frames;
                if (++m_frames < dwell) return;

                if (last) {
                    // Ladder exhausted (invariant 1: this is the sweep's
                    // bound). Lift the player back to a survivable height
                    // before releasing them - see fallback_z.
                    land(a, fallback_z, outcome::no_ground);
                    return;
                }

                m_rung++;
                m_frames = 0;
                const float next = rungs[m_rung];
                a.move(m_req.x, m_req.y, next);
                a.stream(m_req.x, m_req.y, next);
                return;
            }
            }
        }

        phase   state()  const { return m_phase; }
        bool    busy()   const { return m_phase != phase::idle; }

        // Read once the machine goes idle. Latched until the next submit().
        outcome result() const { return m_outcome; }
        bool    landed() const { return m_outcome == outcome::landed; }
        // Which rung answered, -1 if none did. rungs[rung()] is the altitude
        // the winning query was fired from.
        int     rung()   const { return m_hit; }
        // What that query said the ground height was.
        float   ground_height() const { return m_ground; }

    private:
        // Put the subject down, then hand the screen back.
        void land(const actions& a, float z, outcome o) {
            a.move(m_req.x, m_req.y, z);
            finish(a, o);
        }

        // The one and only way out of a flight (invariant 2). Every exit -
        // landed, placed, exhausted ladder, fade that never went black -
        // comes through here, so the freeze set at the first move is always
        // cleared and the screen is always handed back.
        void finish(const actions& a, outcome o) {
            m_outcome = o;
            a.freeze(false);
            a.fade_in();
            m_phase = phase::idle;
        }

        request m_req     = { 0.0f, 0.0f, 0.0f, false };
        phase   m_phase   = phase::idle;
        outcome m_outcome = outcome::none;
        int     m_frames  = 0;
        int     m_rung    = 0;
        int     m_hit     = -1;
        float   m_ground  = 0.0f;
        bool    m_asked   = false;
    };
}
