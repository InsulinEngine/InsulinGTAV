#include "menu/base/submenus/protections.h"
#include "menu/base/submenus/protections_log.h"
#include "menu/base/submenus/main.h"
#include "menu/base/options/button.h"
#include "menu/base/options/dropdown.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/break.h"
#include "menu/base/options/submenu_option.h"
#include "menu/base/util/notify.h"
#include "protections/registry.h"
#include "protections/report.h"
#include "rage/invoker/natives.h"

#include <stdio.h>

namespace {
    // Backing ints for the dropdowns. A dropdown binds to an int by reference,
    // and registry entries hold their mode as an int for exactly this reason -
    // so the menu writes the value the hooks read, with no copy to keep in sync.
    int* mode_ref(int index) { return &protections::at(index)->current; }

    // Local state-keepers, not network filters. An attack tries to put you into
    // a state (on fire, ragdolling, blown up); these just refuse to let it
    // stick. Kept here rather than folded into the filter list above because
    // that list is Off/Log/Enforce gating on a detour, and there is neither a
    // detour nor a detection to gate here - only a per-frame re-assertion of a
    // flag. Grouped under "Local Self-Care", below the filters, for that
    // reason. (stop_entity_fire in particular has no other caller anywhere in
    // the menu, so this state and its toggle stay - moving it without a new
    // home would silently drop a working feature.)
    bool g_anti_fire = false;
    bool g_anti_ragdoll = false;
    bool g_anti_explosion = false;
    bool g_anti_ragdoll_latched = false;

    Ped self_ped() { return native::get_player_ped(-1); }
}

void protections_menu::load() {
    set_name("Protections");
    set_parent<main_menu>();

    add_option(break_option("Filters").ref());

    for (int i = 0; i < protections::count(); i++) {
        protections::filter* f = protections::at(i);

        // A filter that cannot block gets a two-item dropdown. Offering Enforce
        // on one of them would be a control that changes nothing while claiming
        // to drop hostile traffic - and the drain would still, correctly, log
        // "would-block". Rendering the truth in the menu is cheaper than
        // explaining the discrepancy later.
        //
        // A config saved before this change may hold 2 (Enforce) for such a
        // filter; add_savable() clamps to the last item, so it comes back as
        // Log. That is the intended landing, not a silent downgrade of a
        // protection - Enforce never did anything for these.
        dropdown_option row(f->name);
        row.add_index(*mode_ref(i))
           .add_item("Off")
           .add_item("Log");

        if (f->can_block) {
            row.add_item("Enforce")
               .add_tooltip("Off ignores. Log detects and reports without blocking. Enforce blocks.");
        } else {
            row.add_tooltip("Off ignores. Log detects and reports. This filter has nothing to block, so there is no Enforce.");
        }

        add_option(row
            .add_change([i](int value) {
                // Installing on demand keeps a filter's detour out of the
                // process until it is actually wanted. Idempotent.
                if (value != (int)protections::mode::off)
                    protections::ensure_installed(protections::at(i)->id);
            })
            .add_savable(get_submenu_name_stack()));
    }

    add_option(break_option("Diagnostics").ref());

    add_option(button_option("Fire Self Test")
        .add_tooltip("Pushes one report through the whole path: ring, drain, log, notification")
        .add_click([] {
            protections::report(protections::filter_id::self_test, 3, 0, 0xABCDEF01, 7);
        }));

    add_option(button_option("Report Counters")
        .add_tooltip("Total reports and how many were dropped by ring overflow")
        .add_click([] {
            char msg[96];
            snprintf(msg, sizeof(msg), "%u reported, %u dropped",
                     (unsigned)protections::total_reports(),
                     (unsigned)protections::total_dropped());
            menu::notify::stacked("Protections", msg);
        }));

    add_option(submenu_option("Report Log")
        .add_submenu<protections_log_menu>()
        .add_tooltip("The last 32 drained reports - filter, details, and how many more were coalesced into each one"));

    add_option(button_option("Script Events Learned")
        .add_tooltip("Distinct script-event hashes seen this session")
        .add_click([] {
            char msg[64];
            snprintf(msg, sizeof(msg), "%d distinct event hashes", protections::learned_count());
            menu::notify::stacked("Protections", msg);
        }));

    add_option(button_option("Clear Learned Events")
        .add_tooltip("Start a fresh baseline - do this before a known-attack session")
        .add_click([] { protections::learned_clear(); }));

    add_option(button_option("Weapon Hashes Learned")
        .add_tooltip("Distinct weapon hashes seen across damage/give/remove events")
        .add_click([] {
            char msg[64];
            snprintf(msg, sizeof(msg), "%d distinct weapon hashes", protections::weapon_learned_count());
            menu::notify::stacked("Protections", msg);
        }));

    add_option(button_option("Clear Weapon Hashes")
        .add_tooltip("Reset the weapon-hash baseline")
        .add_click([] { protections::weapon_learned_clear(); }));

    add_option(button_option("Sound Hashes Learned")
        .add_tooltip("Distinct network-play-sound hashes seen this session")
        .add_click([] {
            char msg[64];
            snprintf(msg, sizeof(msg), "%d distinct sound hashes", protections::sound_learned_count());
            menu::notify::stacked("Protections", msg);
        }));

    add_option(button_option("Clear Sound Hashes")
        .add_tooltip("Reset the sound-hash baseline")
        .add_click([] { protections::sound_learned_clear(); }));

    add_option(button_option("Explosion Types Learned")
        .add_tooltip("Distinct explosion tags seen this session")
        .add_click([] {
            char msg[64];
            snprintf(msg, sizeof(msg), "%d distinct explosion tags", protections::explosion_learned_count());
            menu::notify::stacked("Protections", msg);
        }));

    add_option(button_option("Clear Explosion Types")
        .add_tooltip("Reset the explosion-tag baseline")
        .add_click([] { protections::explosion_learned_clear(); }));

    add_option(break_option("Local Self-Care").ref());

    add_option(toggle_option("Anti Fire")
        .add_toggle(g_anti_fire)
        .add_tooltip("Puts you out immediately if something sets you alight")
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Anti Ragdoll")
        .add_toggle(g_anti_ragdoll)
        .add_tooltip("Keeps you on your feet through knockdowns")
        .add_savable(get_submenu_name_stack()));

    add_option(toggle_option("Explosion Proof")
        .add_toggle(g_anti_explosion)
        .add_tooltip("The explosion bit of SET_ENTITY_PROOFS, kept applied")
        .add_savable(get_submenu_name_stack()));
}

void protections_menu::feature_update() {
    Ped ped = self_ped();
    if (!ped)
        return;

    if (g_anti_fire && native::is_entity_on_fire(ped))
        native::stop_entity_fire(ped);

    if (g_anti_ragdoll) {
        native::set_ped_can_ragdoll(ped, false);
        g_anti_ragdoll_latched = true;
    } else if (g_anti_ragdoll_latched) {
        native::set_ped_can_ragdoll(ped, true);
        g_anti_ragdoll_latched = false;
    }

    // Only the explosion bit; the rest stay as the Player > Proofs menu left it.
    if (g_anti_explosion)
        native::set_entity_proofs(ped, false, false, true, false, false, false, false, false);
}

protections_menu* protections_menu::get() {
    static protections_menu instance;
    return &instance;
}
