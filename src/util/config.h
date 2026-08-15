#pragma once
#include "platform/stdafx.h"
#include "global/ui_vars.h"
#include "util/math.h"

// Ozark's config API, backed by the Mini-JSON (util/json.h) and persisted to
// /data/InsulinGTAV/config.json. The transformed option files call these exactly
// as the PC source did (name_stack = submenu path, additional_stacks = category
// like {"Values"} / {"Color"}, key = option name).
namespace util::config {
    class config {
    public:
        void load();   // read the file into memory (call once at build())

        stl::string read_string(stl::stack<stl::string> name_stack, stl::string key, stl::string default_string = "", stl::vector<stl::string> additional_stacks = {});
        int read_int(stl::stack<stl::string> name_stack, stl::string key, int default_int = 0, stl::vector<stl::string> additional_stacks = {});
        uint64_t read_uint64(stl::stack<stl::string> name_stack, stl::string key, uint64_t default_int = 0, stl::vector<stl::string> additional_stacks = {});
        float read_float(stl::stack<stl::string> name_stack, stl::string key, float default_float = 0.f, stl::vector<stl::string> additional_stacks = {});
        bool read_bool(stl::stack<stl::string> name_stack, stl::string key, bool default_bool = false, stl::vector<stl::string> additional_stacks = {});
        bool read_color(stl::stack<stl::string> name_stack, stl::string key, color_rgba* color, stl::vector<stl::string> additional_stacks = {});
        bool read_vector(stl::stack<stl::string> name_stack, stl::string key, math::vector3_<float>* vec, stl::vector<stl::string> additional_stacks = {});

        void write_string(stl::stack<stl::string> name_stack, stl::string key, stl::string value, stl::vector<stl::string> additional_stacks = {});
        void write_int(stl::stack<stl::string> name_stack, stl::string key, int value, stl::vector<stl::string> additional_stacks = {});
        void write_uint64(stl::stack<stl::string> name_stack, stl::string key, uint64_t value, stl::vector<stl::string> additional_stacks = {});
        void write_float(stl::stack<stl::string> name_stack, stl::string key, float value, stl::vector<stl::string> additional_stacks = {});
        void write_bool(stl::stack<stl::string> name_stack, stl::string key, bool value, stl::vector<stl::string> additional_stacks = {});
        void write_color(stl::stack<stl::string> name_stack, stl::string key, color_rgba value, stl::vector<stl::string> additional_stacks = {});
        void write_vector(stl::stack<stl::string> name_stack, stl::string key, math::vector3_<float> value, stl::vector<stl::string> additional_stacks = {});
    };

    config* get_config();

    inline void load() { get_config()->load(); }
    // Defer persistence until the outermost end_batch(). Re-entrant.
    void begin_batch();
    void end_batch();

    inline stl::string read_string(stl::stack<stl::string> ns, stl::string k, stl::string d = "", stl::vector<stl::string> a = {}) { return get_config()->read_string(ns, k, d, a); }
    inline int read_int(stl::stack<stl::string> ns, stl::string k, int d = 0, stl::vector<stl::string> a = {}) { return get_config()->read_int(ns, k, d, a); }
    inline uint64_t read_uint64(stl::stack<stl::string> ns, stl::string k, uint64_t d = 0, stl::vector<stl::string> a = {}) { return get_config()->read_uint64(ns, k, d, a); }
    inline float read_float(stl::stack<stl::string> ns, stl::string k, float d = 0.f, stl::vector<stl::string> a = {}) { return get_config()->read_float(ns, k, d, a); }
    inline bool read_bool(stl::stack<stl::string> ns, stl::string k, bool d = false, stl::vector<stl::string> a = {}) { return get_config()->read_bool(ns, k, d, a); }
    inline bool read_color(stl::stack<stl::string> ns, stl::string k, color_rgba* c, stl::vector<stl::string> a = {}) { return get_config()->read_color(ns, k, c, a); }
    inline bool read_vector(stl::stack<stl::string> ns, stl::string k, math::vector3_<float>* v, stl::vector<stl::string> a = {}) { return get_config()->read_vector(ns, k, v, a); }
    inline void write_string(stl::stack<stl::string> ns, stl::string k, stl::string v, stl::vector<stl::string> a = {}) { get_config()->write_string(ns, k, v, a); }
    inline void write_int(stl::stack<stl::string> ns, stl::string k, int v, stl::vector<stl::string> a = {}) { get_config()->write_int(ns, k, v, a); }
    inline void write_uint64(stl::stack<stl::string> ns, stl::string k, uint64_t v, stl::vector<stl::string> a = {}) { get_config()->write_uint64(ns, k, v, a); }
    inline void write_float(stl::stack<stl::string> ns, stl::string k, float v, stl::vector<stl::string> a = {}) { get_config()->write_float(ns, k, v, a); }
    inline void write_bool(stl::stack<stl::string> ns, stl::string k, bool v, stl::vector<stl::string> a = {}) { get_config()->write_bool(ns, k, v, a); }
    inline void write_color(stl::stack<stl::string> ns, stl::string k, color_rgba v, stl::vector<stl::string> a = {}) { get_config()->write_color(ns, k, v, a); }
    inline void write_vector(stl::stack<stl::string> ns, stl::string k, math::vector3_<float> v, stl::vector<stl::string> a = {}) { get_config()->write_vector(ns, k, v, a); }
}
