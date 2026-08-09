#include "menu/base/submenus/main.h"
#include "menu/base/submenu_handler.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/break.h"
#include "menu/base/options/submenu_option.h"
#include "platform/log.h"

// Demo state the toggles bind to (POD globals = constant-initialised).
static bool g_demo_toggle_a = false;
static bool g_demo_toggle_b = true;

void main_menu::load() {
    set_name("InsulinGTAV");

    add_option(submenu_option("Demo Submenu")
        .add_submenu<demo_child>()
        .add_tooltip("Open a child submenu"));

    add_option(break_option("Options").ref());

    add_option(button_option("Say Hello")
        .add_tooltip("Sends a notification")
        .add_click([] { platform::notify("Hello from InsulinGTAV!"); }));

    add_option(toggle_option("Demo Toggle A")
        .add_toggle(g_demo_toggle_a)
        .add_tooltip("A bindable on/off toggle"));

    add_option(toggle_option("Demo Toggle B")
        .add_toggle(g_demo_toggle_b)
        .add_tooltip("Another toggle, starts on"));

    add_option(break_option("More").ref());

    add_option(button_option("Notify Frame Time")
        .add_click([] { platform::notify("tick alive"); }));
}

void main_menu::update_once() {}
void main_menu::update() {}
void main_menu::feature_update() {}

main_menu* main_menu::get() {
    static main_menu instance;
    return &instance;
}

void demo_child::load() {
    set_name("Demo Submenu");
    set_parent<main_menu>();

    add_option(button_option("Child Button 1")
        .add_click([] { platform::notify("child button 1"); }));
    add_option(button_option("Child Button 2")
        .add_click([] { platform::notify("child button 2"); }));
}

demo_child* demo_child::get() {
    static demo_child instance;
    return &instance;
}
