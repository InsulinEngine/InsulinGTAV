#include "menu/base/submenus/player_wardrobe_saveload.h"
#include "menu/base/submenus/player_wardrobe.h"
#include "menu/base/options/button.h"
#include "menu/base/util/notify.h"
#include "util/config.h"
#include "rage/invoker/natives.h"

// Ozark stores outfits as named entries the user types in. There is no on-screen
// keyboard on this platform yet, so this saves to one fixed slot in the config -
// the same file the toggles persist to.
namespace {
    Ped self_ped() { return native::get_player_ped(-1); }

    const int k_components[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
    constexpr int COMPONENT_COUNT = (int)(sizeof(k_components) / sizeof(k_components[0]));

    // The click handlers cannot reach get_submenu_name_stack() - it is a member,
    // and these lambdas capture nothing so the config section is built here.
    stl::stack<stl::string> outfit_section() {
        stl::stack<stl::string> s;
        s.push("Outfit");
        return s;
    }
}

void player_wardrobe_saveload_menu::load() {
    set_name("Save and Load");
    set_parent<player_wardrobe_menu>();

    add_option(button_option("Save Outfit")
        .add_tooltip("Stores what you are wearing into the config")
        .add_click([] {
            Ped ped = self_ped();
            if (!ped) return;
            for (int i = 0; i < COMPONENT_COUNT; i++) {
                char key[32];
                snprintf(key, sizeof(key), "outfit_d%d", k_components[i]);
                util::config::write_int(outfit_section(), key,
                    native::get_ped_drawable_variation(ped, k_components[i]));
                snprintf(key, sizeof(key), "outfit_t%d", k_components[i]);
                util::config::write_int(outfit_section(), key,
                    native::get_ped_texture_variation(ped, k_components[i]));
            }
            menu::notify::stacked("Wardrobe", "Outfit saved");
        }));

    add_option(button_option("Load Outfit")
        .add_click([] {
            Ped ped = self_ped();
            if (!ped) return;
            for (int i = 0; i < COMPONENT_COUNT; i++) {
                char kd[32], kt[32];
                snprintf(kd, sizeof(kd), "outfit_d%d", k_components[i]);
                snprintf(kt, sizeof(kt), "outfit_t%d", k_components[i]);
                int d = util::config::read_int(outfit_section(), kd, -1);
                int t = util::config::read_int(outfit_section(), kt, 0);
                if (d >= 0)
                    native::set_ped_component_variation(ped, k_components[i], d, t, 0);
            }
            menu::notify::stacked("Wardrobe", "Outfit loaded");
        }));
}

player_wardrobe_saveload_menu* player_wardrobe_saveload_menu::get() {
    static player_wardrobe_saveload_menu instance;
    return &instance;
}
