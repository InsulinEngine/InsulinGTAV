#pragma once
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#ifndef STL_STRING_CAP
#define STL_STRING_CAP 128
#endif

namespace stl {
    class string {
    public:
        static const size_t npos = (size_t)-1;

        string() { m_buf[0] = 0; }
        string(const char* s) { set(s); }
        string(const char* s, size_t n) {
            if (!s) { m_buf[0] = 0; return; }
            if (n > STL_STRING_CAP - 1) n = STL_STRING_CAP - 1;
            memcpy(m_buf, s, n); m_buf[n] = 0;
        }

        void set(const char* s) {
            if (!s) { m_buf[0] = 0; return; }
            size_t n = strlen(s);
            if (n > STL_STRING_CAP - 1) n = STL_STRING_CAP - 1;
            memcpy(m_buf, s, n);
            m_buf[n] = 0;
        }

        const char* c_str() const { return m_buf; }
        const char* data() const { return m_buf; }
        size_t length() const { return strlen(m_buf); }
        size_t size() const { return strlen(m_buf); }
        bool empty() const { return m_buf[0] == 0; }
        void clear() { m_buf[0] = 0; }
        int compare(const char* s) const { return strcmp(m_buf, s ? s : ""); }

        char& operator[](size_t i) { return m_buf[i]; }
        char operator[](size_t i) const { return m_buf[i]; }

        bool operator==(const char* s) const { return compare(s) == 0; }
        bool operator==(const string& o) const { return compare(o.c_str()) == 0; }
        bool operator!=(const char* s) const { return compare(s) != 0; }
        bool operator!=(const string& o) const { return compare(o.c_str()) != 0; }
        bool operator<(const string& o) const { return compare(o.c_str()) < 0; }

        string operator+(const char* s) const {
            string out(m_buf);
            size_t cur = out.length();
            if (s && cur < STL_STRING_CAP - 1) {
                size_t room = STL_STRING_CAP - 1 - cur;
                size_t n = strlen(s);
                if (n > room) n = room;
                memcpy(out.m_buf + cur, s, n);
                out.m_buf[cur + n] = 0;
            }
            return out;
        }
        string operator+(const string& o) const { return (*this) + o.c_str(); }

        string& operator+=(const char* s) { *this = *this + s; return *this; }
        string& operator+=(const string& o) { *this = *this + o.c_str(); return *this; }
        string& operator+=(char c) { char t[2] = { c, 0 }; *this = *this + t; return *this; }

        size_t find(const char* s) const {
            const char* p = strstr(m_buf, s);
            return p ? (size_t)(p - m_buf) : npos;
        }

        string substr(size_t pos, size_t n = npos) const {
            string out;
            size_t len = length();
            if (pos >= len) return out;
            if (n > len - pos) n = len - pos;
            if (n > STL_STRING_CAP - 1) n = STL_STRING_CAP - 1;
            memcpy(out.m_buf, m_buf + pos, n);
            out.m_buf[n] = 0;
            return out;
        }

        static string format(const char* fmt, ...) {
            string out;
            va_list ap; va_start(ap, fmt);
            vsnprintf(out.m_buf, STL_STRING_CAP, fmt, ap);
            va_end(ap);
            return out;
        }

    private:
        char m_buf[STL_STRING_CAP];
    };
}
