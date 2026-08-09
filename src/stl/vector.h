#pragma once
#include <stddef.h>
#include <stdlib.h>
#include "stl/new.h"

namespace stl {
    template <typename T>
    class vector {
    public:
        vector() {}
        vector(const vector& o) { copy_from(o); }
        vector& operator=(const vector& o) {
            if (this != &o) { destroy_all(); free(m_data); m_data = nullptr;
                              m_size = m_cap = 0; copy_from(o); }
            return *this;
        }
        ~vector() { destroy_all(); free(m_data); }

        void push_back(const T& val) {
            if (m_size == m_cap) grow(m_cap ? m_cap * 2 : 4);
            new (m_data + m_size) T(val);
            ++m_size;
        }

        size_t size() const { return m_size; }
        bool empty() const { return m_size == 0; }
        T& at(size_t i) { return m_data[i]; }
        const T& at(size_t i) const { return m_data[i]; }
        T& operator[](size_t i) { return m_data[i]; }
        const T& operator[](size_t i) const { return m_data[i]; }

        void resize(size_t n) {
            if (n < m_size) { for (size_t i = n; i < m_size; ++i) m_data[i].~T(); m_size = n; }
            else if (n > m_size) { if (n > m_cap) grow(n);
                for (size_t i = m_size; i < n; ++i) new (m_data + i) T(); m_size = n; }
        }
        void clear() { destroy_all(); m_size = 0; }
        void reserve(size_t n) { if (n > m_cap) grow(n); }

        T* begin() { return m_data; }
        T* end() { return m_data + m_size; }
        const T* begin() const { return m_data; }
        const T* end() const { return m_data + m_size; }

    private:
        void grow(size_t new_cap) {
            T* nd = (T*)malloc(new_cap * sizeof(T));
            for (size_t i = 0; i < m_size; ++i) { new (nd + i) T(m_data[i]); m_data[i].~T(); }
            free(m_data);
            m_data = nd; m_cap = new_cap;
        }
        void destroy_all() { for (size_t i = 0; i < m_size; ++i) m_data[i].~T(); }
        void copy_from(const vector& o) {
            if (o.m_size) { m_data = (T*)malloc(o.m_size * sizeof(T)); m_cap = o.m_size;
                for (size_t i = 0; i < o.m_size; ++i) new (m_data + i) T(o.m_data[i]);
                m_size = o.m_size; }
        }

        T* m_data = nullptr;
        size_t m_size = 0;
        size_t m_cap = 0;
    };
}
