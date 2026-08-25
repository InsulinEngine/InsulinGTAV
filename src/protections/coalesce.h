#pragma once
#include <stdint.h>

// Per-filter push-side coalescing for the report ring.
//
// Why this exists: a filter can be legitimately noisy. `task_ambient_clips`
// fires once per null-anims-group ped per FSM tick, which is many peds per
// frame, and it ships enabled at mode::log. The ring holds 64 records and drops
// the NEWEST on overflow, so one noisy filter would saturate it inside a
// fraction of a frame and silently starve every other filter - on exactly the
// build an operator is meant to evaluate. Rate-limiting the kernel log does not
// help: the ring is already full by then.
//
// The fix is to stop the duplicates entering the ring at all. A filter may have
// at most ONE undrained record in flight; further occurrences bump a counter
// that is emitted alongside the record when it drains, so the information
// "this fired N times" is preserved rather than lost. Ring usage is therefore
// bounded by the number of filters (12 today), never by the event rate, and so
// is the number of log lines per frame.
//
// claim() runs on a network thread inside a hook, so like ring::push it must
// never block, allocate, format, log, or call a native. It is two atomics.
namespace protections {

    class coalescer {
    public:
        // Indexed by the raw filter_id value, not by table position, so the
        // producer needs no registry lookup on the hot path. filter_id values
        // are stable and documented as never renumbered; the highest today is
        // 20 and the full Tier 1-4 scope reaches ~23. An id at or past this
        // ceiling is not coalesced at all - it simply behaves the way the ring
        // did before, which is the safe direction to fail in.
        static const uint16_t id_ceiling = 64;

        // Producer side. Returns true when the caller should push a record,
        // false when one is already queued for this filter - in which case the
        // occurrence has been counted instead of dropped.
        bool claim(uint16_t filter_id) {
            if (filter_id >= id_ceiling) return true;
            uint8_t expected = 0;
            if (__atomic_compare_exchange_n(&m_pending[filter_id], &expected, (uint8_t)1,
                                            false, __ATOMIC_ACQ_REL, __ATOMIC_RELAXED))
                return true;
            __atomic_fetch_add(&m_suppressed[filter_id], 1, __ATOMIC_RELAXED);
            return false;
        }

        // Consumer side, called once the filter's record has been popped.
        // Reopens the filter and returns how many occurrences were coalesced
        // into the record just drained.
        //
        // Order matters: the pending flag is cleared BEFORE the counter is
        // taken. Doing it the other way round leaves a window in which an
        // increment lands against a record that has already been popped and no
        // successor is guaranteed, stranding the count. Reopening first means
        // an increment either happens before the exchange (reported now) or
        // after it, which requires pending to be set again, which requires a
        // fresh record that a later release will collect. Either way no
        // occurrence is lost; at worst a count is attributed to the neighbouring
        // record.
        uint32_t release(uint16_t filter_id) {
            if (filter_id >= id_ceiling) return 0;
            __atomic_store_n(&m_pending[filter_id], (uint8_t)0, __ATOMIC_RELEASE);
            return __atomic_exchange_n(&m_suppressed[filter_id], 0, __ATOMIC_RELAXED);
        }

        bool pending(uint16_t filter_id) const {
            if (filter_id >= id_ceiling) return false;
            return __atomic_load_n(&m_pending[filter_id], __ATOMIC_ACQUIRE) != 0;
        }

        uint32_t suppressed(uint16_t filter_id) const {
            if (filter_id >= id_ceiling) return 0;
            return __atomic_load_n(&m_suppressed[filter_id], __ATOMIC_RELAXED);
        }

        // Not thread-safe. Test-only.
        void reset() {
            for (uint16_t i = 0; i < id_ceiling; i++) { m_pending[i] = 0; m_suppressed[i] = 0; }
        }

    private:
        uint8_t  m_pending[id_ceiling]    = {};
        uint32_t m_suppressed[id_ceiling] = {};
    };
}
