#include "protections/registry.h"

namespace protections {
namespace {
    // Function-local static, not a namespace-scope object: this plugin has no
    // .init_array, so a global constructor would never run.
    filter* table(int* out_count) {
        //                                                       tier  default         current         installed  install                        can_block
        static filter t[] = {
            { filter_id::self_test,          "Self Test",           1, (int)mode::log, (int)mode::log, false, nullptr,                     false },
            { filter_id::skeleton_extension, "Skeleton Extension",  1, (int)mode::log, (int)mode::log, false, &install_skeleton_extension, true  },
            { filter_id::fragment_physics,   "Fragment Physics",    1, (int)mode::log, (int)mode::log, false, nullptr,                     false },
            { filter_id::invalid_decal,      "Invalid Decal",       1, (int)mode::log, (int)mode::log, false, &install_invalid_decal,      true  },
            { filter_id::searchlight,        "Searchlight",         1, (int)mode::log, (int)mode::log, false, &install_searchlight,        true  },
            { filter_id::task_ambient_clips, "Task Ambient Clips",  1, (int)mode::log, (int)mode::log, false, &install_task_ambient_clips, true  },
            { filter_id::task_parachute,     "Task Parachute",      1, (int)mode::log, (int)mode::log, false, &install_task_parachute,     true  },
            { filter_id::render_ped,         "Render Ped",          1, (int)mode::log, (int)mode::log, false, &install_render_ped,         true  },
            { filter_id::render_entity,      "Render Entity",       1, (int)mode::log, (int)mode::log, false, &install_render_entity,      true  },
            { filter_id::render_big_ped,     "Render Big Ped",      1, (int)mode::log, (int)mode::log, false, &install_render_big_ped,     true  },
            { filter_id::pool_exhaustion,    "Pool Exhaustion",     1, (int)mode::log, (int)mode::log, false, nullptr,                     false },
            // Has a detour, but the hook never calls should_block(): retail
            // null-checks every AllocCritical return, so there is nothing to
            // refuse. Detection only until a recovery exists.
            { filter_id::reliable_alloc,     "Reliable Allocator",  1, (int)mode::log, (int)mode::log, false, &install_reliable_alloc,     false },
            // Tier 2. can_block is true: the per-player net_events path genuinely
            // refuses events, even though the filter blocks nothing by type yet.
            { filter_id::script_event,       "Script Events",       2, (int)mode::log, (int)mode::log, false, &install_script_event,       true  },
            // Tier 2 Task 7. can_block true: the per-player net_events path
            // refuses. Weapon-hash validity block is a derived follow-up.
            { filter_id::weapon_damage,      "Weapon Damage",       2, (int)mode::log, (int)mode::log, false, &install_weapon_damage,      true  },
            { filter_id::give_weapon,        "Give Weapon",         2, (int)mode::log, (int)mode::log, false, &install_give_weapon,        true  },
            { filter_id::remove_weapon,      "Remove Weapon",       2, (int)mode::log, (int)mode::log, false, &install_remove_weapon,      true  },
            // Tier 2 Task 8. Control take-over events; per-player path refuses.
            { filter_id::give_control,       "Give Control",        2, (int)mode::log, (int)mode::log, false, &install_give_control,       true  },
            { filter_id::request_control,    "Request Control",     2, (int)mode::log, (int)mode::log, false, &install_request_control,    true  },
            // Tier 2 Task 9. Ped griefing tools; per-player path refuses.
            { filter_id::clear_ped_tasks,    "Clear Ped Tasks",     2, (int)mode::log, (int)mode::log, false, &install_clear_ped_tasks,    true  },
            { filter_id::ragdoll_request,    "Ragdoll Request",     2, (int)mode::log, (int)mode::log, false, &install_ragdoll_request,    true  },
            // Tier 2 Task 10. script_world_state has no installer: its Decide is a
            // 3-byte stub that check_prologue refuses (see hooks_events_scriptstate).
            { filter_id::script_world_state, "Script World State",  2, (int)mode::log, (int)mode::log, false, nullptr,                     false },
            { filter_id::script_entity_state,"Script Entity State", 2, (int)mode::log, (int)mode::log, false, &install_script_entity_state,true  },
            // Tier 2 Task 11. Nuisance events; per-player path refuses.
            { filter_id::play_sound,         "Play Sound",          2, (int)mode::log, (int)mode::log, false, &install_play_sound,         true  },
            { filter_id::change_radio,       "Change Radio Station",2, (int)mode::log, (int)mode::log, false, &install_change_radio,       true  },
            { filter_id::door_break,         "Door Break",          2, (int)mode::log, (int)mode::log, false, &install_door_break,         true  },
        };
        *out_count = (int)(sizeof(t) / sizeof(t[0]));
        return t;
    }
}

int count() {
    int n = 0;
    table(&n);
    return n;
}

filter* at(int index) {
    int n = 0;
    filter* t = table(&n);
    if (index < 0 || index >= n) return nullptr;
    return &t[index];
}

filter* find(filter_id id) {
    int n = 0;
    filter* t = table(&n);
    for (int i = 0; i < n; i++)
        if (t[i].id == id) return &t[i];
    return nullptr;
}

const char* name_of(filter_id id) {
    filter* f = find(id);
    return f ? f->name : "<unknown>";
}

mode mode_of(filter_id id) {
    filter* f = find(id);
    if (!f) return mode::off;
    const int m = __atomic_load_n(&f->current, __ATOMIC_RELAXED);
    if (m != (int)mode::log && m != (int)mode::enforce) return mode::off;
    return (mode)m;
}

void set_mode(filter_id id, mode m) {
    filter* f = find(id);
    if (!f) return;
    __atomic_store_n(&f->current, (int)m, __ATOMIC_RELAXED);
}

bool should_report(filter_id id) {
    const mode m = mode_of(id);
    return m == mode::log || m == mode::enforce;
}

bool should_block(filter_id id) {
    return mode_of(id) == mode::enforce;
}

bool ensure_installed(filter_id id) {
    filter* f = find(id);
    if (!f) return false;
    if (!f->install) return true;      // nothing to hook
    if (f->installed) return true;
    f->installed = f->install();
    return f->installed;
}

void install_enabled_filters() {
    int n = 0;
    filter* t = table(&n);
    for (int i = 0; i < n; i++)
        if (mode_of(t[i].id) != mode::off)
            ensure_installed(t[i].id);
}
}
