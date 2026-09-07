#include "game/teleport_actions.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/natives_hash.h"
#include "rage/invoker/hash_natives.h"

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
        //
        // Those later calls - one per rung of the descending sweep, plus the
        // final placement - all happen while the subject is FROZEN.
        // set_entity_coords_no_offset teleports an entity regardless of the
        // fixed-physics flag freeze_entity_position sets, which is what makes
        // the sweep possible at all. If a console run ever shows the subject
        // stuck on the top rung with the log reporting no rung answered, this
        // assumption is the first thing to doubt: the fix would be to unfreeze
        // and refreeze around each move (safe within one tick, since no frame
        // is processed in between) rather than to abandon the sweep.
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

    // A direct RVA native (natives.h:413) - works from the first frame,
    // unlike act_ground_z above. Holds the subject at whichever rung of the
    // sweep it was last moved to: without this it falls away between queries
    // and the sweep's altitudes stop meaning anything.
    //
    // There used to be an act_collision_ready() next to this one, wrapping
    // has_collision_loaded_around_entity (natives.h:857). It is gone. It asks
    // whether collision is loaded AROUND THE ENTITY, and at 1000m that is the
    // empty air the entity is sitting in: it reported ready instantly while
    // the ground a kilometre below had never been asked for. The sweep's
    // per-rung dwell plus the ground query itself subsume it, and the query
    // is a strictly better signal because it answers the actual question.
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

    bool ground_native_ready() {
        // Same hash act_ground_z calls through: GET_GROUND_Z_FOR_3D_COORD,
        // natives_hash.h:694. Duplicated deliberately - the generated header
        // gives no way to ask "is this one registered", and a sweep that
        // reports "no ground from any rung" because the table is not up yet
        // is a completely different finding from one that streams badly.
        return rage::hash_natives::usable() &&
               rage::hash_natives::find(0xB1EAADCB692D69CEULL) != nullptr;
    }

    const tp::actions& live_actions() {
        // Function-local static: .init_array does not run in this plugin, so a
        // namespace-scope object with an initialiser would stay zeroed.
        static tp::actions a = {
            act_fade_out, act_faded_out, act_fade_in,
            act_move, act_stream, act_ground_z,
            act_freeze
        };
        return a;
    }
}
