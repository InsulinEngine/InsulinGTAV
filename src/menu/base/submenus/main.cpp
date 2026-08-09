#include "menu/base/submenus/main.h"
#include "menu/base/submenu_handler.h"
#include "menu/base/options/button.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/break.h"
#include "menu/base/options/submenu_option.h"
#include "menu/base/options/number.h"
#include "menu/base/options/scroll.h"
#include "menu/base/options/radio.h"
#include "menu/base/options/color_option.h"
#include "menu/base/submenus/self.h"
#include "menu/base/util/notify.h"
#include "menu/base/util/stacked_display.h"
#include "platform/log.h"

// Demo state (POD globals = constant-initialised; the string/map-bearing ones
// are zero-init and populated at runtime in load()).
static bool g_demo_toggle_a = false;
static bool g_demo_toggle_b = true;
static int g_demo_int = 50;
static float g_demo_float = 1.0f;
static int g_demo_list_index = 0;
static scroll_struct<int> g_demo_list[3];
static radio_context g_demo_radio;
static int g_demo_radio_ignored = 0;
static color_rgba g_demo_color = color_rgba(220, 76, 81, 255);
static bool g_show_stat = false;
static bool g_saved_toggle = false;
static int g_saved_int = 25;
static color_rgba g_saved_color = color_rgba(80, 160, 240, 255);

void main_menu::load() {
    set_name("InsulinGTAV");

    // runtime-init the string/map-bearing demo state (no .init_array on console).
    g_demo_list[0].m_name.set("Low");   g_demo_list[0].m_result = 0;
    g_demo_list[1].m_name.set("Medium"); g_demo_list[1].m_result = 1;
    g_demo_list[2].m_name.set("High");  g_demo_list[2].m_result = 2;
    g_demo_radio.m_sprite = stl::make_pair("commonmenu", "shop_art_icon");
    g_demo_radio.m_count = 0;

    add_option(submenu_option("Self")
        .add_submenu<self_menu>()
        .add_tooltip("Player features (godmode, heal, weapons ...)"));

    add_option(submenu_option("Demo Submenu")
        .add_submenu<demo_child>()
        .add_tooltip("Open a child submenu"));

    add_option(break_option("Options").ref());

    add_option(button_option("Say Hello")
        .add_tooltip("Ozark stacked notification")
        .add_click([] { menu::notify::stacked("InsulinGTAV", "Hello from the menu!", global::ui::g_success); }));

    add_option(button_option("Multi-line Notify")
        .add_tooltip("A stacked notification with several lines")
        .add_click([] {
            stl::vector<stl::string> lines;
            lines.push_back("Line one of the notify");
            lines.push_back("Line two, a bit longer");
            lines.push_back("Line three");
            menu::notify::stacked_lines("Notice", lines);
        }));

    add_option(toggle_option("Stacked Display Row")
        .add_toggle(g_show_stat)
        .add_tooltip("Toggle a bottom-right key/value row"));

    add_option(toggle_option("Demo Toggle A")
        .add_toggle(g_demo_toggle_a)
        .add_tooltip("A bindable on/off toggle"));

    add_option(toggle_option("Demo Toggle B")
        .add_toggle(g_demo_toggle_b)
        .add_tooltip("Another toggle, starts on"));

    add_option(number_option<int>(SCROLLSELECT, "Int Slider")
        .add_number(g_demo_int, "%i", 1)
        .add_min(0).add_max(100)
        .add_tooltip("D-Pad left/right to change"));

    add_option(number_option<float>(SCROLLSELECT, "Float Slider")
        .add_number(g_demo_float, "%.1f", 0.1f)
        .add_min(0.f).add_max(10.f)
        .add_tooltip("Float value with step 0.1"));

    add_option(scroll_option<int>(SCROLLSELECT, "List Option")
        .add_scroll(g_demo_list_index, 0, 3, g_demo_list)
        .add_tooltip("A scroll list: Low / Medium / High"));

    add_option(color_option("Color Picker")
        .add_color(g_demo_color)
        .add_tooltip("Select to open the HSV picker"));

    add_option(break_option("Radio Group").ref());

    add_option(radio_option("Radio: Alpha").add_radio(g_demo_radio));
    add_option(radio_option("Radio: Beta").add_radio(g_demo_radio));
    add_option(radio_option("Radio: Gamma").add_radio(g_demo_radio));
}

void main_menu::update_once() {}

void main_menu::update() {
    // Drive the demo stacked-display row each frame from the toggle.
    if (g_show_stat) {
        menu::display::update("demo", "Int Slider", stl::string::format("%i", g_demo_int));
    } else {
        menu::display::disable("demo");
    }
}

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

    add_option(break_option("Saved (survive restart)").ref());

    add_option(toggle_option("Saved Toggle")
        .add_toggle(g_saved_toggle)
        .add_savable(get_submenu_name_stack())
        .add_tooltip("Persists across a restart"));

    add_option(number_option<int>(SCROLLSELECT, "Saved Int")
        .add_number(g_saved_int, "%i", 1)
        .add_min(0).add_max(100)
        .add_savable(get_submenu_name_stack())
        .add_tooltip("Persists across a restart"));

    add_option(color_option("Saved Color")
        .add_color(g_saved_color)
        .add_savable(get_submenu_name_stack())
        .add_tooltip("Persists across a restart"));
}

demo_child* demo_child::get() {
    static demo_child instance;
    return &instance;
}
