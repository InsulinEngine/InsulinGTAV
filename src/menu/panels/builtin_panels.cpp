#include "menu/panels/builtin_panels.h"
#include "menu/base/util/panels.h"
#include "menu/panels/player_panel.h"
#include "menu/panels/vehicle_panel.h"
#include "menu/panels/world_panel.h"
#include "menu/panels/debug_panel.h"

namespace menu::panels {
    // Panels arrive in Tasks 9 and 10; this file owns their registration so
    // menu.cpp - already 300 lines of it - does not absorb four more render
    // callbacks.
    void register_builtin_panels() {
        panel_parent* parent = new panel_parent();
        parent->m_render = true;
        parent->m_id   = "insulin";
        parent->m_name = "Insulin";

        panel_child player{};
        player.m_parent = parent;
        player.m_render = true;
        player.m_id     = "player";
        player.m_name   = "Player";
        player.m_double_sided = true;
        player.m_panel_option_count_left = 8;
        player.m_update = player_panel_update;
        parent->m_children_panels.push_back(player);

        panel_child vehicle{};
        vehicle.m_parent = parent;
        vehicle.m_render = true;
        vehicle.m_id     = "vehicle";
        vehicle.m_name   = "Vehicle";
        vehicle.m_double_sided = true;
        vehicle.m_panel_option_count_left = 5;
        vehicle.m_column = 0;
        vehicle.m_index  = 1;
        vehicle.m_update = vehicle_panel_update;
        parent->m_children_panels.push_back(vehicle);

        panel_child world{};
        world.m_parent = parent;
        world.m_render = true;
        world.m_id     = "world";
        world.m_name   = "World";
        world.m_double_sided = true;
        world.m_panel_option_count_left = 2;
        world.m_column = 1;
        world.m_index  = 0;
        world.m_update = world_panel_update;
        parent->m_children_panels.push_back(world);

        panel_child debug{};
        debug.m_parent = parent;
        debug.m_render = true;
        debug.m_id     = "debug";
        debug.m_name   = "Debug";
        debug.m_double_sided = true;
        debug.m_panel_option_count_left = 5;
        debug.m_column = 1;
        debug.m_index  = 1;
        debug.m_update = debug_panel_update;
        parent->m_children_panels.push_back(debug);

        get_panels().push_back(parent);
    }
}
