#include "protections/report.h"
#include "protections/ring.h"
#include "menu/base/util/notify.h"
#include "platform/log.h"
#include "rage/invoker/natives.h"

#include <stdio.h>

namespace protections {
namespace {
    ring& reports() {
        static ring instance;      // function-local static: no .init_array
        return instance;
    }

    uint32_t g_total   = 0;
    uint32_t g_last_notify_ms[64] = {};   // indexed by filter table position

    const uint32_t NOTIFY_INTERVAL_MS = 1000;
}

void report(filter_id id, int player_index, uint8_t flags,
            uint32_t detail_a, uint32_t detail_b)
{
    record r;
    r.filter_id    = (uint16_t)id;
    r.player_index = (player_index >= 0 && player_index < 32) ? (uint8_t)player_index : 0xFF;
    r.flags        = flags;
    r.detail_a     = detail_a;
    r.detail_b     = detail_b;

    reports().push(r);
    __atomic_fetch_add(&g_total, 1, __ATOMIC_RELAXED);
}

void drain_reports()
{
    record r;
    while (reports().pop(&r)) {
        const filter_id id   = (filter_id)r.filter_id;
        const char*     name = name_of(id);
        const bool      blocked = (mode_of(id) == mode::enforce);

        // Kernel log first: it is the channel that survives a crash, and this
        // is the record that matters when a filter takes the game down.
        if (r.player_index == 0xFF) {
            platform::klogf("prot %s %s a=%08x b=%08x",
                            blocked ? "BLOCK" : "would-block",
                            name, (unsigned)r.detail_a, (unsigned)r.detail_b);
        } else {
            platform::klogf("prot %s %s player=%u a=%08x b=%08x",
                            blocked ? "BLOCK" : "would-block",
                            name, (unsigned)r.player_index,
                            (unsigned)r.detail_a, (unsigned)r.detail_b);
        }

        // Then the screen, rate-limited. A sound-spam attack produces hundreds
        // of these per second; a notification each would be its own denial of
        // service. Find the table slot so the rate limit is per filter.
        int slot = -1;
        for (int i = 0; i < count() && i < 64; i++)
            if (at(i)->id == id) { slot = i; break; }

        if (slot < 0) continue;

        const uint32_t now = native::get_game_timer();
        if (now - g_last_notify_ms[slot] < NOTIFY_INTERVAL_MS) continue;
        g_last_notify_ms[slot] = now;

        char body[96];
        if (r.player_index == 0xFF)
            snprintf(body, sizeof(body), "%s %s", blocked ? "Blocked" : "Detected", name);
        else
            snprintf(body, sizeof(body), "%s %s from player %u",
                     blocked ? "Blocked" : "Detected", name, (unsigned)r.player_index);

        menu::notify::stacked("Protections", body);
    }
}

uint32_t total_reports() { return __atomic_load_n(&g_total, __ATOMIC_RELAXED); }
uint32_t total_dropped() { return reports().dropped(); }
}
