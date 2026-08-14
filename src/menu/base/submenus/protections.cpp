#include "menu/base/submenus/protections.h"
#include "menu/base/submenus/main.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/break.h"
#include "menu/base/util/notify.h"
#include "rage/invoker/natives.h"
#include "rage/invoker/natives_hash.h"

// What this is and is not.
//
// Ozark's protections work by hooking the network-event receive path and dropping
// hostile script events before the game acts on them. This port has no such hook,
// so none of that is available here yet.
//
// What IS possible without one is the local side: keep the states an attack tries
// to put you into from sticking. That covers the common griefs (set on fire, sent
// ragdolling, blown out of a vehicle) without pretending to filter anything.

namespace {
    bool g_anti_fire = false;
    bool g_anti_ragdoll = false;
    bool g_anti_explosion = false;
    bool g_anti_ragdoll_latched = false;

    Ped self_ped() { return native::get_player_ped(-1); }
}

void protections_menu::load() {
    set_name("Protections");
    set_parent<main_menu>();

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

    add_option(break_option("Notes").ref());

    add_option(button_option("Why so few")
        .add_tooltip("Read before expecting Ozark parity")
        .add_click([] {
            menu::notify::stacked("Protections",
                "Event filtering needs a network-event hook this port lacks");
        }));
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

    // Only the explosion bit; the rest stay as the Player > Proofs menu left them.
    if (g_anti_explosion)
        native::set_entity_proofs(ped, false, false, true, false, false, false, false, false);
}

protections_menu* protections_menu::get() {
    static protections_menu instance;
    return &instance;
}
