#include "game/teleport_actions.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/natives_hash.h"

namespace {

    // The subject captured for the life of one teleport. teleport_subject()
    // returns the vehicle when the player is seated, else the ped - but
    // nothing locks player input during the black screen, only the fade and
    // the freeze, so the player can exit the vehicle mid-streaming/resolving
    // and change which entity that function returns. Freezing one entity and
    // later unfreezing a *different* one (re-resolved after the exit) leaves
    // the first stranded, frozen, floating at probe altitude forever - worse
    // than the fall this freeze exists to prevent. Capturing once at
    // act_freeze(true) and reusing that handle everywhere else in the flight
    // keeps freeze/unfreeze paired to the same entity regardless of what the
    // player does in between.
    Entity g_subject = 0;

    void act_fade_out()  { native::do_screen_fade_out(400); }
    bool act_faded_out() { return native::is_screen_faded_out(); }
    void act_fade_in()   { native::do_screen_fade_in(400); }

    void act_move(float x, float y, float z) {
        // The FSM calls move once before freeze(true) (order: move, freeze,
        // stream), so that first call legitimately has nothing captured yet
        // and must resolve live. Every later call in the same flight reuses
        // the captured handle instead.
        Entity e = g_subject ? g_subject : game::teleport_subject();
        if (!e) return;
        native::set_entity_coords_no_offset(e, x, y, z, false, false, false);
    }

    void act_stream(float x, float y, float z) {
        native::request_collision_at_coord(x, y, z);
    }

    bool act_ground_z(float x, float y, float probe, float* out) {
        // A hash native: inert until the command table has been recovered and
        // verified, which takes a few seconds after boot. Reporting false then
        // is exactly right - the machine retries and eventually gives up.
        //
        // It lives in `native`, not `native::hash`: natives.h and
        // natives_hash.h both fill the same namespace, which is why the
        // generator refuses to emit a name that already exists in the other.
        return native::get_ground_z_for_3d_coord(x, y, probe, out, false, false);
    }

    // Direct RVA natives (natives.h:413, natives.h:857) - both work from the
    // first frame, unlike act_ground_z above. Holds the subject at probe
    // altitude while collision streams in: without this it falls away from
    // the point being probed, and the ground query never finds anything
    // under it.
    void act_freeze(bool on) {
        if (on) {
            // The one place g_subject is resolved. Everything else in the
            // flight reuses this handle rather than re-resolving, which is
            // exactly what avoids the vehicle/ped aliasing above.
            g_subject = game::teleport_subject();
            if (g_subject) native::freeze_entity_position(g_subject, true);
            return;
        }
        // Release the captured handle, not whatever teleport_subject() would
        // return now - the player may have exited the vehicle since capture,
        // and re-resolving here is exactly the bug this file exists to avoid.
        if (g_subject) native::freeze_entity_position(g_subject, false);
        g_subject = 0;
    }

    bool act_collision_ready() {
        Entity e = g_subject ? g_subject : game::teleport_subject();
        return e && native::has_collision_loaded_around_entity(e);
    }
}

namespace game {

    Entity teleport_subject() {
        Ped ped = native::get_player_ped(-1);
        if (ped && native::is_ped_in_any_vehicle(ped, false)) {
            Vehicle veh = native::get_vehicle_ped_is_in(ped, false);
            if (veh)
                return veh;
        }
        return ped;
    }

    const tp::actions& live_actions() {
        // Function-local static: .init_array does not run in this plugin, so a
        // namespace-scope object with an initialiser would stay zeroed.
        static tp::actions a = {
            act_fade_out, act_faded_out, act_fade_in,
            act_move, act_stream, act_ground_z,
            act_freeze, act_collision_ready
        };
        return a;
    }
}
