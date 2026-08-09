#pragma once
#include <stddef.h>
#include <stdlib.h>
#include "stl/new.h"

namespace stl {
    template <typename T> class shared_ptr {
    public:
        shared_ptr() {}
        shared_ptr(T* ptr, int* rc) : m_ptr(ptr), m_rc(rc) {}
        shared_ptr(const shared_ptr& o) : m_ptr(o.m_ptr), m_rc(o.m_rc) { retain(); }

        // Upcast from a related pointer type (e.g. Derived -> Base).
        template <typename U>
        shared_ptr(const shared_ptr<U>& o) : m_ptr(o.raw_ptr()), m_rc(o.raw_rc()) { retain(); }

        shared_ptr& operator=(const shared_ptr& o) {
            if (this != &o) { release(); m_ptr = o.m_ptr; m_rc = o.m_rc; retain(); }
            return *this;
        }
        ~shared_ptr() { release(); }

        T* get() const { return m_ptr; }
        T* operator->() const { return m_ptr; }
        T& operator*() const { return *m_ptr; }
        explicit operator bool() const { return m_ptr != nullptr; }

        // for upcast ctor
        T* raw_ptr() const { return m_ptr; }
        int* raw_rc() const { return m_rc; }

    private:
        void retain() { if (m_rc) ++(*m_rc); }
        void release() {
            if (m_rc && --(*m_rc) == 0) { m_ptr->~T(); free((void*)m_ptr); free(m_rc); }
            m_ptr = nullptr; m_rc = nullptr;
        }
        T* m_ptr = nullptr;
        int* m_rc = nullptr;
    };

    template <typename T, typename... Args>
    shared_ptr<T> make_shared(Args... args) {
        T* p = (T*)malloc(sizeof(T));
        new (p) T(args...);
        int* rc = (int*)malloc(sizeof(int));
        *rc = 1;
        return shared_ptr<T>(p, rc);
    }
}
