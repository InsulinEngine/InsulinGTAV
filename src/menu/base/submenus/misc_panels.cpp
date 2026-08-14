#include "menu/base/submenus/misc_panels.h"
#include "menu/base/submenus/misc.h"
#include "menu/base/submenu_handler.h"
#include "menu/base/options/toggle.h"
#include "menu/base/options/number.h"
#include "menu/base/options/break.h"
#include "menu/base/options/button.h"
#include "menu/base/util/panels.h"
#include "menu/base/util/notify.h"
#include "util/config.h"

namespace {
    int g_built = -1;   // how many children the option list was built for

    int child_count() {
        int n = 0;
        for (menu::panels::panel_parent* parent : menu::panels::get_panels())
            n += (int)parent->m_children_panels.size();
        return n;
    }
}

void misc_panels_menu::load() {
    set_name("Panels");
    set_parent<misc_menu>();

    // Restore each panel's saved placement. Pure config and memory, no natives,
    // so this is safe during build().
    for (menu::panels::panel_parent* parent : menu::panels::get_panels()) {
        for (menu::panels::panel_child& child : parent->m_children_panels) {
            int column = util::config::read_int(get_submenu_name_stack(), "Column",
                                                child.m_column, { parent->m_name, child.m_name });
            int index  = util::config::read_int(get_submenu_name_stack(), "Index",
                                                child.m_index, { parent->m_name, child.m_name });
            child.m_render = util::config::read_bool(get_submenu_name_stack(), "Render",
                                                     child.m_render, { parent->m_name, child.m_name });
            menu::panels::rearrange(parent, child.m_id, column, index);
        }
    }
}

void misc_panels_menu::update() {
    // The panel list is data, so rebuild only when it actually changes.
    if (g_built != child_count()) update_once();
}

void misc_panels_menu::update_once() {
    g_built = child_count();
    clear_options(0);

    for (menu::panels::panel_parent* parent : menu::panels::get_panels()) {
        add_option(break_option(parent->m_name).ref());

        for (int i = 0; i < (int)parent->m_children_panels.size(); i++) {
            menu::panels::panel_child& child = parent->m_children_panels[i];

            add_option(toggle_option(child.m_name)
                .add_toggle(child.m_render)
                .add_tooltip("Show this panel")
                .add_savable(get_submenu_name_stack()));

            // Capture the pointer and the index only - 64-byte cap.
            menu::panels::panel_parent* pp = parent;
            add_option(number_option<int>(SCROLLSELECT, "  Column")
                .add_number(child.m_column, "%i", 1).add_min(0).add_max(1)
                .add_update([pp, i](number_option<int>*, int) {
                    menu::panels::panel_child& c = pp->m_children_panels[i];
                    menu::panels::rearrange(pp, c.m_id, c.m_column, c.m_index);
                }));

            add_option(number_option<int>(SCROLLSELECT, "  Order")
                .add_number(child.m_index, "%i", 1).add_min(0).add_max(8)
                .add_update([pp, i](number_option<int>*, int) {
                    menu::panels::panel_child& c = pp->m_children_panels[i];
                    menu::panels::rearrange(pp, c.m_id, c.m_column, c.m_index);
                }));
        }
    }

    add_option(button_option("Save Layout")
        .add_tooltip("Persist column, order and visibility for every panel")
        .add_click([] {
            util::config::begin_batch();
            for (menu::panels::panel_parent* parent : menu::panels::get_panels())
                for (menu::panels::panel_child& child : parent->m_children_panels) {
                    util::config::write_int(misc_panels_menu::get()->get_submenu_name_stack(),
                                            "Column", child.m_column, { parent->m_name, child.m_name });
                    util::config::write_int(misc_panels_menu::get()->get_submenu_name_stack(),
                                            "Index", child.m_index, { parent->m_name, child.m_name });
                    util::config::write_bool(misc_panels_menu::get()->get_submenu_name_stack(),
                                             "Render", child.m_render, { parent->m_name, child.m_name });
                }
            util::config::end_batch();
            menu::notify::stacked("Panels", "Layout saved");
        }));
}

misc_panels_menu* misc_panels_menu::get() {
    static misc_panels_menu instance;
    return &instance;
}
