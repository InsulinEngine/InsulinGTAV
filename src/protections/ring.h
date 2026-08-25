#pragma once
#include <stdint.h>

// Fixed-size lossy report ring. Producers are network threads inside protection
// hooks; the single consumer is the frame callback on the script thread.
//
// Two properties are non-negotiable and drive the design:
//   * A push must never block. A sound-spam attack produces hundreds of blocks
//     per second, and stalling a network thread to report them would be a worse
//     denial of service than the attack.
//   * A record must stay valid after the producing thread has moved on, so it
//     holds no pointers and no strings - only ids the consumer can resolve
//     against the registry.
//
// Overflow drops the NEWEST record rather than overwriting the oldest: during a
// burst the first few reports identify the attack, and the thousandth does not.
namespace protections {

    struct record {
        uint16_t filter_id;
        uint8_t  player_index;   // 0xFF when not attributable to a player
        uint8_t  flags;
        uint32_t detail_a;       // filter-defined: event hash, object id, node id
        uint32_t detail_b;
    };

    class ring {
    public:
        static const uint32_t capacity = 64;   // must stay a power of two
        // The index masking below is `& (capacity - 1)`, which is only a modulo
        // for a power of two. A comment is not enough to enforce that.
        static_assert((capacity & (capacity - 1)) == 0,
                      "ring::capacity must be a power of two - the index masking depends on it");

        void push(const record& r) {
            uint32_t claim;
            for (;;) {
                claim = __atomic_load_n(&m_tail, __ATOMIC_RELAXED);
                uint32_t head = __atomic_load_n(&m_head, __ATOMIC_ACQUIRE);
                if (claim - head >= capacity) {          // unsigned wrap is intended
                    __atomic_fetch_add(&m_dropped, 1, __ATOMIC_RELAXED);
                    return;
                }
                // Claim the slot before writing it. On failure another producer
                // took this index and `claim` has been reloaded for us.
                if (__atomic_compare_exchange_n(&m_tail, &claim, claim + 1, false,
                                                __ATOMIC_ACQ_REL, __ATOMIC_RELAXED))
                    break;
            }

            const uint32_t idx = claim & (capacity - 1);
            m_slots[idx] = r;
            // Release: the consumer must not see ready=1 before the payload.
            __atomic_store_n(&m_ready[idx], (uint8_t)1, __ATOMIC_RELEASE);
        }

        bool pop(record* out) {
            const uint32_t head = m_head;                 // consumer-owned, plain read
            if (head == __atomic_load_n(&m_tail, __ATOMIC_ACQUIRE))
                return false;                             // nothing claimed

            const uint32_t idx = head & (capacity - 1);
            if (!__atomic_load_n(&m_ready[idx], __ATOMIC_ACQUIRE))
                return false;                             // claimed but still being written

            *out = m_slots[idx];
            __atomic_store_n(&m_ready[idx], (uint8_t)0, __ATOMIC_RELAXED);
            __atomic_store_n(&m_head, head + 1, __ATOMIC_RELEASE);
            return true;
        }

        uint32_t dropped() const { return __atomic_load_n(&m_dropped, __ATOMIC_RELAXED); }

        // Not thread-safe. Test-only - there is no menu control that clears the
        // ring, and adding one would need a story for the network threads that
        // may be inside push() at the time.
        void reset() {
            m_head = 0;
            m_tail = 0;
            m_dropped = 0;
            for (uint32_t i = 0; i < capacity; i++) m_ready[i] = 0;
        }

    private:
        record   m_slots[capacity] = {};
        uint8_t  m_ready[capacity] = {};
        uint32_t m_head    = 0;
        uint32_t m_tail    = 0;
        uint32_t m_dropped = 0;
    };
}
