#pragma once
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

// Heap-growable string. The fixed-buffer stl::string (STL_STRING_CAP) is fine
// for short values but cannot hold a whole serialised JSON document, so the
// JSON library uses this for documents and stored string values.
namespace stl {
    class dstring {
    public:
        dstring() { ensure(16); m_data[0] = 0; }
        dstring(const char* s) { size_t n = s ? strlen(s) : 0; ensure(n + 1); if (n) memcpy(m_data, s, n); m_data[n] = 0; m_size = n; }
        dstring(const dstring& o) { ensure(o.m_size + 1); memcpy(m_data, o.m_data, o.m_size + 1); m_size = o.m_size; }
        dstring& operator=(const dstring& o) { if (this != &o) { ensure(o.m_size + 1); memcpy(m_data, o.m_data, o.m_size + 1); m_size = o.m_size; } return *this; }
        dstring& operator=(const char* s) { clear(); append(s); return *this; }
        ~dstring() { free(m_data); }

        const char* c_str() const { return m_data; }
        size_t length() const { return m_size; }
        size_t size() const { return m_size; }
        bool empty() const { return m_size == 0; }
        void clear() { m_size = 0; if (m_data) m_data[0] = 0; }
        char operator[](size_t i) const { return m_data[i]; }

        void append(const char* s) {
            if (!s) return;
            size_t n = strlen(s);
            ensure(m_size + n + 1);
            memcpy(m_data + m_size, s, n);
            m_size += n;
            m_data[m_size] = 0;
        }
        void append(char c) {
            ensure(m_size + 2);
            m_data[m_size++] = c;
            m_data[m_size] = 0;
        }
        void append(const dstring& o) { append(o.c_str()); }

        dstring& operator+=(const char* s) { append(s); return *this; }
        dstring& operator+=(char c) { append(c); return *this; }
        dstring& operator+=(const dstring& o) { append(o); return *this; }

        bool operator==(const char* s) const { return strcmp(m_data, s ? s : "") == 0; }
        bool operator==(const dstring& o) const { return strcmp(m_data, o.m_data) == 0; }
        int compare(const char* s) const { return strcmp(m_data, s ? s : ""); }

        void appendf(const char* fmt, ...) {
            char tmp[128];
            va_list ap; va_start(ap, fmt);
            vsnprintf(tmp, sizeof(tmp), fmt, ap);
            va_end(ap);
            append(tmp);
        }

    private:
        void ensure(size_t need) {
            if (need <= m_cap) return;
            size_t cap = m_cap ? m_cap : 16;
            while (cap < need) cap *= 2;
            char* nd = (char*)realloc(m_data, cap);
            m_data = nd;
            m_cap = cap;
        }

        char* m_data = nullptr;
        size_t m_size = 0;
        size_t m_cap = 0;
    };
}
