#include "game/teleport_actions.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/natives_hash.h"

namespace {

    void act_fade_out()  { native::do_screen_fade_out(400); }
    bool act_faded_out() { return native::is_screen_faded_out(); }
    void act_fade_in()   { native::do_screen_fade_in(400); }

    void act_move(float x, float y, float z) {
        Entity e = game::teleport_subject();
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
            act_move, act_stream, act_ground_z
        };
        return a;
    }
}
