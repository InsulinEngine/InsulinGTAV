#pragma once
#include <stdint.h>

// A small open-addressed set of the distinct event hashes a filter has seen,
// each with a hit count and the FIRST player to send it. It backs "learn mode".
//
// Why this and not the report ring: learn mode wants the SET of hashes, not the
// stream. The coalescer (coalesce.h) keys on filter_id alone, so it would fold
// every distinct hash a learn filter reports into a single "+N more" record and
// destroy the very data being collected. A set with per-hash counts is the right
// shape - written from the network thread inside the hook, read from the script
// thread by the menu.
//
// observe() runs on a network thread, so - like ring::push and coalescer::claim
// - it must never block, allocate, format, log, or call a native. Two network
// threads can deliver events at once, so a slot is claimed with __atomic_* the
// way coalesce.h does. A per-slot state (EMPTY/CLAIMED/READY) rather than a
// sentinel hash does two jobs: hash 0 is a storable value, not "empty", and a
// concurrent reader never sees a half-written slot.
namespace protections {

    class learn_table {
    public:
        static const int capacity = 128;

        // Returns true only when THIS call first admitted the hash, so the hook
        // can report a newly-seen hash and stay silent on every repeat. A repeat
        // returns false and still counts a hit - even when the table is full, so
        // a saturated table keeps counting the events already known to matter. A
        // NEW hash that does not fit counts an overflow() and returns false: the
        // table is saturated, and overflow() is the signal for that, not a
        // per-occurrence report that would flood the ring.
        bool observe(uint32_t hash, uint8_t player_index) {
            const uint32_t start = hash % (uint32_t)capacity;

            for (int probe = 0; probe < capacity; probe++) {
                slot& s = m_slots[(start + (uint32_t)probe) % (uint32_t)capacity];

                uint8_t st = __atomic_load_n(&s.state, __ATOMIC_ACQUIRE);

                // A slot mid-write (CLAIMED) resolves in a handful of stores.
                // Wait it out, but bounded, so a descheduled writer can never
                // park this network thread. On giving up, fall through to treat
                // it as occupied by another hash and probe on - at worst a rare
                // duplicate entry under pathological contention, never a stall.
                for (int spin = 0; st == CLAIMED && spin < kSpin; spin++)
                    st = __atomic_load_n(&s.state, __ATOMIC_ACQUIRE);

                if (st == READY) {
                    if (s.hash == hash) {                        // already known
                        __atomic_fetch_add(&s.hits, 1, __ATOMIC_RELAXED);
                        return false;
                    }
                    continue;                                    // collision, probe on
                }
                if (st == CLAIMED)
                    continue;                                    // writer stalled, probe on

                // st == EMPTY: try to take it.
                uint8_t expected = EMPTY;
                if (!__atomic_compare_exchange_n(&s.state, &expected, (uint8_t)CLAIMED, false,
                                                 __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) {
                    // Another producer took this slot first. Re-examine the SAME
                    // slot next iteration (not the next one) so a concurrent
                    // insert of OUR hash is deduped rather than duplicated.
                    probe--;
                    continue;
                }

                // Won the slot. Publish the payload BEFORE marking it READY, so
                // an acquiring reader sees a complete record or none of it.
                s.hash         = hash;
                s.hits         = 1;
                s.first_player = player_index;
                __atomic_store_n(&s.state, (uint8_t)READY, __ATOMIC_RELEASE);
                __atomic_fetch_add(&m_count, 1, __ATOMIC_RELAXED);
                return true;
            }

            // Probed every slot without a match or a free slot: full, hash new.
            __atomic_fetch_add(&m_overflow, 1, __ATOMIC_RELAXED);
            return false;
        }

        int      count() const    { return __atomic_load_n(&m_count, __ATOMIC_RELAXED); }
        uint32_t overflow() const { return __atomic_load_n(&m_overflow, __ATOMIC_RELAXED); }

        // Reads the index-th distinct hash in slot order (0 <= index < count()).
        // Script-thread reader; a concurrent observe() may shift the enumeration
        // by one between calls, which is acceptable for a live menu list. Refuses
        // a null hash-out or an out-of-range index.
        bool at(int index, uint32_t* hash, uint32_t* hits, uint8_t* first_player) const {
            if (!hash || index < 0 || index >= count())
                return false;

            int seen = 0;
            for (int i = 0; i < capacity; i++) {
                if (__atomic_load_n(&m_slots[i].state, __ATOMIC_ACQUIRE) != READY)
                    continue;
                if (seen == index) {
                    *hash = m_slots[i].hash;
                    if (hits)         *hits         = __atomic_load_n(&m_slots[i].hits, __ATOMIC_RELAXED);
                    if (first_player) *first_player = m_slots[i].first_player;
                    return true;
                }
                seen++;
            }
            return false;
        }

        // Script-thread menu action ("Clear learned"). Not synchronised against a
        // concurrent observe(): the operator clears deliberately, the array is
        // fixed with no pointers, so a racing observe can at worst land a hit in
        // a slot being emptied - a lost count, never corruption.
        void clear() {
            for (int i = 0; i < capacity; i++)
                __atomic_store_n(&m_slots[i].state, (uint8_t)EMPTY, __ATOMIC_RELEASE);
            __atomic_store_n(&m_count, 0, __ATOMIC_RELAXED);
            __atomic_store_n(&m_overflow, (uint32_t)0, __ATOMIC_RELAXED);
        }

    private:
        enum : uint8_t { EMPTY = 0, CLAIMED = 1, READY = 2 };
        static const int kSpin = 512;

        struct slot {
            uint32_t hash;
            uint32_t hits;
            uint8_t  first_player;
            uint8_t  state;          // EMPTY / CLAIMED / READY
        };

        slot     m_slots[capacity] = {};
        int      m_count    = 0;
        uint32_t m_overflow = 0;
    };
}
