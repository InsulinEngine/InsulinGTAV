#pragma once
#include "stl/dstring.h"
#include "stl/vector.h"
#include "stl/pair.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Mini-JSON, modelled on TinyJSON (github.com/xeghosted/TinyJSON): same shape
// (value_t enum, scalar union, vector-of-pairs objects preserving insertion
// order, operator[] auto-vivify, get_*, dump/parse, file I/O) but adapted to the
// PS4 GHPLUGIN: stl:: instead of std::, NO exceptions (parse returns null on
// error; get_*/at return defaults), and sceKernel file I/O.
namespace tj {

    class json {
    public:
        enum value_t { null_t, object_t, array_t, string_t, boolean_t, integer_t, float_t };

        json() : m_type(null_t) {}
        json(bool b) : m_type(boolean_t) { m_value.boolean = b; }
        json(int i) : m_type(integer_t) { m_value.integer = i; }
        json(long long i) : m_type(integer_t) { m_value.integer = i; }
        json(double d) : m_type(float_t) { m_value.number = d; }
        json(float d) : m_type(float_t) { m_value.number = (double)d; }
        json(const char* s) : m_type(string_t) { m_string = new stl::dstring(s); }
        json(const stl::dstring& s) : m_type(string_t) { m_string = new stl::dstring(s); }

        json(const json& o) { copy_from(o); }
        json& operator=(const json& o) { if (this != &o) { destroy(); copy_from(o); } return *this; }
        ~json() { destroy(); }

        // --- type queries ---------------------------------------------------
        value_t type() const { return m_type; }
        bool is_null() const { return m_type == null_t; }
        bool is_object() const { return m_type == object_t; }
        bool is_array() const { return m_type == array_t; }
        bool is_string() const { return m_type == string_t; }
        bool is_boolean() const { return m_type == boolean_t; }
        bool is_number() const { return m_type == integer_t || m_type == float_t; }

        // --- scalar getters (default on mismatch, never throw) --------------
        bool get_bool() const { return m_type == boolean_t ? m_value.boolean : false; }
        long long get_int() const {
            if (m_type == integer_t) return m_value.integer;
            if (m_type == float_t) return (long long)m_value.number;
            return 0;
        }
        double get_float() const {
            if (m_type == float_t) return m_value.number;
            if (m_type == integer_t) return (double)m_value.integer;
            return 0.0;
        }
        const char* get_string() const { return m_type == string_t ? m_string->c_str() : ""; }

        // --- object access (auto-vivifies) ----------------------------------
        json& operator[](const char* key) {
            if (m_type != object_t) { destroy(); m_type = object_t; m_object = new stl::vector<member>(); }
            for (size_t i = 0; i < m_object->size(); i++) {
                if ((*m_object)[i].first == key) return (*m_object)[i].second;
            }
            m_object->push_back(member(stl::dstring(key), json()));
            return (*m_object)[m_object->size() - 1].second;
        }

        bool contains(const char* key) const {
            if (m_type != object_t) return false;
            for (size_t i = 0; i < m_object->size(); i++)
                if ((*m_object)[i].first == key) return true;
            return false;
        }

        // Non-vivifying lookup for read paths; nullptr if absent / not an object.
        const json* try_get(const char* key) const { return find(key); }

        // --- array access ---------------------------------------------------
        json& operator[](size_t index) {
            if (m_type != array_t) { destroy(); m_type = array_t; m_array = new stl::vector<json>(); }
            while (m_array->size() <= index) m_array->push_back(json());
            return (*m_array)[index];
        }

        void push_back(const json& value) {
            if (m_type != array_t) { destroy(); m_type = array_t; m_array = new stl::vector<json>(); }
            m_array->push_back(value);
        }

        size_t size() const {
            if (m_type == object_t) return m_object->size();
            if (m_type == array_t) return m_array->size();
            return 0;
        }

        // --- object iteration in insertion order ----------------------------
        size_t member_count() const { return (m_type == object_t && m_object) ? m_object->size() : 0; }
        const char* key_at(size_t i) const { return (m_type == object_t && m_object && i < m_object->size()) ? (*m_object)[i].first.c_str() : ""; }

        // --- safe typed value with default ----------------------------------
        bool value_bool(const char* key, bool def) const { const json* c = find(key); return c && c->is_boolean() ? c->get_bool() : def; }
        long long value_int(const char* key, long long def) const { const json* c = find(key); return c && c->is_number() ? c->get_int() : def; }
        double value_float(const char* key, double def) const { const json* c = find(key); return c && c->is_number() ? c->get_float() : def; }

        // --- serialisation --------------------------------------------------
        stl::dstring dump(int indent = -1) const {
            stl::dstring out;
            dump_to(out, indent, 0);
            return out;
        }

        static json parse(const char* text) {
            if (!text) return json();
            const char* p = text;
            json result = parse_value(p);
            return result;
        }

        // --- file I/O (sceKernel) -------------------------------------------
        bool save_to_file(const char* path, int indent = 2) const;
        static json load_from_file(const char* path);

    private:
        typedef stl::pair<stl::dstring, json> member;

        value_t m_type = null_t;
        union {
            bool boolean;
            long long integer;
            double number;
        } m_value = { false };
        stl::dstring* m_string = nullptr;
        stl::vector<member>* m_object = nullptr;
        stl::vector<json>* m_array = nullptr;

        const json* find(const char* key) const {
            if (m_type != object_t) return nullptr;
            for (size_t i = 0; i < m_object->size(); i++)
                if ((*m_object)[i].first == key) return &(*m_object)[i].second;
            return nullptr;
        }

        void destroy() {
            delete m_string; m_string = nullptr;
            delete m_object; m_object = nullptr;
            delete m_array;  m_array = nullptr;
            m_type = null_t;
        }

        void copy_from(const json& o) {
            m_type = o.m_type;
            m_value = o.m_value;
            m_string = o.m_string ? new stl::dstring(*o.m_string) : nullptr;
            m_object = o.m_object ? new stl::vector<member>(*o.m_object) : nullptr;
            m_array  = o.m_array  ? new stl::vector<json>(*o.m_array)   : nullptr;
        }

        static void escape(stl::dstring& out, const char* s) {
            out.append('"');
            for (const char* c = s; *c; c++) {
                switch (*c) {
                    case '"':  out.append("\\\""); break;
                    case '\\': out.append("\\\\"); break;
                    case '\n': out.append("\\n"); break;
                    case '\t': out.append("\\t"); break;
                    case '\r': out.append("\\r"); break;
                    default:   out.append(*c); break;
                }
            }
            out.append('"');
        }

        static void newline_indent(stl::dstring& out, int indent, int depth) {
            if (indent < 0) return;
            out.append('\n');
            for (int i = 0; i < indent * depth; i++) out.append(' ');
        }

        void dump_to(stl::dstring& out, int indent, int depth) const {
            char tmp[64];
            switch (m_type) {
                case null_t: out.append("null"); break;
                case boolean_t: out.append(m_value.boolean ? "true" : "false"); break;
                case integer_t: snprintf(tmp, sizeof(tmp), "%lld", m_value.integer); out.append(tmp); break;
                case float_t: snprintf(tmp, sizeof(tmp), "%g", m_value.number); out.append(tmp); break;
                case string_t: escape(out, m_string->c_str()); break;
                case object_t: {
                    out.append('{');
                    for (size_t i = 0; i < m_object->size(); i++) {
                        if (i) out.append(',');
                        newline_indent(out, indent, depth + 1);
                        escape(out, (*m_object)[i].first.c_str());
                        out.append(indent < 0 ? ":" : ": ");
                        (*m_object)[i].second.dump_to(out, indent, depth + 1);
                    }
                    if (m_object->size()) newline_indent(out, indent, depth);
                    out.append('}');
                    break;
                }
                case array_t: {
                    out.append('[');
                    for (size_t i = 0; i < m_array->size(); i++) {
                        if (i) out.append(',');
                        newline_indent(out, indent, depth + 1);
                        (*m_array)[i].dump_to(out, indent, depth + 1);
                    }
                    if (m_array->size()) newline_indent(out, indent, depth);
                    out.append(']');
                    break;
                }
            }
        }

        static void skip_ws(const char*& p) { while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++; }

        static json parse_value(const char*& p) {
            skip_ws(p);
            if (*p == '{') return parse_object(p);
            if (*p == '[') return parse_array(p);
            if (*p == '"') return json(parse_string(p));
            if (*p == 't') { if (strncmp(p, "true", 4) == 0) { p += 4; return json(true); } }
            if (*p == 'f') { if (strncmp(p, "false", 5) == 0) { p += 5; return json(false); } }
            if (*p == 'n') { if (strncmp(p, "null", 4) == 0) { p += 4; return json(); } }
            return parse_number(p);
        }

        static stl::dstring parse_string(const char*& p) {
            stl::dstring out;
            if (*p != '"') return out;
            p++;
            while (*p && *p != '"') {
                if (*p == '\\') {
                    p++;
                    switch (*p) {
                        case '"': out.append('"'); break;
                        case '\\': out.append('\\'); break;
                        case '/': out.append('/'); break;
                        case 'n': out.append('\n'); break;
                        case 't': out.append('\t'); break;
                        case 'r': out.append('\r'); break;
                        case 'b': out.append('\b'); break;
                        case 'f': out.append('\f'); break;
                        case 'u': if (p[1] && p[2] && p[3] && p[4]) p += 4; break; // skip \uXXXX
                        default: out.append(*p); break;
                    }
                    if (*p) p++;
                } else {
                    out.append(*p++);
                }
            }
            if (*p == '"') p++;
            return out;
        }

        static json parse_number(const char*& p) {
            const char* start = p;
            bool is_float = false;
            if (*p == '-') p++;
            while ((*p >= '0' && *p <= '9')) p++;
            if (*p == '.') { is_float = true; p++; while (*p >= '0' && *p <= '9') p++; }
            if (*p == 'e' || *p == 'E') { is_float = true; p++; if (*p == '+' || *p == '-') p++; while (*p >= '0' && *p <= '9') p++; }
            char buf[64];
            size_t n = (size_t)(p - start);
            if (n >= sizeof(buf)) n = sizeof(buf) - 1;
            memcpy(buf, start, n); buf[n] = 0;
            if (n == 0) return json();
            if (is_float) return json((double)strtod(buf, nullptr));
            return json((long long)strtoll(buf, nullptr, 10));
        }

        static json parse_object(const char*& p) {
            json obj; obj.m_type = object_t; obj.m_object = new stl::vector<member>();
            p++; // {
            skip_ws(p);
            if (*p == '}') { p++; return obj; }
            while (*p) {
                skip_ws(p);
                stl::dstring key = parse_string(p);
                skip_ws(p);
                if (*p == ':') p++;
                json value = parse_value(p);
                obj.m_object->push_back(member(key, value));
                skip_ws(p);
                if (*p == ',') { p++; continue; }
                if (*p == '}') { p++; break; }
                break;
            }
            return obj;
        }

        static json parse_array(const char*& p) {
            json arr; arr.m_type = array_t; arr.m_array = new stl::vector<json>();
            p++; // [
            skip_ws(p);
            if (*p == ']') { p++; return arr; }
            while (*p) {
                json value = parse_value(p);
                arr.m_array->push_back(value);
                skip_ws(p);
                if (*p == ',') { p++; continue; }
                if (*p == ']') { p++; break; }
                break;
            }
            return arr;
        }
    };
}
