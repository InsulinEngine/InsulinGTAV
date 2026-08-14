#include "menu/base/submenus/player_scenarios.h"
#include "menu/base/submenus/player_animation.h"
#include "menu/base/options/button.h"
#include "menu/base/options/scroll.h"
#include "menu/base/util/notify.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/natives_hash.h"
#include "game/anim_list.h"

namespace {
    // One scroll index per group, matching Ozark's layout: a scroll option per
    // category, and pressing it plays whatever is selected.
    int g_index[8] = {};

    // scroll_option wants an array of scroll_struct, so each generated group is
    // mirrored into one on load. The generated tables stay the source of truth.
    scroll_struct<int> g_lists[8][40];

    Ped self_ped() { return native::get_player_ped(-1); }
}

void player_scenarios_menu::load() {
    set_name("Scenarios");
    set_parent<player_animation_menu>();

    add_option(button_option("Stop Scenario")
        .add_click([] { native::clear_ped_tasks(self_ped()); }));

    for (int g = 0; g < scenario_group_count && g < 8; g++) {
        const named_group& grp = scenario_groups[g];

        int n = grp.count < 40 ? grp.count : 40;
        for (int i = 0; i < n; i++) {
            g_lists[g][i].m_name.set(grp.items[i].name);
            g_lists[g][i].m_result = i;
        }

        int gi = g;   // tiny capture: stl::function caps captures at 64 bytes
        add_option(scroll_option<int>(SCROLLSELECT, grp.name)
            .add_scroll(g_index[g], 0, n - 1, g_lists[g])
            .add_click([gi] {
                const named_group& sel = scenario_groups[gi];
                int i = g_index[gi];
                if (i < 0 || i >= sel.count)
                    return;
                native::task_start_scenario_in_place(self_ped(), sel.items[i].id, 0, true);
                menu::notify::stacked("Scenario", sel.items[i].name);
            }));
    }
}

player_scenarios_menu* player_scenarios_menu::get() {
    static player_scenarios_menu instance;
    return &instance;
}
