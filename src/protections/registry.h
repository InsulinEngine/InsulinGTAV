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
    };

    struct filter {
        filter_id   id;
        const char* name;          // stable; also the config key
        uint8_t     tier;
        int         default_mode;  // always (int)mode::log
        int         current;       // bound to a dropdown_option by reference
        bool        installed;
        bool      (*install)();    // nullptr for filters with no detour
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
