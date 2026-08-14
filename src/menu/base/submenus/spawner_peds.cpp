#include "menu/base/submenus/spawner_peds.h"
#include "menu/base/submenus/spawner.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/number.h"
#include "menu/base/options/scroll.h"
#include "menu/base/options/break.h"
#include "menu/base/util/notify.h"
#include "menu/base/util/control.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/missing_natives.h"
#include "rage/invoker/natives_hash.h"
#include "game/ped_list.h"

namespace {
    int g_index[8] = {};
    scroll_struct<int> g_lists[8][32];
    bool g_armed = false;

    void spawn_ped(uint32_t hash) {
        if (!native::is_model_in_cdimage(hash) || !native::is_model_valid(hash)) {
            menu::notify::stacked("Spawner", "Not available on this build");
            return;
        }
        menu::control::request_model(hash, [](uint32_t loaded) {
            Ped me = native::get_player_ped(-1);
            math::vector3<float> pos = native::get_offset_from_entity_in_world_coords(me, 0.f, 3.f, 0.f);
            Ped p = native::create_ped(4, loaded, pos.x, pos.y, pos.z,
                                       native::get_entity_heading(me), false, false);
            native::set_entity_as_mission_entity(p, true, true);
            if (g_armed)
                native::give_weapon_to_ped(p, native::get_hash_key("WEAPON_CARBINERIFLE"), 9999, false, true);
            native::set_model_as_no_longer_needed(loaded);
            menu::notify::stacked("Spawner", "Spawned");
        });
    }
}

void spawner_peds_menu::load() {
    set_name("Peds");
    set_parent<spawner_menu>();

    add_option(toggle_option("Give Weapon")
        .add_toggle(g_armed)
        .add_tooltip("Hands the spawned ped a rifle")
        .add_savable(get_submenu_name_stack()));

    add_option(break_option("Categories").ref());

    for (int g = 0; g < ped_category_count && g < 8; g++) {
        const ped_category& cat = ped_categories[g];

        int n = cat.count < 32 ? cat.count : 32;
        for (int i = 0; i < n; i++) {
            g_lists[g][i].m_name.set(cat.items[i].name);
            g_lists[g][i].m_result = i;
        }

        int gi = g;   // tiny capture: stl::function caps captures at 64 bytes
        add_option(scroll_option<int>(SCROLLSELECT, cat.name)
            .add_scroll(g_index[g], 0, n - 1, g_lists[g])
            .add_click([gi] {
                const ped_category& sel = ped_categories[gi];
                int i = g_index[gi];
                if (i >= 0 && i < sel.count)
                    spawn_ped(sel.items[i].hash);
            }));
    }
}

spawner_peds_menu* spawner_peds_menu::get() {
    static spawner_peds_menu instance;
    return &instance;
}
