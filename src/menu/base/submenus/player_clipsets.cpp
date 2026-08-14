#include "menu/base/submenus/player_clipsets.h"
#include "menu/base/submenus/player_animation.h"
#include "menu/base/options/button.h"
#include "menu/base/options/scroll.h"
#include "menu/base/util/notify.h"
#include "rage/invoker/natives.h"
#include "game/anim_list.h"

namespace {
    int g_index[4] = {};
    scroll_struct<int> g_lists[4][32];
    int g_active = -1;      // group whose clipset is being held
    int g_active_item = -1;

    Ped self_ped() { return native::get_player_ped(-1); }
}

void player_clipsets_menu::load() {
    set_name("Clipset");
    set_parent<player_animation_menu>();

    add_option(button_option("Default Motion Clipset")
        .add_click([] {
            g_active = -1;
            native::reset_ped_movement_clipset(self_ped(), 0.f);
            menu::notify::stacked("Clipset", "Reset");
        }));

    for (int g = 0; g < clipset_group_count && g < 4; g++) {
        const named_group& grp = clipset_groups[g];

        int n = grp.count < 32 ? grp.count : 32;
        for (int i = 0; i < n; i++) {
            g_lists[g][i].m_name.set(grp.items[i].name);
            g_lists[g][i].m_result = i;
        }

        int gi = g;
        add_option(scroll_option<int>(SCROLLSELECT, grp.name)
            .add_scroll(g_index[g], 0, n - 1, g_lists[g])
            .add_click([gi] {
                const named_group& sel = clipset_groups[gi];
                int i = g_index[gi];
                if (i < 0 || i >= sel.count)
                    return;
                g_active = gi;
                g_active_item = i;
                native::request_anim_set(sel.items[i].id);
                native::set_ped_movement_clipset(self_ped(), sel.items[i].id, 0.25f);
                menu::notify::stacked("Clipset", sel.items[i].name);
            }));
    }
}

void player_clipsets_menu::feature_update() {
    // The game drops a movement clipset on vehicle entry, damage and respawn, so a
    // chosen one is re-applied rather than set once.
    if (g_active < 0 || g_active >= clipset_group_count)
        return;
    const named_group& sel = clipset_groups[g_active];
    if (g_active_item < 0 || g_active_item >= sel.count)
        return;
    native::set_ped_movement_clipset(self_ped(), sel.items[g_active_item].id, 0.25f);
}

player_clipsets_menu* player_clipsets_menu::get() {
    static player_clipsets_menu instance;
    return &instance;
}
