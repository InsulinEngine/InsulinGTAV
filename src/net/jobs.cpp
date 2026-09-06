#include "net/jobs.h"

namespace {
    // Same primitive the protections ring and game_thread.cpp use. The critical
    // section is a handful of field writes, so a spin is cheaper than anything
    // that could put the HTTP thread to sleep holding the game thread up.
    struct guard {
        volatile int* lock;
        explicit guard(volatile int* l) : lock(l) {
            while (__sync_lock_test_and_set(lock, 1)) { }
        }
        ~guard() { __sync_lock_release(lock); }
    };
}

namespace net {

    void job_ring::reset() {
        guard g(&m_lock);
        m_head = m_tail = m_count = m_dropped = 0;
    }

    bool job_ring::push(const job& j) {
        guard g(&m_lock);
        if (m_count == capacity) {
            m_dropped++;
            return false;
        }
        m_slots[m_head] = j;
        m_head = (m_head + 1) % capacity;
        m_count++;
        return true;
    }

    bool job_ring::pop(job* out) {
        guard g(&m_lock);
        if (m_count == 0) return false;
        *out = m_slots[m_tail];
        m_tail = (m_tail + 1) % capacity;
        m_count--;
        return true;
    }

    unsigned job_ring::dropped() const {
        return m_dropped;
    }

    job_ring& jobs() {
        static job_ring instance;
        return instance;
    }
}
