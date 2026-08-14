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

    // A stable, id-keyed mirror of the panel children. menu::panels::rearrange()
    // re-sorts panel_parent::m_children_panels in place on every call, and that
    // sort swaps struct *values* between slots rather than moving pointers -
    // binding an option straight into m_children_panels[i], or capturing i to
    // reach it later, points the option at whichever child the sort has since
    // shuffled into slot i, not the one it was built for. This vector never
    // gets sorted, so an index into it stays valid; option state binds here,
    // and only the id + parent name (never a slot) is used to find the real
    // child again when applying a change.
    struct row {
        stl::string m_parent_name;   // to find the parent again
        stl::string m_id;            // the child's stable id: "player", "vehicle", ...
        stl::string m_name;          // display name, for the option label
        int  m_column;
        int  m_index;
        bool m_render;
        int  m_applied_column;       // last value pushed to the framework
        int  m_applied_index;
    };
    stl::vector<row> g_rows;         // built once in load(); order fixed after that

    int child_count() {
        int n = 0;
        for (menu::panels::panel_parent* parent : menu::panels::get_panels())
            n += (int)parent->m_children_panels.size();
        return n;
    }

    // Resolve a row back to the live child it names. Always by id - never by
    // the slot a row or option happened to capture - because rearrange() may
    // have moved the child to a different slot since.
    menu::panels::panel_child* find_child(const stl::string& parent_name, const stl::string& id,
                                          menu::panels::panel_parent** out_parent = nullptr) {
        for (menu::panels::panel_parent* parent : menu::panels::get_panels()) {
            if (parent->m_name != parent_name) continue;

            for (menu::panels::panel_child& child : parent->m_children_panels) {
                if (child.m_id != id) continue;
                if (out_parent) *out_parent = parent;
                return &child;
            }
        }
        return nullptr;
    }
}

void misc_panels_menu::load() {
    set_name("Panels");
    set_parent<misc_menu>();

    // Restore each panel's saved placement, and record the id-keyed mirror the
    // options below bind to instead of the live panel tree. Pure config and
    // memory reads plus rearrange() (no natives), so this is safe during
    // build().
    g_rows.clear();
    for (menu::panels::panel_parent* parent : menu::panels::get_panels()) {
        for (menu::panels::panel_child& child : parent->m_children_panels) {
            int column = util::config::read_int(get_submenu_name_stack(), "Column",
                                                child.m_column, { parent->m_name, child.m_name });
            int index  = util::config::read_int(get_submenu_name_stack(), "Index",
                                                child.m_index, { parent->m_name, child.m_name });
            child.m_render = util::config::read_bool(get_submenu_name_stack(), "Render",
                                                     child.m_render, { parent->m_name, child.m_name });
            menu::panels::rearrange(parent, child.m_id, column, index);

            row r;
            r.m_parent_name = parent->m_name;
            r.m_id = child.m_id;
            r.m_name = child.m_name;
            r.m_column = column;
            r.m_index = index;
            r.m_render = child.m_render;
            r.m_applied_column = column;
            r.m_applied_index = index;
            g_rows.push_back(r);
        }
    }
}

void misc_panels_menu::update() {
    // The panel list is data, so rebuild only when it actually changes.
    if (g_built != child_count()) update_once();

    // Visibility applies immediately, every frame this submenu is open - not
    // gated behind a rebuild - so a toggle takes effect right away, including
    // right before the user backs out of the submenu.
    for (row& r : g_rows) {
        menu::panels::panel_child* child = find_child(r.m_parent_name, r.m_id);
        if (child) child->m_render = r.m_render;
    }
}

void misc_panels_menu::update_once() {
    g_built = child_count();
    clear_options(0);

    stl::string last_parent = "";
    for (int i = 0; i < (int)g_rows.size(); i++) {
        if (g_rows[i].m_parent_name != last_parent) {
            add_option(break_option(g_rows[i].m_parent_name).ref());
            last_parent = g_rows[i].m_parent_name;
        }

        // Deliberately not savable: Save Layout below is the single owner of
        // the "Render" key. An auto-saved toggle would persist under its own
        // key (the option name, no stacks) and clobber what Save Layout wrote
        // the next time this menu is built.
        add_option(toggle_option(g_rows[i].m_name)
            .add_toggle(g_rows[i].m_render)
            .add_tooltip("Show this panel"));

        // Capture only the row index - it's stable, unlike a slot in the panel
        // tree. 4 bytes, well under the 64-byte stl::function cap.
        add_option(number_option<int>(SCROLLSELECT, "  Column")
            .add_number(g_rows[i].m_column, "%i", 1).add_min(0).add_max(1)
            .add_update([i](number_option<int>*, int) {
                row& r = g_rows[i];
                if (r.m_column == r.m_applied_column && r.m_index == r.m_applied_index) return;

                menu::panels::panel_parent* parent = nullptr;
                menu::panels::panel_child* child = find_child(r.m_parent_name, r.m_id, &parent);
                if (!child) return;

                menu::panels::rearrange(parent, r.m_id, r.m_column, r.m_index);
                r.m_applied_column = r.m_column;
                r.m_applied_index = r.m_index;
            }));

        add_option(number_option<int>(SCROLLSELECT, "  Order")
            .add_number(g_rows[i].m_index, "%i", 1).add_min(0).add_max(8)
            .add_update([i](number_option<int>*, int) {
                row& r = g_rows[i];
                if (r.m_column == r.m_applied_column && r.m_index == r.m_applied_index) return;

                menu::panels::panel_parent* parent = nullptr;
                menu::panels::panel_child* child = find_child(r.m_parent_name, r.m_id, &parent);
                if (!child) return;

                menu::panels::rearrange(parent, r.m_id, r.m_column, r.m_index);
                r.m_applied_column = r.m_column;
                r.m_applied_index = r.m_index;
            }));
    }

    add_option(button_option("Save Layout")
        .add_tooltip("Persist column, order and visibility for every panel")
        .add_click([] {
            util::config::begin_batch();
            for (row& r : g_rows) {
                util::config::write_int(misc_panels_menu::get()->get_submenu_name_stack(),
                                        "Column", r.m_column, { r.m_parent_name, r.m_name });
                util::config::write_int(misc_panels_menu::get()->get_submenu_name_stack(),
                                        "Index", r.m_index, { r.m_parent_name, r.m_name });
                util::config::write_bool(misc_panels_menu::get()->get_submenu_name_stack(),
                                         "Render", r.m_render, { r.m_parent_name, r.m_name });
            }
            util::config::end_batch();
            menu::notify::stacked("Panels", "Layout saved");
        }));
}

misc_panels_menu* misc_panels_menu::get() {
    static misc_panels_menu instance;
    return &instance;
}
