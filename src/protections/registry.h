#pragma once
#include <stdint.h>

// The filter table. One entry per protection, carrying its persisted mode and
// its detour installer.
//
// Detours install once, on first enable, and are never removed: unhooking while
// another thread sits inside the trampoline is a crash worth not risking. `off`
// therefore means "chain straight through", not "unhooked".
namespace protections {

    enum class mode : int { off = 0, log = 1, enforce = 2 };

    // Stable numeric ids. These are written into report records and must not be
    // renumbered - a saved config and a log line both refer to them.
    enum class filter_id : uint16_t {
        self_test           = 0,   // no detour; proves the report path end to end

        skeleton_extension  = 10,
        fragment_physics    = 11,
        invalid_decal       = 12,
        searchlight         = 13,
        task_ambient_clips  = 14,
        task_parachute      = 15,
        render_ped          = 16,
        render_entity       = 17,
        render_big_ped      = 18,
        pool_exhaustion     = 19,
        reliable_alloc      = 20,

        // Tier 2 - event filters.
        script_event        = 30,
        weapon_damage       = 31,
        give_weapon         = 32,
        remove_weapon       = 33,
        give_control        = 34,
        request_control     = 35,
    };

    struct filter {
        filter_id   id;
        const char* name;          // stable; also the config key
        uint8_t     tier;
        int         default_mode;  // always (int)mode::log
        int         current;       // bound to a dropdown_option by reference
        bool        installed;
        bool      (*install)();    // nullptr for filters with no detour
        // Whether Enforce can actually refuse anything for this filter.
        //
        // False for a filter with no detour (there is no call to refuse) and
        // for reliable_alloc, whose hook deliberately never calls
        // should_block() because retail null-checks every allocation return and
        // there is nothing to block. Two things read it: the drain, which must
        // not print BLOCK for a filter that blocked nothing, and the menu,
        // which must not offer an Enforce that does nothing.
        //
        // INVARIANT, asserted in tests/protections_registry_test.cpp:
        //   install == nullptr  =>  can_block == false.
        // The converse does not hold - reliable_alloc has an installer and
        // still cannot block.
        bool        can_block;
    };

    int     count();
    filter* at(int index);
    filter* find(filter_id id);

    const char* name_of(filter_id id);
    mode        mode_of(filter_id id);
    void        set_mode(filter_id id, mode m);

    // The two gates every hook uses.
    bool should_report(filter_id id);   // log or enforce
    bool should_block(filter_id id);    // enforce only

    // Installs the detour if it is not installed yet. Idempotent; returns true
    // when the filter is live (or needs no detour).
    bool ensure_installed(filter_id id);

    // Guard installers. Each matches the registry's install field.
    bool install_render_ped();
    bool install_render_entity();
    bool install_render_big_ped();
    bool install_task_ambient_clips();
    bool install_task_parachute();
    bool install_invalid_decal();
    bool install_searchlight();
    bool install_skeleton_extension();
    // Detection only - reports network-message allocator exhaustion and never
    // changes the return value. The recovery is deliberately unwritten; see
    // src/protections/hooks_reliable_alloc.cpp.
    bool install_reliable_alloc();

    // Tier 2. CScriptedGameEvent::Decide - learn mode plus the per-player
    // net_events block. See src/protections/hooks_script_event.cpp.
    bool install_script_event();

    // Learn-mode accessors for the menu. Script thread only.
    int  learned_count();
    bool learned_at(int index, uint32_t* hash, uint32_t* hits, uint8_t* first_player);
    void learned_clear();

    // Tier 2 Task 7. Weapon events - WEAPON_DAMAGE / GIVE / REMOVE, hooked at
    // Decide. Learn mode plus the per-player net_events block; see
    // src/protections/hooks_events_weapon.cpp.
    bool install_weapon_damage();
    bool install_give_weapon();
    bool install_remove_weapon();
    int  weapon_learned_count();
    void weapon_learned_clear();

    // Tier 2 Task 8. Control events - GIVE / REQUEST, hooked at Decide. Per-player
    // net_events block plus a report; see src/protections/hooks_events_control.cpp.
    bool install_give_control();
    bool install_request_control();

    // No install_pool_exhaustion(): rage::fwBasePool::New() is identified
    // (RVA 0x1EF6A00) but cannot be detoured safely - the 15-byte steal a
    // 14-byte jump forces contains a rel8 branch that GoldHEN's stub does not
    // relocate. Its registry entry keeps a null installer. See
    // src/protections/hooks_counters.cpp.

    // Installs every filter whose persisted mode is not Off.
    //
    // This exists because dropdown_option::add_savable restores the saved value
    // but deliberately does NOT fire add_change - the same boot rule that stops
    // toggle_option from invoking click handlers during menu::build(). Without
    // this call a filter saved as Enforce comes back showing Enforce with its
    // detour never installed: a protection that reads as on and does nothing.
    // Idempotent; call it once the game is up, never from build().
    void install_enabled_filters();
}
