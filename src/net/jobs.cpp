#include "net/jobs.h"

namespace {
    // Same primitive src/game/game_thread.cpp:35-40 uses (see its :9 note):
    // clang's __atomic builtins are word-sized, lock-free on x86-64, and need
    // no runtime lib. The critical section here is a handful of field writes,
    // so a spin is cheaper than anything that could put the HTTP thread to
    // sleep holding the game thread up.
    struct guard {
        volatile int* lock;
        explicit guard(volatile int* l) : lock(l) {
            while (__atomic_exchange_n(lock, 1, __ATOMIC_ACQUIRE)) { }
        }
        ~guard() { __atomic_store_n(lock, 0, __ATOMIC_RELEASE); }
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
