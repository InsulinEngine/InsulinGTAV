#include "menu/base/submenus/vehicle.h"
#include "menu/base/submenus/main.h"
#include "menu/base/options/button.h"
#include "menu/base/options/break.h"
#include "menu/base/util/control.h"
#include "menu/base/util/notify.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"

// Spawn a vehicle in front of the player: request the model through the control
// manager, then create it in the load callback (the workflow real spawns use).
static void spawn_vehicle(const char* model_name) {
    uint32_t model = native::get_hash_key(model_name);

    menu::control::request_model(model, [model](uint32_t loaded) {
        Ped ped = native::get_player_ped(-1);
        math::vector3<float> pos = native::get_offset_from_entity_in_world_coords(ped, 0.f, 5.f, 0.f);
        float heading = native::get_entity_heading(ped);

        Vehicle veh = native::create_vehicle(loaded, pos.x, pos.y, pos.z, heading, false, false, 0);
        native::set_vehicle_on_ground_properly(veh, 0);
        native::set_model_as_no_longer_needed(model);
        menu::notify::stacked("Vehicle", "Spawned");
    });
}

void vehicle_menu::load() {
    set_name("Vehicle");
    set_parent<main_menu>();

    add_option(break_option("Spawn (in front of you)").ref());

    add_option(button_option("Spawn Adder")
        .add_click([] { spawn_vehicle("adder"); }));
    add_option(button_option("Spawn Insurgent")
        .add_click([] { spawn_vehicle("insurgent"); }));
    add_option(button_option("Spawn Buzzard")
        .add_click([] { spawn_vehicle("buzzard"); }));
    add_option(button_option("Spawn Kuruma")
        .add_click([] { spawn_vehicle("kuruma"); }));
}

vehicle_menu* vehicle_menu::get() {
    static vehicle_menu instance;
    return &instance;
}
