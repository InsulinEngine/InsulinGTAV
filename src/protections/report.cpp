#include "protections/report.h"
#include "protections/ring.h"
#include "protections/coalesce.h"
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

    // Keeps one noisy filter from filling the ring and starving the others.
    // See coalesce.h - a filter has at most one undrained record in flight and
    // further occurrences are counted, not queued.
    coalescer& coalesce() {
        static coalescer instance;
        return instance;
    }

    uint32_t g_total   = 0;
    uint32_t g_last_notify_ms[64] = {};   // indexed by filter table position

    const uint32_t NOTIFY_INTERVAL_MS = 1000;

    // Bounded history for the in-game log view (protections_log_menu). Written
    // only from drain_reports() on the script thread and read only from the
    // menu, also on the script thread - no atomics, unlike the ring above.
    // Plain PODs zero-initialised via aggregate `= {}`, so this needs no
    // .init_array entry.
    struct recent_entry { record r; uint32_t suppressed; };
    recent_entry g_recent[recent_capacity] = {};
    int          g_recent_head  = 0;   // next write slot
    int          g_recent_count = 0;
}

void report(filter_id id, int player_index, uint8_t flags,
            uint32_t detail_a, uint32_t detail_b)
{
    // Every occurrence counts towards the total, including the ones that are
    // coalesced away below - the menu's counter must not understate the traffic
    // just because the ring only carries one record for it.
    __atomic_fetch_add(&g_total, 1, __ATOMIC_RELAXED);

    // If this filter already has an undrained record, the occurrence has been
    // counted against it and there is nothing to queue.
    if (!coalesce().claim((uint16_t)id))
        return;

    record r;
    r.filter_id    = (uint16_t)id;
    r.player_index = (player_index >= 0 && player_index < 32) ? (uint8_t)player_index : 0xFF;
    r.flags        = flags;
    r.detail_a     = detail_a;
    r.detail_b     = detail_b;

    reports().push(r);
}

void drain_reports()
{
    record r;

    // Bounded loop, not `while (pop())`. Releasing a filter reopens it, so a
    // network thread can push again inside this very loop; an unbounded drain
    // could in principle keep finding work and hold the script thread. Nothing
    // legitimate needs more than a ring's worth in one pass.
    for (uint32_t guard = 0; guard < ring::capacity && reports().pop(&r); guard++) {
        const filter_id id   = (filter_id)r.filter_id;
        const filter*   f    = find(id);
        const char*     name = f ? f->name : "<unknown>";

        // "BLOCK" is a claim about what the hook DID, not about the mode it was
        // in. Four filters cannot block anything - self_test, fragment_physics
        // and pool_exhaustion have no detour at all, and reliable_alloc never
        // calls should_block() - so Enforce on any of them must still print
        // "would-block". Deriving the label from the mode alone made "Self Test
        // -> Enforce, Fire Self Test" write `prot BLOCK Self Test` into the
        // kernel log, which is the one artefact this subsystem exists to
        // produce and the one place it must not lie.
        const bool      blocked = f && f->can_block && mode_of(id) == mode::enforce;

        // Reopen the filter immediately and pick up however many occurrences
        // were folded into this record.
        const uint32_t more = coalesce().release(r.filter_id);

        char extra[24];
        extra[0] = '\0';
        if (more) snprintf(extra, sizeof(extra), " +%u more", (unsigned)more);

        // Kernel log first: it is the channel that survives a crash, and this
        // is the record that matters when a filter takes the game down.
        if (r.player_index == 0xFF) {
            platform::klogf("prot %s %s a=%08x b=%08x%s",
                            blocked ? "BLOCK" : "would-block",
                            name, (unsigned)r.detail_a, (unsigned)r.detail_b, extra);
        } else {
            platform::klogf("prot %s %s player=%u a=%08x b=%08x%s",
                            blocked ? "BLOCK" : "would-block",
                            name, (unsigned)r.player_index,
                            (unsigned)r.detail_a, (unsigned)r.detail_b, extra);
        }

        // Append to the in-game log history. `more` is the same coalesced
        // count the klog line above just printed as "+N more" - the menu shows
        // it as its own "+N" column instead of folding it into the row text.
        g_recent[g_recent_head].r          = r;
        g_recent[g_recent_head].suppressed = more;
        g_recent_head = (g_recent_head + 1) % recent_capacity;
        if (g_recent_count < recent_capacity) g_recent_count++;

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
            snprintf(body, sizeof(body), "%s %s%s", blocked ? "Blocked" : "Detected", name, extra);
        else
            snprintf(body, sizeof(body), "%s %s from player %u%s",
                     blocked ? "Blocked" : "Detected", name, (unsigned)r.player_index, extra);

        menu::notify::stacked("Protections", body);
    }
}

uint32_t total_reports() { return __atomic_load_n(&g_total, __ATOMIC_RELAXED); }
uint32_t total_dropped() { return reports().dropped(); }

int recent_count() { return g_recent_count; }

bool recent_at(int index_from_newest, record* out, uint32_t* suppressed_out) {
    if (!out || index_from_newest < 0 || index_from_newest >= g_recent_count)
        return false;
    int slot = g_recent_head - 1 - index_from_newest;
    while (slot < 0) slot += recent_capacity;
    *out = g_recent[slot].r;
    if (suppressed_out) *suppressed_out = g_recent[slot].suppressed;
    return true;
}
}
