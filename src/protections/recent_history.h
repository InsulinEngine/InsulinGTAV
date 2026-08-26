#pragma once
#include <stdint.h>
#include "protections/ring.h"   // for record

// A bounded, newest-first history of the reports drain_reports() has emitted,
// for the in-game log view (protections_log_menu). Script-thread only - written
// from drain_reports() and read from the menu, both on the frame callback - so
// it needs no atomics, unlike the lock-free `ring` it is fed from.
//
// Deliberately not the ring: the ring is drained destructively, and this needs
// the last N records to stay readable while the menu is open.
//
// The generation counter increments on EVERY append. It exists so a consumer can
// tell that the history changed even when the newest record is byte-identical to
// the previous one - which happens once the buffer is full and a filter reports
// the same record twice in a row. A dirty-check keyed on record content alone
// misses that case and silently shows a view stale by one position; comparing
// the generation closes the collision class entirely.
namespace protections {

    template <int Capacity>
    struct recent_history_t {
        struct entry { record r; uint32_t suppressed; };

        entry    m_buf[Capacity] = {};
        int      m_head       = 0;   // next write slot
        int      m_count      = 0;
        uint32_t m_generation = 0;

        void append(const record& r, uint32_t suppressed) {
            m_buf[m_head].r          = r;
            m_buf[m_head].suppressed = suppressed;
            m_head = (m_head + 1) % Capacity;
            if (m_count < Capacity) m_count++;
            m_generation++;          // unconditional: the signal the menu keys on
        }

        int      count() const      { return m_count; }
        uint32_t generation() const { return m_generation; }

        // index_from_newest == 0 is the most recent record.
        bool at(int index_from_newest, record* out, uint32_t* suppressed_out) const {
            if (!out || index_from_newest < 0 || index_from_newest >= m_count)
                return false;
            int slot = m_head - 1 - index_from_newest;
            while (slot < 0) slot += Capacity;
            *out = m_buf[slot].r;
            if (suppressed_out) *suppressed_out = m_buf[slot].suppressed;
            return true;
        }
    };
}
