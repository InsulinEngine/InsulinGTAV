#pragma once
#include <stdint.h>
#include "protections/registry.h"
#include "protections/ring.h"   // for record, used by recent_at()

// Reporting for protection filters.
//
// report() is callable from a network thread: it writes one POD record into the
// ring and returns. It does not allocate, format, log, or notify - all of that
// happens in drain_reports() on the script thread, where it is legal.
namespace protections {

    void report(filter_id id, int player_index, uint8_t flags,
                uint32_t detail_a, uint32_t detail_b);

    // Convenience for the common case: no player, no details.
    inline void report(filter_id id) { report(id, -1, 0, 0, 0); }

    // Drains the ring to the kernel log and to rate-limited notifications.
    // Call once per frame from the script thread.
    void drain_reports();

    uint32_t total_reports();
    uint32_t total_dropped();

    // A bounded history of what drain_reports() has emitted, for the in-game
    // log view. Written and read on the script thread only - no atomics.
    //
    // Deliberately not the ring: the ring is drained destructively and this
    // needs the last N to stay readable while the menu is open.
    static const int recent_capacity = 32;

    int  recent_count();
    bool recent_at(int index_from_newest, record* out, uint32_t* suppressed_out);
}
