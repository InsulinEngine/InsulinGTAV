#include "menu/base/submenus/main.h"
#include "menu/base/submenu_handler.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/break.h"
#include "menu/base/options/submenu_option.h"
#include "menu/base/options/number.h"
#include "menu/base/options/scroll.h"
#include "menu/base/options/scrollbar.h"
#include "menu/base/options/radio.h"
#include "menu/base/options/color_option.h"
#include "menu/base/options/dropdown.h"
#include "menu/base/options/modal.h"
#include "menu/base/util/notify.h"
#include "menu/base/util/animated_texture.h"
#include "menu/base/submenus/player.h"
#include "menu/base/submenus/network.h"
#include "menu/base/submenus/protections.h"
#include "menu/base/submenus/teleport.h"
#include "menu/base/submenus/weapon.h"
#include "menu/base/submenus/spawner.h"
#include "menu/base/submenus/world.h"
#include "menu/base/submenus/misc.h"
#include "menu/base/submenus/vehicle.h"
#include "menu/base/submenus/settings.h"
#include "menu/base/util/notify.h"
#include "menu/base/util/stacked_display.h"
#include "platform/log.h"
#include "rage/gfx.h"

void main_menu::load() {
    set_name("InsulinGTAV");

    // Same order as Ozark: Player, Network, Protections, Teleport, Weapon,
    // Vehicle, Spawner, World, Miscellaneous, Settings.
    add_option(submenu_option("Player").add_submenu<player_menu>());
    add_option(submenu_option("Network").add_submenu<network_menu>());
    add_option(submenu_option("Protections").add_submenu<protections_menu>());
    add_option(submenu_option("Teleport").add_submenu<teleport_menu>());
    add_option(submenu_option("Weapon").add_submenu<weapon_menu>());
    add_option(submenu_option("Vehicle").add_submenu<vehicle_menu>());
    add_option(submenu_option("Spawner").add_submenu<spawner_menu>());
    add_option(submenu_option("World").add_submenu<world_menu>());
    add_option(submenu_option("Miscellaneous").add_submenu<misc_menu>());
    add_option(submenu_option("Settings").add_submenu<settings_menu>());

    add_option(break_option("Custom Textures").ref());

    add_option(button_option("Load Custom Textures")
        .add_tooltip("Loads every .dds/.png in /data/insulin into the \"insulin\" dictionary; \"logo\" becomes the header")
        .add_click([] {
            auto& t = rage::gfx::menu_textures();
            if (t.add_directory("/data/insulin") == 0)
                t.add("logo", "/data/insulin/logo.dds");   // fallback if the dir scan yields nothing
            t.commit();
        }));

    add_option(button_option("Load Banner Animation")
        .add_tooltip("Plays /data/insulin/anim/banner as the menu header")
        .add_click([] {
            menu::animated_texture* a = menu::animation::load_banner();
            if (a) menu::notify::stacked("Animation", stl::string::format("%i frames", a->frame_count()), global::ui::g_success);
            else   menu::notify::stacked("Animation", "Nothing loadable in /data/insulin/anim/banner", global::ui::g_error);
        }));
}

void main_menu::update_once() {}

void main_menu::update() {}

void main_menu::feature_update() {}

main_menu* main_menu::get() {
    static main_menu instance;
    return &instance;
}
