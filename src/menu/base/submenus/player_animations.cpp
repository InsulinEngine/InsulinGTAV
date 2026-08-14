#include "menu/base/submenus/player_animations.h"
#include "menu/base/submenus/player_animation.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/scroll.h"
#include "menu/base/util/notify.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/natives_hash.h"
#include "game/anim_list.h"

namespace {
    bool g_controllable = false;   // keep control of the ped while it plays
    bool g_contort = false;        // upper body only, so you can walk through it

    int g_index[8] = {};
    scroll_struct<int> g_lists[8][60];

    Ped self_ped() { return native::get_player_ped(-1); }

    void play(const anim_entry& e) {
        Ped ped = self_ped();
        if (!ped)
            return;

        // Unlike scenarios these live in dictionaries that have to be streamed
        // first, and TASK_PLAY_ANIM silently does nothing if the dictionary is not
        // in yet - hence the request and the explicit report when it does not land.
        native::request_anim_dict(e.dict);
        if (!native::has_anim_dict_loaded(e.dict)) {
            menu::notify::stacked("Animation", "Streaming, press again");
            return;
        }

        // flag 1 = loop, 8 = upper body only, 16 = keep control, 32 = allow movement
        int flags = 1;
        if (g_contort)      flags |= 8;
        if (g_controllable) flags |= 16 | 32;

        native::task_play_anim(ped, e.dict, e.anim, 8.f, -8.f, -1, flags, 0.f, false, false, false);
        menu::notify::stacked("Animation", e.name);
    }
}

void player_animations_menu::load() {
    set_name("Animations");
    set_parent<player_animation_menu>();

    add_option(button_option("Stop Animation")
        .add_click([] { native::clear_ped_tasks(self_ped()); }));

    add_option(toggle_option("Controllable")
        .add_toggle(g_controllable)
        .add_tooltip("Keep moving while the animation plays")
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Contort")
        .add_toggle(g_contort)
        .add_tooltip("Upper body only")
        .add_savable(get_submenu_name_stack()));

    for (int g = 0; g < animation_group_count && g < 8; g++) {
        const anim_group& grp = animation_groups[g];

        int n = grp.count < 60 ? grp.count : 60;
        for (int i = 0; i < n; i++) {
            g_lists[g][i].m_name.set(grp.items[i].name);
            g_lists[g][i].m_result = i;
        }

        int gi = g;
        add_option(scroll_option<int>(SCROLLSELECT, grp.name)
            .add_scroll(g_index[g], 0, n - 1, g_lists[g])
            .add_click([gi] {
                const anim_group& sel = animation_groups[gi];
                int i = g_index[gi];
                if (i >= 0 && i < sel.count)
                    play(sel.items[i]);
            }));
    }
}

player_animations_menu* player_animations_menu::get() {
    static player_animations_menu instance;
    return &instance;
}
