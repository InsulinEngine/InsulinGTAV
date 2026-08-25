#include "menu/base/submenus/protections_log.h"
#include "menu/base/submenus/protections.h"
#include "menu/base/options/button.h"
#include "protections/report.h"
#include "protections/registry.h"

#include <stddef.h>
#include <stdio.h>

// One row per protections::recent_at() entry, newest first.
//
// Rebuilt only when the count or the newest record changes - the dirty-flag
// pattern vehicle_class_menu.cpp uses for its per-class vehicle list, never a
// rebuild every frame. update() only runs while this submenu is the one on
// screen (submenu_handler::update() calls it on m_current alone), so even the
// comparison itself costs nothing while the log is not being looked at, and a
// rebuild fires at most once per newly-drained record - it cannot thrash,
// because rebuild() immediately re-snapshots what it just built and the next
// frame's comparison finds no difference until another record actually drains.
namespace {
    int                  g_built_count      = -1;
    bool                 g_built_has_newest = false;
    protections::record  g_built_newest     = {};
    uint32_t             g_built_suppressed = 0;

    void format_row(char* out, size_t out_size, const protections::record& r, uint32_t suppressed) {
        char player[16];
        player[0] = '\0';
        if (r.player_index != 0xFF)
            snprintf(player, sizeof(player), " p=%u", (unsigned)r.player_index);

        char extra[16];
        extra[0] = '\0';
        if (suppressed)
            snprintf(extra, sizeof(extra), " +%u", (unsigned)suppressed);

        snprintf(out, out_size, "%s  a=%08x b=%08x%s%s",
                 protections::name_of((protections::filter_id)r.filter_id),
                 (unsigned)r.detail_a, (unsigned)r.detail_b, player, extra);
    }

    // clear_options()/add_option() are public on menu::submenu::submenu, so
    // this can stay a free function taking the instance rather than a member -
    // the header stays to the three methods the brief specifies.
    void rebuild(protections_log_menu* self) {
        int count = protections::recent_count();

        protections::record newest = {};
        uint32_t suppressed = 0;
        bool has_newest = protections::recent_at(0, &newest, &suppressed);

        g_built_count      = count;
        g_built_has_newest = has_newest;
        g_built_newest     = newest;
        g_built_suppressed = suppressed;

        self->clear_options(0);

        if (count == 0) {
            self->add_option(button_option("~m~(no reports yet)").ref());
            return;
        }

        for (int i = 0; i < count; i++) {
            protections::record r;
            uint32_t sup = 0;
            if (!protections::recent_at(i, &r, &sup))
                break;

            char line[128];
            format_row(line, sizeof(line), r, sup);
            self->add_option(button_option(line).ref());
        }
    }
}

void protections_log_menu::load() {
    set_name("Report Log");
    set_parent<protections_menu>();
    rebuild(this);
}

void protections_log_menu::update() {
    int count = protections::recent_count();

    protections::record newest = {};
    uint32_t suppressed = 0;
    bool has_newest = protections::recent_at(0, &newest, &suppressed);

    bool changed = (count != g_built_count) || (has_newest != g_built_has_newest) ||
        (has_newest &&
         (newest.filter_id != g_built_newest.filter_id ||
          newest.player_index != g_built_newest.player_index ||
          newest.flags != g_built_newest.flags ||
          newest.detail_a != g_built_newest.detail_a ||
          newest.detail_b != g_built_newest.detail_b ||
          suppressed != g_built_suppressed));

    if (changed)
        rebuild(this);
}

protections_log_menu* protections_log_menu::get() {
    static protections_log_menu instance;
    return &instance;
}
