#include "menu/base/submenus/player_model.h"
#include "menu/base/submenus/player_appearance.h"
#include "menu/base/options/button.h"
#include "menu/base/options/scroll.h"
#include "menu/base/util/control.h"
#include "menu/base/util/notify.h"
#include "rage/invoker/natives.h"
#include "game/ped_list.h"

namespace {
    int g_index[8] = {};
    scroll_struct<int> g_lists[8][32];

    uint32_t g_last_model = 0;   // re-applied after a death, hence the cache
    bool g_was_dead = false;

    void change_model(uint32_t hash) {
        if (!native::is_model_in_cdimage(hash) || !native::is_model_valid(hash)) {
            menu::notify::stacked("Model", "Not available on this build");
            return;
        }
        menu::control::request_model(hash, [](uint32_t loaded) {
            native::set_player_model(native::player_id(), loaded);
            // A fresh model arrives with no outfit; without this you get the white
            // default shape instead of the ped as the game knows it.
            native::set_ped_default_component_variation(native::get_player_ped(-1));
            native::set_model_as_no_longer_needed(loaded);
            g_last_model = loaded;
            menu::notify::stacked("Model", "Changed");
        });
    }
}

void player_model_menu::load() {
    set_name("Model");
    set_parent<player_appearance_menu>();

    add_option(button_option("Refresh Model")
        .add_tooltip("Clears blood and visible damage")
        .add_click([] {
            Ped ped = native::get_player_ped(-1);
            native::reset_ped_visible_damage(ped);
            native::clear_ped_blood_damage(ped);
        }));

    add_option(button_option("Refresh Model Cache")
        .add_tooltip("Stops the model being re-applied when you die")
        .add_click([] { g_last_model = 0; menu::notify::stacked("Model", "Cache cleared"); }));

    add_option(button_option("Input Custom Model")
        .add_tooltip("No on-screen keyboard on this platform yet - use the lists below")
        .add_click([] { menu::notify::stacked("Model", "Keyboard input not available"); }));

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
                    change_model(sel.items[i].hash);
            }));
    }
}

void player_model_menu::feature_update() {
    if (!g_last_model)
        return;

    // Re-apply on the transition back to alive, never while dead: swapping the
    // model mid-respawn leaves a T-posing ped with no control.
    Ped ped = native::get_player_ped(-1);
    bool dead = !ped || native::is_entity_dead(ped, 0);

    if (!dead && g_was_dead && native::get_entity_model(ped) != g_last_model)
        change_model(g_last_model);

    g_was_dead = dead;
}

player_model_menu* player_model_menu::get() {
    static player_model_menu instance;
    return &instance;
}
