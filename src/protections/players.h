#pragma once
#include <stdint.h>

// Per-player block state, indexed by the physical player index at
// CNetGamePlayer+0x31 (0..31).
//
// Three bitmasks and nothing else. This is deliberately not a player database:
// no infraction history, no reactions, no persistence. Those are a separate
// subsystem and this port does not build them.
//
// Read from hook threads with a bit test, written from the menu thread. An
// aligned 32-bit load cannot tear, so no lock is needed for a value whose worst
// race is one packet handled under the previous setting.
namespace protections {

    enum class block_kind : int {
        net_events   = 0,
        clone_sync   = 1,   // consumed in Tier 3
        clone_create = 2,   // consumed in Tier 3
        count        = 3
    };

    class blocks {
    public:
        bool is_blocked(block_kind kind, int player_index) const {
            if (!valid(kind, player_index)) return false;
            const uint32_t m = __atomic_load_n(&m_mask[(int)kind], __ATOMIC_RELAXED);
            return (m & (1u << player_index)) != 0;
        }

        void set_blocked(block_kind kind, int player_index, bool on) {
            if (!valid(kind, player_index)) return;
            const uint32_t bit = 1u << player_index;
            if (on) __atomic_fetch_or(&m_mask[(int)kind], bit, __ATOMIC_RELAXED);
            else    __atomic_fetch_and(&m_mask[(int)kind], ~bit, __ATOMIC_RELAXED);
        }

        uint32_t mask(block_kind kind) const {
            if ((int)kind < 0 || (int)kind >= (int)block_kind::count) return 0;
            return __atomic_load_n(&m_mask[(int)kind], __ATOMIC_RELAXED);
        }

        void clear() {
            for (int i = 0; i < (int)block_kind::count; i++)
                __atomic_store_n(&m_mask[i], 0u, __ATOMIC_RELAXED);
        }

    private:
        static bool valid(block_kind kind, int player_index) {
            if ((int)kind < 0 || (int)kind >= (int)block_kind::count) return false;
            return player_index >= 0 && player_index < 32;
        }

        uint32_t m_mask[(int)block_kind::count] = {};
    };

    // Function-local static: no .init_array in this plugin.
    inline blocks& player_blocks() {
        static blocks instance;
        return instance;
    }
}
